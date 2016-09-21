#define LOG_TAG "RBluetoothAvrcp"

#include "RBluetoothAvrcp.h"
#include "RBluetoothAdapterHw.h"
#include "RBluetoothAvrcpCallbacks.h"
#include "factory_log.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

RBluetoothAvrcp* RBluetoothAvrcp::mAvrcp = NULL;

void RBluetoothAvrcp::onRemoteFeatures(bt_bdaddr_t* bd_addr, btrc_remote_features_t features)
{
    LOGD("%s", __func__);

    if (mAvrcp->mCallbacks)
        mAvrcp->mCallbacks->onRemoteFeatures(bd_addr, features);
}

void RBluetoothAvrcp::onGetPlayStatus()
{
    LOGD("%s", __func__);

    if (mAvrcp->mCallbacks)
        mAvrcp->mCallbacks->onGetPlayStatus();
}

void RBluetoothAvrcp::onGetElementAttr(uint8_t num_attr, btrc_media_attr_t *p_attrs)
{
    LOGD("%s, attr num: %u", __func__, num_attr);

    if (mAvrcp->mCallbacks)
        mAvrcp->mCallbacks->onGetElementAttr(num_attr, p_attrs);
}

void RBluetoothAvrcp::onRegisterNotification(btrc_event_id_t event_id, uint32_t param)
{
    LOGD("%s, event: %d, param: %u", __func__, event_id, param);

    if (mAvrcp->mCallbacks)
        mAvrcp->mCallbacks->onRegisterNotification(event_id, param);
}

void RBluetoothAvrcp::onVolumeChange(uint8_t volume, uint8_t ctype)
{
    LOGD("%s, volume: %u, type: %u", __func__, volume, ctype);

    if (mAvrcp->mCallbacks)
        mAvrcp->mCallbacks->onVolumeChange(volume, ctype);
}

void RBluetoothAvrcp::onPassthroughCommand(int id, int pressed)
{
    LOGD("%s, id: %d, pressed: %d", __func__, id, pressed);

    if (mAvrcp->mCallbacks)
        mAvrcp->mCallbacks->onPassthroughCommand(id, pressed);
}

RBluetoothAvrcp* RBluetoothAvrcp::instance()
{
    if (mAvrcp == NULL)
        mAvrcp = new RBluetoothAvrcp();

    return mAvrcp;
}

void RBluetoothAvrcp::release()
{
    if (mAvrcp)
        delete mAvrcp;

    mAvrcp = NULL;
}

RBluetoothAvrcp::RBluetoothAvrcp()
{
    mBtAvrcpInterface = NULL;
    mCallbacks = NULL;

    initCallbacks();
}

RBluetoothAvrcp::~RBluetoothAvrcp()
{
}

void RBluetoothAvrcp::setCallbacks(RBluetoothAvrcpCallbacks* callbacks)
{
    mCallbacks = callbacks;
}

void RBluetoothAvrcp::initCallbacks()
{
    btrc_callbacks_t tmp_callbacks = {
        sizeof(tmp_callbacks),
        onRemoteFeatures,
        onGetPlayStatus,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        onGetElementAttr,
        onRegisterNotification,
        onVolumeChange,
        onPassthroughCommand
    };

    mBtAvrcpCallbacks = tmp_callbacks;
}

void RBluetoothAvrcp::init()
{
    const bt_interface_t* btInf;
    bt_status_t status;

    if ((btInf = RBluetoothAdapterHw::instance()->getBluetoothInterface()) == NULL)
    {
        LOGE("Bluetooth module is not loaded");
        return;
    }

    if (mBtAvrcpInterface !=NULL)
    {
         LOGW("Cleaning up Avrcp Interface before initializing...");
         mBtAvrcpInterface->cleanup();
         mBtAvrcpInterface = NULL;
    }

    if ((mBtAvrcpInterface = (btrc_interface_t *)
          btInf->get_profile_interface(BT_PROFILE_AV_RC_ID)) == NULL)
    {
        LOGE("Failed to get Bluetooth Avrcp Interface");
        return;
    }

    if ((status = mBtAvrcpInterface->init(&mBtAvrcpCallbacks)) !=
         BT_STATUS_SUCCESS)
    {
        LOGE("Failed to initialize Bluetooth Avrcp, status: %d", status);
        mBtAvrcpInterface = NULL;
        return;
    }
}

void RBluetoothAvrcp::cleanup()
{
    const bt_interface_t* btInf;

    if ((btInf = RBluetoothAdapterHw::instance()->getBluetoothInterface()) == NULL)
    {
        LOGE("Bluetooth module is not loaded");
        return;
    }

    if (mBtAvrcpInterface !=NULL)
    {
        mBtAvrcpInterface->cleanup();
        mBtAvrcpInterface = NULL;
    }
}

bool RBluetoothAvrcp::getPlayStatusRsp(int playStatus, int songLen, int songPos)
{
    bt_status_t status;

    if (!mBtAvrcpInterface) return false;

    if ((status = mBtAvrcpInterface->get_play_status_rsp((btrc_play_status_t)playStatus,
                                            songLen, songPos)) != BT_STATUS_SUCCESS)
        LOGE("Failed get_play_status_rsp, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothAvrcp::getElementAttrRsp(int numAttr, int* attrIds, const char* texts[])
{
    bt_status_t status;

    int i;
    btrc_element_attr_val_t *pAttrs = NULL;

    if (!mBtAvrcpInterface) return false;

    if (numAttr > BTRC_MAX_ELEM_ATTR_SIZE)
    {
        LOGE("get_element_attr_rsp: number of attributes exceed maximum");
        return false;
    }

    pAttrs = new btrc_element_attr_val_t[numAttr];
    if (!pAttrs)
    {
        LOGE("get_element_attr_rsp: not have enough memeory");
        return false;
    }

    for (i = 0; i < numAttr; ++i)
    {
        pAttrs[i].attr_id = attrIds[i];

        if (strlen(texts[i]) >= BTRC_MAX_ATTR_STR_LEN)
        {
            LOGE("get_element_attr_rsp: string length exceed maximum");
            strncpy((char *)pAttrs[i].text, texts[i], BTRC_MAX_ATTR_STR_LEN-1);
            pAttrs[i].text[BTRC_MAX_ATTR_STR_LEN-1] = 0;
        }
        else
            strcpy((char *)pAttrs[i].text, texts[i]);
    }

    if ((status = mBtAvrcpInterface->get_element_attr_rsp(numAttr, pAttrs)) != BT_STATUS_SUCCESS)
        LOGE("Failed get_element_attr_rsp, status: %d", status);

    delete[] pAttrs;

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothAvrcp::registerNotificationRspPlayStatus(int type, int playStatus)
{
    bt_status_t status;
    btrc_register_notification_t param;

    if (!mBtAvrcpInterface) return false;

    param.play_status = (btrc_play_status_t)playStatus;
    if ((status = mBtAvrcpInterface->register_notification_rsp(BTRC_EVT_PLAY_STATUS_CHANGED,
                    (btrc_notification_type_t)type, &param)) != BT_STATUS_SUCCESS)
        LOGE("Failed register_notification_rsp play status, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothAvrcp::registerNotificationRspTrackChange(int type, uint8_t* track)
{
    bt_status_t status;
    btrc_register_notification_t param;
    int i;

    if (!mBtAvrcpInterface) return false;

    for (i = 0; i < BTRC_UID_SIZE; ++i)
        param.track[i] = track[i];

    if ((status = mBtAvrcpInterface->register_notification_rsp(BTRC_EVT_TRACK_CHANGE,
                    (btrc_notification_type_t)type, &param)) != BT_STATUS_SUCCESS)
        LOGE("Failed register_notification_rsp track change, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothAvrcp::registerNotificationRspPlayPos(int type, int playPos)
{
    bt_status_t status;
    btrc_register_notification_t param;

    if (!mBtAvrcpInterface) return false;

    param.song_pos = (uint32_t)playPos;
    if ((status = mBtAvrcpInterface->register_notification_rsp(BTRC_EVT_PLAY_POS_CHANGED,
                  (btrc_notification_type_t)type, &param)) != BT_STATUS_SUCCESS)
        LOGE("Failed register_notification_rsp play position, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}

bool RBluetoothAvrcp::setVolume(int volume)
{
    bt_status_t status;

    if (!mBtAvrcpInterface) return false;

    if ((status = mBtAvrcpInterface->set_volume((uint8_t)volume)) != BT_STATUS_SUCCESS)
        LOGE("Failed set_volume, status: %d", status);

    return (status == BT_STATUS_SUCCESS) ? true : false;
}
