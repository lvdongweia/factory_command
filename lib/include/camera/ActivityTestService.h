#ifndef __ACTIVITY_TEST_SERVICE_H_
#define __ACTIVITY_TEST_SERVICE_H_

#include "IActivityTestService.h"
#include "ITouchPanelActivity.h"

#include <utils/Mutex.h>
#include <utils/String16.h>

using namespace android;

class ILcdActivity;
class ICameraActivity;

class ActivityTestCallback
{
public:
    virtual ~ActivityTestCallback() {}

    virtual int touch(int num, const int* points) = 0;
};

class ActivityTestService : public BnActivityTestService
{
public:
    static sp<ActivityTestService> instance();
    static void release();

    void setCallback(ActivityTestCallback* callback);

    //camera test
    int startCameraActivity();
    int stopCameraActivity();
    bool isCameraActivity();

    int openCamera();
    int closeCamera();

    int takePicture(int storage, const String16& filename);
    int viewPicture(int storage, const String16& filename);

    int preview();
    int compareImage(int pattern, int storage, const String16& filename);

    //lcd test
    int startLcdActivity();
    int stopLcdActivity();
    bool isLcdActivity();

    int display(int pattern);

    //touch
    int startTouchActivity();
    int stopTouchActivity();
    bool isTouchActivity();

public:
    //binder communication
    virtual void registerCameraActivity(int pid, const sp<ICameraActivity>& cameraActivity);
    virtual void unregisterCameraActivity(const sp<ICameraActivity>& cameraActivity);

    virtual void registerLcdActivity(int pid, const sp<ILcdActivity>& lcdActivity);
    virtual void unregisterLcdActivity(const sp<ILcdActivity>& lcdActivity);

    virtual sp<ITouchPanelActivity> getTouchPanelActivity();

private:
    class TouchPanelActivity : public BnTouchPanelActivity
    {
    public:
        TouchPanelActivity(const sp<ActivityTestService>& service);
        virtual ~TouchPanelActivity();

        virtual int touch(int num , const int* points);

    private:
        sp<ActivityTestService> mService;
    };

    friend class TouchPanelActivity;

    ActivityTestService();
    virtual ~ActivityTestService();

    Mutex mMutex;

    ActivityTestCallback* mCallback;

    int mLcdPid;
    sp<ILcdActivity> mLcdActivity;

    int mCameraPid;
    sp<ICameraActivity> mCameraActivity;

    bool mIsTouchActivity;
    sp<ITouchPanelActivity> mTouchActivity;

    static sp<ActivityTestService> mService;
};

#endif
