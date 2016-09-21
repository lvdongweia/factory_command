#define LOG_TAG "RBluetoothManager"

#include "RBluetoothManager.h"
#include "RBluetoothAdapterHw.h"
#include "RBluetoothListener.h"
#include "RAdapterProperties.h"
#include "RRemoteDevices.h"
#include "RMessage.h"

#include "factory_log.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

RBluetoothManager* RBluetoothManager::mManager = NULL;

RBluetoothManager* RBluetoothManager::instance()
{
    if (mManager == NULL)
        mManager = new RBluetoothManager();

    return mManager;
}

void RBluetoothManager::release()
{
    if (mManager)
        delete mManager;

    mManager = NULL;
}

RBluetoothManager::RBluetoothManager()
{
    mIsOpened = false;
    mIsRunning = false;
    mEnableState = false;

    mListener = new RBluetoothListener(this);

    mHw = RBluetoothAdapterHw::instance();
    mHw->setBluetoothCallbacks(mListener);

    mRemoteDevices = new RRemoteDevices(mHw);
    mProps = new RAdapterProperties(mHw, mRemoteDevices);

    initBt();
}

RBluetoothManager::~RBluetoothManager()
{
    deinitBt();

    RBluetoothAdapterHw::release();

    delete mProps;
    delete mListener;
    delete mRemoteDevices;

    RMessage* msg = NULL;
    android::List<RMessage*>::iterator iter = mMsgList.begin();
    for (; iter != mMsgList.end(); )
    {
        msg = *iter;
        delete msg;

        iter = mMsgList.erase(iter);
    }
}

void RBluetoothManager::initBt()
{
    mIsRunning = true;
    memset(&mThreadId, 0, sizeof(pthread_t));
    pthread_create(&mThreadId, NULL, processMessage, this);

    mHw->init();
    mHw->getAdapterProperty(BT_PROPERTY_BDADDR);
    mHw->getAdapterProperty(BT_PROPERTY_BDNAME);
    mHw->configHciSnoopLog(true);
}

void RBluetoothManager::deinitBt()
{
    if (isBond())
    {
        cancelBond();

        int cancel_timeout = 5;
        while (cancel_timeout-- > 0 && isBond())
            sleep(1);

        if (cancel_timeout <= 0)
            LOGW("wait cancel bond timeout");
    }

    if (mIsOpened || mEnableState)
    {
        closeBt();

        int disable_timeout = 10;
        while (disable_timeout-- > 0 && mEnableState)
            sleep(1);

        if (disable_timeout <= 0)
            LOGW("wait disable bt timeout");
    }

    /* FIXME:
     * Commenting this out as for some reason,
     * the application does not exit otherwise.
     * call cleanup block
     *
     * Note: BT must be already disabled before cleanup
     */
    mHw->cleanup();

    mIsRunning = false;
    {
        android::Mutex::Autolock lock(mListMutex);
        mListCondition.signal();
    }

    pthread_join(mThreadId, NULL);
}

bool RBluetoothManager::openBt()
{
    android::Mutex::Autolock lock(mMutex);

    if (mIsOpened) return true;

    bool ret = mHw->enable();
    if (ret)
    {
        mIsOpened = true;
        mProps->setState(RAdapterProperties::STATE_ON);
        //TODO: wait BT_STATE_ON, if timeout, set off state
    }
    else
    {
        LOGE("Error while turning Bluetooth On");
        mIsOpened = false;
        mProps->setState(RAdapterProperties::STATE_OFF);
    }

    return ret;
}

void RBluetoothManager::closeBt()
{
    android::Mutex::Autolock lock(mMutex);
    if (!mIsOpened) return;

    mProps->cancelDiscovery();

    mProps->setState(RAdapterProperties::STATE_OFF);
    mProps->onBluetoothDisable();

    mHw->disable();

    mIsOpened = false;
    //TODO: wait mEnableState
}

bool RBluetoothManager::isOpened()
{
    android::Mutex::Autolock lock(mMutex);

    return (mIsOpened && mEnableState);
}

void RBluetoothManager::startDiscovery()
{
    return mProps->startDiscovery();
}

void RBluetoothManager::cancelDiscovery()
{
    return mProps->cancelDiscovery();
}

bool RBluetoothManager::isDiscovering()
{
    return mProps->isDiscovering();
}

bool RBluetoothManager::isBond()
{
    if (!isOpened()) return false;

    return mRemoteDevices->isBond();
}

bool RBluetoothManager::pairing(RBluetoothAddr& addr)
{
    if (!isOpened()) return false;

    android::Mutex::Autolock lock(mMutex);
    return mHw->createBond(addr);
}

bool RBluetoothManager::cancelBond()
{
    if (!isBond()) return true;

    return mRemoteDevices->cancelBond();
}

bool RBluetoothManager::isFound()
{
    return mRemoteDevices->isFound();
}

bool RBluetoothManager::isFound(RBluetoothAddr& addr)
{
    return mRemoteDevices->isFound(addr);
}

int RBluetoothManager::getSignalStrength()
{
    if (!isBond()) return 0;

    if (!mRemoteDevices->isFound()) return 0;

    return mRemoteDevices->getSignalStrength();
}

void RBluetoothManager::sendMessage(RMessage* msg)
{
    android::Mutex::Autolock lock(mListMutex);
    mMsgList.push_back(msg);
    mListCondition.signal();
}

RMessage* RBluetoothManager::receiveMessage()
{
    RMessage* tmp = NULL;

    android::Mutex::Autolock lock(mListMutex);

    if (mMsgList.empty())
        mListCondition.wait(mListMutex);

    //recv exit signal
    if (mMsgList.empty()) return NULL;

    tmp = *(mMsgList.begin());
    mMsgList.erase(mMsgList.begin());

    return tmp;
}

void RBluetoothManager::handleAdapterState(RMessage* msg)
{
    android::Mutex::Autolock lock(mMutex);

    int status = msg->data.state;
    if (status == BT_STATE_OFF)
    {
        mEnableState = false;
    }
    else if (status == BT_STATE_ON)
    {
        mEnableState = true;
        mProps->onBluetoothReady();
    }
    else
        LOGE("Incorrect status: %d", status);
}

void RBluetoothManager::handleAdapterProps(RMessage* msg)
{
    mProps->adapterPropertyChanged(msg->data.props.mNum, msg->data.props.mValues);
}

void RBluetoothManager::handleRemoteDeviceProps(RMessage* msg)
{
    mRemoteDevices->devicePropertyChanged(
            msg->data.remote.addr,
            msg->data.remote.props.mNum,
            msg->data.remote.props.mValues);
}

void RBluetoothManager::handleDeviceFound(RMessage* msg)
{
    mRemoteDevices->deviceFound(msg->data.addr);
}

void RBluetoothManager::handleDiscoveryState(RMessage* msg)
{
    mProps->discoveryStateChange(msg->data.state);
}

void RBluetoothManager::handlePinRequest(RMessage* msg)
{
    LOGD("%s, %p", __func__, msg);
}

void RBluetoothManager::handleSspRequest(RMessage* msg)
{
    mRemoteDevices->sspRequest(
            msg->data.ssp.addr,
            msg->data.ssp.name,
            msg->data.ssp.cod,
            msg->data.ssp.variant,
            msg->data.ssp.key);
}

void RBluetoothManager::handleBondState(RMessage* msg)
{
    mRemoteDevices->bondStateChange(
            msg->data.bond_state.status,
            msg->data.bond_state.addr,
            msg->data.bond_state.state);
}

void RBluetoothManager::handleAclState(RMessage* msg)
{
    mRemoteDevices->aclStateChange(
            msg->data.acl_state.status,
            msg->data.acl_state.addr,
            msg->data.acl_state.state);
}

void *RBluetoothManager::processMessage(void *arg)
{
    RBluetoothManager* manager = (RBluetoothManager*)arg;

    while (manager->mIsRunning)
    {
        RMessage* tmp = manager->receiveMessage();
        if (tmp == NULL) continue;

        switch (tmp->id)
        {
            case ADAPTER_STATE_CHANGED:
                manager->handleAdapterState(tmp);
                break;
            case ADAPTER_PROPERTIES:
                manager->handleAdapterProps(tmp);
                break;
            case REMOTE_DEVICE_PROPERTIES:
                manager->handleRemoteDeviceProps(tmp);
                break;
            case DEVICE_FOUND:
                manager->handleDeviceFound(tmp);
                break;
            case DISCOVERY_STATE_CHANGED:
                manager->handleDiscoveryState(tmp);
                break;
            case PIN_REQUEST:
                manager->handlePinRequest(tmp);
                break;
            case SSP_REQUEST:
                manager->handleSspRequest(tmp);
                break;
            case BOND_STATE_CHANGED:
                manager->handleBondState(tmp);
                break;
            case ACL_STATE_CHANGED:
                manager->handleAclState(tmp);
                break;
            default:
                LOGW("unknown message id");
        }

        delete tmp;

        //TODO: sleep, condtion wait
    }

    pthread_exit((void*)"exit process message thread");
    return (void*)NULL;
}
