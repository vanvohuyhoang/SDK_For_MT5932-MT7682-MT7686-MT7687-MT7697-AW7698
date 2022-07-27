
BWCS_SRC = middleware/MTK/bwcs

C_FILES  += $(BWCS_SRC)/src/bwcs.c
C_FILES  += $(BWCS_SRC)/src/bwcs_wifi_interface.c
C_FILES  += $(BWCS_SRC)/src/bwcs_bt_interface.c
C_FILES  += $(BWCS_SRC)/src/bwcs_api.c

#################################################################################
#include path
CFLAGS 	+= -I$(SOURCE_DIR)/$(BWCS_SRC)/inc
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/wifi_service/bwcs/inc
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/wifi_service/combo/inc
