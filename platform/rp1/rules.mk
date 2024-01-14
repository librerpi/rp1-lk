LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

#LINKER_SCRIPT = $(LOCAL_DIR)/linker.ld
LINKER_SCRIPT += $(BUILDDIR)/system-onesegment.ld

MODULE_SRCS += $(LOCAL_DIR)/platform.c

MODULE_DEPS += arch/arm/arm-m/systick

include make/module.mk
