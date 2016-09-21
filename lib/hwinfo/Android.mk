LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	RobotHwInfo.cpp \
	IMountService.cpp \
	RStorageManager.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../include

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := libfactory_hwinfo

include $(BUILD_SHARED_LIBRARY)

#################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	test.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libfactory_hwinfo

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := test_hwinfo

include $(BUILD_EXECUTABLE)

