LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	RWifiHalCmd.cpp \
	RNetworkUtils.cpp \
	RWifiInfo.cpp \
	RWifiSsidPasswd.cpp \
	RDhcpResults.cpp \
	RNetdConnector.cpp \
	RNetworkManagement.cpp \
	RWifiManager.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils

# wifi
LOCAL_SHARED_LIBRARIES += \
	libnetutils \
	libhardware_legacy

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../include/wifi

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := libfactory_wifi

include $(BUILD_SHARED_LIBRARY)

#################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	test.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libfactory_wifi

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../include

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE := test_wifi

include $(BUILD_EXECUTABLE)

