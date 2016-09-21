#define LOG_TAG "IActivityTestService"

#include "IActivityTestService.h"
#include "ILcdActivity.h"
#include "ICameraActivity.h"
#include "ITouchPanelActivity.h"

#include "factory_log.h"

enum {
    REGISTER_CAMERA_ACTIVITY = IBinder::FIRST_CALL_TRANSACTION,
    UNREGISTER_CAMERA_ACTIVITY,
    REGISTER_LCD_ACTIVITY,
    UNREGISTER_LCD_ACTIVITY,
    GET_TOUCH_PANEL_ACTIVITY,
};

class BpActivityTestService : public BpInterface<IActivityTestService>
{
public:
    BpActivityTestService(const sp<IBinder>& impl)
        : BpInterface<IActivityTestService>(impl)
    {
    }

    virtual void registerCameraActivity(int pid, const sp<ICameraActivity>& cameraActivity)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IActivityTestService::getInterfaceDescriptor());

        data.writeInt32(pid);
        data.writeStrongBinder(cameraActivity->asBinder());

        int32_t err;
        if ((err = remote()->transact(REGISTER_CAMERA_ACTIVITY, data, &reply)) != NO_ERROR)
        {
            LOGE("registerCameraActivity could not contact remote: %d", err);
            return;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("registerCameraActivity caught exception: %d", err);
            return;
        }
    }

    virtual void unregisterCameraActivity(const sp<ICameraActivity>& cameraActivity)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IActivityTestService::getInterfaceDescriptor());

        data.writeStrongBinder(cameraActivity->asBinder());

        int32_t err;
        if ((err = remote()->transact(UNREGISTER_CAMERA_ACTIVITY, data, &reply)) != NO_ERROR)
        {
            LOGE("unregisterCameraActivity could not contact remote: %d", err);
            return;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("unregisterCameraActivity caught exception: %d", err);
            return;
        }
    }

    virtual void registerLcdActivity(int pid, const sp<ILcdActivity>& lcdActivity)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IActivityTestService::getInterfaceDescriptor());

        data.writeInt32(pid);
        data.writeStrongBinder(lcdActivity->asBinder());

        int32_t err;
        if ((err = remote()->transact(REGISTER_LCD_ACTIVITY, data, &reply)) != NO_ERROR)
        {
            LOGE("registerLcdActivity could not contact remote: %d", err);
            return;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("registerLcdActivity caught exception: %d", err);
            return;
        }
    }

    virtual void unregisterLcdActivity(const sp<ILcdActivity>& lcdActivity)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IActivityTestService::getInterfaceDescriptor());

        data.writeStrongBinder(lcdActivity->asBinder());

        int32_t err;
        if ((err = remote()->transact(UNREGISTER_LCD_ACTIVITY, data, &reply)) != NO_ERROR)
        {
            LOGE("unregisterLcdActivity could not contact remote: %d", err);
            return;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("unregisterLcdActivity caught exception: %d", err);
            return;
        }
    }

    virtual sp<ITouchPanelActivity> getTouchPanelActivity()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IActivityTestService::getInterfaceDescriptor());

        int32_t err;
        if ((err = remote()->transact(GET_TOUCH_PANEL_ACTIVITY, data, &reply)) != NO_ERROR)
        {
            LOGE("getTouchPanelActivity could not contact remote: %d", err);
            return NULL;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("getTouchPanelActivity caught exception: %d", err);
            return NULL;
        }

        return interface_cast<ITouchPanelActivity>(reply.readStrongBinder());
    }
};

//Note: DESCRIPTOR keep in sync with IActivityTestService.java,
//generating via IActivityTestService.aidl
IMPLEMENT_META_INTERFACE(ActivityTestService, "com.avatarmind.factorycommand.service.IActivityTestService");

status_t BnActivityTestService::onTransact(uint32_t code, const Parcel& data,
        Parcel* reply, uint32_t flags)
{
    switch (code)
    {
        case REGISTER_CAMERA_ACTIVITY:
        {
            CHECK_INTERFACE(IActivityTestService, data, reply);

            int pid = data.readInt32();
            sp<ICameraActivity> cameraActivity =
                interface_cast<ICameraActivity>(data.readStrongBinder());

            registerCameraActivity(pid, cameraActivity);
            reply->writeNoException();
            return NO_ERROR;
        } break;

        case UNREGISTER_CAMERA_ACTIVITY:
        {
            CHECK_INTERFACE(IActivityTestService, data, reply);

            sp<ICameraActivity> cameraActivity =
                interface_cast<ICameraActivity>(data.readStrongBinder());

            unregisterCameraActivity(cameraActivity);
            reply->writeNoException();
            return NO_ERROR;
        } break;

        case REGISTER_LCD_ACTIVITY:
        {
            CHECK_INTERFACE(IActivityTestService, data, reply);

            int pid = data.readInt32();
            sp<ILcdActivity> lcdActivity =
                interface_cast<ILcdActivity>(data.readStrongBinder());

            registerLcdActivity(pid, lcdActivity);
            reply->writeNoException();
            return NO_ERROR;
        } break;

        case UNREGISTER_LCD_ACTIVITY:
        {
            CHECK_INTERFACE(IActivityTestService, data, reply);

            sp<ILcdActivity> lcdActivity =
                interface_cast<ILcdActivity>(data.readStrongBinder());

            unregisterLcdActivity(lcdActivity);
            reply->writeNoException();
            return NO_ERROR;
        } break;

        case GET_TOUCH_PANEL_ACTIVITY:
        {
            CHECK_INTERFACE(IActivityTestService, data, reply);

            sp<ITouchPanelActivity> touchActivity = getTouchPanelActivity();

            reply->writeNoException();
            reply->writeStrongBinder(touchActivity->asBinder());
            return NO_ERROR;
        } break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

