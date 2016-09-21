/*
 * copy from frameworks/base/libs/storage/IMountService.cpp
 */

#define LOG_TAG "IMountService"

#include "IMountService.h"
#include <binder/Parcel.h>

namespace android {

enum {
    TRANSACTION_registerListener = IBinder::FIRST_CALL_TRANSACTION,
    TRANSACTION_unregisterListener,
    TRANSACTION_isUsbMassStorageConnected,
    TRANSACTION_setUsbMassStorageEnabled,
    TRANSACTION_isUsbMassStorageEnabled,
    TRANSACTION_mountVolume,
    TRANSACTION_unmountVolume,
    TRANSACTION_formatVolume,
    TRANSACTION_getStorageUsers,
    TRANSACTION_getVolumeState,
    TRANSACTION_createSecureContainer,
    TRANSACTION_finalizeSecureContainer,
    TRANSACTION_destroySecureContainer,
    TRANSACTION_mountSecureContainer,
    TRANSACTION_unmountSecureContainer,
    TRANSACTION_isSecureContainerMounted,
    TRANSACTION_renameSecureContainer,
    TRANSACTION_getSecureContainerPath,
    TRANSACTION_getSecureContainerList,
    TRANSACTION_shutdown,
    TRANSACTION_finishMediaUpdate,
    TRANSACTION_mountObb,
    TRANSACTION_unmountObb,
    TRANSACTION_isObbMounted,
    TRANSACTION_getMountedObbPath,
    TRANSACTION_isExternalStorageEmulated,
    TRANSACTION_decryptStorage,
    TRANSACTION_encryptStorage,
};

class BpMountService: public BpInterface<IMountService>
{
public:
    BpMountService(const sp<IBinder>& impl)
        : BpInterface<IMountService>(impl)
    {
    }

    String16 getVolumeState(const String16& mountPoint)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IMountService::getInterfaceDescriptor());
        data.writeString16(mountPoint);
        if (remote()->transact(TRANSACTION_getVolumeState, data, &reply) != NO_ERROR) {
            ALOGD("getVolumeState could not contact remote\n");
            return String16("unknown");
        }
        int32_t err = reply.readExceptionCode();
        if (err < 0) {
            ALOGD("getVolumeState caught exception %d\n", err);
            return String16("unknown");;
        }
        return reply.readString16();
    }
};

IMPLEMENT_META_INTERFACE(MountService, "IMountService");

// ----------------------------------------------------------------------

};
