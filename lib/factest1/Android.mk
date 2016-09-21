LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	test_battery.cpp \
	test_mic.cpp \
	test_robot_info.cpp \
	test_sensors.cpp

LOCAL_SHARED_LIBRARIES := \
	libfacutil \
	libsysinfo \
	libnlu_version \
	libutils \
	libcutils \
	libgui \
	libbinder \
	libandroid \
	libmedia

LOCAL_C_INCLUDES := \
	external/robot-control/include \
	external/robot-canbus/include \
	external/robot-manager/rm_factory_test/include \
	frameworks/native/include \
	frameworks/av/include \
	frameworks/robot/include 


LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := libfactest1

include $(BUILD_SHARED_LIBRARY)
################################################################


include $(CLEAR_VARS)

LOCAL_SDK_VERSION := 14
LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_SRC_FILES := ai_ver.cpp
LOCAL_SHARED_LIBRARIES := librobot_nlu_jni

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := libnlu_version
LOCAL_CFLAGS := -O2 -Wall -g

include $(BUILD_SHARED_LIBRARY)


