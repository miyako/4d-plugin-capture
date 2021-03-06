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
		[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 1.2]];
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

#else

#include "dshow.h"
#include "qedit.h"
#pragma comment(lib, "strmiids")

#pragma mark -

void _DEVICE_LIST(ARRAY_TEXT &names)
{
	if(SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		ICreateDevEnum *pDevEnum = NULL;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,
																	NULL,
																	CLSCTX_INPROC_SERVER,
																	IID_PPV_ARGS(&pDevEnum));
		
		if(SUCCEEDED(hr))
		{
			IEnumMoniker *pEnum = NULL;
			hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
			
			if(hr != S_FALSE)
			{
				IMoniker *pMoniker = NULL;
				while(pEnum->Next(1, &pMoniker, NULL) == S_OK)
				{
					IPropertyBag *pPropBag = NULL;
					hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
					
					if(FAILED(hr))
					{
						pMoniker->Release();
						continue;
					}
					
					VARIANT var;
					VariantInit(&var);
					
					CUTF16String name;
					
					hr = pPropBag->Read(L"FriendlyName", &var, 0);
					
					if(SUCCEEDED(hr))
					{
						name = (const PA_Unichar *)var.bstrVal;
						VariantClear(&var);
						names.appendUTF16String(&name);
					}
					
					pPropBag->Release();
					pMoniker->Release();
				}
				
			}
			pDevEnum->Release();
		}
		CoUninitialize();
	}
}

void _DEVICE_Get_default(C_TEXT &name)
{
	ARRAY_TEXT names;
	
	_DEVICE_LIST(names);
	
	if(names.getSize())
	{
		CUTF16String n;
		names.copyUTF16StringAtIndex(&n, 0);
		name.setUTF16String(&n);
	}else
	{
		name.setUTF16String((const PA_Unichar *)L"", 0);
	}
}

IBaseFilter *_DEVICE(C_TEXT &name)
{
	IBaseFilter *pBFilter = NULL;
	
		ICreateDevEnum *pDevEnum = NULL;
		HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,
																	NULL,
																	CLSCTX_INPROC_SERVER,
																	IID_PPV_ARGS(&pDevEnum));
		
		CUTF16String n;
		name.copyUTF16String(&n);
		
		if(SUCCEEDED(hr))
		{
			IEnumMoniker *pEnum = NULL;
			hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
			
			if(hr != S_FALSE)
			{
				IMoniker *pMoniker = NULL;
				while(pEnum->Next(1, &pMoniker, NULL) == S_OK)
				{
					IPropertyBag *pPropBag = NULL;
					hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
					
					if(FAILED(hr))
					{
						pMoniker->Release();
						continue;
					}
					
					VARIANT var;
					VariantInit(&var);
					
					CUTF16String description;
					
					hr = pPropBag->Read(L"FriendlyName", &var, 0);
					
					if(SUCCEEDED(hr))
					{
						description = (const PA_Unichar *)var.bstrVal;
						VariantClear(&var);
						
						if(n.compare(description) == 0)
						{
							hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pBFilter);
						}
						
					}
					
					pPropBag->Release();
					pMoniker->Release();
				}
				
			}
			pDevEnum->Release();
		}
	
	return pBFilter;
}

bool _Snap(C_TEXT &device, C_BLOB &imageData)
{
	bool success = false;
	
	if(SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		IBaseFilter *pDeviceFilter = _DEVICE(device);
		if(pDeviceFilter)
		{
			IGraphBuilder *pGraph = NULL;
			HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL,
																		CLSCTX_INPROC_SERVER,
																		IID_IGraphBuilder,
																		(void **)&pGraph);
			if(SUCCEEDED(hr)){
				ICaptureGraphBuilder2 *pCapture = NULL;
				hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,
															NULL,
															CLSCTX_INPROC_SERVER,
															IID_ICaptureGraphBuilder2,
															(void**)&pCapture);
				
				if(SUCCEEDED(hr)){
					IBaseFilter *pGrabberFilter = NULL;
					hr = CoCreateInstance(CLSID_SampleGrabber,
																NULL,
																CLSCTX_INPROC_SERVER,
																IID_PPV_ARGS(&pGrabberFilter));
					if(SUCCEEDED(hr)){
						ISampleGrabber *pGrabber = NULL;
						hr = pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void **)&pGrabber);
						if(SUCCEEDED(hr)){
							//Set the Media Type
							AM_MEDIA_TYPE mt;
							ZeroMemory(&mt, sizeof(mt));
							mt.majortype = MEDIATYPE_Video;
							mt.subtype = MEDIASUBTYPE_RGB24;
							mt.formattype = FORMAT_VideoInfo;
							hr = pGrabber->SetMediaType(&mt);
							if(SUCCEEDED(hr)){
								hr = pGraph->AddFilter(pGrabberFilter, L"Sample Grabber");
								if(SUCCEEDED(hr)){
									hr = pCapture->SetFiltergraph(pGraph);
									if(SUCCEEDED(hr)){
										IMediaControl *pMControl = NULL;
										hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pMControl);
										if(SUCCEEDED(hr)){
											hr = pGraph->AddFilter(pDeviceFilter, L"Device Filter");
											if(SUCCEEDED(hr)){
												hr = pCapture->RenderStream(&PIN_CATEGORY_PREVIEW,
																										NULL,
																										pDeviceFilter,
																										pGrabberFilter,
																										NULL);
												if(SUCCEEDED(hr)){
													IVideoWindow *pVWindow = NULL;
													hr = pGraph->QueryInterface(IID_IVideoWindow, (void **)&pVWindow);
													if(SUCCEEDED(hr)){
														
														pVWindow->put_Visible(OAFALSE);
														pVWindow->put_AutoShow(OAFALSE);
														pVWindow->HideCursor(OAFALSE);
														pVWindow->SetWindowPosition(0, 0, 0, 0);

														pGrabber->SetBufferSamples(TRUE);
														pMControl->Run();
														Sleep(1200);

														AM_MEDIA_TYPE am_media_type;
														ZeroMemory(&am_media_type, sizeof(am_media_type));
														pGrabber->GetConnectedMediaType(&am_media_type);
														VIDEOINFOHEADER *pVideoInfoHeader = (VIDEOINFOHEADER *)am_media_type.pbFormat;														
														long size = am_media_type.lSampleSize;
														std::vector<uint8_t> buf(size);
														
														pGrabber->GetCurrentBuffer(&size, (long *)&buf[0]);
														
														pGrabber->SetBufferSamples(FALSE);
														pMControl->Pause();
														
														BITMAPFILEHEADER bmphdr;
														memset(&bmphdr, 0, sizeof(bmphdr));
														bmphdr.bfType = ('M' << 8) | 'B';
														bmphdr.bfSize = sizeof(bmphdr) + sizeof(BITMAPINFOHEADER) + size;
														bmphdr.bfOffBits = sizeof(bmphdr) + sizeof(BITMAPINFOHEADER);

														imageData.addBytes((const uint8_t *)&bmphdr, sizeof(bmphdr));
														imageData.addBytes((const uint8_t *)&pVideoInfoHeader->bmiHeader, sizeof(BITMAPINFOHEADER));
														imageData.addBytes((const uint8_t *)&buf[0], size);
														
														success = true;
														
														pVWindow->Release();
													}//pVWindow
												}//RenderStream
											}//AddFilter
											pMControl->Release();
										}//pMControl
									}//SetFiltergraph
								}//AddFilter
							}//SetMediaType
							pGrabber->Release();
						}//pGrabber
						pGrabberFilter->Release();
					}//pGrabberFilter
					pCapture->Release();
				}//pCapture
				pGraph->Release();
			}//pGraph
			pDeviceFilter->Release();
		}//pDeviceFilter
	
		CoUninitialize();
	}
	return success;
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
	_DEVICE_Get_default(returnValue);
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
	_DEVICE_LIST(Param1);
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
	C_BLOB imageData;
	if(_Snap(Param1, imageData))
	{
		PA_Picture *Param2 = (PA_Picture *)(pParams[1]);
		
		if (*Param2)
			PA_DisposePicture(*Param2);
		
		*Param2 = PA_CreatePicture((void *)imageData.getBytesPtr(), imageData.getBytesLength());
		returnValue.setIntValue(1);
	}
#endif

	returnValue.setReturn(pResult);
}

