#define LOG_TAG "ITouchPanelActivity"

#include "ITouchPanelActivity.h"
#include "factory_log.h"

enum {
    TOUCH = IBinder::FIRST_CALL_TRANSACTION,
};

class BpTouchPanelActivity : public BpInterface<ITouchPanelActivity>
{
public:
    BpTouchPanelActivity(const sp<IBinder>& impl)
        : BpInterface<ITouchPanelActivity>(impl)
    {
    }

    virtual int touch(int num, const int* points)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ITouchPanelActivity::getInterfaceDescriptor());

        data.writeInt32(num);
        data.writeInt32Array(2 * num, points);

        int32_t err;
        if ((err = remote()->transact(TOUCH, data, &reply)) != NO_ERROR)
        {
            LOGE("touch could not contact remote: %d", err);
            return -1;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("touch caught exception: %d", err);
            return -1;
        }

        return reply.readInt32();
    }
};

//Note: DESCRIPTOR keep in sync with ITouchPanelActivity.java,
//generating via ITouchPanelActivity.aidl
IMPLEMENT_META_INTERFACE(TouchPanelActivity, "com.avatarmind.factorycommand.service.ITouchPanelActivity");

status_t BnTouchPanelActivity::onTransact(uint32_t code, const Parcel& data,
        Parcel* reply, uint32_t flags)
{
    switch (code)
    {
        case TOUCH:
        {
            CHECK_INTERFACE(ITouchPanelActivity, data, reply);

            int num = data.readInt32();
            int len = data.readInt32();

            const int32_t* points =
                (const int32_t*)data.readInplace(len * sizeof(int32_t));

            reply->writeNoException();
            reply->writeInt32(touch(num, points));
            return NO_ERROR;
        } break;

        default:
        return BBinder::onTransact(code, data, reply, flags);
    }
}
