LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	RBluetoothAdapterHw.cpp \
	RBluetoothHandsfree.cpp \
	RBluetoothAvrcp.cpp \
	RAdapterProperties.cpp \
	RBluetoothListener.cpp \
	RBluetoothManager.cpp \
	RRemoteDevices.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils

# bluetooth
LOCAL_SHARED_LIBRARIES += \
	libhardware

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../include/bluetooth

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := libfactory_bt

include $(BUILD_SHARED_LIBRARY)

#################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	test.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libfactory_bt

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := test_bt

include $(BUILD_EXECUTABLE)

