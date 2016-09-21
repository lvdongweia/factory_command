/*
 * Update ITouchPanelActivity.aidl if you change this file.
 * In particular, the ordering of the methods below must match and ITouchPanelActivity.aidl
 */

#ifndef __ITOUCH_PANEL_ACTIVITY_H_
#define __ITOUCH_PANEL_ACTIVITY_H_

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

using namespace android;

class ITouchPanelActivity : public IInterface
{
public:
    DECLARE_META_INTERFACE(TouchPanelActivity);

    virtual int touch(int num, const int* points) = 0;
};

class BnTouchPanelActivity : public BnInterface<ITouchPanelActivity>
{
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

#endif
