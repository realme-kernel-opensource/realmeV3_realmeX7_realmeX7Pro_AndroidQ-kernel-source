obj-$(CONFIG_OPPO_FINGERPRINT) += oppo_fp_common/
#obj-$(CONFIG_OPPO_FINGERPRINT) += fpc/

ifneq ($(filter MSM_17085 MSM_17081, $(OPPO_TARGET_DEVICE)),)
obj-$(CONFIG_OPPO_FINGERPRINT) += goodix_fp/
endif

ifeq ($(CONFIG_OPPO_FINGERPRINT_MTK),y)
#MT6853
ifeq ($(shell echo $(TARGET_PRODUCT) | sed -e 's/_1.*//g'),full_oppo6853)

ifneq ($(filter oppo6853_20611 oppo6853_20630, $(OPPO_TARGET_DEVICE)),)
obj-$(CONFIG_OPPO_FINGERPRINT) += goodix_optical_fp/
endif

ifneq ($(filter oppo6853_20625 oppo6853_20626, $(OPPO_TARGET_DEVICE)),)
obj-$(CONFIG_OPPO_FINGERPRINT) += silead_mtk/
endif
endif

#MT6889
ifeq ($(shell echo $(TARGET_PRODUCT) | sed -e 's/_1.*//g'),full_oppo6889)
obj-$(CONFIG_OPPO_FINGERPRINT) += goodix_optical_fp/
endif

endif

ifeq ($(CONFIG_OPPO_FINGERPRINT_QCOM),y)
obj-$(CONFIG_OPPO_FINGERPRINT) += silead/
endif
