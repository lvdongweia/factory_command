#define LOG_TAG "RBluetoothListener"

#include "RBluetoothListener.h"
#include "RBluetoothManager.h"
#include "RMessage.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

RBluetoothListener::RBluetoothListener(RBluetoothManager* manager)
{
    mManager = manager;
}

RBluetoothListener::~RBluetoothListener()
{
}

void RBluetoothListener::onAdapterStateChanged(bt_state_t status)
{
    RMessage* msg = new RMessage;
    msg->id = ADAPTER_STATE_CHANGED;
    msg->data.state = status;

    mManager->sendMessage(msg);
}

void RBluetoothListener::onAdapterProperties(int num_properties, bt_property_t *properties)
{
    RMessage* msg = new RMessage;
    msg->id = ADAPTER_PROPERTIES;
    msg->data.props.copyProps(num_properties, properties);

    mManager->sendMessage(msg);
}

void RBluetoothListener::onRemoteDeviceProperties(
        bt_bdaddr_t *bd_addr, int num_properties, bt_property_t *properties)
{
    RMessage* msg = new RMessage;
    msg->id = REMOTE_DEVICE_PROPERTIES;

    memcpy(&(msg->data.remote.addr), bd_addr, sizeof(bt_bdaddr_t));
    msg->data.remote.props.copyProps(num_properties, properties);

    mManager->sendMessage(msg);
}

void RBluetoothListener::onDeviceFound(bt_bdaddr_t *bd_addr)
{
    RMessage* msg = new RMessage;
    msg->id = DEVICE_FOUND;
    memcpy(&(msg->data.addr), bd_addr, sizeof(bt_bdaddr_t));

    mManager->sendMessage(msg);
}

void RBluetoothListener::onDiscoveryStateChanged(bt_discovery_state_t state)
{
    RMessage* msg = new RMessage;
    msg->id = DISCOVERY_STATE_CHANGED;
    msg->data.state = state;

    mManager->sendMessage(msg);
}

void RBluetoothListener::onPinRequest(bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod)
{
    RMessage* msg = new RMessage;
    msg->id = PIN_REQUEST;

    memcpy(&(msg->data.pin.addr), bd_addr, sizeof(bt_bdaddr_t));
    memcpy(&(msg->data.pin.name), bdname, sizeof(bt_bdname_t));

    msg->data.pin.cod = cod;

    mManager->sendMessage(msg);
}

void RBluetoothListener::onSspRequest(
        bt_bdaddr_t *bd_addr, bt_bdname_t *bdname,
        uint32_t cod, bt_ssp_variant_t pairing_variant, uint32_t pass_key)
{
    RMessage* msg = new RMessage;
    msg->id = SSP_REQUEST;

    memcpy(&(msg->data.ssp.addr), bd_addr, sizeof(bt_bdaddr_t));
    memcpy(&(msg->data.ssp.name), bdname, sizeof(bt_bdname_t));

    msg->data.ssp.cod = cod;
    msg->data.ssp.variant = pairing_variant;
    msg->data.ssp.key = pass_key;

    mManager->sendMessage(msg);
}

void RBluetoothListener::onBondStateChanged(
        bt_status_t status, bt_bdaddr_t *bd_addr, bt_bond_state_t state)
{
    RMessage* msg = new RMessage;
    msg->id = BOND_STATE_CHANGED;

    msg->data.bond_state.status = status;

    memcpy(&(msg->data.bond_state.addr), bd_addr, sizeof(bt_bdaddr_t));

    msg->data.bond_state.state = state;

    mManager->sendMessage(msg);
}

void RBluetoothListener::onAclStateChanged(
        bt_status_t status, bt_bdaddr_t *bd_addr, bt_acl_state_t state)
{
    RMessage* msg = new RMessage;
    msg->id = ACL_STATE_CHANGED;

    msg->data.acl_state.status = status;

    memcpy(&(msg->data.acl_state.addr), bd_addr, sizeof(bt_bdaddr_t));

    msg->data.acl_state.state = state;

    mManager->sendMessage(msg);
}
