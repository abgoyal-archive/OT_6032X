ifeq ($(PROJECT),jrdsz89_we_jb2)
EXTRA_CFLAGS += -DRAV4_LED
endif
ifeq ($(PROJECT),diablox)
EXTRA_CFLAGS += -DDIABLOX_LED
endif
ifeq ($(PROJECT),diabloxtd)
EXTRA_CFLAGS += -DDIABLOX_TD_LED
endif
ifeq ($(PROJECT),scribepro)
EXTRA_CFLAGS += -DSCRIBEPRO_LED
endif
ifeq ($(PROJECT),scribeprotd)
EXTRA_CFLAGS += -DSCRIBEPRO_TD_LED
endif
ifeq ($(PROJECT),diabloalpha)
EXTRA_CFLAGS += -DDIABLOALPHA_LED
endif
ifeq ($(PROJECT),diabloalphatd)
EXTRA_CFLAGS += -DDIABLOALPHA_TD_LED
endif
obj-$(CONFIG_MTK_LEDS) := leds.o
ifeq ($(PROJECT),diabloalphatd)
obj-y += sn3199.o sn3193.o
endif
ifeq ($(PROJECT),diabloalpha)
obj-y += sn3199.o sn3193.o
endif
ifeq ($(PROJECT),scribepro)
obj-y += sn3731.o
endif
ifeq ($(PROJECT),scribeprotd)
obj-y += sn3731.o
endif
