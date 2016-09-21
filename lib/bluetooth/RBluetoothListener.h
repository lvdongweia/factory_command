#ifndef __RBLUETOOTH_LISTENER_H_
#define __RBLUETOOTH_LISTENER_H_

#include "RBluetoothCallbacks.h"

class RBluetoothManager;
class RBluetoothListener : public RBluetoothCallbacks
{
public:
    RBluetoothListener(RBluetoothManager* manager);
    virtual ~RBluetoothListener();

    virtual void onAdapterStateChanged(bt_state_t status);

    virtual void onAdapterProperties(int num_properties, bt_property_t *properties);

    virtual void onRemoteDeviceProperties(bt_bdaddr_t *bd_addr,
            int num_properties, bt_property_t *properties);

    virtual void onDeviceFound(bt_bdaddr_t *bd_addr);

    virtual void onDiscoveryStateChanged(bt_discovery_state_t state);

    virtual void onPinRequest(bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod);

    virtual void onSspRequest(bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod,
            bt_ssp_variant_t pairing_variant, uint32_t pass_key);

    virtual void onBondStateChanged(bt_status_t status, bt_bdaddr_t *bd_addr,
            bt_bond_state_t state);

    virtual void onAclStateChanged(bt_status_t status, bt_bdaddr_t *bd_addr,
            bt_acl_state_t state);
private:
    RBluetoothManager* mManager;
};

#endif
