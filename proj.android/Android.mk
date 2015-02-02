LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := AssetBundleParser
LOCAL_CFLAGS := -Wall 
LOCAL_CFLAGS += -std=c99
LOCAL_CFLAGS += -Wno-psabi
LOCAL_EXPORT_CFLAGS += -Wno-psabi

LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/../AssetBundle/*.c)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../AssetBundle/utils/*.c)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

$(info LOCAL_PATH= $(LOCAL_PATH))
$(info LOCAL_SRC_FILES= $(LOCAL_SRC_FILES))

include $(BUILD_SHARED_LIBRARY)
