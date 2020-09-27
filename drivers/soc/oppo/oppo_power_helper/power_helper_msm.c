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
#include <linux/soc/qcom/smem.h>
#include <linux/module.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/log2.h>
#include <linux/regmap.h>
#include <linux/file.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <linux/power/power_helper.h>
#include "../../../../../kernel/msm-4.19/drivers/power/oppo/oppo_charger.h"
#include <linux/power/power_helper_utils.h>

#ifdef CONFIG_POWER_HELPER_SUPPORT

#define MSM_ARCH_TIMER_FREQ 19200000
#define PRINT_BUF_SIZE 640
#define RPMH_STATS_MAX_NUM 5
#define WLAN_WAKEUP_INFO_SIZE 32
#define IPV6_ADDR_STR "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"
#define IPV4_PRINT_FORMAT "IP[%d:%d:%d:%d(%d)->%d:%d:%d:%d(%d)]"
#define IPV6_PRINT_FORMAT "IP["IPV6_ADDR_STR"(%d)->"IPV6_ADDR_STR"(%d)]"
#define MPSS_STATS_FORMAT "\tVersion: 0x%x\n\tSleep Count:0x%x\n\tSleep Last Entered At: 0x%lx\n\tSleep Last Exited At: 0x%lx\n\tSleep Accumulated Duration: 0x%lx\n"
#define LOG_TAG "[power_helper]"
#define BAT_INFO_FORMAT ""LOG_TAG"[bat_info][vol %d, cur %d, temp %d, soc %d, ui_soc %d, fcc %d, rm %d]"

#define LOW_SLEEP_RATIO_TIME_THRESHOLD (3 * 3600 *1000)//3h
#define LOW_SLEEP_RATIO_COUNT_THRESHOLD (3)//3 soc change
#define LOW_SLEEP_RATIO_TARGETS (2)
#define LOW_SLEEP_RATIO_THRESHOLD (10)
#define LOW_SLEEP_RATIO_UPDATE_MIN_TIME (5*60*1000)//5min

#define PON_KPDPWR_S1_TIMER(pon) ((pon)->base + 0x40)
#define PON_KPDPWR_S2_TIMER(pon) ((pon)->base + 0x41)
#define PON_KPDPWR_S2_CNTL(pon)  ((pon)->base + 0x42)
#define PON_KPDPWR_S2_CNTL2(pon) ((pon)->base + 0x43)
#define PON_S1_COUNT_MAX_NUM     0xF
#define PON_S2_CNTL_EN           BIT(7)
#define PON_S1_TIMER_MASK        (0xF)
#define PON_S2_TIMER_MASK        (0x7)
#define PON_S2_CNTL_TYPE_MASK    (0xF)
static u32 s1_delay_array[PON_S1_COUNT_MAX_NUM + 1] = {
	0, 32, 56, 80, 138, 184, 272, 408, 608, 904, 1352, 2048, 3072, 4480,
	6720, 10256
};

enum rpmh_master_smem_id {
	UNDEFINE = -1,
	MPSS = 605,
	ADSP,
	CDSP,
	SLPI,
	GPU,
	DISPLAY,
	SLPI_ISLAND = 613,
};

enum rpmh_master_pid {
	PID_UNDEFINE = -1,
	PID_APSS = 0,
	PID_MPSS = 1,
	PID_ADSP = 2,
	PID_SLPI = 3,
	PID_CDSP = 5,
	PID_GPU = PID_APSS,
	PID_DISPLAY = PID_APSS,
};

enum rpmh_master_index {
	INDEX_APSS = 0,
	INDEX_MPSS_EXT = 1,
	INDEX_MPSS = 2,
	INDEX_ADSP = 3,
	INDEX_ADSP_ISLAND = 4,
	INDEX_CDSP = 5,
	INDEX_SLPI = 6,
	INDEX_SLPI_ISLAND = 7,
	INDEX_GPU = 8,
	INDEX_DISPLAY = 9,
};

enum rpmh_stats_index {
	INDEX_AOSD = 0,
	INDEX_CXSD = 1,
	INDEX_DDR = 2,
};

enum power_helper_data_type {
	RPMH_STATS = 0,
	RPMH_MASTER = 1,
	BATTERY = 2,
	WLAN = 3,
	RAMDUMP = 4,
};

enum power_helper_ramdump_type {
	SLEEP_RAMDUMP = 1,
	AUTO_RAMDUMP = 2,
};

struct rpmh_sleep_ratio {
	int sleep_ratio;
	int last_sleep_ratio;
	uint64_t low_sleep_ratio_time;
	uint64_t low_sleep_ratio_count;
};

struct rpmh_master_stats {
	uint32_t version_id;
	uint32_t counts;
	uint64_t last_entered;
	uint64_t last_exited;
	uint64_t accumulated_duration;
};

struct rpmh_master_data {
	char *master_name;
	enum rpmh_master_smem_id smem_id;
	enum rpmh_master_pid pid;
	struct rpmh_master_stats master_stats;
	struct rpmh_master_stats last_master_stats;
	struct rpmh_sleep_ratio master_sleep_ratio;
	bool invaid;
};

struct rpmh_stats_desc {
	u32 stat_type;
	u32 count;
	u64 last_entered_at;
	u64 last_exited_at;
	u64 accumulated;
#if defined(CONFIG_MSM_RPM_SMD)
	u32 client_votes;
	u32 reserved[3];
#endif
};

struct rpmh_stats_data {
	struct rpmh_stats_desc rpmh_stats[RPMH_STATS_MAX_NUM];
	struct rpmh_stats_desc last_rpmh_stats[RPMH_STATS_MAX_NUM];
	struct rpmh_sleep_ratio stats_sleep_ratio[RPMH_STATS_MAX_NUM];
};

struct oppo_charge_data {
	struct delayed_work        battery_info_show_work;
	int last_ui_soc;
};

enum wlan_wakeup_info_type {
	PROTOCOL = 0,
	REASON = 1,
	IP = 2,
};

struct wlan_wakeup_info {
	char protocol[WLAN_WAKEUP_INFO_SIZE];
	char reason[WLAN_WAKEUP_INFO_SIZE];
	char src_ip[WLAN_WAKEUP_INFO_SIZE];
	char dst_ip[WLAN_WAKEUP_INFO_SIZE];
	uint16_t src_port;
	uint16_t dst_port;
	int is_ipv4;
};

struct pon_config {
	u32 pon_type;
	u32 support_reset;
	u32 key_code;
	u32 s1_timer;
	u32 s2_timer;
	u32 s2_type;
	bool pull_up;
	int state_irq;
	int bark_irq;
	u16 s2_cntl_addr;
	u16 s2_cntl2_addr;
	bool old_state;
	bool use_bark;
	bool config_reset;
};

struct ramdump_data {
	bool   auto_ramdump_enable;
};

struct powre_helper_data {
	struct rpmh_master_data *rpmh_master_data_pri;
	struct rpmh_stats_data  rpmh_stats_data_pri;
	struct oppo_charge_data oppo_charge_data_pri;
	struct wlan_wakeup_info wlan_wakeup_info_pri;
	struct ramdump_data ramdump_data_pri;
	struct delayed_work     sleep_ratio_update_work;
	struct proc_dir_entry *power_helper_procdir;
};

struct powre_helper_init_info {
	bool power_helper_support;
	/*for master stats*/
	void *apps_master_stats;
	/*for rpmh stats*/
	void __iomem *reg_base;
	phys_addr_t phys_addr_base;
	u32 phys_size;
	u32 num_records;
	/*for charge info*/
	struct oppo_chg_chip *charge_chip;
	void (*charge_get_bat_data_func)(struct oppo_chg_chip*);
	/*for ramdump*/
	struct regmap *reg;
	u16 base;
	void *cfg;
};

static struct rpmh_master_data g_rpmh_masters[] = {
	{.master_name = "APSS",			.smem_id = UNDEFINE,	.pid = PID_UNDEFINE,	.invaid = false},
	{.master_name = "MPSS_EXT",		.smem_id = UNDEFINE,	.pid = PID_UNDEFINE,	.invaid = false},
	{.master_name = "MPSS",			.smem_id = MPSS,		.pid = PID_MPSS,		.invaid = false},
	{.master_name = "ADSP",			.smem_id = ADSP,		.pid = PID_ADSP,		.invaid = false},
	{.master_name = "ADSP_ISLAND",	.smem_id = SLPI_ISLAND,	.pid = PID_ADSP,		.invaid = false},
	{.master_name = "CDSP",			.smem_id = CDSP,		.pid = PID_CDSP,		.invaid = false},
	{.master_name = "SLPI",			.smem_id = SLPI,		.pid = PID_SLPI,		.invaid = false},
	{.master_name = "SLPI_ISLAND",	.smem_id = SLPI_ISLAND,	.pid = PID_SLPI,		.invaid = false},
	{.master_name = "GPU",			.smem_id = GPU,			.pid = PID_GPU,			.invaid = false},
	{.master_name = "DISPLAY",		.smem_id = DISPLAY,		.pid = PID_DISPLAY,		.invaid = false},
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
		power_helper_data_pri->rpmh_master_data_pri = g_rpmh_masters;
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
			result = &get_power_helper_data()->rpmh_stats_data_pri;
			break;
		case RPMH_MASTER:
			result = get_power_helper_data()->rpmh_master_data_pri;
			break;
		case BATTERY:
			result = &get_power_helper_data()->oppo_charge_data_pri;
			break;
		case WLAN:
			result = &get_power_helper_data()->wlan_wakeup_info_pri;
			break;
		case RAMDUMP:
			result = &get_power_helper_data()->ramdump_data_pri;
			break;
		default:
			break;
	}
	return result;
}

static inline u64 get_time_in_msec(u64 counter)
{
	do_div(counter, (MSM_ARCH_TIMER_FREQ/MSEC_PER_SEC));
	return counter;
}

static bool is_ext_mpss(void)
{
	static int is_ext_mpss = -1;
	size_t size = 0;
	struct rpmh_master_stats *ext_mpss_master = NULL;
	struct rpmh_master_data *rpmh_master_data_instance = NULL;

	if(is_ext_mpss != -1)
		return !!is_ext_mpss;

	rpmh_master_data_instance = (struct rpmh_master_data *)get_data_instance(RPMH_MASTER);
	ext_mpss_master = (struct rpmh_master_stats *)qcom_smem_get(rpmh_master_data_instance[INDEX_MPSS].pid, rpmh_master_data_instance[INDEX_MPSS].smem_id, &size);
	if (!IS_ERR_OR_NULL(ext_mpss_master))
		is_ext_mpss = 0;
	else
		is_ext_mpss = 1;

	if(!!is_ext_mpss)
		rpmh_master_data_instance[INDEX_MPSS_EXT].invaid = true;

	return !!is_ext_mpss;
}

static int caculate_subsys_sleep_ratio(bool is_from_ext_mpss);
void power_helper_update_ext_mpss_master_stats(char *sleep_info)
{
	struct rpmh_master_data *rpmh_master_data_instance = NULL;
	struct rpmh_master_stats *ext_mpss_master = NULL, *last_ext_mpss_master = NULL;
	static u64 last_update_time = 0;
	u64 update_time_now = 0;
	bool is_freq_update = false;

	if(!check_power_helper_support())
		return;

	if(sleep_info == NULL || !is_ext_mpss())
		return;

	rpmh_master_data_instance = (struct rpmh_master_data *)get_data_instance(RPMH_MASTER);
	ext_mpss_master = &rpmh_master_data_instance[INDEX_MPSS_EXT].master_stats;
	last_ext_mpss_master = &rpmh_master_data_instance[INDEX_MPSS_EXT].last_master_stats;

	update_time_now = ktime_to_ms(ktime_get_boottime());
	is_freq_update = (update_time_now - last_update_time < LOW_SLEEP_RATIO_UPDATE_MIN_TIME);

	if(!is_freq_update)
		memcpy(last_ext_mpss_master, ext_mpss_master, sizeof(struct rpmh_master_stats));//bakeup last
	sscanf(sleep_info, MPSS_STATS_FORMAT,
		&ext_mpss_master->version_id,
		&ext_mpss_master->counts,
		&ext_mpss_master->last_entered,
		&ext_mpss_master->last_exited,
		&ext_mpss_master->accumulated_duration);

	if(is_freq_update)
	{
		last_update_time = update_time_now;
		return;
	}

	last_update_time = update_time_now;

	power_helper_save_to_buf("%s last_counts: 0x%x last_time: %llu counts: 0x%x time: %llu",
		__func__, last_ext_mpss_master->counts,
		get_time_in_msec(last_ext_mpss_master->accumulated_duration),
		ext_mpss_master->counts,
		get_time_in_msec(ext_mpss_master->accumulated_duration));

	caculate_subsys_sleep_ratio(true);
}

static inline u32 rpmstats_read_long_register(void __iomem *regbase, int index, int offset)
{
	return readl_relaxed(regbase + offset + index * sizeof(struct rpmh_stats_desc));
}

static inline u64 rpmstats_read_quad_register(void __iomem *regbase, int index, int offset)
{
	u64 dst;
	memcpy_fromio(&dst, regbase + offset + index * sizeof(struct rpmh_stats_desc), 8);
	return dst;
}

static void rpmh_update_stats_sleep_ratio(char *ratio_buf, unsigned int *count, u64 time_internal)
{
	int i;
	u64 sleep_time;
	int ratio;
	char stat_type[5];

	struct rpmh_stats_desc *ptr_rpmh_stats, *ptr_last_rpmh_stats;
	struct rpmh_stats_data *rpmh_stats_data_instance = (struct rpmh_stats_data *)get_data_instance(RPMH_STATS);
	struct powre_helper_init_info *powre_helper_init_info_instance = get_power_helper_init_info();

	if(powre_helper_init_info_instance->reg_base == NULL)
	{
		powre_helper_init_info_instance->reg_base= ioremap_nocache(powre_helper_init_info_instance->phys_addr_base,
			powre_helper_init_info_instance->phys_size);
		if (!powre_helper_init_info_instance->reg_base) {
			pr_err("%s: ERROR could not ioremap start=%pa, len=%u\n",
				__func__, &powre_helper_init_info_instance->phys_addr_base, powre_helper_init_info_instance->phys_size);
			return;
		}
	}

	for (i = 0; i < powre_helper_init_info_instance->num_records; i++)
	{
		ptr_rpmh_stats = &rpmh_stats_data_instance->rpmh_stats[i];
		ptr_last_rpmh_stats = &rpmh_stats_data_instance->last_rpmh_stats[i];
		memcpy(ptr_last_rpmh_stats, ptr_rpmh_stats, sizeof(struct rpmh_stats_desc));//bakeup last
		ptr_rpmh_stats->stat_type = rpmstats_read_long_register(powre_helper_init_info_instance->reg_base,
			i, offsetof(struct rpmh_stats_desc, stat_type));
		ptr_rpmh_stats->count =     rpmstats_read_long_register(powre_helper_init_info_instance->reg_base,
			i, offsetof(struct rpmh_stats_desc, count));
		ptr_rpmh_stats->accumulated =  rpmstats_read_quad_register(powre_helper_init_info_instance->reg_base,
			i, offsetof(struct rpmh_stats_desc, accumulated));

		sleep_time = get_time_in_msec(ptr_rpmh_stats->accumulated)
			- get_time_in_msec(ptr_last_rpmh_stats->accumulated);
		ratio = (sleep_time * 100)/time_internal;
		if(ratio < 0 || ratio > 100)
			ratio = -1;//invaid
		rpmh_stats_data_instance->stats_sleep_ratio[i].last_sleep_ratio =
			rpmh_stats_data_instance->stats_sleep_ratio[i].sleep_ratio;
		rpmh_stats_data_instance->stats_sleep_ratio[i].sleep_ratio = ratio;
		
		stat_type[4] = 0;
		memcpy(stat_type, &ptr_rpmh_stats->stat_type, sizeof(u32));
		(*count) += snprintf(ratio_buf + (*count), PRINT_BUF_SIZE - (*count), "[%s %d%%] ",
		stat_type, ratio);
	}
}

static void rpmh_update_master_sleep_ratio(char *ratio_buf, unsigned int *count, u64 time_internal)
{

	u64 sleep_time;
	int ratio;
	size_t size = 0;
	struct rpmh_master_stats *record = NULL;
	struct rpmh_master_stats *last_record = NULL;
	struct rpmh_master_data *rpmh_master_data_instance = (struct rpmh_master_data *)get_data_instance(RPMH_MASTER);
	struct rpmh_master_stats *apss_master = (struct rpmh_master_stats *)(get_power_helper_init_info()->apps_master_stats);
	int i = 0, rpmh_masters_size = ARRAY_SIZE(g_rpmh_masters);

	if(apss_master == NULL)
	{
		pr_err("power_helper: rpmh master not init!\n");
		return;
	}

	memcpy(&rpmh_master_data_instance[INDEX_APSS].last_master_stats,
		&rpmh_master_data_instance[INDEX_APSS].master_stats, sizeof(struct rpmh_master_stats));//bakeup last
	memcpy(&rpmh_master_data_instance[INDEX_APSS].master_stats, apss_master, sizeof(struct rpmh_master_stats));
	rpmh_master_data_instance[INDEX_APSS].invaid = true;

	for (i = 2; i < rpmh_masters_size; i++)
	{
		record = (struct rpmh_master_stats *)qcom_smem_get(rpmh_master_data_instance[i].pid, rpmh_master_data_instance[i].smem_id, &size);
		if (!IS_ERR_OR_NULL(record))
		{
			memcpy(&rpmh_master_data_instance[i].last_master_stats,
				&rpmh_master_data_instance[i].master_stats, sizeof(struct rpmh_master_stats));//bakeup last
			memcpy(&rpmh_master_data_instance[i].master_stats, record, sizeof(struct rpmh_master_stats));
			rpmh_master_data_instance[i].invaid = true;
		}
		else
			rpmh_master_data_instance[i].invaid = false;
	}

	for (i = 0; i < rpmh_masters_size; i++)
	{
		record = &rpmh_master_data_instance[i].master_stats;
		last_record = &rpmh_master_data_instance[i].last_master_stats;

		if(!rpmh_master_data_instance[i].invaid)
			continue;

		if ((record->last_entered > record->last_exited) && (i != INDEX_MPSS_EXT))
			record->accumulated_duration += (arch_counter_get_cntvct() - record->last_entered);

		sleep_time = get_time_in_msec(record->accumulated_duration)
			- get_time_in_msec(last_record->accumulated_duration);
		ratio = (sleep_time * 100)/time_internal;
		if(ratio < 0 || ratio > 100)
			ratio = -1;//invaid
		power_helper_save_to_buf("cur: %llu last: %llu sleep: %llu internal: %llu name: %s", get_time_in_msec(record->accumulated_duration),
			get_time_in_msec(last_record->accumulated_duration), sleep_time, time_internal, rpmh_master_data_instance[i].master_name);
		rpmh_master_data_instance[i].master_sleep_ratio.last_sleep_ratio =
			rpmh_master_data_instance[i].master_sleep_ratio.sleep_ratio;
		rpmh_master_data_instance[i].master_sleep_ratio.sleep_ratio = ratio;
		(*count) += snprintf(ratio_buf + (*count), PRINT_BUF_SIZE - (*count), "[%s %d%%] ",
		rpmh_master_data_instance[i].master_name, ratio);
	}
}

static void trigger_ramdump_if_needed(u64 time_internal)
{
	struct rpmh_sleep_ratio* sleep_ratio_targets[LOW_SLEEP_RATIO_TARGETS];
	int i, sleep_ratio, last_sleep_ratio;
	struct rpmh_stats_data *rpmh_stats_data_instance = (struct rpmh_stats_data *)get_data_instance(RPMH_STATS);
	struct rpmh_master_data *rpmh_master_data_instance = (struct rpmh_master_data *)get_data_instance(RPMH_MASTER);
	struct ramdump_data *ramdump_data_instance = (struct ramdump_data *)get_data_instance(RAMDUMP);
	sleep_ratio_targets[0] = &rpmh_stats_data_instance->stats_sleep_ratio[INDEX_CXSD];
	sleep_ratio_targets[1] = &rpmh_master_data_instance[INDEX_MPSS_EXT].master_sleep_ratio;

	if(!ramdump_data_instance->auto_ramdump_enable)
		return;

	for(i = 0;i < LOW_SLEEP_RATIO_TARGETS; i++)
	{
		sleep_ratio = sleep_ratio_targets[i]->sleep_ratio;
		last_sleep_ratio = sleep_ratio_targets[i]->last_sleep_ratio;

		if(((last_sleep_ratio < LOW_SLEEP_RATIO_THRESHOLD) && (last_sleep_ratio > 0))
			&& ((sleep_ratio < LOW_SLEEP_RATIO_THRESHOLD)  && (sleep_ratio > 0)))
		{
			sleep_ratio_targets[i]->low_sleep_ratio_count++;
			sleep_ratio_targets[i]->low_sleep_ratio_time += time_internal;
		}
		else
		{
			sleep_ratio_targets[i]->low_sleep_ratio_count = 0;
			sleep_ratio_targets[i]->low_sleep_ratio_time = 0;
		}

		if((sleep_ratio_targets[i]->low_sleep_ratio_count >= LOW_SLEEP_RATIO_COUNT_THRESHOLD)
			&&(sleep_ratio_targets[i]->low_sleep_ratio_time >= LOW_SLEEP_RATIO_TIME_THRESHOLD))
		{
			pr_err("trigger ramdump by low sleep ratio by the %d target!!!\n", i);
			BUG();
		}
	}
}

static void sleep_ratio_update_work_handler(struct work_struct *work)
{
	static char ratio_buf[PRINT_BUF_SIZE];
	static u64 last_update_time_ms = 0;
	unsigned int count = 0;
	u64 update_time_now = ktime_to_ms(ktime_get_boottime());
	u64 time_internal = update_time_now - last_update_time_ms;

	count += snprintf(ratio_buf + count, PRINT_BUF_SIZE - count, ""LOG_TAG"[sleep_ratio]{ ");
	rpmh_update_stats_sleep_ratio(ratio_buf, &count, time_internal);
	rpmh_update_master_sleep_ratio(ratio_buf, &count, time_internal);
	count += snprintf(ratio_buf + count, PRINT_BUF_SIZE - count, "}");

	trigger_ramdump_if_needed(time_internal);

	last_update_time_ms = update_time_now;

	pr_err("%s\n", ratio_buf);
	power_helper_save_to_buf("%s", ratio_buf);
}

void power_helper_set_apss_master_stats(void *apss_master)
{
	get_power_helper_init_info()->apps_master_stats = (struct rpmh_master_stats *)apss_master;
}

void power_helper_rpmh_stats_data_init(phys_addr_t phys_addr_base, u32 phys_size, u32 num_records)
{
	struct powre_helper_init_info *powre_helper_init_info_instance = get_power_helper_init_info();
	powre_helper_init_info_instance->phys_addr_base = phys_addr_base;
	powre_helper_init_info_instance->phys_size      = phys_size;
	powre_helper_init_info_instance->num_records    = num_records;
}

void power_helper_charge_data_init(struct oppo_chg_chip *chip,
	void (*charge_get_bat_data_func)(struct oppo_chg_chip *))
{
	get_power_helper_init_info()->charge_chip = chip;
	get_power_helper_init_info()->charge_get_bat_data_func = charge_get_bat_data_func;
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


static int caculate_subsys_sleep_ratio(bool is_from_ext_mpss)
{
	if(is_from_ext_mpss != is_ext_mpss())
		return -1;

	schedule_delayed_work(&get_power_helper_data()->sleep_ratio_update_work, round_jiffies_relative(msecs_to_jiffies(1000)));
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
			caculate_subsys_sleep_ratio(false);

		oppo_charge_data_instance->last_ui_soc = chip->ui_soc;

		log_buf[count] = '\0';
		pr_err("%s\n", log_buf);
	}
}

void power_helper_wlan_wakup_info_show(void)
{

	char host_wakeup_info[PRINT_BUF_SIZE];
	int info_count = 0;
	struct wlan_wakeup_info *wlan_wakeup_info_instance = NULL;
	char *src_ip = NULL, *dst_ip = NULL;

	if(!check_power_helper_support())
		return;

	wlan_wakeup_info_instance = (struct wlan_wakeup_info *)get_data_instance(WLAN);
	src_ip = wlan_wakeup_info_instance->src_ip;
	dst_ip = wlan_wakeup_info_instance->dst_ip;

	if(!strncmp(wlan_wakeup_info_instance->reason, "UNSPECIFIED", WLAN_WAKEUP_INFO_SIZE)
		|| !strncmp(wlan_wakeup_info_instance->reason, "", WLAN_WAKEUP_INFO_SIZE))
		return;

	info_count += snprintf(host_wakeup_info+info_count, PRINT_BUF_SIZE-info_count, ""LOG_TAG"[wlan_wakeup] ",
		wlan_wakeup_info_instance->reason);

	info_count += snprintf(host_wakeup_info+info_count, PRINT_BUF_SIZE-info_count, "reason[%s] ",
		wlan_wakeup_info_instance->reason);

	info_count += snprintf(host_wakeup_info+info_count, PRINT_BUF_SIZE-info_count, "protocol[%s] ",
		wlan_wakeup_info_instance->protocol);

	if(src_ip != NULL && dst_ip!= NULL){
		if(wlan_wakeup_info_instance->is_ipv4)
			info_count += snprintf(host_wakeup_info+info_count, PRINT_BUF_SIZE-info_count, IPV4_PRINT_FORMAT,
				src_ip[0], src_ip[1],src_ip[2], src_ip[3], wlan_wakeup_info_instance->src_port,
				dst_ip[0], dst_ip[1],dst_ip[2], dst_ip[3], wlan_wakeup_info_instance->dst_port);
		else
			info_count += snprintf(host_wakeup_info+info_count, PRINT_BUF_SIZE-info_count, IPV6_PRINT_FORMAT,
				src_ip[0],src_ip[1], src_ip[2], src_ip[3], src_ip[4],
				src_ip[5], src_ip[6], src_ip[7], src_ip[8],src_ip[9],
				src_ip[10], src_ip[11],src_ip[12], src_ip[13], src_ip[14],src_ip[15], wlan_wakeup_info_instance->src_port,
				dst_ip[0],dst_ip[1], dst_ip[2], dst_ip[3], dst_ip[4],
				dst_ip[5], dst_ip[6], dst_ip[7], dst_ip[8],dst_ip[9],
				dst_ip[10], dst_ip[11],dst_ip[12], dst_ip[13], dst_ip[14],dst_ip[15], wlan_wakeup_info_instance->dst_port);
	}
	host_wakeup_info[info_count] = '\0';
	pr_err("%s\n", host_wakeup_info);
	power_helper_save_to_buf("%s", host_wakeup_info);
}
EXPORT_SYMBOL(power_helper_wlan_wakup_info_show);

void power_helper_get_wlan_wakup_info(int type, const char *info1, const char *info2, uint32_t pri1, int ipv4)
{
	struct wlan_wakeup_info *wlan_wakeup_info_instance = NULL;

	if(!check_power_helper_support())
		return;

	wlan_wakeup_info_instance = (struct wlan_wakeup_info *)get_data_instance(WLAN);
	switch (type) {
		case PROTOCOL:
			if(NULL != info1){
				strncpy(wlan_wakeup_info_instance->protocol, info1, WLAN_WAKEUP_INFO_SIZE);
				wlan_wakeup_info_instance->protocol[WLAN_WAKEUP_INFO_SIZE-1] = '\0';
			}
			break;
		case REASON:
			if(NULL != info1){
				strncpy(wlan_wakeup_info_instance->reason, info1, WLAN_WAKEUP_INFO_SIZE);
				wlan_wakeup_info_instance->reason[WLAN_WAKEUP_INFO_SIZE-1] = '\0';
			}
			break;
		case IP:
			if(NULL != info1 && NULL != info2){
				strncpy(wlan_wakeup_info_instance->src_ip, info1, WLAN_WAKEUP_INFO_SIZE);
				strncpy(wlan_wakeup_info_instance->dst_ip, info2, WLAN_WAKEUP_INFO_SIZE);
				wlan_wakeup_info_instance->src_ip[WLAN_WAKEUP_INFO_SIZE-1] = '\0';
				wlan_wakeup_info_instance->dst_ip[WLAN_WAKEUP_INFO_SIZE-1] = '\0';
				wlan_wakeup_info_instance->src_port = ((pri1 >> 16) & 0xffff);
				wlan_wakeup_info_instance->dst_port = (pri1 & 0xffff);
				wlan_wakeup_info_instance->is_ipv4 = ipv4;
			}
			break;
		default:
			break;
	}
}
EXPORT_SYMBOL(power_helper_get_wlan_wakup_info);

void power_helper_get_pon_info(struct regmap *reg, u16 base, void *cfg)
{
	if(NULL == cfg)
		return;//not ready

	get_power_helper_init_info()->reg  = reg;
	get_power_helper_init_info()->base = base;
	get_power_helper_init_info()->cfg  = cfg;
}

static int ramdump_helper_show(struct seq_file *m, void *v)
{
	struct powre_helper_init_info *powre_helper_init_info_instance = get_power_helper_init_info();
	int rc = -1;
	u32 s1_timer = -1, s2_timer = -1, s2_type = -1, en_val = -1;
	char show_buf[64];

	if(NULL == powre_helper_init_info_instance->reg){
		return rc;
	}

	rc = regmap_read(powre_helper_init_info_instance->reg, PON_KPDPWR_S1_TIMER(powre_helper_init_info_instance), &s1_timer);
	if (rc) {
		pr_err("Unable to read PON KPDPWR_S1_TIMER\n");
		return rc;
	}

	rc = regmap_read(powre_helper_init_info_instance->reg, PON_KPDPWR_S2_TIMER(powre_helper_init_info_instance), &s2_timer);
	if (rc) {
		pr_err("Unable to read PON KPDPWR_S2_TIMER\n");
		return rc;
	}

	rc = regmap_read(powre_helper_init_info_instance->reg, PON_KPDPWR_S2_CNTL(powre_helper_init_info_instance), &s2_type);
	if (rc) {
		pr_err("Unable to read PON KPDPWR_S2_CNTL\n");
		return rc;
	}

	rc = regmap_read(powre_helper_init_info_instance->reg, PON_KPDPWR_S2_CNTL2(powre_helper_init_info_instance), &en_val);
	if (rc) {
		pr_err("Unable to read PON KPDPWR_S2_CNTL2\n");
		return rc;
	}
	snprintf(show_buf, 64, "s1_timer: %d s2_timer: %d s2_type: %d en_val: %d\n", s1_timer,
			s2_timer, s2_type, en_val);

	seq_printf(m, "%s", show_buf);

	return 0;
}


static int pon_masked_write(struct powre_helper_init_info *powre_helper_init_info_pri, u16 addr, u8 mask, u8 val)
{
	int rc;

	rc = regmap_update_bits(powre_helper_init_info_pri->reg, addr, mask, val);
	if (rc)
		pr_err("Register write failed, addr=0x%04X, rc=%d\n",
			addr, rc);
	return rc;
}

static int pon_config_reset(struct powre_helper_init_info *powre_helper_init_info_pri)
{
	u16 s1_timer_addr = PON_KPDPWR_S1_TIMER(powre_helper_init_info_pri);
	u16 s2_timer_addr = PON_KPDPWR_S2_TIMER(powre_helper_init_info_pri);
	int rc;
	u8 i;
	struct pon_config *cfg = (struct pon_config *)(powre_helper_init_info_pri->cfg);

	/* Disable S2 reset */
	rc = pon_masked_write(powre_helper_init_info_pri, cfg->s2_cntl2_addr, PON_S2_CNTL_EN, 0);
	if (rc)
		return rc;

	usleep_range(100, 120);

	/* configure s1 timer, s2 timer and reset type */
	for (i = 0; i < PON_S1_COUNT_MAX_NUM + 1; i++) {
		if (cfg->s1_timer <= s1_delay_array[i])
			break;
	}
	rc = pon_masked_write(powre_helper_init_info_pri, s1_timer_addr, PON_S1_TIMER_MASK, i);
	if (rc)
		return rc;

	i = 0;
	if (cfg->s2_timer) {
		i = cfg->s2_timer / 10;
		i = ilog2(i + 1);
	}

	rc = pon_masked_write(powre_helper_init_info_pri, s2_timer_addr, PON_S2_TIMER_MASK, i);
	if (rc)
		return rc;

	rc = pon_masked_write(powre_helper_init_info_pri, cfg->s2_cntl_addr, PON_S2_CNTL_TYPE_MASK, (u8)cfg->s2_type);
	if (rc)
		return rc;

	/* Enable S2 reset */
	return pon_masked_write(powre_helper_init_info_pri, cfg->s2_cntl2_addr, PON_S2_CNTL_EN, PON_S2_CNTL_EN);
}

extern void oppo_switch_fulldump(int on);
static ssize_t ramdump_helper_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct ramdump_data *ramdump_data_instance = (struct ramdump_data *)get_data_instance(RAMDUMP);
	struct powre_helper_init_info *powre_helper_init_info_instance = get_power_helper_init_info();
	char write_info[64] = {0};
	u32 value;
	struct pon_config *cfg = NULL;

	if (copy_from_user(write_info, buf, count))
		return -EFAULT;

	kstrtou32(write_info, 10, &value);

	if(NULL == powre_helper_init_info_instance->reg){
		return EFAULT;
	}

	if(SLEEP_RAMDUMP == value)
	{
		cfg = (struct pon_config *)(powre_helper_init_info_instance->cfg);
		if(NULL != cfg){
			if (cfg->key_code) {
				disable_irq(cfg->state_irq);
				pr_info("ramdump_helper_store disable_irq for state_irq\n");
				if (cfg->support_reset){
					disable_irq(cfg->bark_irq);
					pr_info("ramdump_helper_store disable_irq for bark_irq\n");
				}
			}
			cfg->s1_timer = 1000;
			cfg->s2_timer = 1000;
			cfg->s2_type  = 1;//warm reset
			pon_config_reset(powre_helper_init_info_instance);
		}else
			pr_err("ramdump_helper_store cfg is null\n");
	}else if(AUTO_RAMDUMP == value){
		ramdump_data_instance->auto_ramdump_enable = true;
	}
	oppo_switch_fulldump(true);
	return count;
}

static int ramdump_helper_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, ramdump_helper_show, PDE_DATA(inode));
}

static const struct file_operations ramdump_helper_fops = {
	.open		= ramdump_helper_proc_open,
	.read		= seq_read,
	.llseek 	= seq_lseek,
	.release	= single_release,
	.write		= ramdump_helper_write,
};

static struct of_device_id power_helper_of_match[] = {
	{.compatible = "oplus,power-helper"},
	{},
};

extern void power_ultil_exit(void);
extern int power_ultil_init(void);
static int power_helper_probe(struct platform_device *pdev)
{

	struct oppo_charge_data *oppo_charge_data_instance = (struct oppo_charge_data *)get_data_instance(BATTERY);
	struct ramdump_data *ramdump_data_instance = (struct ramdump_data *)get_data_instance(RAMDUMP);

	INIT_DELAYED_WORK(&oppo_charge_data_instance->battery_info_show_work, show_battery_info_for_wakeup);

	if (proc_create("ramdump_helper", 0664, power_helper_get_proc_dir(), &ramdump_helper_fops) == NULL)
	{
		pr_err("%s: failed to new ramdump_helper proc file\n", __func__);
	}
	ramdump_data_instance->auto_ramdump_enable = false;
	power_ultil_init();
	get_power_helper_init_info()->power_helper_support = true;
	return 0;
}

static int power_helper_remove(struct platform_device *pdev)
{
	struct powre_helper_data *powre_helper_data_pri = get_power_helper_data();
	iounmap(get_power_helper_init_info()->reg_base);
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
void power_helper_set_apss_master_stats(void *master_stats){}
void power_helper_rpmh_stats_data_init(phys_addr_t phys_addr_base, u32 phys_size, u32 num_records){}
void power_helper_charge_data_init(struct oppo_chg_chip *chip, void (*charge_get_bat_data_func)(struct oppo_chg_chip *)){}
void power_helper_show_bat_info(void){}
void power_helper_update_ext_mpss_master_stats(char *sleep_info){}
struct proc_dir_entry *power_helper_get_proc_dir(void){return NULL;}
void power_helper_get_pon_info(struct regmap *reg, u16 base, void *cfg){}
void power_helper_get_wlan_wakup_info(int type, const char *info1, const char *info2, uint32_t pri1, int ipv4){}
bool check_power_helper_support(void){return false;}
#endif
