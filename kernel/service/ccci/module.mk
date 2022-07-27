
C_FILES +=  kernel/service/ccci/src/ccci.c
CFLAGS += -DCCCI_SUPPORT_NON_BLOCKING_SEND
CFLAGS += CCCI_ENABLE
#################################################################################
#include path
CFLAGS += -I$(SOURCE_DIR)/kernel/service/ccci/inc

