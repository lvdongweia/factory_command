#ifndef _RBLUETOOTH_CALLBACKS_H_
#define _RBLUETOOTH_CALLBACKS_H_

#include "RBluetoothCommon.h"

class RBluetoothCallbacks
{
public:
    virtual ~RBluetoothCallbacks() {}

    virtual void onAdapterStateChanged(bt_state_t status) = 0;

    virtual void onAdapterProperties(int num_properties, bt_property_t *properties) = 0;

    virtual void onRemoteDeviceProperties(bt_bdaddr_t *bd_addr, int num_properties, bt_property_t *properties) = 0;

    virtual void onDeviceFound(bt_bdaddr_t *bd_addr) = 0;

    virtual void onDiscoveryStateChanged(bt_discovery_state_t state) = 0;

    virtual void onPinRequest(bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod) = 0;

    virtual void onSspRequest(bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod,
            bt_ssp_variant_t pairing_variant, uint32_t pass_key) = 0;

    virtual void onBondStateChanged(bt_status_t status, bt_bdaddr_t *bd_addr,
            bt_bond_state_t state) = 0;

    virtual void onAclStateChanged(bt_status_t status, bt_bdaddr_t *bd_addr,
            bt_acl_state_t state) = 0;

    //TODO: unused
    //virtual void onThreadEvent(bt_cb_thread_evt event) = 0;
    //virtual void onDutModeRecv(uint16_t opcode, uint8_t *buf, uint8_t len) = 0;
    //virtual void onLeTestModeRecv(bt_status_t status, uint16_t packet_count) = 0;
};

#endif
