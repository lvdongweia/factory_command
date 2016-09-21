#ifndef __RBLUETOOTH_HFP_CALLBACKS_H_
#define __RBLUETOOTH_HFP_CALLBACKS_H_

#include "RBluetoothCommon.h"
#include <hardware/bt_hf.h>

class RBluetoothHfpCallbacks
{
public:
    virtual ~RBluetoothHfpCallbacks();

    virtual void onConnectionState(bthf_connection_state_t state, bt_bdaddr_t* bd_addr) = 0;
    virtual void onAudioState(bthf_audio_state_t state, bt_bdaddr_t* bd_addr) = 0;
    virtual void onVoiceRecognition(bthf_vr_state_t state) = 0;
    virtual void onAnswerCall() = 0;
    virtual void onHangupCall() = 0;
    virtual void onVolumeControl(bthf_volume_type_t type, int volume) = 0;
    virtual void onDialCall(char *number) = 0;
    virtual void onDtmfCmd(char dtmf) = 0;
    virtual void onNoiceReduction(bthf_nrec_t nrec) = 0;
    virtual void onAtChld(bthf_chld_type_t chld) = 0;
    virtual void onAtCnum() = 0;
    virtual void onAtCind() = 0;
    virtual void onAtCops() = 0;
    virtual void onAtClcc() = 0;
    virtual void onUnknownAt(char *at_string) = 0;
    virtual void onKeyPressed() = 0;
};

#endif
