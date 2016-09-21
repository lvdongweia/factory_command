#define LOG_TAG "RRemoteDevices"

#include "RRemoteDevices.h"
#include "RBluetoothAdapterHw.h"

#include "factory_log.h"

//helper function
static int compare_addr(const bt_bdaddr_t& addr1, const bt_bdaddr_t& addr2)
{
    for (int i = 0; i < 6; i++)
    {
        int ret = addr1.address[i] - addr2.address[i];

        if (ret)
            return ret;
    }

    return 0;
}

bool RRemoteDevices::AddrKey::operator<(const AddrKey& rhs) const
{
    int ret = compare_addr(addr, rhs.addr);
    return (ret < 0 ? true : false);
}

RRemoteDevices::DeviceProperties::DeviceProperties(const bt_bdaddr_t& addr)
{
    memset(mName, 0, sizeof(mName));
    memset(mAlias, 0, sizeof(mAlias));
    memset(mAddress, 0, sizeof(mAddress));

    mRssi = 0;
    mBtClass = 0;
    mDeviceType = 0;
    mBondState = BT_BOND_STATE_NONE;

    memcpy(mAddress, addr.address, sizeof(addr));
}

RRemoteDevices::DeviceProperties::~DeviceProperties()
{
    mUuids.clear();
}

void RRemoteDevices::DeviceProperties::setDeviceName(void* val, int len)
{
    setName(mName, sizeof(mName), val, len);
}

void RRemoteDevices::DeviceProperties::setFriendlyName(void* val, int len)
{
    setName(mAlias, sizeof(mAlias), val, len);
}

void RRemoteDevices::DeviceProperties::setName(void* name, size_t size, void* val, int len)
{
    memset(name, 0, size);
    memcpy(name, val, len);
}

void RRemoteDevices::DeviceProperties::setUuids(void *val, int len)
{
    int num = len / sizeof(bt_uuid_t);

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

        LOGD("Remote Device uuid: %d, %s", i, format);
    }
}

RRemoteDevices::RRemoteDevices(RBluetoothAdapterHw* hw)
{
    mIsFound = false;
    mBondDevice = NULL;

    mHw = hw;
}

RRemoteDevices::~RRemoteDevices()
{
    for (size_t i = 0; i < mDevices.size(); i++)
    {
        DeviceProperties* device = mDevices.editValueAt(i);
        delete device;
    }

    mDevices.clear();
}

void RRemoteDevices::deviceFound(bt_bdaddr_t& addr)
{
    DeviceProperties* device = getDevice(addr);
    if (device == NULL)
        device = addDevice(addr);

    android::Mutex::Autolock lock(mMutex);

    mIsFound = true;
    device->mIsFound = true;

    PRINT_ADDR(addr.address)
}

bool RRemoteDevices::isFound()
{
    android::Mutex::Autolock lock(mMutex);
    return mIsFound;
}

bool RRemoteDevices::isFound(RBluetoothAddr& addr)
{
    bt_bdaddr_t address;
    memcpy(&address, &addr, sizeof(bt_bdaddr_t));

    DeviceProperties* device = getDevice(address);

    android::Mutex::Autolock lock(mMutex);
    return (device != NULL && device->mIsFound);
}

void RRemoteDevices::startFound()
{
    android::Mutex::Autolock lock(mMutex);
    mIsFound = false;
}

RRemoteDevices::DeviceProperties* RRemoteDevices::getDevice(const bt_bdaddr_t& addr)
{
    android::Mutex::Autolock lock(mMutex);
    if (mDevices.isEmpty()) return NULL;

    for (size_t i = 0; i < mDevices.size(); i++)
    {
        const AddrKey tmp_addr = mDevices.keyAt(i);
        if (compare_addr(tmp_addr.addr, addr) == 0)
            return mDevices.editValueAt(i);
    }

    return NULL;
}

RRemoteDevices::DeviceProperties* RRemoteDevices::addDevice(const bt_bdaddr_t& addr)
{
    android::Mutex::Autolock lock(mMutex);
    DeviceProperties* device = new DeviceProperties(addr);

    AddrKey key = AddrKey(addr);
    mDevices.add(key, device);

    return device;
}

void RRemoteDevices::devicePropertyChanged(bt_bdaddr_t& addr, int num, bt_property_t* props)
{
    DeviceProperties*  device = getDevice(addr);
    if (device == NULL)
        device = addDevice(addr);

    for (int i = 0; i < num; i++)
    {
        android::Mutex::Autolock lock(mMutex);

        LOGD("type: %d, len %d", props[i].type, props[i].len);
        switch(props[i].type)
        {
            case BT_PROPERTY_BDNAME:
                device->setDeviceName(props[i].val, props[i].len);
                LOGD("Remote Device Name: %s", device->mName);
                break;
            case BT_PROPERTY_REMOTE_FRIENDLY_NAME:
                device->setFriendlyName(props[i].val, props[i].len);
                LOGD("Friendly Name: %s", device->mAlias);
                break;
            case BT_PROPERTY_BDADDR:
                memcpy(device->mAddress, props[i].val, props[i].len);
                LOGD("Remote address:");
                PRINT_ADDR(device->mAddress)
                break;
            case BT_PROPERTY_CLASS_OF_DEVICE:
                memcpy(&(device->mBtClass), props[i].val, props[i].len);
                LOGD("Remote Device class: %d", device->mBtClass);
                break;
            case BT_PROPERTY_UUIDS:
                device->setUuids(props[i].val, props[i].len);
                break;
            case BT_PROPERTY_TYPE_OF_DEVICE:
                memcpy(&(device->mDeviceType), props[i].val, props[i].len);
                LOGD("Remote Device Type: %d", device->mDeviceType);
                break;
            case BT_PROPERTY_REMOTE_RSSI:
                memcpy(&(device->mRssi), props[i].val, props[i].len);
                LOGD("Remote device signal: %d", device->mRssi);
                break;
            default:
                LOGW("unknown remote type");
        }
    }
}

void RRemoteDevices::aclStateChange(int status, bt_bdaddr_t& addr, int state)
{
    LOGD("acl state: %d, %d", status, state);
    PRINT_ADDR(addr.address)

    DeviceProperties* device = getDevice(addr);
    if (device == NULL)
    {
        LOGW("Device is NULL in aclStateChange");
        return;
    }

    android::Mutex::Autolock lock(mMutex);
    if (state == BT_ACL_STATE_CONNECTED)
    {
        LOGD("State: Connected");
        //TODO: update mBondDevice;
    }
    else
    {
        //state == ACTION_ACL_DISCONNECTED
        LOGD("State: DisConnected");
        //TODO: update mBondDevice;
    }
}

void RRemoteDevices::sspRequest(bt_bdaddr_t& addr, bt_bdname_t& name,
        uint32_t cod, bt_ssp_variant_t& variant, uint32_t key)
{
    DeviceProperties* device = getDevice(addr);
    if (device == NULL)
        device = addDevice(addr);

    LOGD("sspRequest: ");
    PRINT_ADDR(addr.address);
    LOGD("name: %s, cod: %u, variant: %d, passKey: %u",
            (const char*)(name.name), cod, variant, key);

    bool need_key = false;
    if (variant == BT_SSP_VARIANT_PASSKEY_CONFIRMATION)
    {
        //TODO:
    }
    else if (variant == BT_SSP_VARIANT_CONSENT)
    {
        //TODO:
    }
    else if (variant == BT_SSP_VARIANT_PASSKEY_ENTRY)
    {
        //TODO:
        need_key = true;
    }
    else if (variant == BT_SSP_VARIANT_PASSKEY_NOTIFICATION)
    {
        //TODO:
        need_key = true;
    }
    else
    {
        LOGE("SSP Pairing variant not present");
        return;
    }

    RBluetoothAddr tmp_addr;
    memcpy(tmp_addr.addr, addr.address, sizeof(addr.address));
    if (need_key)
        mHw->sspReply(tmp_addr, variant, true, key);
    else
        mHw->sspReply(tmp_addr, variant, true, 0);
}

void RRemoteDevices::bondStateChange(int status, bt_bdaddr_t& addr, int state)
{
    LOGD("bond state: %d, %d", status, state);
    PRINT_ADDR(addr.address);

    DeviceProperties *device = getDevice(addr);
    if (device == NULL)
        device = addDevice(addr);

    android::Mutex::Autolock lock(mMutex);
    if (state == BT_BOND_STATE_BONDING)
    {
        device->mBondState = BT_BOND_STATE_BONDING;
    }
    else if (state == BT_BOND_STATE_BONDED && status == BT_STATUS_SUCCESS)
    {
        device->mBondState = BT_BOND_STATE_BONDED;
        mBondDevice = device;
    }
    else
    {
        device->mBondState = BT_BOND_STATE_NONE;
        mBondDevice = NULL;
    }

    if (status != BT_STATUS_SUCCESS)
        LOGE("Bond device Error: %d", status);
}

bool RRemoteDevices::isBond()
{
    android::Mutex::Autolock lock(mMutex);

    return (mBondDevice && mBondDevice->mBondState == BT_BOND_STATE_BONDED);
}

bool RRemoteDevices::cancelBond()
{
    android::Mutex::Autolock lock(mMutex);

    RBluetoothAddr addr;
    memcpy(addr.addr, mBondDevice->mAddress, sizeof(mBondDevice->mAddress));

    //TODO: update bond state or wait callback???
    return (mHw->cancelBond(addr) && mHw->removeBond(addr));
}

int RRemoteDevices::getSignalStrength()
{
    android::Mutex::Autolock lock(mMutex);

    if (mBondDevice)
        return mBondDevice->mRssi;

    return 0;
}
