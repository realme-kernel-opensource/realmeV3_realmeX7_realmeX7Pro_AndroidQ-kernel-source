# ifdef VENDOR_EDIT
#add for sdcard mt6853
ifeq ($(shell echo $(TARGET_PRODUCT) | sed -e 's/_1.*//g'),full_oppo6853)
obj-$(CONFIG_MT6360_LDO) += mt6360_ldo_i2c_mt6853.o mt6360_ldo_regmap.o
else
obj-$(CONFIG_MT6360_LDO) += mt6360_ldo_i2c.o mt6360_ldo_regmap.o
endif
#endif
