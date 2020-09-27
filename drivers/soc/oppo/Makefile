#ifdef VENDOR_EDIT
# Bin.Li@EXP.BSP.bootloader.bootflow, 2017/05/15, Add for oppo device info and oppo project
obj-$(CONFIG_OPPO_COMMON_SOFT)  +=  oppo_project/
obj-$(CONFIG_OPPO_DEVICE_IFNO)  +=  oppo_devinfo/
obj-$(CONFIG_OPPO_ALS_CALI)   +=  oppo_als_cali_value/
obj-y += oppo_custom/
#ifdef VENDOR_EDIT
#//wenbin.liu@PSW.Kernel.MM, 2018/05/23, Add for Healthinfo monitor
obj-y +=  oppo_healthinfo/
#endif /* VENDOR_EDIT */
#Liang.Zhang@TECH.Storage.Stability.OPPO_SHUTDOWN_DETECT, 2019/04/28, Add for shutdown detect
obj-$(CONFIG_OPPO_SHUTDOWN_DETECT)  +=  oppo_shutdown_detect/
#endif /* VENDOR_EDIT */
obj-$(CONFIG_OPPO_HYPNUS)     +=  oppo_hypnus/

#endif /* VENDOR_EDIT */
#Kun.Hu@TECH.BSP.Stability.PHOENIX_PROJECT 2019/06/15, Add for PHOENIX_PROJECT
obj-$(CONFIG_OPPO_PHOENIX)     +=  oppo_phoenix/

obj-y += oppo_power_helper/

#ifdef VENDOR_EDIT
#/* Yu.Kun@CN.NFC.Basic.Hardware, 2020/03/24, Add for oppo nfc chipset */
obj-$(CONFIG_OPPO_NFC)     +=  oppo_nfc/
#endif /* VENDOR_EDIT */
#Hailong.Liu@BSP.Kernel.MM, 2020/06/17, Add for dump mem when shrink
obj-$(CONFIG_OPLUS_FEATURE_LOWMEM_DBG)	+=  lowmem_dbg/
#endif /* OPLUS_FEATURE_LOWMEM_DBG */

#ifdef VENDOR_EDIT
#Hailong.Liu@BSP.MM 2019/03/18, Add for svelte
obj-$(CONFIG_OPPO_SVELTE)     +=  oppo_svelte/
#endif /* VENDOR_EDIT */
