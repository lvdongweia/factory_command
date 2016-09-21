#define LOG_TAG "ActivityTestService"

#include "ActivityTestService.h"
#include "ILcdActivity.h"
#include "ICameraActivity.h"
#include "activity_path.h"
#include "factory_log.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <stdlib.h>

//helper function
static int execute_cmd(const char* activity_pkg, bool is_start)
{
    char cmd[256] = {0};

    if (is_start)
        snprintf(cmd, 256, "am start -n %s", activity_pkg);
    else
        snprintf(cmd, 256, "am force-stop %s", activity_pkg);

    if (system(cmd) < 0)
    {
        LOGE("%s %s failed: %s", is_start ? "start" : "stop", activity_pkg, strerror(errno));
        return -1;
    }

    return 0;
}

sp<ActivityTestService> ActivityTestService::mService = NULL;

sp<ActivityTestService> ActivityTestService::instance()
{
    if (mService == NULL)
    {
        mService = new ActivityTestService();
        defaultServiceManager()->addService(String16("IActivityTestService"), mService);
        ProcessState::self()->startThreadPool();
    }

    return mService;
}

void ActivityTestService::release()
{
    if (mService != NULL)
        mService.clear();

    mService = NULL;
}

ActivityTestService::ActivityTestService()
{
    mLcdPid = 0;
    mCameraPid = 0;

    mIsTouchActivity = false;

    mCallback = NULL;

    mLcdActivity = NULL;
    mCameraActivity = NULL;

    mTouchActivity = new TouchPanelActivity(this);
}

ActivityTestService::~ActivityTestService()
{
    LOGD("ActivityTestService distructor~");

    if (mLcdActivity != NULL)
        mLcdActivity.clear();

    if (mCameraActivity != NULL)
        mCameraActivity.clear();

    mTouchActivity.clear();
}

void ActivityTestService::setCallback(ActivityTestCallback* callback)
{
    Mutex::Autolock lock(mMutex);
    mCallback = callback;
}

int ActivityTestService::startCameraActivity()
{
    Mutex::Autolock lock(mMutex);
    return execute_cmd(CAMERA_ACTIVITY, true);
}

int ActivityTestService::stopCameraActivity()
{
    Mutex::Autolock lock(mMutex);
    return execute_cmd(PACKAGE_NAME, false);
}

bool ActivityTestService::isCameraActivity()
{
    Mutex::Autolock lock(mMutex);
    if (mCameraPid != 0 && mCameraActivity != NULL)
    {
        int ret = kill(mCameraPid, 0);
        if (ret < 0 && errno == ESRCH)
        {
            LOGE("Camera Activity is died");

            mCameraPid = 0;
            mCameraActivity.clear();
            mCameraActivity = NULL;

            return false;
        }

        return true;
    }

    return false;
}

int ActivityTestService::openCamera()
{
    if (!isCameraActivity()) return -1;

    Mutex::Autolock lock(mMutex);
    return mCameraActivity->openCamera();
}

int ActivityTestService::closeCamera()
{
    if (!isCameraActivity()) return -1;

    Mutex::Autolock lock(mMutex);
    return mCameraActivity->closeCamera();
}

int ActivityTestService::takePicture(int storage, const String16& filename)
{
    if (!isCameraActivity()) return -1;

    Mutex::Autolock lock(mMutex);
    return mCameraActivity->takePicture(storage, filename);
}

int ActivityTestService::viewPicture(int storage, const String16& filename)
{
    if (!isCameraActivity()) return -1;

    Mutex::Autolock lock(mMutex);
    return mCameraActivity->viewPicture(storage, filename);
}

int ActivityTestService::preview()
{
    if (!isCameraActivity()) return -1;

    Mutex::Autolock lock(mMutex);
    return mCameraActivity->preview();
}

int ActivityTestService::compareImage(int pattern, int storage, const String16& filename)
{
    //TODO: compare

    if (!isCameraActivity()) return -1;

    Mutex::Autolock lock(mMutex);
    return mCameraActivity->compareImage(pattern, storage, filename);
}

int ActivityTestService::startLcdActivity()
{
    Mutex::Autolock lock(mMutex);
    return execute_cmd(LCD_ACTIVITY, true);
}

int ActivityTestService::stopLcdActivity()
{
    Mutex::Autolock lock(mMutex);
    return execute_cmd(PACKAGE_NAME, false);
}

bool ActivityTestService::isLcdActivity()
{
    Mutex::Autolock lock(mMutex);
    if (mLcdPid != 0 && mLcdActivity != NULL)
    {
        int ret = kill(mLcdPid, 0);
        if (ret < 0 && errno == ESRCH)
        {
            LOGE("LCD Activity is died");

            mLcdPid = 0;
            mLcdActivity.clear();
            mLcdActivity = NULL;

            return false;
        }

        return true;
    }

    return false;
}

int ActivityTestService::display(int pattern)
{
    if (!isLcdActivity()) return -1;

    Mutex::Autolock lock(mMutex);
    return mLcdActivity->display(pattern);
}

int ActivityTestService::startTouchActivity()
{
    Mutex::Autolock lock(mMutex);
    return execute_cmd(TOUCH_ACTIVITY, true);
}

int ActivityTestService::stopTouchActivity()
{
    Mutex::Autolock lock(mMutex);
    return execute_cmd(PACKAGE_NAME, false);
}

bool ActivityTestService::isTouchActivity()
{
    Mutex::Autolock lock(mMutex);
    return mIsTouchActivity;
}

void ActivityTestService::registerCameraActivity(int pid, const sp<ICameraActivity>& cameraActivity)
{
    Mutex::Autolock lock(mMutex);
    mCameraPid = pid;
    mCameraActivity = cameraActivity;
}

void ActivityTestService::unregisterCameraActivity(const sp<ICameraActivity>& cameraActivity)
{
    Mutex::Autolock lock(mMutex);
    if (mCameraActivity == cameraActivity)
    {
        mCameraActivity.clear();
        mCameraActivity = NULL;
        mCameraPid = 0;
    }
}

void ActivityTestService::registerLcdActivity(int pid, const sp<ILcdActivity>& lcdActivity)
{
    Mutex::Autolock lock(mMutex);
    mLcdPid = pid;
    mLcdActivity = lcdActivity;
}

void ActivityTestService::unregisterLcdActivity(const sp<ILcdActivity>& lcdActivity)
{
    Mutex::Autolock lock(mMutex);
    if (mLcdActivity == lcdActivity)
    {
        mLcdActivity.clear();
        mLcdActivity = NULL;
        mLcdPid = 0;
    }
}

sp<ITouchPanelActivity> ActivityTestService::getTouchPanelActivity()
{
    Mutex::Autolock lock(mMutex);
    mIsTouchActivity = true;
    return mTouchActivity;
}

ActivityTestService::TouchPanelActivity::TouchPanelActivity(const sp<ActivityTestService>& service)
{
    mService = service;
}

ActivityTestService::TouchPanelActivity::~TouchPanelActivity()
{
}

int ActivityTestService::TouchPanelActivity::touch(int num, const int* points)
{
    Mutex::Autolock lock(mService->mMutex);
    if (mService->mCallback)
        return mService->mCallback->touch(num, points);

    return -1;
}
