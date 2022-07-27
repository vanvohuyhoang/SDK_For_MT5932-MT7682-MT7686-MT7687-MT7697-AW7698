IC_CONFIG                             = aw7698
BOARD_CONFIG                          = aw7698_evk
# debug level: none, error, warning, and info
MTK_DEBUG_LEVEL                       = info
# System service debug feature for internal use
MTK_SUPPORT_HEAP_DEBUG              = n
MTK_HEAP_SIZE_GUARD_ENABLE          = n
MTK_OS_CPU_UTILIZATION_ENABLE       = y
#SWLA
MTK_SWLA_ENABLE                     = n

#NVDM
MTK_NVDM_ENABLE                     = y

#WIFI features
#If this feature is off, than other Wi-Fi and LWIP feature options won't take effect
MTK_WIFI_ENABLE                     = y
MTK_WIFI_TGN_VERIFY_ENABLE          = n
MTK_WIFI_DIRECT_ENABLE              = n
MTK_WIFI_PROFILE_ENABLE             = y
MTK_CM4_WIFI_TASK_ENABLE            = y
MTK_WIFI_ROM_ENABLE                 = y
MTK_WIFI_CLI_ENABLE                 = y
#Wi-Fi&BT coex
MTK_BWCS_ENABLE                     = n

# system hang debug: none, o1 and o2
MTK_SYSTEM_HANG_TRACER_ENABLE        = n
#LWIP features
MTK_PING_OUT_ENABLE                 = y
#CLI features
MTK_MINICLI_ENABLE                  = y
MTK_CLI_TEST_MODE_ENABLE            = y
MTK_HAL_LOWPOWER_ENABLE             = y
MTK_HIF_GDMA_ENABLE                 = y

#AT command
MTK_WIFI_AT_COMMAND_ENABLE          = y
MTK_AT_CMD_DISABLE                  = n
MTK_ATCI_ENABLE                     = y

MTK_FOTA_ENABLE                     = y
MTK_FOTA_AT_COMMAND_ENABLE          = y
MTK_FOTA_CLI_ENABLE                 = y

MTK_SECURE_BOOT_ENABLE              = n
