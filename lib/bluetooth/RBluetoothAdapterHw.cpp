#define LOG_TAG "RBluetoothAdapterHw"

#include "RBluetoothAdapterHw.h"
#include "RBluetoothCallbacks.h"
#include "factory_log.h"

#include <cutils/properties.h>

#include <string.h>

RBluetoothAdapterHw *RBluetoothAdapterHw::mDevice = NULL;

void RBluetoothAdapterHw::onAdapterStateChanged(bt_state_t status)
{
    LOGD("%s: Status is: %d", __func__, status);
    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onAdapterStateChanged(status);
}

void RBluetoothAdapterHw::onAdapterProperties(
        bt_status_t status,
        int num_properties,
        bt_property_t *properties)
{
    LOGD("%s: Status is: %d, Properties: %d", __func__, status, num_properties);
    if (status != BT_STATUS_SUCCESS)
    {
        LOGE("%s: Status %d is incorrect", __func__, status);
        return;
    }

    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onAdapterProperties(num_properties, properties);
}

void RBluetoothAdapterHw::onRemoteDeviceProperties(
        bt_status_t status, bt_bdaddr_t *bd_addr,
        int num_properties, bt_property_t *properties)
{
    LOGD("%s: Status is: %d, Properties: %d", __func__, status, num_properties);

    if (status != BT_STATUS_SUCCESS)
    {
        LOGE("%s: Status %d is incorrect", __func__, status);
        return;
    }

    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onRemoteDeviceProperties(bd_addr, num_properties, properties);
}

void RBluetoothAdapterHw::onDeviceFound(int num_properties, bt_property_t *properties)
{
    int addr_index = -1;
    RBluetoothAddr addr;

    for (int i = 0; i < num_properties; i++)
    {
        if (properties[i].type == BT_PROPERTY_BDADDR)
            addr_index = i;
    }

    if (addr_index < 0)
    {
        LOGE("Address is NULL in %s", __func__);
        return;
    }

    LOGD("%s: Properties: %d, Address: %s", __func__, num_properties,
            (const char *)properties[addr_index].val);

    onRemoteDeviceProperties(BT_STATUS_SUCCESS, (bt_bdaddr_t *)properties[addr_index].val,
            num_properties, properties);

    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onDeviceFound((bt_bdaddr_t *)properties[addr_index].val);
}

void RBluetoothAdapterHw::onDiscoveryStateChanged(bt_discovery_state_t state)
{
    LOGD("%s: DiscoveryState: %d", __func__, state);
    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onDiscoveryStateChanged(state);
}

void RBluetoothAdapterHw::onPinRequest(bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod)
{
    LOGD("%s: device name: %s", __func__, (const char*)bdname);

    if (!bd_addr)
    {
        LOGE("Address is null in %s", __func__);
        return;
    }

    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onPinRequest(bd_addr, bdname, cod);
}

void RBluetoothAdapterHw::onSspRequest(
        bt_bdaddr_t *bd_addr, bt_bdname_t *bdname, uint32_t cod,
        bt_ssp_variant_t pairing_variant, uint32_t pass_key)
{
    LOGD("%s: device name: %s", __func__, (const char*)bdname);

    if (!bd_addr)
    {
        LOGE("Address is null in %s", __func__);
        return;
    }

    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onSspRequest(bd_addr, bdname, cod, pairing_variant, pass_key);
}

void RBluetoothAdapterHw::onBondStateChanged(
        bt_status_t status, bt_bdaddr_t *bd_addr,
        bt_bond_state_t state)
{
    LOGD("%s: state: %d", __func__, state);

    if (!bd_addr)
    {
        LOGE("Address is null in %s", __func__);
        return;
    }

    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onBondStateChanged(status, bd_addr, state);
}

void RBluetoothAdapterHw::onAclStateChanged(
        bt_status_t status, bt_bdaddr_t *bd_addr,
        bt_acl_state_t state)
{
    LOGD("%s: state: %d", __func__, state);

    if (!bd_addr)
    {
        LOGE("Address is null in %s", __func__);
        return;
    }

    if (mDevice->mCallbacks)
        mDevice->mCallbacks->onAclStateChanged(status, bd_addr, state);
}

void RBluetoothAdapterHw::onThreadEvent(bt_cb_thread_evt event)
{
    //JavaVM* vm = AndroidRuntime::getJavaVM();
    if (event == ASSOCIATE_JVM)
    {
        LOGD("attach thread");
        //JavaVMAttachArgs args;
        //char name[] = "BT Service Callback Thread";
        //args.version = JNI_VERSION_1_6;
        //args.name = name;
        //args.group = NULL;
        //vm->AttachCurrentThread(&callbackEnv, &args);
    }
    else if (event == DISASSOCIATE_JVM)
    {
        LOGD("detach thread");
        //TODO
        //vm->DetachCurrentThread();
    }
}

void RBluetoothAdapterHw::onDutModeRecv(uint16_t opcode, uint8_t *buf, uint8_t len)
{
}

void RBluetoothAdapterHw::onLeTestModeRecv(bt_status_t status, uint16_t packet_count)
{
    LOGD("%s: status:%d packet_count:%d ", __func__, status, packet_count);
}

RBluetoothAdapterHw* RBluetoothAdapterHw::instance()
{
    if (mDevice == NULL)
        mDevice = new RBluetoothAdapterHw();

    return mDevice;
}

void RBluetoothAdapterHw::release()
{
    if (mDevice != NULL)
        delete mDevice;

    mDevice = NULL;
}

RBluetoothAdapterHw::RBluetoothAdapterHw()
{
    mBtModule = NULL;
    mBtDevice = NULL;

    mBtInterface = NULL;
    mBtSocketInterface = NULL;

    mCallbacks = NULL;

    loadBluetoothModule();
    initBluetoothCallbacks();
}

RBluetoothAdapterHw::~RBluetoothAdapterHw()
{
    unloadBluetoothModule();
}

void RBluetoothAdapterHw::loadBluetoothModule()
{
    char value[PROPERTY_VALUE_MAX] = {0};
    property_get("bluetooth.mock_stack", value, "");

    const char *id = (strcmp(value, "1")? BT_STACK_MODULE_ID : BT_STACK_TEST_MODULE_ID);

    int err = hw_get_module(id, (hw_module_t const**)&mBtModule);
    if (err == 0)
    {
        err = mBtModule->methods->open(mBtModule, id, &mBtDevice);
        if (err == 0)
        {
            //define 'typedef bluetooth_device_t bluetooth_module_t;' in bluetooth.h
            bluetooth_module_t* btStack = (bluetooth_module_t *)mBtDevice;
            mBtInterface = btStack->get_bluetooth_interface();
        }
        else
            LOGE("Error while opening Bluetooth library");
    }
    else
        LOGE("No Bluetooth Library found");
}

void RBluetoothAdapterHw::unloadBluetoothModule()
{
    /* FIXME:
     * Commenting this out as for some reason,
     * the application does not exit otherwise.
     * call close block
     */
    if (mBtDevice != NULL)
        mBtDevice->close(mBtDevice);

    mBtDevice = NULL;
    mBtModule = NULL;
    mBtInterface = NULL;
    mBtSocketInterface = NULL;
}

const bt_interface_t* RBluetoothAdapterHw::getBluetoothInterface()
{
    return mBtInterface;
}

void RBluetoothAdapterHw::setBluetoothCallbacks(RBluetoothCallbacks* callbacks)
{
    mCallbacks = callbacks;
}

void RBluetoothAdapterHw::initBluetoothCallbacks()
{
    bt_callbacks_t tmp_callbacks = {
        sizeof(tmp_callbacks),
        onAdapterStateChanged,
        onAdapterProperties,
        onRemoteDeviceProperties,
        onDeviceFound,
        onDiscoveryStateChanged,
        onPinRequest,
        onSspRequest,
        onBondStateChanged,
        onAclStateChanged,
        onThreadEvent,
        onDutModeRecv,
        onLeTestModeRecv
    };

    mBtCallbacks = tmp_callbacks;
}

bool RBluetoothAdapterHw::init()
{
    if (mBtInterface)
    {
        int ret = mBtInterface->init(&mBtCallbacks);
        if (ret != BT_STATUS_SUCCESS)
        {
            LOGE("Error while setting the callbacks: %d \n", ret);
            mBtInterface = NULL;
            return false;
        }

        mBtSocketInterface =
            (btsock_interface_t*)mBtInterface->get_profile_interface(BT_PROFILE_SOCKETS_ID);
        if (mBtSocketInterface == NULL)
            LOGE("Error getting socket interface");

        return true;
    }

    return false;
}

bool RBluetoothAdapterHw::cleanup()
{
    bool result = false;
    if (!mBtInterface) return result;

    mBtInterface->cleanup();
    LOGI("%s: return from cleanup",__FUNCTION__);

    return true;
}

bool RBluetoothAdapterHw::enable()
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->enable();
    LOGD("enable: %d", ret);

    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::disable()
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->disable();
    /* Retrun false only when BTIF explicitly reports
     * BT_STATUS_FAIL. It is fine for the BT_STATUS_NOT_READY
     * case which indicates that stack had not been enabled.
     */
    result = (ret == BT_STATUS_FAIL) ? false : true;
    return result;
}

bool RBluetoothAdapterHw::startDiscovery()
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->start_discovery();
    result = (ret == BT_STATUS_SUCCESS) ? true : false;
    return result;
}

bool RBluetoothAdapterHw::cancelDiscovery()
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->cancel_discovery();
    result = (ret == BT_STATUS_SUCCESS) ? true : false;
    return result;
}

bool RBluetoothAdapterHw::createBond(RBluetoothAddr& addr)
{
    bool result = false;

    if (!mBtInterface) return result;

    int ret = mBtInterface->create_bond((bt_bdaddr_t *)addr.addr);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::removeBond(RBluetoothAddr& addr)
{
    bool result;
    if (!mBtInterface) return false;

    int ret = mBtInterface->remove_bond((bt_bdaddr_t *)addr.addr);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::cancelBond(RBluetoothAddr& addr)
{
    bool result;
    if (!mBtInterface) return false;

    int ret = mBtInterface->cancel_bond((bt_bdaddr_t *)addr.addr);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::pinReply(RBluetoothAddr& addr,
        bool accept, int len, RBluetoothPinCode& pin)
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->pin_reply(
            (bt_bdaddr_t*)addr.addr, accept, len,
            (bt_pin_code_t *)pin.pin);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::sspReply(RBluetoothAddr& addr,
        int type, bool accept, uint32_t passkey)
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->ssp_reply((bt_bdaddr_t *)addr.addr,
         (bt_ssp_variant_t) type, accept, passkey);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::setAdapterProperty(int type, void* value, int len)
{
    bool result = false;
    if (!mBtInterface) return result;

    bt_property_t prop;
    prop.type = (bt_property_type_t)type;
    prop.len = len;
    prop.val = value;

    int ret = mBtInterface->set_adapter_property(&prop);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::getAdapterProperties()
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->get_adapter_properties();
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::getAdapterProperty(int type)
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->get_adapter_property((bt_property_type_t)type);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::getDeviceProperty(RBluetoothAddr& addr, int type)
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->get_remote_device_property(
            (bt_bdaddr_t *)addr.addr,
            (bt_property_type_t)type);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::setDeviceProperty(RBluetoothAddr& addr, int type,
        void *value, int len)
{
    bool result = false;
    if (!mBtInterface) return result;

    bt_property_t prop;
    prop.type = (bt_property_type_t)type;
    prop.len = len;
    prop.val = value;

    int ret = mBtInterface->set_remote_device_property((bt_bdaddr_t *)addr.addr, &prop);

    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}

bool RBluetoothAdapterHw::getRemoteServices(RBluetoothAddr& addr)
{
    bool result = false;
    if (!mBtInterface) return result;

    int ret = mBtInterface->get_remote_services((bt_bdaddr_t *)addr.addr);
    result = (ret == BT_STATUS_SUCCESS) ? true : false;
    return result;
}

int RBluetoothAdapterHw::connectSocket(
        RBluetoothAddr& addr, int type,
        const uint8_t *uuid, int channel, int flag)
{
    int socket_fd;
    bt_status_t status;

    if (!mBtSocketInterface) return -1;

    status = mBtSocketInterface->connect(
            (bt_bdaddr_t *)addr.addr,
            (btsock_type_t)type,
            (const uint8_t*)uuid,
            channel, &socket_fd, flag);

    if (status != BT_STATUS_SUCCESS)
    {
        LOGE("Socket connection failed: %d", status);
        goto Fail;
    }

    if (socket_fd < 0)
    {
        LOGE("Fail to creat file descriptor on socket fd");
        goto Fail;
    }

    return socket_fd;

Fail:
    return -1;
}

int RBluetoothAdapterHw::createSocketChannel(
        int type, const char* service_name,
        const uint8_t *uuid, int channel, int flag)
{
    int socket_fd;
    bt_status_t status;

    if (!mBtSocketInterface) return -1;

    LOGE("SOCK FLAG = %x ***********************",flag);
    status = mBtSocketInterface->listen(
            (btsock_type_t) type, service_name,
            uuid, channel, &socket_fd, flag);
    if (status != BT_STATUS_SUCCESS)
    {
        LOGE("Socket listen failed: %d", status);
        goto Fail;
    }

    if (socket_fd < 0)
    {
        LOGE("Fail to creat file descriptor on socket fd");
        goto Fail;
    }

    return socket_fd;

Fail:

    return -1;
}

bool RBluetoothAdapterHw::configHciSnoopLog(bool enable)
{
    bool result = false;

    if (!mBtInterface) return result;

    int ret = mBtInterface->config_hci_snoop_log(enable);

    result = (ret == BT_STATUS_SUCCESS) ? true : false;

    return result;
}
