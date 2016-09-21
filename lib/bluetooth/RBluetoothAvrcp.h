#ifndef __RBLUETOOTH_AVRCP_H_
#define __RBLUETOOTH_AVRCP_H_

#include "RBluetoothCommon.h"
#include <hardware/bt_rc.h>

class RBluetoothAvrcpCallbacks;
class RBluetoothAvrcp
{
public:
    static RBluetoothAvrcp* instance();
    static void release();

    void setCallbacks(RBluetoothAvrcpCallbacks* callbacks);

    void init();
    void cleanup();
    bool getPlayStatusRsp(int playStatus, int songLen, int songPos);
    bool getElementAttrRsp(int numAttr, int* attrIds, const char* texts[]);
    bool registerNotificationRspPlayStatus(int type, int playStatus);
    bool registerNotificationRspTrackChange(int type, uint8_t* track);
    bool registerNotificationRspPlayPos(int type, int playPos);
    bool setVolume(int volume);

private:
    RBluetoothAvrcp();
    ~RBluetoothAvrcp();

    void initCallbacks();

    static void onRemoteFeatures(bt_bdaddr_t* bd_addr, btrc_remote_features_t features);
    static void onGetPlayStatus();
    static void onGetElementAttr(uint8_t num_attr, btrc_media_attr_t *p_attrs);
    static void onRegisterNotification(btrc_event_id_t event_id, uint32_t param);
    static void onVolumeChange(uint8_t volume, uint8_t ctype);
    static void onPassthroughCommand(int id, int pressed);

    btrc_callbacks_t mBtAvrcpCallbacks;
    const btrc_interface_t *mBtAvrcpInterface;

    RBluetoothAvrcpCallbacks* mCallbacks;

    static RBluetoothAvrcp* mAvrcp;
};

#endif
