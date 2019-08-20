LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES = src/flinger.cpp

LOCAL_CFLAGS += -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libdvnc_flinger
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libui \
	liblog \

ifeq ($(PLATFORM_SDK_VERSION),$(filter $(PLATFORM_SDK_VERSION),10 9))
LOCAL_SHARED_LIBRARIES += libsurfaceflinger_client
else
LOCAL_SHARED_LIBRARIES += libgui
endif

include $(BUILD_SHARED_LIBRARY)
