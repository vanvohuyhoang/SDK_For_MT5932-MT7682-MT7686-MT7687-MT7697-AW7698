# Copyright Statement:
#
# (C) 2019  Airoha Technology Corp. All rights reserved.
#
# This software/firmware and related documentation ("Airoha Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
# Without the prior written permission of Airoha and/or its licensors,
# any reproduction, modification, use or disclosure of Airoha Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
# You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
# if you have agreed to and been bound by the applicable license agreement with
# Airoha ("License Agreement") and been granted explicit permission to do so within
# the License Agreement ("Permitted User").If you are not a Permitted User,
# please cease any access or use of Airoha Software immediately.
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
# ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
# AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
#

#######################################################
# Project vairables assignment

SOURCE_DIR = ../../../../..
BINPATH     = ~/gcc-arm-none-eabi/bin
DATIME      = $(shell date --iso=seconds)
SDK_PATH = $(abspath $(PWD)/$(SOURCE_DIR))
PWD= $(shell pwd)
FEATURE ?= feature.mk
DEBUG = 0
FLOAT_TYPE = hard
BUILD_DIR = $(PWD)/build

# Project name
PROJ_NAME = $(shell basename $(dir $(PWD)))
PROJ_PATH = $(abspath $(dir $(PWD)))/GCC
OUTPATH = $(PWD)/build

CFLAGS += -Werror
# LD Flags
LDFLAGS = $(ALLFLAGS) $(FPUFLAGS) --specs=nano.specs -lnosys -nostartfiles
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-wrap=malloc -Wl,-wrap=calloc -Wl,-wrap=realloc -Wl,-wrap=free
LDFLAGS += -Wl,-Taw7698_flash.ld -Wl,--gc-sections
# Add for aws_iot, support float print/snprintf
LDFLAGS += -u _printf_float -u _scanf_float
ifeq ($(DEBUG_IO), 1)
LDFLAGS += --specs=rdimon.specs -lrdimon
CFLAGS += -DDEBUG_IO
endif
#######################################################
# Include Module

include $(FEATURE)
# Gloabl Config
-include $(SOURCE_DIR)/.config
# IC Config
-include $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk
# Board Config
-include $(SOURCE_DIR)/config/board/$(BOARD_CONFIG)/board.mk
# HAL driver files
include $(SOURCE_DIR)/driver/chip/$(IC_CONFIG)/module.mk
# EPT Config
-include $(SOURCE_DIR)/driver/board/$(BOARD_CONFIG)/ept/module.mk

# RTOS source files
include $(SOURCE_DIR)/kernel/rtos/FreeRTOS/module.mk
# kernel service files
include $(SOURCE_DIR)/kernel/service/module.mk
# NVDM files
ifeq ($(MTK_NVDM_ENABLE),y)
include $(SOURCE_DIR)/middleware/MTK/nvdm/module.mk
endif

include $(SOURCE_DIR)/driver/board/$(BOARD_CONFIG)/util/module.mk

ifeq ($(MTK_WIFI_ENABLE),y)
# Connsys
include $(SOURCE_DIR)/middleware/MTK/connsys/module.mk

# WiFi  driver files
ifeq ($(MTK_CM4_WIFI_TASK_ENABLE),y)
ifeq ($(MTK_WIFI_ROM_ENABLE), y)
ifneq ($(wildcard $(strip $(SOURCE_DIR))/middleware/MTK/wifi_stack),)
include $(SOURCE_DIR)/middleware/MTK/wifi_stack/module.mk
else
include $(SOURCE_DIR)/prebuilt/middleware/MTK/wifi_stack/module.mk
endif
endif
endif

ifeq ($(findstring y,$(MTK_SMTCN_V4_ENABLE)$(MTK_SMTCN_V5_ENABLE)),y)
-include $(SOURCE_DIR)/middleware/MTK/smtcn/module.mk
endif
endif

ifeq ($(MTK_MINICLI_ENABLE),y)
include $(SOURCE_DIR)/middleware/MTK/minicli/module.mk
endif

ifeq ($(MTK_WIFI_ENABLE),y)
#include lwip
include $(SOURCE_DIR)/middleware/third_party/lwip/module.mk

#include ping
include $(SOURCE_DIR)/middleware/third_party/ping/module.mk

#include iperf
include $(SOURCE_DIR)/middleware/MTK/iperf/module.mk

#include dhcpd
include $(SOURCE_DIR)/middleware/MTK/dhcpd/module.mk

# wifi src
include $(SOURCE_DIR)/middleware/MTK/wifi_service/combo/module.mk

# add ATCI files
ifeq ($(MTK_ATCI_ENABLE),y)
include $(SOURCE_DIR)/middleware/MTK/atci/module.mk
endif

# add AWS IoT files
ifeq ($(AWS_IOT_SUPPORT),y)
include $(SOURCE_DIR)/middleware/third_party/aws_iot/module.mk

#include mbedtls
include $(SOURCE_DIR)/middleware/third_party/mbedtls/module.mk

CFLAGS += -DAWS_IOT_TEST_ENABLE

ifeq ($(AWS_IOT_DEBUG_ENABLE), y)
CFLAGS += -DENABLE_IOT_DEBUG
endif

ifeq ($(AWS_IOT_ERROR_ENABLE), y)
CFLAGS += -DENABLE_IOT_ERROR
endif

ifeq ($(AWS_IOT_TRACE_ENABLE), y)
CFLAGS += -DENABLE_IOT_TRACE
endif

ifeq ($(AWS_IOT_INFO_ENABLE), y)
CFLAGS += -DENABLE_IOT_INFO
endif

ifeq ($(AWS_IOT_WARN_ENABLE), y)
CFLAGS += -DENABLE_IOT_WARN
endif

endif   # end AWS_IOT_SUPPORT


#bwcs
ifeq ($(MTK_BWCS_ENABLE),y)
include $(SOURCE_DIR)/middleware/MTK/bwcs/module.mk
endif
endif
#######################################################
# Main APP files


# Main APP files
APP_PATH        = $(patsubst $(SDK_PATH)/%,%,$(abspath $(dir $(PWD))))
APP_PATH_SRC    = $(APP_PATH)/src


##############################################################################
#
# SDK source files
#
##############################################################################
APP_FILES      += $(APP_PATH_SRC)/main.c
APP_FILES      += $(APP_PATH_SRC)/regions_init.c
APP_FILES      += $(APP_PATH_SRC)/sys_init.c
APP_FILES      += $(APP_PATH_SRC)/ept_gpio_var.c
APP_FILES      += $(APP_PATH_SRC)/ept_eint_var.c
APP_FILES      += $(APP_PATH)/GCC/syscalls.c
APP_FILES      += $(APP_PATH_SRC)/mem_layout_info.c
APP_FILES      += $(APP_PATH_SRC)/nvdm_config.c
ifeq ($(MTK_WIFI_ENABLE),y)
APP_FILES      += $(APP_PATH_SRC)/wifi_lwip_helper.c
APP_FILES      += $(APP_PATH_SRC)/network_default_config.c
APP_FILES      += $(APP_PATH_SRC)/wifi_nvdm_config.c
endif
ifeq ($(MTK_MINICLI_ENABLE),y)
APP_FILES      += $(APP_PATH_SRC)/cli_def.c
APP_FILES      += $(APP_PATH_SRC)/cli_cmds.c
endif
SYS_FILES      += $(APP_PATH_SRC)/system_aw7698.c

C_FILES        += $(APP_FILES) $(SYS_FILES)
S_FILES        += $(APP_PATH)/GCC/startup_aw7698.s

ifeq ($(AWS_IOT_SUPPORT),y)
APP_FILES      += $(APP_PATH_SRC)/aws_iot_test.c
endif
##############################################################################
#
# SDK object files
#
##############################################################################


C_OBJS          = $(C_FILES:%.c=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)
S_OBJS          = $(S_FILES:%.s=$(BUILD_DIR)/%.o)
CXX_OBJS        = $(CXX_FILES:%.cpp=$(BUILD_DIR)/%.o)

#######################################################
# Include path

CFLAGS += -I$(SOURCE_DIR)/$(APP_PATH)/inc
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Include
CFLAGS += -I$(SOURCE_DIR)/driver/CMSIS/Device/airoha/aw7698/Include
CFLAGS += -I$(SOURCE_DIR)/driver/chip/aw7698/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc

ifeq ($(AWS_IOT_SUPPORT),y)
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/aws_iot/samples/freertos
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/aws_iot/tests/freertos_integration/include
endif

USE_TEST_CERT := $(strip $(wildcard $(SOURCE_DIR)/project/common/certs_protected/aws))
ifneq ($(USE_TEST_CERT),)
CFLAGS += -I$(SOURCE_DIR)/project/common/certs_protected/aws
else
CFLAGS += -I$(SOURCE_DIR)/project/common/certs/aws
endif

#######################################################
# Rules

.PHONY: proj clean $(MODULE_PATH)

all: cleanlog proj
	@mkdir -p $(BUILD_DIR)
	@$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf
	@echo "Generate Assembly from elf:"
	#@$(OBJDUMP) -S $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).elf.s
	@$(SOURCE_DIR)/tools/scripts/build/copy_firmware.sh $(SOURCE_DIR) $(OUTPATH) $(IC_CONFIG) $(BOARD_CONFIG) $(PROJ_NAME).bin $(PWD) $(MTK_SECURE_BOOT_ENABLE)
	@cp *.cmm $(OUTPATH)/

ifeq ($(MTK_SECURE_BOOT_ENABLE),y)
	@$(SOURCE_DIR)/tools/secure_boot/scripts/build/build_sboot_header.sh $(SOURCE_DIR) $(OUTPATH) $(BOARD_CONFIG) $(PROJ_NAME).bin
endif


MOD_EXTRA = BUILD_DIR=$(BUILD_DIR) OUTPATH=$(OUTPATH) PROJ_PATH=$(PROJ_PATH)

$(LIBS): $(MODULE_PATH)

$(MODULE_PATH):
	@+make -C $@ $(MOD_EXTRA) $($@_EXTRA)

proj: $(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(C_OBJS) $(CXX_OBJS) $(S_OBJS) $(LIBS) $(WIFI_ROM_SYM)
	@echo Linking...
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).map" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).map"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).hex" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).hex"; fi
	@if [ -e "$(OUTPATH)/$(PROJ_NAME).bin" ]; then rm -f "$(OUTPATH)/$(PROJ_NAME).bin"; fi
	@echo $(LDFLAGS) -Wl,--whole-archive -Wl,--start-group $(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%, $^) -Wl,--end-group -Wl,--no-whole-archive -Wl,-Map=$(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%,$(OUTPATH))/$(PROJ_NAME).map -lm -o $(patsubst $(SDK_PATH)/%,$(SOURCE_DIR)/%, $@) > $(OUTPATH)/link_option.tmp
	@$(CXX) @$(OUTPATH)/link_option.tmp 2>>$(ERR_LOG)
	@rm -f $(OUTPATH)/link_option.tmp
#	@$(OBJDUMP) -D $(OUTPATH)/$(PROJ_NAME).elf > $(OUTPATH)/$(PROJ_NAME).dis
	@$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	@$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin
	@echo Done


include $(SOURCE_DIR)/.rule.mk

clean:
	@rm -rf $(OUTPATH)

.PHONY: options

options:
	@for opt in `sed 's/[()]/\n/g' $(SOURCE_DIR)/config/chip/$(IC_CONFIG)/chip.mk|sed 's/-D/ /'|sort|uniq|grep ^MTK_|grep _ENABLE$$`; do echo $$opt; done
print-%  : ; @echo $* = $($*)
