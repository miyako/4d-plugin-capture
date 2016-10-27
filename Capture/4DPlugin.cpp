/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : Capture
 #	author : miyako
 #	2016/10/27
 #
 # --------------------------------------------------------------------------------*/

#include "4DPluginAPI.h"
#include "4DPlugin.h"

#if VERSIONMAC

#import <QTKit/QTKit.h>

@interface ImageSnap : NSObject
{
	QTCaptureSession                    *mCaptureSession;
	QTCaptureDeviceInput                *mCaptureDeviceInput;
	QTCaptureDecompressedVideoOutput    *mCaptureDecompressedVideoOutput;
	CVImageBufferRef                    mCurrentImageBuffer;
}

+(NSArray *)videoDevices;
+(QTCaptureDevice *)defaultVideoDevice;
+(QTCaptureDevice *)deviceForModel:(NSString *)model;
+(NSImage *)snapshotFromDevice:(QTCaptureDevice *)device;

-(id)init;
-(void)dealloc;
-(BOOL)startSession:(QTCaptureDevice *)device;
-(NSImage *)snapshot;
-(void)stopSession;
- (void)captureOutput:(QTCaptureOutput *)captureOutput
	didOutputVideoFrame:(CVImageBufferRef)videoFrame
		 withSampleBuffer:(QTSampleBuffer *)sampleBuffer
			 fromConnection:(QTCaptureConnection *)connection;

@end

@implementation ImageSnap

- (id)init
{
	self = [super init];
	mCaptureSession = nil;
	mCaptureDeviceInput = nil;
	mCaptureDecompressedVideoOutput = nil;
	mCurrentImageBuffer = nil;
	return self;
}

- (void)dealloc
{
	if(mCaptureSession)					[mCaptureSession release];
	if(mCaptureDeviceInput)				[mCaptureDeviceInput release];
	if(mCaptureDecompressedVideoOutput)	[mCaptureDecompressedVideoOutput release];
	
	mCaptureSession = nil;
	mCaptureDeviceInput = nil;
	mCaptureDecompressedVideoOutput = nil;
	
	if(mCurrentImageBuffer) CVBufferRelease(mCurrentImageBuffer);
	
	[super dealloc];
}

+ (NSArray *)videoDevices
{
	NSMutableArray *results = [NSMutableArray arrayWithCapacity:3];
	[results addObjectsFromArray:[QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo]];
	[results addObjectsFromArray:[QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeMuxed]];
	return results;
}

+ (QTCaptureDevice *)defaultVideoDevice
{
	QTCaptureDevice *device = nil;
	device = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
	if(!device){
		device = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeMuxed];
	}
	return device;
}

+(QTCaptureDevice *)deviceForModel:(NSString *)model
{
	QTCaptureDevice *result = nil;
	NSArray *devices = [ImageSnap videoDevices];
	for(QTCaptureDevice *device in devices){
		if ([model isEqualToString:[device modelUniqueID]])
		{
			result = device;
			break;
		}
	}
	
	return result;
}

+(NSImage *)snapshotFromDevice:(QTCaptureDevice *)device
{
	NSImage *image = nil;
	
	ImageSnap *snap = [[ImageSnap alloc]init];
	
	if([snap startSession:device])
	{
		image = [snap snapshot];
		[snap stopSession];
	}
	
	[snap release];
	
	return image;
	
}

-(NSImage *)snapshot
{
	CVImageBufferRef frame = nil;
	
	while(!frame)
	{
		@synchronized(self)
		{
			frame = mCurrentImageBuffer;
			CVBufferRetain(frame);
		}
		
		if(!frame)
		{
			[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 0.1]];
		}
		
	}
	
	NSCIImageRep *imageRep = [NSCIImageRep imageRepWithCIImage:[CIImage imageWithCVImageBuffer:frame]];
	NSImage *image = [[[NSImage alloc] initWithSize:[imageRep size]] autorelease];
	[image addRepresentation:imageRep];
	
	return image;
}

-(void)stopSession
{
	while(mCaptureSession != nil)
	{
		[mCaptureSession stopRunning];
		
		if([mCaptureSession isRunning])
		{
			[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
		}else
		{
			if(mCaptureSession)					[mCaptureSession release];
			if(mCaptureDeviceInput)				[mCaptureDeviceInput release];
			if(mCaptureDecompressedVideoOutput)	[mCaptureDecompressedVideoOutput release];
			
			mCaptureSession = nil;
			mCaptureDeviceInput = nil;
			mCaptureDecompressedVideoOutput = nil;
			
		}
	}
}

-(BOOL)startSession:(QTCaptureDevice *)device
{
	if(!device)
	{
		return NO;
	}
	
	NSError *error = nil;
	
	if([device isEqual:[mCaptureDeviceInput device]] &&
		 mCaptureSession != nil &&
		 [mCaptureSession isRunning])
	{
		return YES;
	}
	
	else if(mCaptureSession)
	{
		[self stopSession];
	}
	
	mCaptureSession = [[QTCaptureSession alloc]init];
	
	if(![device open:&error])
	{
		[mCaptureSession release];
		mCaptureSession = nil;
		return NO;
	}
	
	mCaptureDeviceInput = [[QTCaptureDeviceInput alloc]initWithDevice:device];
	
	if (![mCaptureSession addInput:mCaptureDeviceInput error:&error])
	{
		[mCaptureSession release];
		[mCaptureDeviceInput release];
		mCaptureSession = nil;
		mCaptureDeviceInput = nil;
		return NO;
	}
	
	mCaptureDecompressedVideoOutput = [[QTCaptureDecompressedVideoOutput alloc]init];
	[mCaptureDecompressedVideoOutput setDelegate:self];
	
	if (![mCaptureSession addOutput:mCaptureDecompressedVideoOutput error:&error])
	{
		[mCaptureSession release];
		[mCaptureDeviceInput release];
		[mCaptureDecompressedVideoOutput release];
		mCaptureSession = nil;
		mCaptureDeviceInput = nil;
		mCaptureDecompressedVideoOutput = nil;
		return NO;
	}
	
	@synchronized(self)
	{
		if(mCurrentImageBuffer != nil)
		{
			CVBufferRelease(mCurrentImageBuffer);
			mCurrentImageBuffer = nil;
		}
	}
	
	[mCaptureSession startRunning];
	
	return YES;
}

- (void)captureOutput:(QTCaptureOutput *)captureOutput
	didOutputVideoFrame:(CVImageBufferRef)videoFrame
		 withSampleBuffer:(QTSampleBuffer *)sampleBuffer
			 fromConnection:(QTCaptureConnection *)connection
{
	
	if (videoFrame == nil)
	{
		return;
	}
	
	CVImageBufferRef imageBufferToRelease;
	CVBufferRetain(videoFrame);
	
	@synchronized(self){
		imageBufferToRelease = mCurrentImageBuffer;
		mCurrentImageBuffer = videoFrame;
	}
	CVBufferRelease(imageBufferToRelease);
}

@end

#pragma mark -

typedef struct
{
	NSImage *image;
	QTCaptureDevice *device;
} SnapContext;

void _Snap(SnapContext *context)
{	
	if(!context->device)
		context->device = [ImageSnap defaultVideoDevice];
	
	if(context->device)
		context->image = [ImageSnap snapshotFromDevice:context->device];
}

#endif

#pragma mark -

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams);
	}
	catch(...)
	{

	}
}

void CommandDispatcher (PA_long32 pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
		case kInitPlugin :
		case kServerInitPlugin :
#if VERSIONMAC
#if CGFLOAT_IS_DOUBLE
			
#else
			NSApplicationLoad();
#endif
#endif
			break;
// --- Device
			
		case 1 :
			CAPTURE_DEVICE_Get_default(pResult, pParams);
			break;

		case 2 :
			CAPTURE_DEVICE_LIST(pResult, pParams);
			break;

// --- Snap

		case 3 :
			CAPTURE_Snap(pResult, pParams);
			break;

	}
}

#pragma mark -

// ------------------------------------ Device ------------------------------------

void CAPTURE_DEVICE_Get_default(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT returnValue;

#if VERSIONMAC
	QTCaptureDevice *device = [ImageSnap defaultVideoDevice];
	if(device)
		returnValue.setUTF16String([device modelUniqueID]);
#else
	
#endif
	
	returnValue.setReturn(pResult);
}

void CAPTURE_DEVICE_LIST(sLONG_PTR *pResult, PackagePtr pParams)
{
	ARRAY_TEXT Param1;
	Param1.setSize(1);
	
#if VERSIONMAC
	NSArray *devices = [ImageSnap videoDevices];
	for(QTCaptureDevice *device in devices)
	{
		Param1.appendUTF16String([device modelUniqueID]);
	}
#else
	
#endif
	
	Param1.toParamAtIndex(pParams, 1);
}

// ------------------------------------- Snap -------------------------------------

void CAPTURE_Snap(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;
	C_LONGINT returnValue;

	Param1.fromParamAtIndex(pParams, 1);

#if VERSIONMAC
	SnapContext context;
	
	NSString *model = Param1.copyUTF16String();
	context.image = nil;
	context.device = [ImageSnap deviceForModel:model];
	[model release];
	
	PA_RunInMainProcess((PA_RunInMainProcessProcPtr)_Snap, &context);
	
	if(context.image)
	{
		NSData *data = [context.image TIFFRepresentation];
		PA_Picture *Param2 = (PA_Picture *)(pParams[1]);
		
		if (*Param2)
			PA_DisposePicture(*Param2);
		
		*Param2 = PA_CreatePicture((void *)[data bytes], [data length]);
		returnValue.setIntValue(1);
	}
#else
	
#endif

	returnValue.setReturn(pResult);
}
