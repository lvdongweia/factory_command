#ifndef __RREMOTE_DEVICES_H_
#define __RREMOTE_DEVICES_H_

#include "RBluetoothCommon.h"

#include <utils/Mutex.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>

#include <unistd.h>
#include <stdio.h>

class RBluetoothAdapterHw;
class RRemoteDevices
{
public:
    RRemoteDevices(RBluetoothAdapterHw* hw);
    ~RRemoteDevices();

    bool cancelBond();
    bool isBond();

    bool isFound();
    bool isFound(RBluetoothAddr& addr);
    void startFound();

    void deviceFound(bt_bdaddr_t& addr);
    void devicePropertyChanged(bt_bdaddr_t& addr, int num, bt_property_t* props);
    void aclStateChange(int status, bt_bdaddr_t& addr, int state);
    void sspRequest(bt_bdaddr_t& addr, bt_bdname_t& name,
            uint32_t cod, bt_ssp_variant_t& variant, uint32_t key);

    void bondStateChange(int status, bt_bdaddr_t& addr, int state);

    int getSignalStrength();

private:
    class DeviceProperties
    {
    public:
        DeviceProperties(const bt_bdaddr_t& addr);
        ~DeviceProperties();

        void setDeviceName(void* val, int len);
        void setFriendlyName(void* val, int len);
        void setUuids(void *val, int len);

    private:
        void setName(void* name, size_t size, void* val, int len);

        char mName[250];
        char mAlias[250];

        uint8_t mAddress[6];

        int32_t mRssi;
        uint32_t mBtClass;

        int mDeviceType;
        int mBondState;

        bool mIsFound;

        android::Vector<bt_uuid_t> mUuids;

        friend class RRemoteDevices;
    };

    struct AddrKey
    {
        AddrKey() {}
        AddrKey(const bt_bdaddr_t& address) { addr = address; }
        ~AddrKey() {}

        bool operator<(const AddrKey& rhs) const;

        bt_bdaddr_t addr;
    };

    bool mIsFound;

    DeviceProperties* getDevice(const bt_bdaddr_t& addr);
    DeviceProperties* addDevice(const bt_bdaddr_t& addr);

    DeviceProperties* mBondDevice;
    RBluetoothAdapterHw* mHw;

    android::Mutex mMutex;
    android::KeyedVector<AddrKey, DeviceProperties*> mDevices;
};

#endif
