/*
 * Update IActivityTestService.aidl if you change this file.
 * In particular, the ordering of the methods below must match and IActivityTestService.aidl
 */

#ifndef __IACTIVITY_TEST_SERVICE_H_
#define __IACTIVITY_TEST_SERVICE_H_

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

using namespace android;

class ILcdActivity;
class ICameraActivity;
class ITouchPanelActivity;

class IActivityTestService : public IInterface
{
public:
    DECLARE_META_INTERFACE(ActivityTestService);

    virtual void registerCameraActivity(int pid, const sp<ICameraActivity>& cameraActivity) = 0;
    virtual void unregisterCameraActivity(const sp<ICameraActivity>& cameraActivity) = 0;

    virtual void registerLcdActivity(int pid, const sp<ILcdActivity>& lcdActivity) = 0;
    virtual void unregisterLcdActivity(const sp<ILcdActivity>& lcdActivity) = 0;

    virtual sp<ITouchPanelActivity> getTouchPanelActivity() = 0;
};

class BnActivityTestService : public BnInterface<IActivityTestService>
{
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

#endif
