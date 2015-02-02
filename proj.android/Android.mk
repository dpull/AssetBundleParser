LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lua
LOCAL_CFLAGS := -Wall -DLUA_COMPAT_ALL -DLUA_USE_LINUX

LOCAL_CFLAGS += -Wno-psabi
LOCAL_EXPORT_CFLAGS += -Wno-psabi

LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/../src/*.c)
LOCAL_SRC_FILES += $(wildcard $(LOCAL_PATH)/../src/*.cpp)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES:$(LOCAL_PATH)/%=%)

LOCAL_SRC_FILES := $(subst  ../src/lua.c,,$(LOCAL_SRC_FILES)) 
LOCAL_SRC_FILES := $(subst  ../src/luac.c,,$(LOCAL_SRC_FILES)) 

$(info LOCAL_PATH= $(LOCAL_PATH))
$(info LOCAL_SRC_FILES= $(LOCAL_SRC_FILES))

include $(BUILD_SHARED_LIBRARY)

# LOCAL_ARM_MODE := arm
# LOCAL_MODULE_FILENAME := liblua
# LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include/lua
# LOCAL_LDFLAGS += -static
# LOCAL_STATIC_LIBRARIES += libc
# LOCAL_WHOLE_STATIC_LIBRARIES += libc
# LOCAL_LDLIBS := -lm  -Llibs/armeabi