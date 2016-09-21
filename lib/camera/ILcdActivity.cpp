#define LOG_TAG "ILcdActivity"

#include "ILcdActivity.h"
#include "factory_log.h"

enum {
    DISPLAY = IBinder::FIRST_CALL_TRANSACTION,
};

class BpLcdActivity : public BpInterface<ILcdActivity>
{
public:
    BpLcdActivity(const sp<IBinder>& impl)
        : BpInterface<ILcdActivity>(impl)
    {
    }

    virtual int display(int pattern)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ILcdActivity::getInterfaceDescriptor());

        data.writeInt32(pattern);

        int32_t err;
        if ((err = remote()->transact(DISPLAY, data, &reply)) != NO_ERROR)
        {
            LOGE("display could not contact remote: %d", err);
            return -1;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("display caught exception: %d", err);
            return -1;
        }

        return reply.readInt32();
    }
};

//Note: DESCRIPTOR keep in sync with ILcdActivity.java,
//generating via ILcdActivity.aidl
IMPLEMENT_META_INTERFACE(LcdActivity, "com.avatarmind.factorycommand.service.ILcdActivity");

status_t BnLcdActivity::onTransact(uint32_t code, const Parcel& data,
        Parcel* reply, uint32_t flags)
{
    switch (code)
    {
        case DISPLAY:
        {
            CHECK_INTERFACE(ILcdActivity, data, reply);

            int pattern = data.readInt32();

            reply->writeNoException();
            reply->writeInt32(display(pattern));
            return NO_ERROR;
        } break;

        default:
        return BBinder::onTransact(code, data, reply, flags);
    }
}
