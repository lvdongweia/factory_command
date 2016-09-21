#define LOG_TAG "RAdapterProperties"

#include "RAdapterProperties.h"
#include "RBluetoothAdapterHw.h"
#include "RRemoteDevices.h"

#include "factory_log.h"

RAdapterProperties::RAdapterProperties(RBluetoothAdapterHw* hw, RRemoteDevices* remote)
{
    mHw = hw;
    mRemote = remote;

    mState = STATE_OFF;
    mScanMode = BT_SCAN_MODE_NONE;
    mBtClass = 0;
    mDiscoverableTimeout = 0;

    mIsSearching = false;
    mDiscovering = false;
    mBluetoothDisabling = true;

    memset(mName, 0, sizeof(mName));
    memset(mAddress, 0, sizeof(mAddress));
}

RAdapterProperties::~RAdapterProperties()
{
    mUuids.clear();
}

void RAdapterProperties::setName(void* val, int len)
{
    memset(mName, 0, sizeof(mName));
    memcpy(mName, val, len);
    LOGD("Adapter Name: %s", mName);
}

void RAdapterProperties::setUuids(void* val, int num)
{
    bt_uuid_t* uuids = (bt_uuid_t*)val;
    for (int i = 0; i < num; i++)
    {
        bt_uuid_t uu = uuids[i];
        mUuids.push(uu);
    }

    for (int i = 0; i < num; i++)
    {
        bt_uuid_t uu = mUuids[i];
        char format[64] = {0};
        for (int j = 0; j < 16; j++)
            snprintf(format + j * 2, 64 - j * 2, "%02x", uu.uu[j]);

        LOGD("uuid: %d, %s", i, format);
    }
}

void RAdapterProperties::adapterPropertyChanged(int num, bt_property_t* props)
{
    for (int i = 0; i < num; i++)
    {
        android::Mutex::Autolock lock(mMutex);

        LOGD("type: %d, len %d", props[i].type, props[i].len);
        switch(props[i].type)
        {
            case BT_PROPERTY_BDNAME:
                setName(props[i].val, props[i].len);
                break;
            case BT_PROPERTY_BDADDR:
                memcpy(mAddress, props[i].val, props[i].len);
                PRINT_ADDR(mAddress)
                break;
            case BT_PROPERTY_CLASS_OF_DEVICE:
                memcpy(&mBtClass, props[i].val, props[i].len);
                LOGD("BT Class: %d", mBtClass);
                break;
            case BT_PROPERTY_ADAPTER_SCAN_MODE:
                memcpy(&mScanMode, props[i].val, props[i].len);
                LOGD("SCAN Mode: %d", mScanMode);
                updateScanMode();
                if (mBluetoothDisabling)
                {
                    mBluetoothDisabling = false;
                    //TODO: start bt disable
                }
                break;
            case BT_PROPERTY_UUIDS:
                setUuids(props[i].val, props[i].len / sizeof(bt_uuid_t));
                break;
            case BT_PROPERTY_ADAPTER_BONDED_DEVICES:
                //TODO: val is array of bt_bdaddr_t
                //TODO: bonded devices addr
                LOGW("need handle bonded deivces");
                break;
            case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
                memcpy(&mDiscoverableTimeout, props[i].val, props[i].len);
                LOGD("Discoverable Timeout: %u", mDiscoverableTimeout);
                break;
            default:
                LOGW("don't handle type: %d", props[i].type);
        }
    }
}

void RAdapterProperties::discoveryStateChange(int state)
{
    android::Mutex::Autolock lock(mMutex);
    if (state == BT_DISCOVERY_STOPPED)
    {
        if (mState == STATE_ON && mIsSearching)
        {
            mHw->startDiscovery();
        }
        else if (mState == STATE_ON && mDiscovering
                && !mIsSearching)
        {
            mHw->cancelDiscovery();
        }
        else if (mState == STATE_OFF && (mDiscovering || !mIsSearching))
        {
            mHw->cancelDiscovery();
        }

        mDiscovering = false;

    }
    else if (state == BT_DISCOVERY_STARTED)
    {
        mDiscovering = true;
    }
}

bool RAdapterProperties::isDiscovering()
{
    android::Mutex::Autolock lock(mMutex);
    return mIsSearching || mDiscovering;
}

void RAdapterProperties::onBluetoothDisable()
{
    android::Mutex::Autolock lock(mMutex);

    mBluetoothDisabling = true;
    if (mState == STATE_OFF)
        setScanMode(BT_SCAN_MODE_NONE);
}

void RAdapterProperties::onBluetoothReady()
{
    android::Mutex::Autolock lock(mMutex);
    if (mState == STATE_ON && mScanMode == BT_SCAN_MODE_NONE)
    {
        if (mDiscoverableTimeout != 0)
            setScanMode(BT_SCAN_MODE_CONNECTABLE);
        else
            setScanMode(BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);

        setDiscoverableTimeout(mDiscoverableTimeout);
    }
}

void RAdapterProperties::startDiscovery()
{
    android::Mutex::Autolock lock(mMutex);
    if (mState == STATE_ON && !mIsSearching)
    {
        setDiscoverableTimeout(mDiscoverableTimeout);
        setScanMode(BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        mIsSearching = true;
    }
}

void RAdapterProperties::cancelDiscovery()
{
    android::Mutex::Autolock lock(mMutex);
    if (mState == STATE_ON && mIsSearching)
    {
        mHw->cancelDiscovery();
        mIsSearching = false;
    }
}

void RAdapterProperties::updateScanMode()
{
    if (mState == STATE_ON && mScanMode == BT_SCAN_MODE_CONNECTABLE)
    {
        setDiscoverableTimeout(mDiscoverableTimeout);
        setScanMode(BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
    }
    else if (mState == STATE_ON && mIsSearching
            && mScanMode == BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE)
    {
        mRemote->startFound();
        mHw->startDiscovery();
    }
}

void RAdapterProperties::setState(int state)
{
    android::Mutex::Autolock lock(mMutex);
    mState = state;
}

int RAdapterProperties::getState()
{
    android::Mutex::Autolock lock(mMutex);
    return mState;
}

bool RAdapterProperties::setScanMode(bt_scan_mode_t mode)
{
    return mHw->setAdapterProperty(
            BT_PROPERTY_ADAPTER_SCAN_MODE,
            &mode,
            sizeof(bt_scan_mode_t));
}

bool RAdapterProperties::setDiscoverableTimeout(int timeout)
{
    return mHw->setAdapterProperty(
            BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,
            &timeout, sizeof(uint32_t));
}
