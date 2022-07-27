FatFS module usage guide

Brief:          This module is the file system implementation to manage file operation.
Usage:          GCC: Include the module with "include $(SOURCE_DIR)/middleware/third_party/fatfs/module.mk" in your GCC project Makefile, add "MTK_FATFS_ON_SPI_SD = y" in your GCC project feature.mk if SD is implemented based on SPI.
                KEIL/IAR: Copy the ff.c, diskio.c, syscall.c, and unicode.c files (according to current configuration) to your project and add middleware/third_party/fatfs/src to include paths, add "MTK_FATFS_ON_SPI_SD" if SD is implemented based on SPI.
Dependency:     If the device uses a SD module, please define HAL_SD_MODULE_ENABLED in hal_feature_config.h under the inc folder of your project. If the device uses the nor flash, please include the midddleware/MTK/flash_manager/snor/module.mk. The nor flash is only supported on 7686.
Notice:         Middleware/third_party/fatfs/src/diskio.c does not have RTC implementation.
                Please configure the RTC for FatFS under middleware/third_party/fatfs/src/diskio.c if you require time information.
Related doc:   Please refer to the open source user guide under the doc folder for more information.
Example project:None.
                You can do the porting of the examples to our platform in FatFS official website as shown in the following procedure.
                    Step 1. Download the FatFS example project (such as ffsample/avr_foolproof) by clicking "FatFs sample projects for various platforms" on the FatFS official website.
                    Step 2. Delete the line with "# inlcude <avr/io.h>" in main.c under ffsample/avr_foolproof/.
                    Step 3. Copy project/mt2523_hdk/apps/freertos_thread_creation/ and replace the main.c with the file you created in step2.
                    Step 4. Rename the copied "freertos_thread_creation" project.
                    Step 5. Enable FatFS module according to the above method.
