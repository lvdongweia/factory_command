LOCAL_PATH := $(call my-dir)


# librobot_fac
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/include
LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE    := librobot_fac
LOCAL_SRC_FILES := \
		src/robot_fac.c
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := \
		$(LOCAL_PATH)/include
LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE    := librobot_fac
LOCAL_SRC_FILES := \
		src/robot_fac.c
LOCAL_SHARED_LIBRARIES := \
                libutils \
                liblog
include $(BUILD_SHARED_LIBRARY)


#####FACLIB_TEST
include $(CLEAR_VARS)
LOCAL_SRC_FILES := src/faclib_test.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(KERNEL_HEADERS)

LOCAL_SHARED_LIBRARIES := \
                libutils \
                liblog \
                librobot_fac \
		libc \
		libcutils \
		libbinder \
		libhardware

LOCAL_MODULE_TAGS := eng optional
LOCAL_MODULE = faclib_test
include $(BUILD_EXECUTABLE)


include $(call all-makefiles-under,$(LOCAL_PATH))
