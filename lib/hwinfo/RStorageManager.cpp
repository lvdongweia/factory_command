#define LOG_TAG "RStorageManager"

#include "IMountService.h"
#include "RStorageManager.h"
#include "factory_log.h"

#include <binder/Binder.h>
#include <binder/IServiceManager.h>

#include <utils/String16.h>

RStorageManager* RStorageManager::mManager = NULL;

RStorageManager* RStorageManager::instance()
{
    if (mManager == NULL)
        mManager = new RStorageManager();

    return mManager;
}

void RStorageManager::release()
{
    if (mManager != NULL)
        delete mManager;

    mManager = NULL;
}

RStorageManager::RStorageManager()
{
    mMountService = NULL;

    initialize();
}

RStorageManager::~RStorageManager()
{
    mMountService.clear();
}

bool RStorageManager::initialize()
{
    if (mMountService != NULL)
        return true;

    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == NULL)
    {
        LOGE("get service manager failed!");
        return false;
    }

    //maybe block if mount service don't up
    mMountService = interface_cast<IMountService>(sm->getService(String16("mount")));
    if (mMountService == NULL)
    {
        LOGE("Couldn't get connection to MountService");
        return false;
    }

    //TODO: new callback and listener
    //TODO: register listener
    return true;
}

bool RStorageManager::isTFCardMounted()
{
    Mutex::Autolock lock(mMutex);

    if (!initialize()) return false;

    String16 state = mMountService->getVolumeState(String16("/mnt/external_sd"));
    bool is_mounted = (state == String16("mounted")) ? true : false;

    return is_mounted;
}
