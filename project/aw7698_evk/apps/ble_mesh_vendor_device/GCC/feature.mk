IC_CONFIG                             = aw7698
BOARD_CONFIG                          = aw7698_evk
MTK_MBEDTLS_CONFIG_FILE             = config-mtk-homekit.h
MTK_LED_ENABLE                      = y
# debug level: none, error, warning, and info
MTK_DEBUG_LEVEL                     = info
# System service debug feature for internal use
MTK_SUPPORT_HEAP_DEBUG              = n
MTK_HEAP_SIZE_GUARD_ENABLE          = n
MTK_OS_CPU_UTILIZATION_ENABLE       = y
#SWLA
MTK_SWLA_ENABLE                     = n

#NVDM
MTK_NVDM_ENABLE                     = y

#WIFI features
MTK_WIFI_TGN_VERIFY_ENABLE          = n
MTK_WIFI_DIRECT_ENABLE              = n
MTK_WIFI_PROFILE_ENABLE             = y
MTK_SMTCN_V5_ENABLE                 = y
MTK_CM4_WIFI_TASK_ENABLE            = y
MTK_WIFI_ROM_ENABLE                 = y

# system hang debug: none, o1 and o2
MTK_SYSTEM_HANG_TRACER_ENABLE       = n
#LWIP features
MTK_IPERF_ENABLE                    = y
MTK_PING_OUT_ENABLE                 = y
MTK_USER_FAST_TX_ENABLE             = n

#CLI features
MTK_MINICLI_ENABLE                  = y
MTK_CLI_TEST_MODE_ENABLE            = y

MTK_HAL_LOWPOWER_ENABLE             = y
MTK_HIF_GDMA_ENABLE                 = y
MTK_OS_CLI_ENABLE                   = y

#secure boot
MTK_SECURE_BOOT_ENABLE      		    = n

#Bluetooth
MTK_HCI_CONSOLE_MIX_ENABLE          = y
MTK_BLE_CLI_ENABLE                  = n
MTK_BLE_BQB_CLI_ENABLE              = n

# bt module enable
MTK_BT_ENABLE                       = y
MTK_BLE_ONLY_ENABLE                 = y
MTK_BT_MESH_ENABLE                  = y
#BWCS
MTK_BWCS_ENABLE                 = y

# mesh example project: none, pts_device, test, pts_provisioner, switch, vendor_device, pts_lighting_client, pts_lighting_server
MTK_BT_MESH_EXAMPLE_PROJECT         = vendor_device
MTK_BT_MESH_CLI_ENABLE              = y
