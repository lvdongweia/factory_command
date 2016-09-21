/*
 * Handsfree Interface: BT_PROFILE_HANDSFREE_ID
 * hfp: handsfree profile
 */
#ifndef __RBLUETOOTH_HANDSFREE_H_
#define __RBLUETOOTH_HANDSFREE_H_

#include "RBluetoothCommon.h"
#include <hardware/bt_hf.h>

class RBluetoothHfpCallbacks;
class RBluetoothHandsfree
{
public:
    static RBluetoothHandsfree* instance();
    static void release();

    void setCallbacks(RBluetoothHfpCallbacks* callbacks);

    void initialize();
    void cleanup();

    bool connectHfp(RBluetoothAddr& addr);
    bool disconnectHfp(RBluetoothAddr& addr);

    bool connectAudio(RBluetoothAddr& addr);
    bool disconnectAudio(RBluetoothAddr& addr);

    bool startVoiceRecognition();
    bool stopVoiceRecognition();

    bool setVolume(int volume_type, int volume);

    bool notifyDeviceStatus(int network_state, int service_type,
            int signal, int battery_charge);

    bool copsResponse(const char* operator_name);

    bool cindResponse(int service, int num_active, int num_held,
            int call_state, int signal, int roam, int battery_charge);

    bool atResponseString(const char* response);
    bool atResponseCode(int response_code, int cmee_code);

    bool clccResponse(int index, int dir, int callStatus,
            int mode, bool mpty, const char* number, int type);

    bool phoneStateChange(int num_active, int num_held,
            int call_state, const char* number, int type);

private:
    RBluetoothHandsfree();
    ~RBluetoothHandsfree();

    void initCallbacks();

    //BT-HF callback
    static void onConnectionState(bthf_connection_state_t state, bt_bdaddr_t* bd_addr);
    static void onAudioState(bthf_audio_state_t state, bt_bdaddr_t* bd_addr);
    static void onVoiceRecognition(bthf_vr_state_t state);
    static void onAnswerCall();
    static void onHangupCall();
    static void onVolumeControl(bthf_volume_type_t type, int volume);
    static void onDialCall(char *number);
    static void onDtmfCmd(char dtmf);
    static void onNoiceReduction(bthf_nrec_t nrec);
    static void onAtChld(bthf_chld_type_t chld);
    static void onAtCnum();
    static void onAtCind();
    static void onAtCops();
    static void onAtClcc();
    static void onUnknownAt(char *at_string);
    static void onKeyPressed();

    bthf_callbacks_t mBthfCallbacks;
    const bthf_interface_t *mBtHfpInterface;

    RBluetoothHfpCallbacks* mCallbacks;

    static RBluetoothHandsfree* mHfp;
};

#endif
