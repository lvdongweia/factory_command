LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	fac_util.cpp \
	fac_can.cpp

LOCAL_C_INCLUDES := \
	external/robot-canbus/include \
	$(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := \
	librm_can \
	libutils \
	liblog

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := libfacutil
include $(BUILD_SHARED_LIBRARY)


##########################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	factory_test_main.cpp \
	serial.cpp \
	transport.cpp \

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libfacutil \
	libfactest1 \
	libfactory_hwinfo \
	libfactory_wifi \
	librobot_fac \
	libfactory_bt \
	libfactory_activity

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/lib/include \
	$(LOCAL_PATH)/lib/factest1 

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := rm_factory_test

include $(BUILD_EXECUTABLE)


##########################################
include $(call all-makefiles-under, $(LOCAL_PATH))
