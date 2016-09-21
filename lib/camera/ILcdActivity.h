/*
 * Update ILcdActivity.aidl if you change this file.
 * In particular, the ordering of the methods below must match and ILcdActivity.aidl
 */

#ifndef __ILCD_ACTIVITY_H_
#define __ILCD_ACTIVITY_H_

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

using namespace android;

class ILcdActivity : public IInterface
{
public:
    DECLARE_META_INTERFACE(LcdActivity);

    virtual int display(int pattern) = 0;
};

//BnLcdActivity implement in APP
class BnLcdActivity : public BnInterface<ILcdActivity>
{
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

#endif
