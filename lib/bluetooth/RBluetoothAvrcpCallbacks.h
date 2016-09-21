#ifndef __RBLUETOOTH_AVRCP_CALLBACKS_H_
#define __RBLUETOOTH_AVRCP_CALLBACKS_H_

#include "RBluetoothCommon.h"
#include <hardware/bt_rc.h>

class RBluetoothAvrcpCallbacks
{
public:
    virtual ~RBluetoothAvrcpCallbacks() {}

    virtual void onRemoteFeatures(bt_bdaddr_t* bd_addr, btrc_remote_features_t features) = 0;
    virtual void onGetPlayStatus() = 0;
    virtual void onGetElementAttr(uint8_t num_attr, btrc_media_attr_t *p_attrs) = 0;
    virtual void onRegisterNotification(btrc_event_id_t event_id, uint32_t param) = 0;
    virtual void onVolumeChange(uint8_t volume, uint8_t ctype) = 0;
    virtual void onPassthroughCommand(int id, int pressed) = 0;
};

#endif
