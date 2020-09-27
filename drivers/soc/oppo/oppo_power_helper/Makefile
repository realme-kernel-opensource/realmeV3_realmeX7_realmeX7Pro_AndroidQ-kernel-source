ccflags-y += -I$(srctree)/drivers//power/oppo/

ifdef CONFIG_ARCH_QCOM
else
ccflags-y += -I$(srctree)/drivers/misc/mediatek/lpm/modules/include/
endif

ifdef CONFIG_ARCH_QCOM
power_helper_ins-y := power_helper_msm.o
else
power_helper_ins-y := power_helper_mtk.o
endif

power_helper-objs := power_helper_utils.o
power_helper-objs += $(power_helper_ins-y)

obj-y += power_helper.o
