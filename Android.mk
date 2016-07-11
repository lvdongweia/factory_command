LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	factory_test_main.cpp \
	fac_util.cpp \
	serial.cpp \
	transport.cpp \
	fw_version.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libsysinfo

LOCAL_C_INCLUDES := \
	external/robot-control/include


LOCAL_MODULE := rm_factory_test

include $(BUILD_EXECUTABLE)


####################################################
include $(CLEAR_VARS)

#LOCAL_SRC_FILES := \
#	serial.cpp \
#	transport.cpp \
#	transport_jni.cpp
#
#LOCAL_SHARED_LIBRARIES := \
#	libcutils \
#	libutils \
#	libandroid_runtime 
#
#LOCAL_CFLAGS := -DBUILD_FOR_JNI
#
#LOCAL_MODULE := libuart

#include $(BUILD_SHARED_LIBRARY)
