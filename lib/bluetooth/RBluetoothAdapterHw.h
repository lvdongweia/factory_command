/*
 * bluetooth device handle
 */
#ifndef __RBLUETOOTH_ADAPTER_HW_H_
#define __RBLUETOOTH_ADAPTER_HW_H_

#include "RBluetoothCommon.h"
#include <hardware/bt_sock.h>

class RBluetoothCallbacks;
class RBluetoothAdapterHw
{
public:
    static RBluetoothAdapterHw* instance();
    static void release();

    void setBluetoothCallbacks(RBluetoothCallbacks* callbacks);
    const bt_interface_t* getBluetoothInterface();

    bool init();
    bool cleanup();

    bool enable();
    bool disable();

    bool startDiscovery();
    bool cancelDiscovery();

    bool createBond(RBluetoothAddr& addr);
    bool removeBond(RBluetoothAddr& addr);
    bool cancelBond(RBluetoothAddr& addr);

    bool pinReply(RBluetoothAddr& addr, bool accept, int len, RBluetoothPinCode& pin);

    //type: bt_ssp_variant_t
    bool sspReply(RBluetoothAddr& addr, int type, bool accept, uint32_t passkey);

    //type: bt_property_type_t
    bool setAdapterProperty(int type, void* value, int len);
    bool getAdapterProperties();
    bool getAdapterProperty(int type);

    bool getDeviceProperty(RBluetoothAddr& addr, int type);
    bool setDeviceProperty(RBluetoothAddr& addr, int type, void *value, int len);

    bool getRemoteServices(RBluetoothAddr& addr);

    int connectSocket(RBluetoothAddr& addr, int type,
            const uint8_t *uuid, int channel, int flag);

    int createSocketChannel(int type, const char* service_name,
            const uint8_t *uuid, int channel, int flag);

    bool configHciSnoopLog(bool enable);

private:
    //Bluetooth Interface callbacks
    static void onAdapterStateChanged(bt_state_t status);
    static void onAdapterProperties(bt_status_t status,
            int num_properties, bt_property_t *properties);
    static void onRemoteDeviceProperties(bt_status_t status, bt_bdaddr_t *bd_addr,
            int num_properties, bt_property_t *properties);
    static void onDeviceFound(int num_properties, bt_property_t *properties);
    static void onDiscoveryStateChanged(bt_discovery_state_t state);
    static void onPinRequest(bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod);
    static void onSspRequest(bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod,
            bt_ssp_variant_t pairing_variant, uint32_t pass_key);
    static void onBondStateChanged(bt_status_t status, bt_bdaddr_t *bd_addr,
            bt_bond_state_t state);
    static void onAclStateChanged(bt_status_t status, bt_bdaddr_t *bd_addr,
            bt_acl_state_t state);
    static void onThreadEvent(bt_cb_thread_evt event);
    static void onDutModeRecv(uint16_t opcode, uint8_t *buf, uint8_t len);
    static void onLeTestModeRecv(bt_status_t status, uint16_t packet_count);

    RBluetoothAdapterHw();
    ~RBluetoothAdapterHw();

    void loadBluetoothModule();
    void unloadBluetoothModule();
    void initBluetoothCallbacks();

    hw_module_t *mBtModule;
    hw_device_t *mBtDevice;

    bt_callbacks_t mBtCallbacks;
    const bt_interface_t *mBtInterface;
    btsock_interface_t *mBtSocketInterface;

    RBluetoothCallbacks* mCallbacks;
    static RBluetoothAdapterHw *mDevice;
};

#endif
