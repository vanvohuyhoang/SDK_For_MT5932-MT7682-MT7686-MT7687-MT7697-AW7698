IC_CONFIG                   = aw7698
BOARD_CONFIG                = aw7698_evk
MTK_BL_LOAD_ENABLE          = y

#can modify
MTK_BL_FOTA_CUST_ENABLE     = n
MTK_BL_DEBUG_ENABLE         = y
MTK_FOTA_ENABLE             = y
MTK_FOTA_FS_ENABLE          = n
MTK_USER_FAST_TX_ENABLE     = n

#internal use
MTK_BL_FPGA_LOAD_ENABLE     = n

#factory
MTK_CAL_DCXO_CAPID          = n
# DCXO calibration value is in SW
MTK_BL_DCXO_KVALUE_SW       = n

#secure boot
ifneq ($(wildcard $(strip $(SOURCE_DIR))/tools/secure_boot/),)
MTK_SECURE_BOOT_ENABLE      = y
else
MTK_SECURE_BOOT_ENABLE      = n
endif
MTK_BOOTLOADER_USE_MBEDTLS  = y

MTK_BOOTLOADER_SUPPORT_PARTITION_FOTA = n