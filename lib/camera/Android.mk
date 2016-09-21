LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	ICameraActivity.cpp \
	ILcdActivity.cpp \
	ITouchPanelActivity.cpp \
	IActivityTestService.cpp \
	ActivityTestService.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../include/camera

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := libfactory_activity

include $(BUILD_SHARED_LIBRARY)

#################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	test.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libfactory_activity

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := test_activity

include $(BUILD_EXECUTABLE)

