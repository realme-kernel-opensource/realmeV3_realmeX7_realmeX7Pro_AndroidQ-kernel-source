/**********************************************************************************
* Description: power debug tool and info.
* ------------------------------ Revision History: --------------------------------
* <version>           <date>                <author>                            <desc>
* Revision 1.0        2020-05-05        yangmingjin@realme.com                  Created
***********************************************************************************/
#include <asm/arch_timer.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/file.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <mt-plat/mtk_ccci_common.h>
#include <mtk_dbg_common_v1.h>

#include <linux/power/power_helper.h>
#include "oppo_charger.h"
#include <linux/power/power_helper_utils.h>

#ifdef CONFIG_POWER_HELPER_SUPPORT

#define PRINT_BUF_SIZE 640
#define R13_MAX_NUM 32
#define CONSS_MODULE_COUNT 5
#define LOG_TAG "[power_helper]"
#define BAT_INFO_FORMAT ""LOG_TAG"[bat_info][vol %d, cur %d, temp %d, soc %d, ui_soc %d, fcc %d, rm %d]"
#define CONSS_SLEEP_INFO_FORMAT "TOP:%llu,%llu;MCU:%llu,%llu;BT:%llu,%llu;WIFI:%llu,%llu;GPS:%llu,%llu"

#define RTC_CLOCK_FREQ  (32768)

enum rpmh_stats_index {
	INDEX_VMIN = 0,
	INDEX_APSS = 1,
	INDEX_MPSS = 2,
	INDEX_WMT_TOP = 3,
	INDEX_WMT_MCU = 4,
	INDEX_WMT_BT = 5,
	INDEX_WMT_WF = 6,
	INDEX_WMT_GPS = 7,
};

enum power_helper_data_type {
	RPMH_STATS = 0,
	BATTERY = 1,
	R13_INFO = 2,
};

struct rpmh_stats_data {
	char *name;
	u64 total_sleep_time;
	u64 last_total_sleep_time;
	int sleep_ratio;
	int last_sleep_ratio;
};

struct oppo_charge_data {
	struct delayed_work        battery_info_show_work;
	int last_ui_soc;
};

struct r13_sleep_info {
	u32 bitinfo;
	char *name;
	u64 count;
	u64 duration;
};

struct r13_sleep_info_data {
	struct r13_sleep_info r13_sleep[R13_MAX_NUM];
	struct r13_sleep_info last_r13_sleep[R13_MAX_NUM];
};

struct powre_helper_data {
	struct rpmh_stats_data  *rpmh_stats_data_pri;
	struct oppo_charge_data oppo_charge_data_pri;
	struct r13_sleep_info_data r13_sleep_info_data_pri;
	struct delayed_work     sleep_ratio_update_work;
	struct proc_dir_entry *power_helper_procdir;
};

struct powre_helper_init_info {
	bool power_helper_support;
	/*for charge info*/
	struct oppo_chg_chip *charge_chip;
	void (*charge_get_bat_data_func)(struct oppo_chg_chip *);
	/*for r13 info*/
	struct r13_sleep_info *r13_table;
	int table_size;
};

static struct rpmh_stats_data g_rpmh_stats[] = {
	{.name = "VMIN"},
	{.name = "APSS"},
	{.name = "MPSS"},
	{.name = "CONSS_TOP"},
	{.name = "CONSS_MCU"},
	{.name = "CONSS_BT"},
	{.name = "CONSS_WF"},
	{.name = "CONSS_GPS"},
};

static void sleep_ratio_update_work_handler(struct work_struct *work);
static struct powre_helper_data* get_power_helper_data(void)
{
	static struct powre_helper_data* power_helper_data_pri = NULL;

	if(power_helper_data_pri != NULL)
		return power_helper_data_pri;

	power_helper_data_pri = (struct powre_helper_data *)kzalloc(sizeof(struct powre_helper_data), GFP_KERNEL);
	if(power_helper_data_pri != NULL)
	{
		power_helper_data_pri->rpmh_stats_data_pri = g_rpmh_stats;
		INIT_DELAYED_WORK(&power_helper_data_pri->sleep_ratio_update_work, sleep_ratio_update_work_handler);
		power_helper_data_pri->power_helper_procdir = proc_mkdir("power_helper", NULL);
		if(!power_helper_data_pri->power_helper_procdir) {
			pr_err("%s: failed to new power_helper proc dir\n", __func__);
		}
	}

	return power_helper_data_pri;
}

static struct powre_helper_init_info* get_power_helper_init_info(void)
{
	static struct powre_helper_init_info* powre_helper_init_info_pri = NULL;
	static struct powre_helper_init_info powre_helper_init_info_instance;

	if(powre_helper_init_info_pri != NULL)
		return powre_helper_init_info_pri;

	memset(&powre_helper_init_info_instance, 0, sizeof(struct powre_helper_init_info));
	powre_helper_init_info_pri = &powre_helper_init_info_instance;

	return powre_helper_init_info_pri;
}

bool check_power_helper_support(void)
{
	return get_power_helper_init_info()->power_helper_support;
}

struct proc_dir_entry *power_helper_get_proc_dir(void)
{
	return get_power_helper_data()->power_helper_procdir;
}

static void *get_data_instance(enum power_helper_data_type data_type)
{
	void *result = NULL;
	switch (data_type) {
		case RPMH_STATS:
			result = get_power_helper_data()->rpmh_stats_data_pri;
			break;
		case BATTERY:
			result = &get_power_helper_data()->oppo_charge_data_pri;
			break;
		case R13_INFO:
			result = &get_power_helper_data()->r13_sleep_info_data_pri;
			break;
		default:
			break;
	}
	return result;
}

static void rpmh_update_stats_sleep_ratio(char *ratio_buf, unsigned int *count, u64 time_internal)
{

	u64 sleep_time;
	int ratio;
	struct rpmh_stats_data *rpmh_stats_data_instance = (struct rpmh_stats_data *)get_data_instance(RPMH_STATS);
	int i = 0, rpmh_stats_size = ARRAY_SIZE(g_rpmh_stats);
	struct rpmh_stats_data *record;
#if defined(CONFIG_MTK_ECCCI_DRIVER)
	u32 *share_mem = NULL;
	struct md_sleep_status md_data;
	memset(&md_data, 0, sizeof(struct md_sleep_status));

	share_mem = (u32 *)get_smem_start_addr(MD_SYS1,
		SMEM_USER_LOW_POWER, NULL);
	if(NULL != share_mem){
		share_mem = share_mem + 4;
		memcpy(&md_data, share_mem, sizeof(struct md_sleep_status));
	}

	rpmh_stats_data_instance[INDEX_VMIN].total_sleep_time = PCM_TICK_TO_MILLI_SEC(spm_26M_off_duration);
	rpmh_stats_data_instance[INDEX_APSS].total_sleep_time = PCM_TICK_TO_MILLI_SEC(ap_slp_duration);
	rpmh_stats_data_instance[INDEX_MPSS].total_sleep_time = PCM_TICK_TO_MILLI_SEC(md_data.sleep_time);
#else
	rpmh_stats_data_instance[INDEX_VMIN].total_sleep_time = 0;
	rpmh_stats_data_instance[INDEX_APSS].total_sleep_time = 0;
	rpmh_stats_data_instance[INDEX_MPSS].total_sleep_time = 0;
#endif

	for (i = 0; i < rpmh_stats_size; i++)
	{
		record = &rpmh_stats_data_instance[i];

		sleep_time = record->total_sleep_time - record->last_total_sleep_time;
		ratio = (sleep_time * 100)/time_internal;

		if(ratio < 0 || ratio > 100)
			ratio = -1;//invaid

		//power_helper_save_to_buf("cur: %llu last: %llu sleep: %llu internal: %llu name: %s", record->total_sleep_time,
			//record->last_total_sleep_time, sleep_time, time_internal, record->name);

		record->last_sleep_ratio = record->sleep_ratio;
		record->last_total_sleep_time = record->total_sleep_time;
		record->sleep_ratio = ratio;
		if(i > INDEX_MPSS){//clear conss sleep info
			record->last_total_sleep_time = 0;
			record->total_sleep_time = 0;
		}

		(*count) += snprintf(ratio_buf + (*count), PRINT_BUF_SIZE - (*count), "[%s %d%%] ", record->name, ratio);
	}
}

static void update_r13_sleep_ratio(char *ratio_buf, unsigned int *count, u64 time_internal)
{

	u64 nsleep_time;
	int ratio, nratio;
	struct r13_sleep_info_data *r13_sleep_info_data_instance = (struct r13_sleep_info_data *)get_data_instance(R13_INFO);
	int i = 0;
	struct r13_sleep_info *record, *last_record;
	struct r13_sleep_info *r13_table = get_power_helper_init_info()->r13_table;
	int r13_table_size = get_power_helper_init_info()->table_size;

	for (i = 0; i < r13_table_size; i++)
	{
		record = &r13_table[i];
		memcpy(&r13_sleep_info_data_instance->last_r13_sleep[i],
			&r13_sleep_info_data_instance->r13_sleep[i], sizeof(struct r13_sleep_info));//bakeup last
		memcpy(&r13_sleep_info_data_instance->r13_sleep[i], record, sizeof(struct r13_sleep_info));//get cur
	}


	for (i = 0; i < r13_table_size; i++)
	{
		record = &r13_sleep_info_data_instance->r13_sleep[i];
		last_record = &r13_sleep_info_data_instance->last_r13_sleep[i];

		nsleep_time = record->duration - last_record->duration;
		nsleep_time /= RTC_CLOCK_FREQ;
		nratio = (nsleep_time * 100)/time_internal;
		ratio = 100 - nratio;

		if(ratio < 0 || ratio > 100)
			ratio = -1;//invaid

		//power_helper_save_to_buf("cur: %llu last: %llu nsleep_time: %llu internal: %llu name: %s", record->duration/RTC_CLOCK_FREQ,
			//last_record->duration/RTC_CLOCK_FREQ, nsleep_time, time_internal, record->name);

		(*count) += snprintf(ratio_buf + (*count), PRINT_BUF_SIZE - (*count), "[%s %d%%] ", record->name + 4, ratio);
	}
}

static void sleep_ratio_update_work_handler(struct work_struct *work)
{
	static char ratio_buf[PRINT_BUF_SIZE];
	static char r13_ratio_buf[PRINT_BUF_SIZE];
	static u64 last_update_time_ms = 0;
	unsigned int count = 0, r13_count = 0;
	u64 update_time_now = ktime_to_ms(ktime_get_boottime());
	u64 time_internal = update_time_now - last_update_time_ms;

	count += snprintf(ratio_buf + count, PRINT_BUF_SIZE - count, ""LOG_TAG"[sleep_ratio]{ ");
	rpmh_update_stats_sleep_ratio(ratio_buf, &count, time_internal);
	count += snprintf(ratio_buf + count, PRINT_BUF_SIZE - count, "}");

	if(0){
		r13_count += snprintf(r13_ratio_buf + r13_count, PRINT_BUF_SIZE - r13_count, ""LOG_TAG"[r13_sleep_ratio]{ ");
		update_r13_sleep_ratio(r13_ratio_buf, &r13_count, time_internal);
		r13_count += snprintf(r13_ratio_buf + r13_count, PRINT_BUF_SIZE - r13_count, "}");
	}

	last_update_time_ms = update_time_now;

	pr_err("%s\n", ratio_buf);
	power_helper_save_to_buf("%s", ratio_buf);
	if(0){
		pr_err("%s\n", r13_ratio_buf);
		power_helper_save_to_buf("%s", r13_ratio_buf);
	}
}

void power_helper_charge_data_init(struct oppo_chg_chip *chip,
	void (*charge_get_bat_data_func)(struct oppo_chg_chip *))
{
	get_power_helper_init_info()->charge_chip = chip;
	get_power_helper_init_info()->charge_get_bat_data_func = charge_get_bat_data_func;
}

void power_helper_consys_sleep_info_update(char* info)
{
	struct rpmh_stats_data *rpmh_stats_data_instance = (struct rpmh_stats_data *)get_data_instance(RPMH_STATS);
	u64 sleep_count[CONSS_MODULE_COUNT] = {0}, sleep_time[CONSS_MODULE_COUNT] = {0};
	struct rpmh_stats_data *record;
	int i = 0;

	if(info != NULL)
		sscanf(info, CONSS_SLEEP_INFO_FORMAT,
			&sleep_count[0],  &sleep_time[0],
			&sleep_count[1],  &sleep_time[1],
			&sleep_count[2],   &sleep_time[2],
			&sleep_count[3], &sleep_time[3],
			&sleep_count[4],  &sleep_time[4]);

	for (i = 0; i < CONSS_MODULE_COUNT; i++){
		record = &rpmh_stats_data_instance[INDEX_WMT_TOP + i];
		record->total_sleep_time += PCM_TICK_TO_MILLI_SEC(sleep_time[i]);
	}
	/*for (i = 0; i < CONSS_MODULE_COUNT; i++)
	{
		record = &rpmh_stats_data_instance[INDEX_WMT_TOP + i];
		pr_err("%s: %llu %llu", record->name, sleep_time[i], record->total_sleep_time);
	}
	pr_err("\n");*/

}
EXPORT_SYMBOL(power_helper_consys_sleep_info_update);

void power_helper_r13_sleep_data_init(void *r13_table, int table_size)
{
	get_power_helper_init_info()->r13_table = (struct r13_sleep_info *)r13_table;
	get_power_helper_init_info()->table_size = table_size;
}

void power_helper_show_bat_info(void)
{

	struct oppo_charge_data *oppo_charge_data_instance = NULL;
	struct oppo_chg_chip *chip = NULL;

	if(!check_power_helper_support())
		return;

	oppo_charge_data_instance = (struct oppo_charge_data *)get_data_instance(BATTERY);
	chip = get_power_helper_init_info()->charge_chip;

	if (!chip)
	{	
		return;
	}

	if(!chip->chging_on && !chip->led_on){
		schedule_delayed_work(&oppo_charge_data_instance->battery_info_show_work, 0);
	}
}


static int caculate_subsys_sleep_ratio(void)
{
	schedule_delayed_work(&get_power_helper_data()->sleep_ratio_update_work, 0);
	return 0;
}

static void show_battery_info_for_wakeup(struct work_struct *work)
{

	char log_buf[PRINT_BUF_SIZE];
	int count = 0;
	struct oppo_charge_data *oppo_charge_data_instance = (struct oppo_charge_data *)get_data_instance(BATTERY);
	struct oppo_chg_chip *chip = get_power_helper_init_info()->charge_chip;

	if(!chip->chging_on && !chip->led_on)
	{
		(get_power_helper_init_info()->charge_get_bat_data_func)(chip);
		count += snprintf(log_buf+count, PRINT_BUF_SIZE-count, BAT_INFO_FORMAT,
			chip->batt_volt, chip->icharging, chip->temperature, chip->soc, chip->ui_soc, chip->batt_fcc, chip->batt_rm);
		power_helper_save_to_buf("%s", log_buf);
		count += power_helper_get_utc_time(log_buf+count, PRINT_BUF_SIZE-count);

		if(oppo_charge_data_instance->last_ui_soc != chip->ui_soc)
			caculate_subsys_sleep_ratio();

		oppo_charge_data_instance->last_ui_soc = chip->ui_soc;

		log_buf[count] = '\0';
		pr_err("%s\n", log_buf);
	}
}

static struct of_device_id power_helper_of_match[] = {
	{.compatible = "oplus,power-helper"},
	{},
};

extern void power_ultil_exit(void);
extern int power_ultil_init(void);
static int power_helper_probe(struct platform_device *pdev)
{

	struct oppo_charge_data *oppo_charge_data_instance = (struct oppo_charge_data *)get_data_instance(BATTERY);

	INIT_DELAYED_WORK(&oppo_charge_data_instance->battery_info_show_work, show_battery_info_for_wakeup);

	power_ultil_init();
	get_power_helper_init_info()->power_helper_support = true;
	return 0;
}

static int power_helper_remove(struct platform_device *pdev)
{
	struct powre_helper_data *powre_helper_data_pri = get_power_helper_data();
	kfree(powre_helper_data_pri);
	power_ultil_exit();

	return 0;
}

static struct platform_driver power_helper_driver = {
	.driver = {
		.name		 = "power-helper",
		.owner 	 = THIS_MODULE,
		.of_match_table = power_helper_of_match,
	},
	.probe  = power_helper_probe,
	.remove = power_helper_remove,
};

static int __init power_helper_init(void)
{
	int ret;

	ret = platform_driver_register(&power_helper_driver);
	if (ret)
	{
		pr_err("%s: platform_driver_register failed!\n", __func__);
		return ret;
	}
	return 0;
}

static void __exit power_helper_exit(void)
{
	platform_driver_unregister(&power_helper_driver);
}

module_init(power_helper_init);
module_exit(power_helper_exit);
MODULE_LICENSE("GPL");
#else
void power_helper_charge_data_init(struct oppo_chg_chip *chip, void (*charge_get_bat_data_func)(struct oppo_chg_chip *)){}
void power_helper_show_bat_info(void){}
struct proc_dir_entry *power_helper_get_proc_dir(void){return NULL;}
bool check_power_helper_support(void){return false;}
void power_helper_r13_sleep_data_init(void *r13_table, int table_size){}
void power_helper_consys_sleep_info_update(char* info){}
#endif
