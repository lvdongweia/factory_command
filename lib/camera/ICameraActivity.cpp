#define LOG_TAG "ICameraActivity"

#include "ICameraActivity.h"
#include "factory_log.h"

enum {
    OPEN_CAMERA = IBinder::FIRST_CALL_TRANSACTION,
    CLOSE_CAMERA,
    TAKE_PICTURE,
    VIEW_PITCTURE,
    PREVIEW,
    COMPARE_IMAGE,
};

class BpCameraActivity : public BpInterface<ICameraActivity>
{
public:
    BpCameraActivity(const sp<IBinder>& impl)
        : BpInterface<ICameraActivity>(impl)
    {
    }

    virtual int openCamera()
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraActivity::getInterfaceDescriptor());

        int32_t err;
        if ((err = remote()->transact(OPEN_CAMERA, data, &reply)) != NO_ERROR)
        {
            LOGE("openCamera could not contact remote: %d", err);
            return -1;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("openCamera caught exception: %d", err);
            return -1;
        }

        return reply.readInt32();
    }

    virtual int closeCamera()
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraActivity::getInterfaceDescriptor());

        int32_t err;
        if ((err = remote()->transact(CLOSE_CAMERA, data, &reply)) != NO_ERROR)
        {
            LOGE("closeCamera could not contact remote: %d", err);
            return -1;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("closeCamera caught exception: %d", err);
            return -1;
        }

        return reply.readInt32();
    }

    virtual int takePicture(int storage, const String16& filename)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraActivity::getInterfaceDescriptor());

        data.writeInt32(storage);
        data.writeString16(filename);

        int32_t err;
        if ((err = remote()->transact(TAKE_PICTURE, data, &reply)) != NO_ERROR)
        {
            LOGE("takePicture could not contact remote: %d", err);
            return -1;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("takePicture caught exception: %d", err);
            return -1;
        }

        return reply.readInt32();
    }

    virtual int viewPicture(int storage, const String16& filename)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraActivity::getInterfaceDescriptor());

        data.writeInt32(storage);
        data.writeString16(filename);

        int32_t err;
        if ((err = remote()->transact(VIEW_PITCTURE, data, &reply)) != NO_ERROR)
        {
            LOGE("viewPicture could not contact remote: %d", err);
            return -1;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("viewPicture caught exception: %d", err);
            return -1;
        }

        return reply.readInt32();
    }

    virtual int preview()
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraActivity::getInterfaceDescriptor());

        int32_t err;
        if ((err = remote()->transact(PREVIEW, data, &reply)) != NO_ERROR)
        {
            LOGE("preview could not contact remote: %d", err);
            return -1;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("preview caught exception: %d", err);
            return -1;
        }

        return reply.readInt32();
    }

    virtual int compareImage(int pattern, int storage, const String16& filename)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICameraActivity::getInterfaceDescriptor());

        data.writeInt32(pattern);
        data.writeInt32(storage);
        data.writeString16(filename);

        int32_t err;
        if ((err = remote()->transact(COMPARE_IMAGE, data, &reply)) != NO_ERROR)
        {
            LOGE("compareImage could not contact remote: %d", err);
            return -1;
        }

        err = reply.readExceptionCode();
        if (err < 0)
        {
            LOGE("compareImage caught exception: %d", err);
            return -1;
        }

        return reply.readInt32();
    }
};

//Note: DESCRIPTOR keep in sync with ICameraActivity.java,
//generating via ICameraActivity.aidl
IMPLEMENT_META_INTERFACE(CameraActivity, "com.avatarmind.factorycommand.service.ICameraActivity");

status_t BnCameraActivity::onTransact(uint32_t code, const Parcel& data,
        Parcel* reply, uint32_t flags)
{
    switch (code)
    {
        case OPEN_CAMERA:
        {
            CHECK_INTERFACE(ICameraActivity, data, reply);

            reply->writeNoException();
            reply->writeInt32(openCamera());
            return NO_ERROR;
        } break;

        case CLOSE_CAMERA:
        {
            CHECK_INTERFACE(ICameraActivity, data, reply);

            reply->writeNoException();
            reply->writeInt32(closeCamera());
            return NO_ERROR;
        } break;

        case TAKE_PICTURE:
        {
            CHECK_INTERFACE(ICameraActivity, data, reply);

            int storage = data.readInt32();
            String16 filename = data.readString16();

            reply->writeNoException();
            reply->writeInt32(takePicture(storage, filename));
            return NO_ERROR;
        } break;

        case VIEW_PITCTURE:
        {
            CHECK_INTERFACE(ICameraActivity, data, reply);

            int storage = data.readInt32();
            String16 filename = data.readString16();

            reply->writeNoException();
            reply->writeInt32(viewPicture(storage, filename));
            return NO_ERROR;

        } break;

        case PREVIEW:
        {
            CHECK_INTERFACE(ICameraActivity, data, reply);

            reply->writeNoException();
            reply->writeInt32(preview());
            return NO_ERROR;
        } break;

        case COMPARE_IMAGE:
        {
            CHECK_INTERFACE(ICameraActivity, data, reply);

            int pattern = data.readInt32();
            int storage = data.readInt32();
            String16 filename = data.readString16();

            reply->writeNoException();
            reply->writeInt32(compareImage(pattern, storage, filename));
            return NO_ERROR;
        } break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}
