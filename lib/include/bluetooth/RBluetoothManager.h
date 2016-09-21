#ifndef __RBLUETOOTH_MANAGER_H_
#define __RBLUETOOTH_MANAGER_H_

#include "RBluetoothCommon.h"

#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/List.h>

#include <pthread.h>

struct RMessage;

class RBluetoothListener;
class RBluetoothAdapterHw;
class RAdapterProperties;
class RRemoteDevices;

class RBluetoothManager
{
public:
    static RBluetoothManager* instance();
    static void release();

    bool openBt();
    void closeBt();
    bool isOpened();

    void startDiscovery();
    void cancelDiscovery();
    bool isDiscovering();

    bool pairing(RBluetoothAddr& addr);
    bool cancelBond();
    bool isBond();

    //any device
    bool isFound();

    //specify device
    bool isFound(RBluetoothAddr& addr);

    int getSignalStrength();

private:
    void sendMessage(RMessage* msg);
    RMessage* receiveMessage();

    RBluetoothManager();
    ~RBluetoothManager();

    void deinitBt();
    void initBt();

    void handleAdapterState(RMessage* msg);
    void handleAdapterProps(RMessage* msg);
    void handleRemoteDeviceProps(RMessage* msg);
    void handleDeviceFound(RMessage* msg);
    void handleDiscoveryState(RMessage* msg);
    void handlePinRequest(RMessage* msg);
    void handleSspRequest(RMessage* msg);
    void handleBondState(RMessage* msg);
    void handleAclState(RMessage* msg);

    static void* processMessage(void *arg);

    bool mEnableState;
    bool mIsOpened;
    android::Mutex mMutex;

    android::Mutex mListMutex;
    android::Condition mListCondition;
    android::List<RMessage*> mMsgList;

    pthread_t mThreadId;
    bool mIsRunning;

    RBluetoothListener* mListener;
    RBluetoothAdapterHw* mHw;
    RAdapterProperties* mProps;
    RRemoteDevices* mRemoteDevices;

    static RBluetoothManager* mManager;

    friend class RBluetoothListener;
};

#endif
