/*
 * Update ICameraActivity.aidl if you change this file.
 * In particular, the ordering of the methods below must match and ICameraActivity.aidl
 */
#ifndef __ICAMERA_ACTIVITY_H_
#define __ICAMERA_ACTIVITY_H_

#include <utils/RefBase.h>
#include <utils/String16.h>

#include <binder/IInterface.h>
#include <binder/Parcel.h>

using namespace android;

class ICameraActivity : public IInterface
{
public:
    DECLARE_META_INTERFACE(CameraActivity);

    virtual int openCamera() = 0;
    virtual int closeCamera() = 0;

    virtual int takePicture(int storage, const String16& filename) = 0;
    virtual int viewPicture(int storage, const String16& filename) = 0;
    virtual int preview() = 0;
    virtual int compareImage(int pattern, int storage, const String16& filename) = 0;
};

//BnCameraActivity implement in APP
class BnCameraActivity : public BnInterface<ICameraActivity>
{
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

#endif
