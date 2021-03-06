4d-plugin-capture
=================

Take a picture from a connect video (camera) device.

###Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🆗|🆗|

###Version

<img src="https://cloud.githubusercontent.com/assets/1725068/18940649/21945000-8645-11e6-86ed-4a0f800e5a73.png" width="32" height="32" /> <img src="https://cloud.githubusercontent.com/assets/1725068/18940648/2192ddba-8645-11e6-864d-6d5692d55717.png" width="32" height="32" />

###Syntax

```
success:=CAPTURE Snap(device;photo)
```

param|type|description
------------|------|----
device|TEXT|device id (you may pass ``CAPTURE DEVICE Get default``)
photo|TEXT|the captured image (TIFF on Mac, BMP 24 bits on Windows) 
success|INT32|1 for success

```
device:=CAPTURE DEVICE Get default
```

param|type|description
------------|------|----
device|TEXT|device id for the default device

```
CAPTURE DEVICE LIST(devices)
```

param|type|description
------------|------|----
devices|ARRAY TEXT|device ids for each connected device

![](https://github.com/miyako/4d-plugin-capture/blob/master/m.png)

![](https://github.com/miyako/4d-plugin-capture/blob/master/w.png)

###Remarks

There is an intended 1.2 second delay to capture the video. This is to allow the camera to calibrate for environment lighting.

###Compatibility break

``CAPTURE DEVICE LIST`` no longer returns the description in ``$2``.

The old version can be found in the ``v1`` branch.
