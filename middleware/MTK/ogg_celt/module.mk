
OGGCELT_DIR = middleware/MTK/ogg_celt

C_FILES  += $(OGGCELT_DIR)/src/framing.c
C_FILES  += $(OGGCELT_DIR)/src/oggcelt_api.c
C_FILES  += $(OGGCELT_DIR)/src/opus_header.c
C_FILES  += $(OGGCELT_DIR)/src/ogg_pack_api.c
#################################################################################
#include path
CFLAGS 	 += -I$(SOURCE_DIR)/middleware/MTK/ogg_celt/inc
CFLAGS   += -I$(SOURCE_DIR)/middleware/MTK/audio/celt_codec/inc


