#ifndef __radapter_properties_h_
#define __radapter_properties_h_

#include "RBluetoothCommon.h"

#include <utils/Mutex.h>
#include <utils/Vector.h>

#include <unistd.h>
#include <stdio.h>

class RBluetoothAdapterHw;
class RRemoteDevices;
class RAdapterProperties
{
public:
    enum BtState
    {
        STATE_ON,
        STATE_OFF,
    };

public:
    RAdapterProperties(RBluetoothAdapterHw* hw, RRemoteDevices* remote);
    ~RAdapterProperties();

    void adapterPropertyChanged(int num, bt_property_t* props);
    void discoveryStateChange(int state);

    void onBluetoothDisable();
    void onBluetoothReady();

    void setState(int state);
    int getState();

    bool isDiscovering();
    void startDiscovery();
    void cancelDiscovery();

private:
    void setName(void *val, int len);
    void setUuids(void *val, int num);

    bool setScanMode(bt_scan_mode_t mode);
    bool setDiscoverableTimeout(int timeout);

    void updateScanMode();

    char mName[250];

    int mState;

    int mScanMode;
    uint8_t mAddress[6];
    uint32_t mBtClass;
    uint32_t mDiscoverableTimeout;

    bool mIsSearching;
    bool mDiscovering;
    bool mBluetoothDisabling;

    android::Vector<bt_uuid_t> mUuids;
    android::Mutex mMutex;

    RBluetoothAdapterHw* mHw;
    RRemoteDevices* mRemote;
};

#endif
