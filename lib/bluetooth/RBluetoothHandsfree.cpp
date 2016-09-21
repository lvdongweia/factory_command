#define LOG_TAG "RBluetoothHandsfree"

#include "RBluetoothHandsfree.h"
#include "RBluetoothHfpCallbacks.h"
#include "RBluetoothAdapterHw.h"

#include "factory_log.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

RBluetoothHandsfree* RBluetoothHandsfree::mHfp = NULL;

void RBluetoothHandsfree::onConnectionState(bthf_connection_state_t state, bt_bdaddr_t* bd_addr)
{
    LOGD("%s, state: %d, addr: %s", __func__, state, (const char*)bd_addr);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onConnectionState(state, bd_addr);
}

void RBluetoothHandsfree::onAudioState(bthf_audio_state_t state, bt_bdaddr_t* bd_addr)
{
    LOGD("%s, state: %d, addr: %s", __func__, state, (const char*)bd_addr);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onAudioState(state, bd_addr);
}

void RBluetoothHandsfree::onVoiceRecognition(bthf_vr_state_t state)
{
    LOGD("%s, state: %d", __func__, state);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onVoiceRecognition(state);
}

void RBluetoothHandsfree::onAnswerCall()
{
    LOGD("%s", __func__);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onAnswerCall();
}

void RBluetoothHandsfree::onHangupCall()
{
    LOGD("%s", __func__);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onHangupCall();
}

void RBluetoothHandsfree::onVolumeControl(bthf_volume_type_t type, int volume)
{
    LOGD("%s, type: %d, valume: %d", __func__, type, volume);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onVolumeControl(type, volume);
}

void RBluetoothHandsfree::onDialCall(char *number)
{
    LOGD("%s, number: %s", __func__, number);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onDialCall(number);
}

void RBluetoothHandsfree::onDtmfCmd(char dtmf)
{
    LOGD("%s, dtmf: %d", __func__, dtmf);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onDtmfCmd(dtmf);
}

void RBluetoothHandsfree::onNoiceReduction(bthf_nrec_t nrec)
{
    LOGD("%s, reduction: %d", __func__, nrec);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onNoiceReduction(nrec);
}

void RBluetoothHandsfree::onAtChld(bthf_chld_type_t chld)
{
    LOGD("%s, type: %d", __func__, chld);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onAtChld(chld);
}

void RBluetoothHandsfree::onAtCnum()
{
    LOGD("%s", __func__);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onAtCnum();
}

void RBluetoothHandsfree::onAtCind()
{
    LOGD("%s", __func__);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onAtCind();
}

void RBluetoothHandsfree::onAtCops()
{
    LOGD("%s", __func__);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onAtCops();
}

void RBluetoothHandsfree::onAtClcc()
{
    LOGD("%s", __func__);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onAtClcc();
}

void RBluetoothHandsfree::onUnknownAt(char *at_string)
{
    LOGD("%s, str: %s", __func__, at_string);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onUnknownAt(at_string);
}

void RBluetoothHandsfree::onKeyPressed()
{
    LOGD("%s", __func__);

    if (mHfp->mCallbacks)
        mHfp->mCallbacks->onAtClcc();
}

RBluetoothHandsfree* RBluetoothHandsfree::instance()
{
    if (mHfp == NULL)
        mHfp = new RBluetoothHandsfree();

    return mHfp;
}

void RBluetoothHandsfree::release()
{
    if (mHfp)
        delete mHfp;

    mHfp = NULL;
}

RBluetoothHandsfree::RBluetoothHandsfree()
{
    mBtHfpInterface = NULL;
    mCallbacks = NULL;

    initCallbacks();
}

RBluetoothHandsfree::~RBluetoothHandsfree()
{
}

void RBluetoothHandsfree::setCallbacks(RBluetoothHfpCallbacks* callbacks)
{
    mCallbacks = callbacks;
}

void RBluetoothHandsfree::initCallbacks()
{
    bthf_callbacks_t tmp_callbacks = {
        sizeof(tmp_callbacks),
        onConnectionState,
        onAudioState,
        onVoiceRecognition,
        onAnswerCall,
        onHangupCall,
        onVolumeControl,
        onDialCall,
        onDtmfCmd,
        onNoiceReduction,
        onAtChld,
        onAtCnum,
        onAtCind,
        onAtCops,
        onAtClcc,
        onUnknownAt,
        onKeyPressed
    };

    mBthfCallbacks = tmp_callbacks;
}

void RBluetoothHandsfree::initialize()
{
    const bt_interface_t* btInf;
    bt_status_t status;

    if ( (btInf = RBluetoothAdapterHw::instance()->getBluetoothInterface()) == NULL)
    {
        LOGE("Bluetooth module is not loaded");
        return;
    }

    if (mBtHfpInterface != NULL)
    {
        LOGW("Cleaning up Bluetooth Handsfree Interface before initializing...");
        mBtHfpInterface->cleanup();
        mBtHfpInterface = NULL;
    }

    if ((mBtHfpInterface = (bthf_interface_t *)btInf->get_profile_interface(
                    BT_PROFILE_HANDSFREE_ID)) == NULL)
    {
        LOGE("Failed to get Bluetooth Handsfree Interface");
        return;
    }

    if ((status = mBtHfpInterface->init(&mBthfCallbacks)) != BT_STATUS_SUCCESS)
    {
        LOGE("Failed to initialize Bluetooth HFP, status: %d", status);
        mBtHfpInterface = NULL;
        return;
    }
}

void RBluetoothHandsfree::cleanup()
{
    const bt_interface_t* btInf;
    bt_status_t status;

    if ((btInf = RBluetoothAdapterHw::instance()->getBluetoothInterface()) == NULL)
    {
        LOGE("Bluetooth module is not loaded");
        return;
    }

    if (mBtHfpInterface != NULL)
    {
        LOGW("Cleaning up Bluetooth Handsfree Interface...");
        mBtHfpInterface->cleanup();
        mBtHfpInterface = NULL;
    }
}

bool RBluetoothHandsfree::connectHfp(RBluetoothAddr& addr)
{
    bt_status_t status;

    LOGI("%s: BluetoothHfpInterface: %p", __func__, mBtHfpInterface);
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->connect((bt_bdaddr_t*)addr.addr)) != BT_STATUS_SUCCESS)
        LOGE("Failed HF connection, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::disconnectHfp(RBluetoothAddr& addr)
{
    bt_status_t status;

    if (!mBtHfpInterface) return false;

    if ( (status = mBtHfpInterface->disconnect((bt_bdaddr_t *)addr.addr)) != BT_STATUS_SUCCESS)
        LOGE("Failed HF disconnection, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::connectAudio(RBluetoothAddr& addr)
{
    bt_status_t status;

    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->connect_audio((bt_bdaddr_t *)addr.addr)) != BT_STATUS_SUCCESS)
        LOGE("Failed HF audio connection, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::disconnectAudio(RBluetoothAddr& addr)
{
    bt_status_t status;

    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->disconnect_audio((bt_bdaddr_t*)addr.addr)) != BT_STATUS_SUCCESS)
        LOGE("Failed HF audio disconnection, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::startVoiceRecognition()
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->start_voice_recognition()) != BT_STATUS_SUCCESS)
        LOGE("Failed to start voice recognition, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::stopVoiceRecognition()
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->stop_voice_recognition()) != BT_STATUS_SUCCESS)
        LOGE("Failed to stop voice recognition, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::setVolume(int volume_type, int volume)
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->volume_control((bthf_volume_type_t)volume_type,
                    volume)) != BT_STATUS_SUCCESS)
        LOGE("FAILED to control volume, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::notifyDeviceStatus(
        int network_state, int service_type,
        int signal, int battery_charge)
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->device_status_notification(
                    (bthf_network_state_t)network_state,
                    (bthf_service_type_t)service_type,
                    signal, battery_charge)) != BT_STATUS_SUCCESS)
    {
        LOGE("FAILED to notify device status, status: %d", status);
    }

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::copsResponse(const char* operator_name)
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->cops_response(operator_name)) != BT_STATUS_SUCCESS)
        LOGE("Failed sending cops response, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::cindResponse(
        int service, int num_active, int num_held, int call_state,
        int signal, int roam, int battery_charge)
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->cind_response(
                    service, num_active, num_held,
                    (bthf_call_state_t) call_state,
                    signal, roam, battery_charge)) != BT_STATUS_SUCCESS)
    {
        LOGE("Failed cind_response, status: %d", status);
    }

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::atResponseString(const char* response)
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->formatted_at_response(response)) != BT_STATUS_SUCCESS)
        LOGE("Failed formatted AT response, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::atResponseCode(int response_code, int cmee_code)
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->at_response((bthf_at_response_t)response_code, cmee_code))
            != BT_STATUS_SUCCESS)
        LOGE("Failed AT response, status: %d", status);
    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::clccResponse(
        int index, int dir, int callStatus,
        int mode, bool mpty, const char* number, int type)
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->clcc_response(
                    index, (bthf_call_direction_t)dir,
                    (bthf_call_state_t) callStatus, (bthf_call_mode_t)mode,
                    mpty ? BTHF_CALL_MPTY_TYPE_MULTI : BTHF_CALL_MPTY_TYPE_SINGLE,
                    number, (bthf_call_addrtype_t) type)) != BT_STATUS_SUCCESS)
    {
        LOGE("Failed sending CLCC response, status: %d", status);
    }

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothHandsfree::phoneStateChange(
        int num_active, int num_held,
        int call_state, const char* number, int type)
{
    bt_status_t status;
    if (!mBtHfpInterface) return false;

    if ((status = mBtHfpInterface->phone_state_change(
                    num_active, num_held,
                    (bthf_call_state_t)call_state, number,
                    (bthf_call_addrtype_t) type)) != BT_STATUS_SUCCESS)
    {
        LOGE("Failed report phone state change, status: %d", status);
    }

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

