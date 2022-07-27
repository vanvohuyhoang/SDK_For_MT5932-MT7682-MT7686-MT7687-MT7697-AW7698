
###############################################################################
# feature option dependency
###############################################################################

## MTK_FOTA_ENABLE
## Brief:       This option is to enable and disable FOTA.
## Usage:       If the value is "y", the MTK_FOTA_ENABLE compile option will be defined. You must also include the gva3\middleware\MTK\fota\module.mk in your Makefile before setting the option to "y". 
## Path:        middleware/MTK/fota
## Dependency:  HAL_FLASH_MODULE_ENABLED and HAL_WDT_MODULE_ENABLED must also defined in the hal_feature_config.h  under the project inc folder. If MTK_FOTA_CLI_ENABLE defined in the module.mk is "y", please also include the middleware/MTK/tftp/module.mk. 
## Notice:      Please use the driver/CMSIS/Device/MTK/mt2523/linkerscript/gcc/full_bin_fota/flash.ld for gcc build environment. 
##              Please use the driver/CMSIS/Device/MTK/mt2523/linkerscript/rvct/full_bin_fota/flash.sct for KEIL build environment. 

ifeq ($(MTK_FOTA_ENABLE),y)
CFLAGS += -DMTK_FOTA_ENABLE
ifeq ($(MTK_FOTA_VIA_RACE_CMD),y)
ifeq ($(MTK_FOTA_CM4_FS_ENABLE), y)
$(error Both MTK_FOTA_VIA_RACE_CMD and MTK_FOTA_CM4_FS_ENABLE are enabled. )
endif
ifeq ($(MTK_FOTA_FS_ENABLE), y)
$(error Both MTK_FOTA_VIA_RACE_CMD and MTK_FOTA_FS_ENABLE are enabled. )
endif
CFLAGS += -DMTK_FOTA_VIA_RACE_CMD

ifeq ($(MTK_FOTA_VIA_RACE_CMD_DUAL_DEVICES),y)
CFLAGS += -DMTK_FOTA_VIA_RACE_CMD_DUAL_DEVICES
endif

ifneq ($(IC_TYPE), $(filter $(IC_TYPE),ab1555))
# CFLAGS += -DMTK_FOTA_EXTERNAL_FLASH_SUPPORT
endif

ifeq ($(MTK_FOTA_STORE_IN_EXTERNAL_FLASH),y)
ifneq ($(IC_TYPE), $(filter $(IC_TYPE),ab1555))
$(error MTK_FOTA_EXTERNAL_FLASH_SUPPORT is not defined! Check if the chip does not support the external flash. )
endif
# CFLAGS += -DMTK_FOTA_STORE_IN_EXTERNAL_FLASH
endif
endif
endif

ifeq ($(MTK_FOTA_CMD_ENABLE),y)
CFLAGS += -DMTK_FOTA_CMD_ENABLE
endif

ifeq ($(MTK_FOTA_UBIN_ENABLE),y)
CFLAGS += -DMTK_FOTA_UBIN_ENABLE
endif


ifeq ($(MTK_FOTA_CM4_FS_ENABLE), y)
CFLAGS += -DFOTA_FS_ENABLE
endif

ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),mt7687 mt7697))
CFLAGS += -DMTK_FOTA_ON_7687

ifeq ($(MTK_FOTA_DUAL_IMAGE_ENABLE), y)
CFLAGS += -DMTK_FOTA_DUAL_IMAGE_ENABLE

ifeq ($(MTK_FOTA_DUAL_IMAGE_ONE_PACK_ENABLE), y)
CFLAGS += -DMTK_FOTA_DUAL_IMAGE_ONE_PACK
endif

endif
endif

ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),mt7682 mt7686 aw7698))
CFLAGS += -DMTK_FOTA_ON_7686
endif

ifeq ($(MTK_BOOTLOADER_SUPPORT_PARTITION_FOTA), y)
CFLAGS += -DMTK_BOOTLOADER_SUPPORT_PARTITION_FOTA
endif

#################################################################################
# source files
#################################################################################

FOTA_SRC = middleware/MTK/fota

C_FILES  += $(FOTA_SRC)/src/fota.c
C_FILES  += $(FOTA_SRC)/src/common/fota_platform.c
ifneq ($(MTK_FOTA_VIA_RACE_CMD), y)
ifneq ($(MTK_FOTA_CM4_FS_ENABLE), y)
C_FILES  += $(FOTA_SRC)/src/internal/fota_internal.c
C_FILES  += $(FOTA_SRC)/src/internal/fota_config.c
C_FILES  += $(FOTA_SRC)/src/internal/fota_port.c
endif
ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),mt7687 mt7697 mt7682 mt7686 aw7698))
C_FILES  += $(FOTA_SRC)/src/internal/fota_cli.c
ifeq ($(MTK_FOTA_DUAL_IMAGE_ENABLE),y)
C_FILES  += $(FOTA_SRC)/src/internal/fota_dual_image.c
else
C_FILES  += $(FOTA_SRC)/src/internal/fota_download_interface.c
endif
endif

else

ifneq ($(MTK_BL_LOAD_ENABLE),y)
C_FILES  += $(FOTA_SRC)/src/race/fota_timer.c
endif
C_FILES  += $(FOTA_SRC)/src/race/crc32.c
C_FILES  += $(FOTA_SRC)/src/race/fota_util.c
C_FILES  += $(FOTA_SRC)/src/race/fota_multi_info_util.c
C_FILES  += $(FOTA_SRC)/src/race/fota_multi_info.c
C_FILES  += $(FOTA_SRC)/src/race/fota_flash.c
C_FILES  += $(FOTA_SRC)/src/race/fota_flash_config.c
C_FILES  += $(FOTA_SRC)/src/fota_flash_for_gsound.c
endif

#################################################################################
# include path
#################################################################################

ifneq ($(MTK_FOTA_VIA_RACE_CMD), y)
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/fota/inc
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/fota/inc/internal
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/fota/inc/common
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/inc
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/tftp/inc
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/httpclient/inc

ifeq ($(MTK_BL_LOAD_ENABLE),y)
CFLAGS  += -I$(SOURCE_DIR)/driver/board/mt76x7_hdk/bootloader/src/mt7687
CFLAGS  += -DBL_FOTA_DEBUG
endif

ifeq ($(MTK_FOTA_DUAL_IMAGE_ENABLE),y)
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/include
CFLAGS  += -I$(SOURCE_DIR)/middleware/third_party/httpclient/inc
endif

else

CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/fota/inc/race
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/fota/inc/common
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/fota/inc/
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/race_cmd/inc
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/inc
endif

