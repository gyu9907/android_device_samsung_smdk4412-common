LOCAL_PATH := $(call my-dir)
ifeq ($(BOARD_MODEM_TYPE),cmc221)
include $(CLEAR_VARS)
LOCAL_MODULE := libc1-ril-compat
LOCAL_VENDOR_MODULE := true
LOCAL_SRC_FILES := c1-ril-symbols.cpp
LOCAL_SHARED_LIBRARIES := libutils
LOCAL_CFLAGS := -Wall -Wextra -Werror
# The later dlopen of libsec-ril must see these exports in the global group.
LOCAL_LDFLAGS := -Wl,-z,global
include $(BUILD_SHARED_LIBRARY)
endif
