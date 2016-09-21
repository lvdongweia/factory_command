#ifndef __RMESSAGE_H_
#define __RMESSAGE_H_

#include <hardware/bluetooth.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

class Properties
{
public:
    int mNum;
    bt_property_t mValues[15];

    Properties();
    ~Properties();

    void copyProps(int num, bt_property_t* props);
    void releaseProps();
    Properties& operator=(const Properties& right);
};

inline Properties::Properties()
{
    mNum = 0;

    for (int i = 0; i < 15; i++)
    {
        mValues[i].len = 0;
        mValues[i].val = NULL;
    }
}

inline Properties::~Properties()
{
    releaseProps();
}

inline void Properties::copyProps(int num, bt_property_t* props)
{
    if (mNum != 0)
        releaseProps();

    mNum = num;
    for (int i = 0; i < num; i++)
    {
        mValues[i].val = malloc(props[i].len);
        mValues[i].type = props[i].type;
        mValues[i].len = props[i].len;
        memcpy(mValues[i].val, props[i].val, props[i].len);
    }
}

inline void Properties::releaseProps()
{
    if (mNum == 0) return;

    for (int i = 0; i < mNum; i++)
    {
        free(mValues[i].val);
        mValues[i].val = NULL;
        mValues[i].len = 0;
    }

    mNum = 0;
}

inline Properties& Properties::operator=(const Properties& right)
{
    mNum = right.mNum;
    for (int i  = 0; i < mNum; i++)
    {
        mValues[i].type = right.mValues[i].type;
        mValues[i].len = right.mValues[i].len;
        mValues[i].val = right.mValues[i].val;
    }

    return (*this);
}

enum RMessageId
{
    ADAPTER_STATE_CHANGED = 0,
    ADAPTER_PROPERTIES,
    REMOTE_DEVICE_PROPERTIES,
    DEVICE_FOUND,
    DISCOVERY_STATE_CHANGED,
    PIN_REQUEST,
    SSP_REQUEST,
    BOND_STATE_CHANGED,
    ACL_STATE_CHANGED,
    MSG_MAX_ID,
};

//TODO: with constructor not allowed in union
struct RMessageData
{
    int state;
    bt_bdaddr_t addr;

    Properties props;

    struct remote_device
    {
        bt_bdaddr_t addr;
        Properties props;
    }remote;

    struct pin_request
    {
        bt_bdaddr_t addr;
        bt_bdname_t name;
        uint32_t cod;
    }pin;

    struct ssp_request
    {
        bt_bdaddr_t addr;
        bt_bdname_t name;
        uint32_t cod;
        bt_ssp_variant_t variant;
        uint32_t key;
    }ssp;

    struct bond_state
    {
        bt_status_t status;
        bt_bdaddr_t addr;
        bt_bond_state_t state;
    }bond_state;

    struct acl_state
    {
        bt_status_t status;
        bt_bdaddr_t addr;
        bt_acl_state_t state;
    }acl_state;
};

struct RMessage
{
    RMessageId id;
    RMessageData data;
};

#endif
