#ifndef __REALME_POWER_HELPER_H__
#define __REALME_POWER_HELPER_H__
#include "../../../../kernel-4.14/drivers/power/oppo/oppo_charger.h"
#include <linux/proc_fs.h>
#ifdef CONFIG_ARCH_QCOM
	void power_helper_set_apss_master_stats(void *master_stats);
	void power_helper_rpmh_stats_data_init(phys_addr_t phys_addr_base, u32 phys_size, u32 num_records);
	void power_helper_update_ext_mpss_master_stats(char *sleep_info);
	void power_helper_get_pon_info(struct regmap *reg, u16 base, void *cfg);
	void power_helper_get_wlan_wakup_info(int type, const char *info1, const char *info2, uint32_t pri1, int ipv4);
#else
	void power_helper_r13_sleep_data_init(void *r13_table, int table_size);
	void power_helper_consys_sleep_info_update(char* info);
#endif
	void power_helper_charge_data_init(struct oppo_chg_chip *chip, void (*charge_get_bat_data_func)(struct oppo_chg_chip *));
	void power_helper_show_bat_info(void);
	struct proc_dir_entry *power_helper_get_proc_dir(void);
	bool check_power_helper_support(void);
#endif
