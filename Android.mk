LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	factory_test_main.cpp \
	serial.cpp \
	transport.cpp

LOCAL_C_INCLUDES := 


LOCAL_SHARED_LIBRARIES := \
	libcutils

LOCAL_MODULE := rm_factory_test

include $(BUILD_EXECUTABLE)










