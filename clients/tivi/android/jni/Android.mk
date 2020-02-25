#
# Copyright (c) 2019 Silent Circle.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
# @author Werner Dittmann <Werner.Dittmann@t-online.de>
#
# ZRTP version: @VERSION@

commit := $(shell git rev-parse --short HEAD)

LOCAL_PATH := @CMAKE_SOURCE_DIR@
ROOT_SRC_PATH := $(LOCAL_PATH)

#
# Define and build the zrtpcpp static lib
#
include $(CLEAR_VARS)
LOCAL_MODULE := zrtpcpp
LOCAL_CPP_FEATURES := @local_cpp_features@

dummy := $(shell echo "char zrtpBuildInfo[] = \"@VERSION@:$(commit):$(TARGET_ARCH_ABI)\";" > $(ROOT_SRC_PATH)/buildinfo_$(TARGET_ARCH_ABI).c)

#
# set to false if testing/compiling new modules to catch undefined symbols (if build shared lib without TIVI_ENV)
# LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

# include paths for zrtpcpp modules
LOCAL_C_INCLUDES += $(ROOT_SRC_PATH) $(ROOT_SRC_PATH)/srtp $(ROOT_SRC_PATH)/zrtp $(ROOT_SRC_PATH)/bnlib \
                    $(ROOT_SRC_PATH)/clients/tivi $(ROOT_SRC_PATH)/clients/tivi/android/jni/@sql_include@

LOCAL_SRC_FILES := buildinfo_$(TARGET_ARCH_ABI).c
LOCAL_CFLAGS := -DSUPPORT_NON_NIST @sql_cipher_define@

SIDH := @has_sidh@

ifeq "$(SIDH)" "TRUE"
  SIDH_armeabi-v7a := -D_ARM_ -D_GENERIC_ -D__LINUX__
  SIDH_EXTRA_armeabi-v7a := cryptcommon/sidhp751/generic/fp_generic.c

  SIDH_x86 := -D_X86_ -D_GENERIC_ -D__LINUX__
  SIDH_EXTRA_x86 := cryptcommon/sidhp751/generic/fp_generic.c

  SIDH_arm64-v8a := -D_ARM64_ -D__LINUX__
  SIDH_EXTRA_arm64-v8a := cryptcommon/sidhp751/ARM64/fp_arm64.c cryptcommon/sidhp751/ARM64/fp_arm64_asm.S

  SIDH_CFLAGS := $(SIDH_$(TARGET_ARCH_ABI))
  SIDH_EXTRA_SRC := $(SIDH_EXTRA_$(TARGET_ARCH_ABI))

  LOCAL_SRC_FILES += $(SIDH_EXTRA_SRC)
  LOCAL_CFLAGS += $(SIDH_CFLAGS)
endif

# For this Android build we can set the visibility to hidden. Access to ZRTP is only inside
# the shared lib that we build later for Silent Phone.
LOCAL_CFLAGS += @axo_support@ @LOG_MAX_LEVEL@ -fPIC -fvisibility=hidden -fvisibility-inlines-hidden

LOCAL_SRC_FILES += @sqlite_src@
LOCAL_SRC_FILES += @zrtpcpp_src_spc@

include $(BUILD_STATIC_LIBRARY)
