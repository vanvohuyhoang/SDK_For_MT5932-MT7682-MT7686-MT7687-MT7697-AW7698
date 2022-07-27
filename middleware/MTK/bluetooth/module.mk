ifeq ($(MTK_BT_ENABLE), y)
###################################################
# Libs
###################################################

#bt stack libs release version
ifeq ($(MTK_BT_LIB_RELEASE_ENABLE), y)
ifeq ($(MTK_BLE_ONLY_ENABLE), y)
ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),mt2523 mt2523s mt2533))
$(error mt25x3 serail not support MTK_BLE_ONLY_ENABLE)
endif
CFLAGS += -D__MTK_BLE_ONLY_ENABLE__
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libble_release.a
else
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_release.a
ifeq ($(MTK_BT_HFP_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_hfp_release.a
endif
ifeq ($(MTK_BT_HSP_ENABLE), y)
CFLAGS += -DMTK_BT_HSP_ENABLE
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_hsp_release.a
endif
ifeq ($(MTK_BT_A2DP_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_a2dp_release.a
endif
ifeq ($(MTK_BT_AVRCP_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_avrcp_release.a
endif
ifeq ($(MTK_BT_AVRCP_ENH_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_avrcp_enhance_release.a
endif
ifeq ($(MTK_BT_PBAP_ENABLE), y)
CFLAGS += -DMTK_BT_PBAP_ENABLE
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_pbapc_release.a
endif
ifeq ($(MTK_BT_SPP_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_spp_release.a
endif
ifeq ($(MTK_BT_AWS_ENABLE), y)
CFLAGS += -DMTK_BT_AWS_ENABLE
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_aws_release.a
endif
ifeq ($(MTK_AWS_MCE_ENABLE), y)
CFLAGS += -D__MTK_AWS_MCE_ENABLE__
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_aws_mce_release.a
endif
ifeq ($(MTK_AIRUPDATE_ENABLE), y)
CFLAGS += -D__MTK_AIRUPDATE_ENABLE__
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_airupdate_release.a
endif
ifeq ($(MTK_AVM_DIRECT), y)
CFLAGS += -D__MTK_AVM_DIRECT__
#LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_avm_release.a
endif
endif
#bt driver libs release version
ifeq ($(MTK_BT_DONGLE_TEST), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_bt_dongle.a
else ifeq ($(PLATFORM_DEVICE), BAND)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_2523_tx.a
else ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),mt2523 mt2523s))
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_2523_release.a
else ifeq ($(IC_CONFIG), mt2533)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_2523_release.a
else ifeq ($(IC_CONFIG), mt7687)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_7697_release.a
else ifeq ($(IC_CONFIG), mt7697)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_7697_release.a
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libble_multi_adv.a
else ifeq ($(IC_CONFIG), aw7698)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_7698_release.a
else ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),ab155x ab1552 ab1555 ab1556 ab1558))
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_155x_release.a
endif

else #MTK_BT_LIB_RELEASE_ENABLE
#bt stack libs debug version
ifeq ($(MTK_BLE_ONLY_ENABLE), y)
ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),mt2523 mt2523s mt2533))
$(error mt25x3 serail not support MTK_BLE_ONLY_ENABLE)
endif
CFLAGS += -D__MTK_BLE_ONLY_ENABLE__
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libble.a
else
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt.a
ifeq ($(MTK_BT_HFP_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_hfp.a
endif
ifeq ($(MTK_BT_HSP_ENABLE), y)
CFLAGS += -DMTK_BT_HSP_ENABLE
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_hsp.a
endif
ifeq ($(MTK_BT_A2DP_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_a2dp.a
endif
ifeq ($(MTK_BT_AVRCP_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_avrcp.a
endif
ifeq ($(MTK_BT_AVRCP_ENH_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_avrcp_enhance.a
endif
ifeq ($(MTK_BT_AVRCP_BROWSE_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_avrcp_browse.a
endif
ifeq ($(MTK_BT_PBAP_ENABLE), y)
CFLAGS += -DMTK_BT_PBAP_ENABLE
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_pbapc.a
endif
ifeq ($(MTK_BT_SPP_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_spp.a
endif

ifeq ($(MTK_BT_AWS_ENABLE), y)
CFLAGS += -DMTK_BT_AWS_ENABLE
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_aws.a
endif

ifeq ($(MTK_BT_HID_ENABLE), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_hid.a
endif

ifeq ($(MTK_AWS_MCE_ENABLE), y)
CFLAGS += -D__MTK_AWS_MCE_ENABLE__
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_aws_mce.a
endif

ifeq ($(MTK_AIRUPDATE_ENABLE), y)
CFLAGS += -D__MTK_AIRUPDATE_ENABLE__
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_airupdate.a
endif

ifeq ($(MTK_AVM_DIRECT), y)
CFLAGS += -D__MTK_AVM_DIRECT__
#LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbt_avm.a
endif

endif

#bt drvier libs debug version
ifeq ($(MTK_BT_DONGLE_TEST), y)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_bt_dongle.a
else ifeq ($(PLATFORM_DEVICE), BAND)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_2523_tx.a
else ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),mt2523 mt2523s))
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_2523.a
else ifeq ($(IC_CONFIG), mt2533)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_2523.a
else ifeq ($(IC_CONFIG), mt7687)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_7697.a
else ifeq ($(IC_CONFIG), mt7697)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_7697.a
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libble_multi_adv.a
else ifeq ($(IC_CONFIG), aw7698)
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_7698.a
else ifeq ($(IC_CONFIG), $(filter $(IC_CONFIG),ab155x ab1552 ab1555 ab1556 ab1558))
LIBS += $(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/lib/libbtdriver_155x.a
endif


endif

ifeq ($(MTK_BT_TIMER_EXTERNAL_ENABLE), y)
CFLAGS += -DMTK_BT_TIMER_EXTERNAL_ENABLE
endif

###################################################
# Sources
###################################################

CFLAGS += -D__BT_DEBUG__
include $(SOURCE_DIR)/middleware/third_party/micro_ecc/module.mk
BLUETOOTH_SRC = middleware/MTK/bluetooth/src
BLUETOOTH_FILES = $(BLUETOOTH_SRC)/bt_debug.c \
               $(BLUETOOTH_SRC)/bt_log.c \
               $(BLUETOOTH_SRC)/bt_hci_log.c \
               $(BLUETOOTH_SRC)/bt_os_layer_api.c \
               $(BLUETOOTH_SRC)/bt_task.c              

ifeq ($(MTK_BT_TIMER_EXTERNAL_ENABLE), y)
BLUETOOTH_FILES += $(BLUETOOTH_SRC)/bt_timer_external.c
ifeq ($(MTK_AWS_MCE_ENABLE),y)
ifeq ($(SUPPORT_ROLE_HANDOVER_SERVICE),y)
BLUETOOTH_FILES += $(BLUETOOTH_SRC)/bt_timer_external_rho.c
endif
endif
endif

ifeq ($(IC_CONFIG), mt7687)
BLUETOOTH_FILES += $(BLUETOOTH_SRC)/bt_cli.c
else ifeq ($(IC_CONFIG), mt7697)
BLUETOOTH_FILES += $(BLUETOOTH_SRC)/bt_cli.c
else ifeq ($(IC_CONFIG), aw7698)
BLUETOOTH_FILES += $(BLUETOOTH_SRC)/bt_cli.c
endif

C_FILES += $(BLUETOOTH_FILES)

###################################################
# include path
###################################################

CFLAGS += -I$(SOURCE_DIR)/middleware/MTK/bluetooth/inc
CFLAGS += -I$(SOURCE_DIR)/prebuilt/middleware/MTK/bluetooth/inc
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/include
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/mbedtls/configs
CFLAGS += -I$(SOURCE_DIR)/middleware/third_party/micro_ecc/inc
# definition

endif
