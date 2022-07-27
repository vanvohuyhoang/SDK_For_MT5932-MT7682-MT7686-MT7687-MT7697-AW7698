
###################################################
# Sources
ATCI_SRC = middleware/MTK/atci

ATCI_FILES = $(ATCI_SRC)/src/atci_adapter.c \
                     $(ATCI_SRC)/src/atci_handler.c \
                     $(ATCI_SRC)/src/atci_main.c
#atci enable					 
CFLAGS += -DATCI_ENABLE

ifneq ($(MTK_AT_CMD_DISABLE), y)
#default query version command
CFLAGS += -DMTK_QUERY_SDK_VERSION
endif

##
## MTK_SWITCH_TO_RACE_COMMAND_ENABLE
## Brief:       This option is to enable and disable RACE command.
## Usage:       If the value is "y", the MTK_SWITCH_TO_RACE_COMMAND_ENABLE compile option will be defined. You must include some module in your Makefile before setting.
##              the option to "y"
##              include the /middleware/MTK/atci/module.mk
## Path:        middleware/MTK/atci/
## Dependency:  need add race command related module.
##
ifeq ($(MTK_SWITCH_TO_RACE_COMMAND_ENABLE),y)
CFLAGS         += -DMTK_SWITCH_TO_RACE_COMMAND_ENABLE
endif



##
## MTK_ATCI_VIA_PORT_SERVICE
## Brief:       This option is to enable ATCI through port service feature for data transmission.
## Usage:       Enable the feature by configuring it as y.
## Path:        middleware/MTK/atci/src/, middleware/MTK/atci/inc/
## Dependency:  Must enable the MTK_PORT_SERVICE_ENABLE for port service feature
## Notice:      The default implementation of ATCI for data transmission is using HAL UART directly.
## Relative doc:None
##
ifeq ($(MTK_PORT_SERVICE_ENABLE),y)
ifeq ($(MTK_ATCI_VIA_PORT_SERVICE),y)
  CFLAGS += -DMTK_ATCI_VIA_PORT_SERVICE
endif
endif

ifeq ($(MTK_BT_AT_COMMAND_ENABLE), y)
CFLAGS         += -DMTK_BT_AT_COMMAND_ENABLE
endif

ifeq ($(MTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE),y)
CFLAGS         += -DMTK_CAPID_IN_NVDM_AT_COMMAND_ENABLE
endif

ifeq ($(MTK_NVDM_ENABLE),y)
CFLAGS         += -DMTK_NVDM_ENABLE
endif

ifeq ($(MTK_ATCI_BUFFER_SLIM),y)
CFLAGS         += -DMTK_ATCI_BUFFER_SLIM
endif

ifeq ($(MTK_AT_CMD_DISABLE), y)
 ATCMD_FILES = $(ATCI_SRC)/at_command/at_command.c
else
 ATCMD_FILES = $(ATCI_SRC)/at_command/at_command.c \
             $(ATCI_SRC)/at_command/at_command_sdkinfo.c \
             $(ATCI_SRC)/at_command/at_command_sys.c \
             $(ATCI_SRC)/at_command/at_command_gnss.c \
             $(ATCI_SRC)/at_command/at_command_charger.c \
             $(ATCI_SRC)/at_command/at_command_pmu.c \
             $(ATCI_SRC)/at_command/at_command_external_pmu.c \
             $(ATCI_SRC)/at_command/at_command_external_gauge.c \
             $(ATCI_SRC)/at_command/at_command_msdc.c \
             $(ATCI_SRC)/at_command/at_command_crypto.c \
             $(ATCI_SRC)/at_command/at_command_ctp.c \
             $(ATCI_SRC)/at_command/at_command_keypad.c \
             $(ATCI_SRC)/at_command/at_command_bt.c \
             $(ATCI_SRC)/at_command/at_command_audio.c \
             $(ATCI_SRC)/at_command/at_command_backlight.c \
             $(ATCI_SRC)/at_command/at_command_rtc.c \
             $(ATCI_SRC)/at_command/at_command_wdt.c \
             $(ATCI_SRC)/at_command/at_command_gsensor.c \
             $(ATCI_SRC)/at_command/at_command_sleep_manager.c \
             $(ATCI_SRC)/at_command/at_command_reg.c \
             $(ATCI_SRC)/at_command/at_command_dvfs.c \
             $(ATCI_SRC)/at_command/at_command_system.c \
             $(ATCI_SRC)/at_command/at_command_nvdm.c \
             $(ATCI_SRC)/at_command/at_command_vibrator.c \
             $(ATCI_SRC)/at_command/at_command_led.c \
             $(ATCI_SRC)/at_command/at_command_clock.c  \
             $(ATCI_SRC)/at_command/at_command_ctp_simulate.c \
             $(ATCI_SRC)/at_command/at_command_serial_port.c \
             $(ATCI_SRC)/at_command/at_command_mem.c \
             $(ATCI_SRC)/at_command/at_command_usimsmt.c \
             $(ATCI_SRC)/at_command/at_command_keypad_simulate.c \
             $(ATCI_SRC)/at_command/at_command_gpio.c \
             $(ATCI_SRC)/at_command/at_command_switch.c \
             $(ATCI_SRC)/at_command/at_command_adc.c \
             $(ATCI_SRC)/at_command/at_command_eint_key.c \
             $(ATCI_SRC)/at_command/at_command_captouch.c \
             $(ATCI_SRC)/at_command/at_command_pwm.c

ATCI_FILES += $(ATCI_SRC)/at_command/at_command_bt_testbox.c
ATCI_FILES += $(ATCI_SRC)/at_command/at_command_calibration_capid.c

ifeq ($(TOOL_APP_MODULE),testframework)
ATCMD_FILES  += $(ATCI_SRC)/at_command/at_command_testframework.c
endif

ifeq ($(MTK_AUDIO_TUNING_ENABLED), y)
  ifeq ($(IC_CONFIG),mt2533)
    ATCI_FILES += $(ATCI_SRC)/at_command/at_command_audio_tuning_mt2533.c
  else
    ATCI_FILES += $(ATCI_SRC)/at_command/at_command_audio_tuning.c
  endif
endif

ifeq ($(MTK_AUDIO_AT_CMD_PROMPT_SOUND_ENABLE),y)
CFLAGS += -DMTK_AUDIO_AT_CMD_PROMPT_SOUND_ENABLE
endif

ifeq ($(MTK_SMT_AUDIO_TEST),y)
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_audio_ata_test.c
include $(SOURCE_DIR)/middleware/MTK/audio_fft/module.mk
CFLAGS += -DMTK_SMT_AUDIO_TEST
endif

ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG), mt2523 mt2533))
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_lcm.c
endif

ifeq ($(MTK_SENSOR_AT_COMMAND_ENABLE), y)
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_sensor.c
CFLAGS += -DMTK_SENSOR_AT_COMMAND_ENABLE
endif

ifeq ($(MTK_SENSOR_BIO_USE),MT2511)
# include kiss_fft
include $(SOURCE_DIR)/middleware/third_party/kiss_fft/module.mk
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_biogui.c
endif

ifeq ($(MTK_FOTA_AT_COMMAND_ENABLE),y)
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_fota.c
CFLAGS += -DMTK_FOTA_AT_COMMAND_ENABLE
endif

ifneq ($(MTK_DEBUG_LEVEL), none)
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_syslog.c
endif

ifeq ($(MTK_SENSOR_ACCELEROMETER_USE),BMA255)
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_gsensor.c
endif

ifeq ($(MTK_NVDM_MODEM_ENABLE),y)
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_md_nvdm.c
endif


ifeq ($(MTK_SPI_EXTERNAL_SERIAL_FLASH_ENABLED),y)
ATCMD_FILES += $(ATCI_SRC)/at_command/at_command_serial_flash.c
endif

endif

C_FILES += $(ATCI_FILES)
C_FILES += $(ATCMD_FILES)
###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/atci/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/nbiot/modem/l1/n1.mod/api
CFLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/MTK/nbiot/inc/modem/l1/n1.mod/inc/
ifneq ($(MTK_ATCI_APB_PROXY_ADAPTER_ENABLE), y)
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/backlight
CFLAGS += -I$(SOURCE_DIR)/driver/board/mt25x3_hdk/keypad/inc
CFLAGS += -I$(SOURCE_DIR)/driver/board/component/biosensor/mt2511/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt7687/inc
CFLAGS += -I$(SOURCE_DIR)/driver/chip/mt7686/inc
endif
ifeq ($(MTK_BT_AT_COMMAND_ENABLE), y)
  ifeq ($(IC_CONFIG),mt2533)
    CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bluetooth/inc
  endif
  ifeq ($(IC_CONFIG),mt2523)
    CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bluetooth/inc
  endif
endif

