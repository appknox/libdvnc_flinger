LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
# ifeq ($(PLATFORM_SDK_VERSION),28)
LOCAL_SRC_FILES = src/flinger_secure.cpp src/ScreenFrame.cpp
# else
# LOCAL_SRC_FILES = src/flinger_app.cpp src/ScreenFrame.cpp
# endif

# LOCAL_SRC_FILES = src/flinger_app.cpp src/ScreenFrame.cpp

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
# include $(BUILD_EXECUTABLE)
