obj-$(CONFIG_MTK_MMC) += dbg.o
obj-$(CONFIG_MTK_MMC) += sd.o mt_sd_misc.o


ifeq ($(CONFIG_MTK_AEE_KDUMP),y)
obj-$(CONFIG_MTK_MMC) += mt_dump.o
endif

#add project name marco for choose IMEI function
ifeq ($(MTK_PROJECT),diabloalphatd)
KBUILD_CFLAGS += -DDIABLOALPHATD
endif
ifeq ($(MTK_PROJECT),diabloxtd)
KBUILD_CFLAGS += -DDIABLOXTD
endif
ifeq ($(MTK_PROJECT),scribeprotd)
KBUILD_CFLAGS += -DSCRIBEPROTD
endif

ifeq ($(MTK_EMMC_SUPPORT_OTP),yes)
obj-$(CONFIG_MTK_MMC) += emmc_otp.o
endif
