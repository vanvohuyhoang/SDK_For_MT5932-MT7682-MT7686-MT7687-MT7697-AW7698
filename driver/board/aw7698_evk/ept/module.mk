
EPT_7686_SRC = driver/board/aw7698_evk

C_FILES  += $(EPT_7686_SRC)/ept/src/bsp_gpio_ept_config.c

#################################################################################
#include path
CFLAGS 	+= -Iinclude
CFLAGS  += -I$(SOURCE_DIR)/$(EPT_7686_SRC)/ept/inc
CFLAGS  += -I$(SOURCE_DIR)/driver/CMSIS/Device/airoha/aw7698/Include


