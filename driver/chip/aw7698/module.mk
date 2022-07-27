include $(SOURCE_DIR)/driver/chip/aw7698/src/GCC/module.mk

ifneq ($(wildcard $(strip $(SOURCE_DIR))/driver/chip/aw7698/src_core/),)
include $(SOURCE_DIR)/driver/chip/aw7698/src_core/GCC/module.mk
else
LIBS += $(SOURCE_DIR)/prebuilt/driver/chip/aw7698/lib/libhal_core_CM4_GCC.a
endif

ifneq ($(wildcard $(strip $(SOURCE_DIR))/driver/chip/aw7698/src_protected/),)
include $(SOURCE_DIR)/driver/chip/aw7698/src_protected/GCC/module.mk
else
LIBS += $(SOURCE_DIR)/prebuilt/driver/chip/aw7698/lib/libhal_protected_CM4_GCC.a
endif


