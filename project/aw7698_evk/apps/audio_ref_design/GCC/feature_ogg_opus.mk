ifeq ($(PROJ_PATH),)
include feature.mk
else
include $(PROJ_PATH)/feature.mk
endif

#OGG and OPUS_CLET
MTK_OGG_OPUS_ENABLE                 = y
