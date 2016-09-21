/*
 * copy from frameworks/base/include/storage/IMountService.h
 * Note: getVolumeState return type mismatch in IMountService.cpp and IMountService.java
 */

#ifndef ANDROID_IMOUNTSERVICE_H
#define ANDROID_IMOUNTSERVICE_H

#include <utils/String16.h>

#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

class IMountService: public IInterface {
public:
    DECLARE_META_INTERFACE(MountService);

    virtual String16 getVolumeState(const String16& mountPoint) = 0;
};


}
; // namespace android

#endif // ANDROID_IMOUNTSERVICE_H
