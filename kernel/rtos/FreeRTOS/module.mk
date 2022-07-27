
###################################################
# Sources
RTOS_SRC = kernel/rtos/FreeRTOS/Source

RTOS_FILES =	$(RTOS_SRC)/tasks.c \
		$(RTOS_SRC)/list.c \
		$(RTOS_SRC)/queue.c \
		$(RTOS_SRC)/timers.c \
		$(RTOS_SRC)/event_groups.c \
		$(RTOS_SRC)/portable/MemMang/heap_4.c \
		$(RTOS_SRC)/stream_buffer.c \
		$(RTOS_SRC)/portable/GCC/ARM_CM4F/port.c \
		$(RTOS_SRC)/portable/MemMang/heap_4.c

ifeq ($(PRODUCT_VERSION),7687)
RTOS_FILES +=   $(RTOS_SRC)/portable/GCC/mt7687/port_tick.c
endif

ifeq ($(PRODUCT_VERSION),7697)
RTOS_FILES +=   $(RTOS_SRC)/portable/GCC/mt7687/port_tick.c
endif

ifeq ($(PRODUCT_VERSION),2523)
RTOS_FILES += 	$(RTOS_SRC)/portable/GCC/mt2523/port_tick.c 
CFLAGS     += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/mt2523
endif

ifeq ($(PRODUCT_VERSION),2533)
RTOS_FILES += 	$(RTOS_SRC)/portable/GCC/mt2523/port_tick.c 
CFLAGS     += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/mt2523
endif

ifeq ($(PRODUCT_VERSION),7686)
RTOS_FILES += 	$(RTOS_SRC)/portable/GCC/mt7686/port_tick.c 
CFLAGS     += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/mt7686
endif

ifeq ($(PRODUCT_VERSION),7682)
RTOS_FILES += 	$(RTOS_SRC)/portable/GCC/mt7686/port_tick.c 
CFLAGS     += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/mt7686
endif

ifeq ($(PRODUCT_VERSION),5932)
RTOS_FILES += 	$(RTOS_SRC)/portable/GCC/mt7686/port_tick.c 
CFLAGS     += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/mt7686
endif

ifeq ($(PRODUCT_VERSION),1552)
RTOS_FILES += 	$(RTOS_SRC)/portable/GCC/ab155x/port_tick.c 
CFLAGS     += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/ab155x
endif


ifeq ($(PRODUCT_VERSION),7698)
RTOS_FILES += 	$(RTOS_SRC)/portable/GCC/aw7698/port_tick.c 
CFLAGS     += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/aw7698
endif

C_FILES += $(RTOS_FILES)

###################################################
# include path
CFLAGS += -I$(SOURCE_DIR)/$(RTOS_SRC)/include
CFLAGS += -I$(SOURCE_DIR)/$(RTOS_SRC)/portable/GCC/ARM_CM4F

CFLAGS += -I$(SOURCE_DIR)/kernel/service/inc

###################################################
#Enable the feature by configuring
FREERTOS_VERSION = V10_AND_LATER
CFLAGS += -DFREERTOS_ENABLE
CFLAGS += -DFREERTOS_VERSION=$(FREERTOS_VERSION)