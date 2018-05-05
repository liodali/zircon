# Copyright 2018 The Fuchsia Authors
#
# Use of this source code is governed by a MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT

LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/pasm.cpp \
	$(LOCAL_DIR)/pasm_tests.cpp

MODULE_INCLUDES += \
	$(LOCAL_DIR)/include

MODULE_DEPS += \
	kernel/lib/fbl

include make/module.mk
