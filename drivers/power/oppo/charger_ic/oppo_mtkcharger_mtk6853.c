/************************************************************************************
** VENDOR_EDIT
** Copyright (C), 2018-2019, OPPO Mobile Comm Corp., Ltd
**
** Description:
**    For P80 charger ic driver
**
** Version: 1.0
** Date created: 2018-11-09
** Author: Jianchao.Shi@PSW.BSP.CHG
**
** --------------------------- Revision History: ------------------------------------
* <version>       <date>         <author>              			<desc>
* Revision 1.0    2018-11-09   Jianchao.Shi@PSW.BSP.CHG   	Created for new architecture
*************************************************************************************/
#include <linux/init.h>		/* For init/exit macros */
#include <linux/module.h>	/* For MODULE_ marcros  */
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/power_supply.h>
#include <linux/pm_wakeup.h>
#include <linux/time.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/scatterlist.h>
#include <linux/suspend.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/reboot.h>
#include <linux/timer.h>

#include <mt-plat/charger_type.h>
#include <mt-plat/mtk_battery.h>
#include <mt-plat/mtk_boot.h>
#include <pmic.h>
#include <mtk_gauge_time_service.h>

//CONFIG_OPPO_TI_CHGIC_NEW
//#include "mtk_charger_intf.h"
//#include "mtk_charger_init.h"


#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for charging */
//====================================================================//
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include "../oppo_charger.h"
#include "../oppo_gauge.h"
#include "../oppo_vooc.h"
#include "../oppo_adapter.h"
#include "../oppo_short.h"
//#include "../gauge_ic/oppo_bq27541.h"
#include "op_charge.h"
#include "../../../misc/mediatek/typec/tcpc/inc/tcpci.h"
#include "../../../misc/mediatek/pmic/mt6360/inc/mt6360_pmu.h"
//#include "oppo_bq25790_charger.h"
#include "oppo_bq25910.h"
#include "oppo_mp2650.h"	//lkl for boost charger
#ifdef VENDOR_EDIT
/* LiYue@BSP.CHG.Basic, 2019/09/26, add for usb temperature check */
#include <linux/iio/consumer.h>
#endif

static bool em_mode = false;
static bool is_vooc_project(void);
struct oppo_chg_chip *g_oppo_chip = NULL;
extern int battery_meter_get_charger_voltage(void);
static int oppo_mt6360_reset_charger(void);
static int oppo_mt6360_enable_charging(void);
static int oppo_mt6360_disable_charging(void);
static int oppo_mt6360_float_voltage_write(int vflaot_mv);
static int oppo_mt6360_suspend_charger(void);
static int oppo_mt6360_unsuspend_charger(void);
static int oppo_mt6360_charging_current_write_fast(int chg_curr);
static int oppo_mt6360_set_termchg_current(int term_curr);
static int oppo_mt6360_set_rechg_voltage(int rechg_mv);
int oppo_tbatt_power_off_task_init(struct oppo_chg_chip *chip);
#ifdef VENDOR_EDIT
/*Baoquan.Lai@@BSP.CHG.Basic, 2020/05/22, Add for chargeric temp */
static void oppo_get_chargeric_temp_volt(struct charger_data *pdata);
static void get_chargeric_temp(struct charger_data *pdata);
#endif

static struct task_struct *oppo_usbtemp_kthread;
static struct timer_list usbtemp_timer;

static DECLARE_WAIT_QUEUE_HEAD(oppo_usbtemp_wq);
void oppo_set_otg_switch_status(bool value);
void oppo_wake_up_usbtemp_thread(void);

#ifdef CONFIG_OPPO_CHARGER_MTK
struct mtk_hv_flashled_pinctrl {
	int chgvin_gpio;
	int pmic_chgfunc_gpio;
	int bc1_2_done;
	bool hv_flashled_support;

	struct pinctrl *pinctrl;
	struct pinctrl_state *chgvin_enable;
	struct pinctrl_state *chgvin_disable;
	struct pinctrl_state *pmic_chgfunc_enable;
	struct pinctrl_state *pmic_chgfunc_disable;
};

struct mtk_hv_flashled_pinctrl mtkhv_flashled_pinctrl;

#endif

//====================================================================//
#endif /* VENDOR_EDIT */


//====================================================================//
#ifdef VENDOR_EDIT
//#define oppo_mt_get_vbus_status	oppo_mt6360_get_vbus_status
//#define oppo_chg_get_pd_type oppo_mt6360_get_pd_type
/* Jianchao.Shi@BSP.CHG.Basic, 2019/01/22, sjc Add for usb status */
#define USB_TEMP_HIGH		0x01//bit0
#define USB_WATER_DETECT	0x02//bit1
#define USB_RESERVE2		0x04//bit2
#define USB_RESERVE3		0x08//bit3
#define USB_RESERVE4		0x10//bit4
#define USB_DONOT_USE		0x80000000//bit31
static int usb_status = 0;

static void oppo_set_usb_status(int status)
{
	usb_status = usb_status | status;
}

static void oppo_clear_usb_status(int status)
{
	usb_status = usb_status & (~status);
}

static int oppo_get_usb_status(void)
{
	return usb_status;
}
//lkl need to make sure for add for vooc
int is_vooc_cfg = false;
static bool is_vooc_project(void)
{
	chg_err("%s is_vooc_cfg = %d\n", __func__, is_vooc_cfg);
	return is_vooc_cfg;
}
#endif /* VENDOR_EDIT */
//====================================================================//

static struct charger_manager *pinfo;
static struct list_head consumer_head = LIST_HEAD_INIT(consumer_head);
static DEFINE_MUTEX(consumer_mutex);

#ifndef VENDOR_EDIT
/* LiYue@BSP.CHG.Basic, 2020/02/12, remove to mtk_pe40_intf.c, resolve compile error */
bool mtk_is_TA_support_pd_pps(struct charger_manager *pinfo)
{
	if (pinfo->enable_pe_4 == false && pinfo->enable_pe_5 == false)
		return false;

	if (pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK_APDO)
		return true;
	return false;
}
#endif

bool is_power_path_supported(void)
{
	if (pinfo == NULL)
		return false;

	if (pinfo->data.power_path_support == true)
		return true;

	return false;
}

bool is_disable_charger(void)
{
	if (pinfo == NULL)
		return true;

	if (pinfo->disable_charger == true || IS_ENABLED(CONFIG_POWER_EXT))
		return true;
	else
		return false;
}

void BATTERY_SetUSBState(int usb_state_value)
{
	if (is_disable_charger()) {
		chr_err("[%s] in FPGA/EVB, no service\n", __func__);
	} else {
		if ((usb_state_value < USB_SUSPEND) ||
			((usb_state_value > USB_CONFIGURED))) {
			chr_err("%s Fail! Restore to default value\n",
				__func__);
			usb_state_value = USB_UNCONFIGURED;
		} else {
			chr_err("%s Success! Set %d\n", __func__,
				usb_state_value);
			if (pinfo)
				pinfo->usb_state = usb_state_value;
		}
	}
}

unsigned int set_chr_input_current_limit(int current_limit)
{
	return 500;
}

int get_chr_temperature(int *min_temp, int *max_temp)
{
	*min_temp = 25;
	*max_temp = 30;

	return 0;
}

int set_chr_boost_current_limit(unsigned int current_limit)
{
	return 0;
}

int set_chr_enable_otg(unsigned int enable)
{
	return 0;
}

int mtk_chr_is_charger_exist(unsigned char *exist)
{
	if (mt_get_charger_type() == CHARGER_UNKNOWN)
		*exist = 0;
	else
		*exist = 1;
	return 0;
}

/*=============== fix me==================*/
int chargerlog_level = CHRLOG_ERROR_LEVEL;

int chr_get_debug_level(void)
{
	return chargerlog_level;
}

#ifdef MTK_CHARGER_EXP
#include <linux/string.h>

char chargerlog[1000];
#define LOG_LENGTH 500
int chargerlog_level = 10;
int chargerlogIdx;

int charger_get_debug_level(void)
{
	return chargerlog_level;
}

void charger_log(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsprintf(chargerlog + chargerlogIdx, fmt, args);
	va_end(args);
	chargerlogIdx = strlen(chargerlog);
	if (chargerlogIdx >= LOG_LENGTH) {
		chr_err("%s", chargerlog);
		chargerlogIdx = 0;
		memset(chargerlog, 0, 1000);
	}
}

void charger_log_flash(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vsprintf(chargerlog + chargerlogIdx, fmt, args);
	va_end(args);
	chr_err("%s", chargerlog);
	chargerlogIdx = 0;
	memset(chargerlog, 0, 1000);
}
#endif

void _wake_up_charger(struct charger_manager *info)
{
#ifdef VENDOR_EDIT
	/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Modify for charging */
		return;
#else
	unsigned long flags;

	if (info == NULL)
		return;

	spin_lock_irqsave(&info->slock, flags);
	if (!info->charger_wakelock.active)
		__pm_stay_awake(&info->charger_wakelock);
	spin_unlock_irqrestore(&info->slock, flags);
	info->charger_thread_timeout = true;
	wake_up(&info->wait_que);
#endif
}

/* charger_manager ops  */
static int _mtk_charger_change_current_setting(struct charger_manager *info)
{
	if (info != NULL && info->change_current_setting)
		return info->change_current_setting(info);

	return 0;
}

static int _mtk_charger_do_charging(struct charger_manager *info, bool en)
{
	if (info != NULL && info->do_charging)
		info->do_charging(info, en);
	return 0;
}
/* charger_manager ops end */


/* user interface */
struct charger_consumer *charger_manager_get_by_name(struct device *dev,
	const char *name)
{
	struct charger_consumer *puser;

	puser = kzalloc(sizeof(struct charger_consumer), GFP_KERNEL);
	if (puser == NULL)
		return NULL;

	mutex_lock(&consumer_mutex);
	puser->dev = dev;

	list_add(&puser->list, &consumer_head);
	if (pinfo != NULL)
		puser->cm = pinfo;

	mutex_unlock(&consumer_mutex);

	return puser;
}
EXPORT_SYMBOL(charger_manager_get_by_name);

int charger_manager_enable_high_voltage_charging(
			struct charger_consumer *consumer, bool en)
{
	struct charger_manager *info = consumer->cm;
	struct list_head *pos;
	struct list_head *phead = &consumer_head;
	struct charger_consumer *ptr;

	if (!info)
		return -EINVAL;

	pr_debug("[%s] %s, %d\n", __func__, dev_name(consumer->dev), en);

	if (!en && consumer->hv_charging_disabled == false)
		consumer->hv_charging_disabled = true;
	else if (en && consumer->hv_charging_disabled == true)
		consumer->hv_charging_disabled = false;
	else {
		pr_info("[%s] already set: %d %d\n", __func__,
			consumer->hv_charging_disabled, en);
		return 0;
	}

	mutex_lock(&consumer_mutex);
	list_for_each(pos, phead) {
		ptr = container_of(pos, struct charger_consumer, list);
		if (ptr->hv_charging_disabled == true) {
			info->enable_hv_charging = false;
			break;
		}
		if (list_is_last(pos, phead))
			info->enable_hv_charging = true;
	}
	mutex_unlock(&consumer_mutex);

	pr_info("%s: user: %s, en = %d\n", __func__, dev_name(consumer->dev),
		info->enable_hv_charging);

	if (mtk_pe50_get_is_connect(info) && !info->enable_hv_charging)
		mtk_pe50_stop_algo(info, true);

	_wake_up_charger(info);

	return 0;
}
EXPORT_SYMBOL(charger_manager_enable_high_voltage_charging);

int charger_manager_enable_power_path(struct charger_consumer *consumer,
	int idx, bool en)
{
	int ret = 0;
	bool is_en = true;
	struct charger_manager *info = consumer->cm;
	struct charger_device *chg_dev;


	if (!info)
		return -EINVAL;

	switch (idx) {
	case MAIN_CHARGER:
		chg_dev = info->chg1_dev;
		break;
	case SLAVE_CHARGER:
		chg_dev = info->chg2_dev;
		break;
	default:
		return -EINVAL;
	}

	ret = charger_dev_is_powerpath_enabled(chg_dev, &is_en);
	if (ret < 0) {
		chr_err("%s: get is power path enabled failed\n", __func__);
		return ret;
	}
	if (is_en == en) {
		chr_err("%s: power path is already en = %d\n", __func__, is_en);
		return 0;
	}

	pr_info("%s: enable power path = %d\n", __func__, en);
	return charger_dev_enable_powerpath(chg_dev, en);
}

static int _charger_manager_enable_charging(struct charger_consumer *consumer,
	int idx, bool en)
{
	struct charger_manager *info = consumer->cm;

	chr_err("%s: dev:%s idx:%d en:%d\n", __func__, dev_name(consumer->dev),
		idx, en);

	if (info != NULL) {
		struct charger_data *pdata;

		if (idx == MAIN_CHARGER)
			pdata = &info->chg1_data;
		else if (idx == SLAVE_CHARGER)
			pdata = &info->chg2_data;
		else
			return -ENOTSUPP;

		if (en == false) {
			_mtk_charger_do_charging(info, en);
			pdata->disable_charging_count++;
		} else {
			if (pdata->disable_charging_count == 1) {
				_mtk_charger_do_charging(info, en);
				pdata->disable_charging_count = 0;
			} else if (pdata->disable_charging_count > 1)
				pdata->disable_charging_count--;
		}
		chr_err("%s: dev:%s idx:%d en:%d cnt:%d\n", __func__,
			dev_name(consumer->dev), idx, en,
			pdata->disable_charging_count);

		return 0;
	}
	return -EBUSY;

}

int charger_manager_enable_charging(struct charger_consumer *consumer,
	int idx, bool en)
{
	struct charger_manager *info = consumer->cm;
	int ret = 0;

	mutex_lock(&info->charger_lock);
	ret = _charger_manager_enable_charging(consumer, idx, en);
	mutex_unlock(&info->charger_lock);
	return ret;
}

int charger_manager_set_input_current_limit(struct charger_consumer *consumer,
	int idx, int input_current)
{
	struct charger_manager *info = consumer->cm;

	if (info != NULL) {
		struct charger_data *pdata;

		if (info->data.parallel_vbus) {
			if (idx == TOTAL_CHARGER) {
				info->chg1_data.thermal_input_current_limit =
					input_current;
				info->chg2_data.thermal_input_current_limit =
					input_current;
			} else
				return -ENOTSUPP;
		} else {
			if (idx == MAIN_CHARGER)
				pdata = &info->chg1_data;
			else if (idx == SLAVE_CHARGER)
				pdata = &info->chg2_data;
			else
				return -ENOTSUPP;
			pdata->thermal_input_current_limit = input_current;
		}

		chr_err("%s: dev:%s idx:%d en:%d\n", __func__,
			dev_name(consumer->dev), idx, input_current);
		_mtk_charger_change_current_setting(info);
		_wake_up_charger(info);
		return 0;
	}
	return -EBUSY;
}

int charger_manager_set_charging_current_limit(
	struct charger_consumer *consumer, int idx, int charging_current)
{
	struct charger_manager *info = consumer->cm;

	if (info != NULL) {
		struct charger_data *pdata;

		if (idx == MAIN_CHARGER)
			pdata = &info->chg1_data;
		else if (idx == SLAVE_CHARGER)
			pdata = &info->chg2_data;
		else
			return -ENOTSUPP;

		pdata->thermal_charging_current_limit = charging_current;
		chr_err("%s: dev:%s idx:%d en:%d\n", __func__,
			dev_name(consumer->dev), idx, charging_current);
		_mtk_charger_change_current_setting(info);
		_wake_up_charger(info);
		return 0;
	}
	return -EBUSY;
}

int charger_manager_get_charger_temperature(struct charger_consumer *consumer,
	int idx, int *tchg_min,	int *tchg_max)
{
	struct charger_manager *info = consumer->cm;

	if (info != NULL) {
		struct charger_data *pdata;

#ifndef VENDOR_EDIT
/* Baoquan.Lai@BSP.CHG.Basic, 2020/05/13, add for updating mtkscharger when no cable in*/
		if (!upmu_get_rgs_chrdet()) {
			pr_err("[%s] No cable in, skip it\n", __func__);
			*tchg_min = -127;
			*tchg_max = -127;
			return -EINVAL;
		}
#endif

		if (idx == MAIN_CHARGER)
			pdata = &info->chg1_data;
		else if (idx == SLAVE_CHARGER)
			pdata = &info->chg2_data;
		else
			return -ENOTSUPP;

		if (!pdata) {
                pr_err("%s, pdata null\n", __func__);
                return -1;
        }

		oppo_get_chargeric_temp_volt(pdata);
		get_chargeric_temp(pdata);

		pdata->junction_temp_min = pdata->chargeric_temp;
		pdata->junction_temp_max = pdata->chargeric_temp;
		*tchg_min = pdata->chargeric_temp;
		*tchg_max = pdata->chargeric_temp;

		*tchg_min = pdata->junction_temp_min;
		*tchg_max = pdata->junction_temp_max;

		return 0;
	}
	return -EBUSY;
}

int charger_manager_get_current_charging_type(struct charger_consumer *consumer)
{
	struct charger_manager *info = consumer->cm;

	if (info != NULL) {
		if (mtk_pe20_get_is_connect(info))
			return 2;
	}

	return 0;
}

int charger_manager_get_zcv(struct charger_consumer *consumer, int idx, u32 *uV)
{
	struct charger_manager *info = consumer->cm;
	int ret = 0;
	struct charger_device *pchg;


	if (info != NULL) {
		if (idx == MAIN_CHARGER) {
			pchg = info->chg1_dev;
			ret = charger_dev_get_zcv(pchg, uV);
		} else if (idx == SLAVE_CHARGER) {
			pchg = info->chg2_dev;
			ret = charger_dev_get_zcv(pchg, uV);
		} else
			ret = -1;

	} else {
		chr_err("%s info is null\n", __func__);
	}
	chr_err("%s zcv:%d ret:%d\n", __func__, *uV, ret);

	return 0;
}
#if 0
int charger_manager_enable_kpoc_shutdown(struct charger_consumer *consumer,
	bool en)
{
#ifndef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Delete for charging */
	struct charger_manager *info = consumer->cm;

	if (en)
		atomic_set(&info->enable_kpoc_shdn, 1);
	else
		atomic_set(&info->enable_kpoc_shdn, 0);
#endif /* VENDOR_EDIT */
	return 0;
}
#endif
int charger_manager_enable_chg_type_det(struct charger_consumer *consumer,
	bool en)
{
	struct charger_manager *info = consumer->cm;
	struct charger_device *chg_dev;
	int ret = 0;

	if (info != NULL) {
		switch (info->data.bc12_charger) {
		case MAIN_CHARGER:
			chg_dev = info->chg1_dev;
			break;
		case SLAVE_CHARGER:
			chg_dev = info->chg2_dev;
			break;
		default:
			chg_dev = info->chg1_dev;
			chr_err("%s: invalid number, use main charger as default\n",
				__func__);
			break;
		}

		chr_err("%s: chg%d is doing bc12\n", __func__,
			info->data.bc12_charger + 1);
		ret = charger_dev_enable_chg_type_det(chg_dev, en);
		if (ret < 0) {
			chr_err("%s: en chgdet fail, en = %d\n", __func__, en);
			return ret;
		}
	} else {
		chr_err("%s: charger_manager is null\n", __func__);
	}


	return 0;
}

int register_charger_manager_notifier(struct charger_consumer *consumer,
	struct notifier_block *nb)
{
	int ret = 0;
	struct charger_manager *info = consumer->cm;


	mutex_lock(&consumer_mutex);
	if (info != NULL)
		ret = srcu_notifier_chain_register(&info->evt_nh, nb);
	else
		consumer->pnb = nb;
	mutex_unlock(&consumer_mutex);

	return ret;
}

int unregister_charger_manager_notifier(struct charger_consumer *consumer,
				struct notifier_block *nb)
{
	int ret = 0;
	struct charger_manager *info = consumer->cm;

	mutex_lock(&consumer_mutex);
	if (info != NULL)
		ret = srcu_notifier_chain_unregister(&info->evt_nh, nb);
	else
		consumer->pnb = NULL;
	mutex_unlock(&consumer_mutex);

	return ret;
}

/* internal algorithm common function */
bool is_dual_charger_supported(struct charger_manager *info)
{
	if (info->chg2_dev == NULL)
		return false;
	return true;
}

int charger_enable_vbus_ovp(struct charger_manager *pinfo, bool enable)
{
	int ret = 0;
	u32 sw_ovp = 0;

	if (enable)
		sw_ovp = pinfo->data.max_charger_voltage_setting;
	else
		sw_ovp = 15000000;

	/* Enable/Disable SW OVP status */
	pinfo->data.max_charger_voltage = sw_ovp;

	chr_err("[%s] en:%d ovp:%d\n",
			    __func__, enable, sw_ovp);
	return ret;
}

bool is_typec_adapter(struct charger_manager *info)
{
	int rp;

	rp = adapter_dev_get_property(info->pd_adapter, TYPEC_RP_LEVEL);
	if (info->pd_type == MTK_PD_CONNECT_TYPEC_ONLY_SNK &&
			rp != 500 &&
			info->chr_type != STANDARD_HOST &&
			info->chr_type != CHARGING_HOST &&
			mtk_pe20_get_is_connect(info) == false &&
			mtk_pe_get_is_connect(info) == false &&
			info->enable_type_c == true)
		return true;

	return false;
}

int charger_get_vbus(void)
{
	int ret = 0;
	int vchr = 0;

	if (pinfo == NULL)
		return 0;
	ret = charger_dev_get_vbus(pinfo->chg1_dev, &vchr);
	if (ret < 0) {
		chr_err("%s: get vbus failed: %d\n", __func__, ret);
		return ret;
	}

	vchr = vchr / 1000;
	return vchr;
}

int mtk_get_dynamic_cv(struct charger_manager *info, unsigned int *cv)
{
	int ret = 0;
	u32 _cv, _cv_temp;
	unsigned int vbat_threshold[4] = {3400000, 0, 0, 0};
	u32 vbat_bif = 0, vbat_auxadc = 0, vbat = 0;
	u32 retry_cnt = 0;

	if (pmic_is_bif_exist()) {
		do {
			vbat_auxadc = battery_get_bat_voltage() * 1000;
			ret = pmic_get_bif_battery_voltage(&vbat_bif);
			vbat_bif = vbat_bif * 1000;
			if (ret >= 0 && vbat_bif != 0 &&
			    vbat_bif < vbat_auxadc) {
				vbat = vbat_bif;
				chr_err("%s: use BIF vbat = %duV, dV to auxadc = %duV\n",
					__func__, vbat, vbat_auxadc - vbat_bif);
				break;
			}
			retry_cnt++;
		} while (retry_cnt < 5);

		if (retry_cnt == 5) {
			ret = 0;
			vbat = vbat_auxadc;
			chr_err("%s: use AUXADC vbat = %duV, since BIF vbat = %duV\n",
				__func__, vbat_auxadc, vbat_bif);
		}

		/* Adjust CV according to the obtained vbat */
		vbat_threshold[1] = info->data.bif_threshold1;
		vbat_threshold[2] = info->data.bif_threshold2;
		_cv_temp = info->data.bif_cv_under_threshold2;

		if (!info->enable_dynamic_cv && vbat >= vbat_threshold[2]) {
			_cv = info->data.battery_cv;
			goto out;
		}

		if (vbat < vbat_threshold[1])
			_cv = 4608000;
		else if (vbat >= vbat_threshold[1] && vbat < vbat_threshold[2])
			_cv = _cv_temp;
		else {
			_cv = info->data.battery_cv;
			info->enable_dynamic_cv = false;
		}
out:
		*cv = _cv;
		chr_err("%s: CV = %duV, enable_dynamic_cv = %d\n",
			__func__, _cv, info->enable_dynamic_cv);
	} else
		ret = -ENOTSUPP;

	return ret;
}

int charger_manager_notifier(struct charger_manager *info, int event)
{
	return srcu_notifier_call_chain(&info->evt_nh, event, NULL);
}

int charger_psy_event(struct notifier_block *nb, unsigned long event, void *v)
{
	struct charger_manager *info =
			container_of(nb, struct charger_manager, psy_nb);
	struct power_supply *psy = v;
	union power_supply_propval val;
	int ret;
	int tmp = 0;

	if (strcmp(psy->desc->name, "battery") == 0) {
		ret = power_supply_get_property(psy,
				POWER_SUPPLY_PROP_TEMP, &val);
		if (!ret) {
			tmp = val.intval / 10;
			if (info->battery_temp != tmp
			    && mt_get_charger_type() != CHARGER_UNKNOWN) {
				_wake_up_charger(info);
				chr_err("%s: %ld %s tmp:%d %d chr:%d\n",
					__func__, event, psy->desc->name, tmp,
					info->battery_temp,
					mt_get_charger_type());
			}
		}
	}

	return NOTIFY_DONE;
}

void mtk_charger_int_handler(void)
{
#ifdef VENDOR_EDIT			//lkl need to make sure
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for charging */
	chr_err("%s\n", __func__);
	if (is_vooc_project() == false) {
		if (mt_get_charger_type() != CHARGER_UNKNOWN) {
			oppo_wake_up_usbtemp_thread();

			pr_err("%s, Charger Plug In\n", __func__);
			charger_manager_notifier(pinfo, CHARGER_NOTIFY_START_CHARGING);
			pinfo->step_status = STEP_CHG_STATUS_STEP1;
			pinfo->step_status_pre = STEP_CHG_STATUS_INVALID;
			pinfo->step_cnt = 0;
			pinfo->step_chg_current = pinfo->data.step1_current_ma;
			schedule_delayed_work(&pinfo->step_charging_work, msecs_to_jiffies(5000));
		} else {
			pr_err("%s, Charger Plug Out\n", __func__);
			charger_manager_notifier(pinfo, CHARGER_NOTIFY_STOP_CHARGING);
			cancel_delayed_work(&pinfo->step_charging_work);
		}
	} else {
		if (mt_get_charger_type() != CHARGER_UNKNOWN){
			oppo_wake_up_usbtemp_thread();
			if (mtkhv_flashled_pinctrl.hv_flashled_support){
				mtkhv_flashled_pinctrl.bc1_2_done = true;
				if (g_oppo_chip->camera_on) {
					pinctrl_select_state(mtkhv_flashled_pinctrl.pinctrl, mtkhv_flashled_pinctrl.chgvin_disable);
					chr_err("[%s] camera_on %d\n", __func__, g_oppo_chip->camera_on);
				}
			}
			chr_err("Charger Plug In\n");
		} else {
			if (mtkhv_flashled_pinctrl.hv_flashled_support){
				mtkhv_flashled_pinctrl.bc1_2_done = false;
				pinctrl_select_state(mtkhv_flashled_pinctrl.pinctrl, mtkhv_flashled_pinctrl.chgvin_enable);
			}
			chr_err("Charger Plug Out\n");
		}

		charger_dev_set_input_current(g_oppo_chip->chgic_mtk.oppo_info->chg1_dev, 500000);
		/* LiYue@BSP.CHG.Basic, 2020/04/20, suspend mt6360 for svooc */
		if (g_oppo_chip && g_oppo_chip->vbatt_num == 2) {
			oppo_mt6360_suspend_charger();
		}
	}

#else
	chr_err("%s\n", __func__);

	if (pinfo == NULL) {
		chr_err("charger is not rdy ,skip1\n");
		return;
	}

	if (pinfo->init_done != true) {
		chr_err("charger is not rdy ,skip2\n");
		return;
	}

	if (mt_get_charger_type() == CHARGER_UNKNOWN) {
		mutex_lock(&pinfo->cable_out_lock);
		pinfo->cable_out_cnt++;
		chr_err("cable_out_cnt=%d\n", pinfo->cable_out_cnt);
		mutex_unlock(&pinfo->cable_out_lock);
		charger_manager_notifier(pinfo, CHARGER_NOTIFY_STOP_CHARGING);
	} else
		charger_manager_notifier(pinfo, CHARGER_NOTIFY_START_CHARGING);

	chr_err("wake_up_charger\n");
	_wake_up_charger(pinfo);
#endif /* VENDOR_EDIT */
}

static int mtk_chgstat_notify(struct charger_manager *info)
{
	int ret = 0;
	char *env[2] = { "CHGSTAT=1", NULL };

	chr_err("%s: 0x%x\n", __func__, info->notify_code);
	ret = kobject_uevent_env(&info->pdev->dev.kobj, KOBJ_CHANGE, env);
	if (ret)
		chr_err("%s: kobject_uevent_fail, ret=%d", __func__, ret);

	return ret;
}

static int mtk_charger_parse_dt(struct charger_manager *info,
				struct device *dev)
{
	struct device_node *np = dev->of_node;
	u32 val;

	chr_err("%s: starts\n", __func__);

	if (!np) {
		chr_err("%s: no device node\n", __func__);
		return -EINVAL;
	}

	info->enable_type_c = of_property_read_bool(np, "enable_type_c");

	/* dynamic mivr */
	if (of_property_read_u32(np, "min_charger_voltage_1", &val) >= 0)
		info->data.min_charger_voltage_1 = val;
	else {
		chr_err("use default V_CHARGER_MIN_1:%d\n", V_CHARGER_MIN_1);
		info->data.min_charger_voltage_1 = V_CHARGER_MIN_1;
	}

	if (of_property_read_u32(np, "min_charger_voltage_2", &val) >= 0)
		info->data.min_charger_voltage_2 = val;
	else {
		chr_err("use default V_CHARGER_MIN_2:%d\n", V_CHARGER_MIN_2);
		info->data.min_charger_voltage_2 = V_CHARGER_MIN_2;
	}

	if (of_property_read_u32(np, "max_dmivr_charger_current", &val) >= 0)
		info->data.max_dmivr_charger_current = val;
	else {
		chr_err("use default MAX_DMIVR_CHARGER_CURRENT:%d\n",
			MAX_DMIVR_CHARGER_CURRENT);
		info->data.max_dmivr_charger_current =
					MAX_DMIVR_CHARGER_CURRENT;
	}

	info->data.power_path_support =
				of_property_read_bool(np, "power_path_support");
	chr_debug("%s: power_path_support: %d\n",
		__func__, info->data.power_path_support);

	return 0;
}

static int oppo_step_charging_parse_dt(struct charger_manager *info,
                                struct device *dev)
{
        struct device_node *np = dev->of_node;
        u32 val;

        if (!np) {
                chr_err("%s: no device node\n", __func__);
                return -EINVAL;
        }

	if (of_property_read_u32(np, "qcom,dual_charger_support", &val) >= 0) {
		info->data.dual_charger_support = val;
	} else {
		chr_err("use dual_charger_support: disable\n");
		info->data.dual_charger_support = 0;
	}
	chr_err("oppo dual_charger_support: %d\n", info->data.dual_charger_support);
	//info->data.dual_charger_support = 1;

	if (of_property_read_u32(np, "qcom,step1_time", &val) >= 0) {
		info->data.step1_time = val;
	} else {
		chr_err("use step1_time: 300s\n");
		info->data.step1_time = 300;
	}

	if (of_property_read_u32(np, "qcom,step1_current_ma", &val) >= 0) {
		info->data.step1_current_ma = val;
	} else {
		chg_err("use step1_current_ma: 3200mA\n");
		info->data.step1_current_ma = 3200;
	}

        if (of_property_read_u32(np, "qcom,step2_time", &val) >= 0) {
		info->data.step2_time = val;
	} else {
		chg_err("use step2_time: 900s\n");
		info->data.step2_time = 900;
	}

	if (of_property_read_u32(np, "qcom,step2_current_ma", &val) >= 0) {
		info->data.step2_current_ma = val;
	} else {
		chg_err("use step2_current_ma: 3000mA\n");
		info->data.step2_current_ma = 3000;
	}

        if (of_property_read_u32(np, "qcom,step3_current_ma", &val) >= 0) {
		info->data.step3_current_ma = val;
	} else {
		chg_err("use step3_current_ma: 2640mA\n");
		info->data.step3_current_ma = 2640;
	}

	chg_err("step1_time: %d, step1_current: %d, step2_time: %d, step2_current: %d, step3_current: %d\n",
		info->data.step1_time, info->data.step1_current_ma, info->data.step2_time, info->data.step2_current_ma, info->data.step3_current_ma);

	return 0;
}

static ssize_t show_BatNotify(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct charger_manager *pinfo = dev->driver_data;

	pr_debug("[Battery] show_BatteryNotify: 0x%x\n", pinfo->notify_code);

	return sprintf(buf, "%u\n", pinfo->notify_code);
}

static ssize_t store_BatNotify(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct charger_manager *pinfo = dev->driver_data;
	unsigned int reg = 0;
	int ret;

	pr_debug("[Battery] store_BatteryNotify\n");
	if (buf != NULL && size != 0) {
		pr_debug("[Battery] buf is %s and size is %zu\n", buf, size);
		ret = kstrtouint(buf, 16, &reg);
		pinfo->notify_code = reg;
		pr_debug("[Battery] store code: 0x%x\n", pinfo->notify_code);
		mtk_chgstat_notify(pinfo);
	}
	return size;
}

static DEVICE_ATTR(BatteryNotify, 0664, show_BatNotify, store_BatNotify);

/* Create sysfs and procfs attributes */
static int mtk_charger_setup_files(struct platform_device *pdev)
{
	int ret = 0;
	struct proc_dir_entry *battery_dir = NULL;
	struct charger_manager *info = platform_get_drvdata(pdev);
	/* struct charger_device *chg_dev; */

	/* Battery warning */
	ret = device_create_file(&(pdev->dev), &dev_attr_BatteryNotify);
	if (ret)
		goto _out;
_out:
	return ret;
}

void notify_adapter_event(enum adapter_type type, enum adapter_event evt,
	void *val)
{
	chr_err("%s %d %d\n", __func__, type, evt);
	switch (type) {
	case MTK_PD_ADAPTER:
		switch (evt) {
		case  MTK_PD_CONNECT_NONE:
			mutex_lock(&pinfo->charger_pd_lock);
			chr_err("PD Notify Detach\n");
			pinfo->pd_type = MTK_PD_CONNECT_NONE;
			mutex_unlock(&pinfo->charger_pd_lock);
			/* reset PE40 */
			break;

		case MTK_PD_CONNECT_HARD_RESET:
			mutex_lock(&pinfo->charger_pd_lock);
			chr_err("PD Notify HardReset\n");
			pinfo->pd_type = MTK_PD_CONNECT_NONE;
			pinfo->pd_reset = true;
			mutex_unlock(&pinfo->charger_pd_lock);
			_wake_up_charger(pinfo);
			/* reset PE40 */
			break;

		case MTK_PD_CONNECT_PE_READY_SNK:
			mutex_lock(&pinfo->charger_pd_lock);
			chr_err("PD Notify fixe voltage ready\n");
			pinfo->pd_type = MTK_PD_CONNECT_PE_READY_SNK;
			mutex_unlock(&pinfo->charger_pd_lock);
			/* PD is ready */
			break;

		case MTK_PD_CONNECT_PE_READY_SNK_PD30:
			mutex_lock(&pinfo->charger_pd_lock);
			chr_err("PD Notify PD30 ready\r\n");
			pinfo->pd_type = MTK_PD_CONNECT_PE_READY_SNK_PD30;
			mutex_unlock(&pinfo->charger_pd_lock);
			/* PD30 is ready */
			break;

		case MTK_PD_CONNECT_PE_READY_SNK_APDO:
			mutex_lock(&pinfo->charger_pd_lock);
			chr_err("PD Notify APDO Ready\n");
			pinfo->pd_type = MTK_PD_CONNECT_PE_READY_SNK_APDO;
			mutex_unlock(&pinfo->charger_pd_lock);
			/* PE40 is ready */
			_wake_up_charger(pinfo);
			break;

		case MTK_PD_CONNECT_TYPEC_ONLY_SNK:
			mutex_lock(&pinfo->charger_pd_lock);
			chr_err("PD Notify Type-C Ready\n");
			pinfo->pd_type = MTK_PD_CONNECT_TYPEC_ONLY_SNK;
			mutex_unlock(&pinfo->charger_pd_lock);
			/* type C is ready */
			_wake_up_charger(pinfo);
			break;
		case MTK_TYPEC_WD_STATUS:
			chr_err("wd status = %d\n", *(bool *)val);
			mutex_lock(&pinfo->charger_pd_lock);
			pinfo->water_detected = *(bool *)val;
			mutex_unlock(&pinfo->charger_pd_lock);

			if (pinfo->water_detected == true) {
				pinfo->notify_code |= CHG_TYPEC_WD_STATUS;
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2019/01/22, sjc Add for usb status */
			oppo_set_usb_status(USB_WATER_DETECT);
			oppo_vooc_set_disable_adapter_output(true);
			if (g_oppo_chip && g_oppo_chip->usb_psy)
				power_supply_changed(g_oppo_chip->usb_psy);
#endif
			mtk_chgstat_notify(pinfo);
		} else {
			pinfo->notify_code &= ~CHG_TYPEC_WD_STATUS;
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2019/01/22, sjc Add for usb status */
			oppo_clear_usb_status(USB_WATER_DETECT);
			oppo_vooc_set_disable_adapter_output(false);
			if (g_oppo_chip && g_oppo_chip->usb_psy)
				power_supply_changed(g_oppo_chip->usb_psy);
#endif
			mtk_chgstat_notify(pinfo);
		}
		break;
		case MTK_TYPEC_HRESET_STATUS:
			chr_err("hreset status = %d\n", *(bool *)val);
			mutex_lock(&pinfo->charger_pd_lock);
			if (*(bool *)val)
				atomic_set(&pinfo->enable_kpoc_shdn, 1);
			else
				atomic_set(&pinfo->enable_kpoc_shdn, 0);
			mutex_unlock(&pinfo->charger_pd_lock);
		break;
		}
	}
}

#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for charging */
//====================================================================//
void oppo_mt6360_dump_registers(void)
{
	struct charger_device *chg = NULL;
	static bool musb_hdrc_release = false;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	if (musb_hdrc_release == false &&
			g_oppo_chip->unwakelock_chg == 1 &&
			mt_get_charger_type() == NONSTANDARD_CHARGER) {
		musb_hdrc_release = true;
		mt_usb_disconnect();
	} else {
		if (musb_hdrc_release == true &&
				g_oppo_chip->unwakelock_chg == 0 &&
				mt_get_charger_type() == NONSTANDARD_CHARGER) {
			musb_hdrc_release = false;
			mt_usb_connect();
		}
	}

	/*This function runs for more than 400ms, so return when no charger for saving power */
	if (g_oppo_chip->charger_type == POWER_SUPPLY_TYPE_UNKNOWN
			|| oppo_get_chg_powersave() == true) {
		return;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

	if (pinfo->data.dual_charger_support) {
		charger_dev_dump_registers(chg);
		bq25910_dump_registers();
	} else {
		charger_dev_dump_registers(chg);
	}

	return;
}

static int oppo_mt6360_kick_wdt(void)
{
	int rc = 0;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;
	rc = charger_dev_kick_wdt(chg);
	if (rc < 0) {
		chg_debug("charger_dev_kick_wdt fail\n");
	}
	return 0;
}

static int oppo_mt6360_hardware_init(void)
{
	int hw_aicl_point = 4400;
	//int sw_aicl_point = 4500;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

	oppo_mt6360_reset_charger();

	if (get_boot_mode() == KERNEL_POWER_OFF_CHARGING_BOOT) {
		oppo_mt6360_disable_charging();
		oppo_mt6360_float_voltage_write(4400);
		msleep(100);
	}

	oppo_mt6360_float_voltage_write(4385);

	//set_complete_charge_timeout(OVERTIME_DISABLED);
	mt6360_set_register(0x1C, 0xFF, 0xF9);

	//set_prechg_current(300);
	mt6360_set_register(0x18, 0xF, 0x4);

	oppo_mt6360_charging_current_write_fast(512);

	oppo_mt6360_set_termchg_current(150);

	oppo_mt6360_set_rechg_voltage(100);

	charger_dev_set_mivr(chg, hw_aicl_point * 1000);

#ifdef CONFIG_OPPO_CHARGER_MTK
	if (get_boot_mode() == META_BOOT || get_boot_mode() == FACTORY_BOOT
			|| get_boot_mode() == ADVMETA_BOOT || get_boot_mode() == ATE_FACTORY_BOOT) {
		oppo_mt6360_suspend_charger();
		oppo_mt6360_disable_charging();
	} else {
		oppo_mt6360_unsuspend_charger();
		oppo_mt6360_enable_charging();
	}
#else /* CONFIG_OPPO_CHARGER_MTK */
	oppo_mt6360_unsuspend_charger();
#endif /* CONFIG_OPPO_CHARGER_MTK */

	//set_wdt_timer(REG05_BQ25601D_WATCHDOG_TIMER_40S);
	//mt6360_set_register(0x1D, 0x80, 0x80);
	mt6360_set_register(0x1D, 0x30, 0x10);

	if (pinfo->data.dual_charger_support){
		bq25910_hardware_init();
	}
	return 0;
}

static int oppo_mt6360_charging_current_write_fast(int chg_curr)
{
	int rc = 0;
	u32 ret_chg_curr = 0;
	struct charger_device *chg = NULL;
	int main_cur = 0;
	int slave_cur = 0;


	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}

	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

	if (pinfo->data.dual_charger_support) {
		if (g_oppo_chip->tbatt_status == BATTERY_STATUS__NORMAL) {
			#ifdef VENDOR_EDIT
			/* xiejianglong@BSP.CHG.Basic,2020/8/19,add for 20625cake temperature drop set charge current error */
			if (chg_curr > pinfo->data.step3_current_ma && pinfo->step_chg_current >= pinfo->data.step3_current_ma)
			#else
			if (chg_curr > pinfo->step_chg_current)
			#endif
				chg_curr = pinfo->step_chg_current;
		}

		if (g_oppo_chip->dual_charger_support &&
				(g_oppo_chip->slave_charger_enable || em_mode)) {
			main_cur = (chg_curr * (100 - g_oppo_chip->slave_pct))/100;
			main_cur -= main_cur % 100;
			slave_cur = chg_curr - main_cur;

			rc = charger_dev_set_charging_current(chg, main_cur * 1000);
			if (rc < 0) {
				chg_debug("set fast charge current:%d fail\n", main_cur);
			}

			rc = bq25910_charging_current_write_fast(slave_cur);
			if (rc < 0) {
				chg_debug("set sub fast charge current:%d fail\n", slave_cur);
			}
		} else {
			rc = charger_dev_set_charging_current(chg, chg_curr * 1000);
			if (rc < 0) {
				chg_debug("set fast charge current:%d fail\n", chg_curr);
			}
		}
	} else {
		rc = charger_dev_set_charging_current(chg, chg_curr * 1000);
		if (rc < 0) {
			chg_debug("set fast charge current:%d fail\n", chg_curr);
		} else {
			charger_dev_get_charging_current(chg, &ret_chg_curr);
			chg_debug("set fast charge current:%d ret_chg_curr = %d\n", chg_curr, ret_chg_curr);
		}
	}
	return 0;
}

static void oppo_mt6360_set_aicl_point(int vbatt)
{
	int rc = 0;
	static int hw_aicl_point = 4400;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

	if (hw_aicl_point == 4400 && vbatt > 4100) {
		hw_aicl_point = 4500;
		//sw_aicl_point = 4550;
	} else if (hw_aicl_point == 4500 && vbatt <= 4100) {
		hw_aicl_point = 4400;
		//sw_aicl_point = 4500;
	}
	rc = charger_dev_set_mivr(chg, hw_aicl_point * 1000);
	if (rc < 0) {
		chg_debug("set aicl point:%d fail\n", hw_aicl_point);
	}
}

static int usb_icl[] = {
	300, 500, 900, 1200, 1350, 1500, 2000, 2400, 3000,
};
static int oppo_mt6360_input_current_limit_write(int value)
{
	int rc = 0;
	int i = 0;
	int chg_vol = 0;
	int aicl_point = 0;
	int vbus_mv = 0;
	int ibus_ma = 0;
	int main_cur = 0;
	int slave_cur = 0;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	chg_debug("usb input max current limit=%d setting %02x\n", value, i);

	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

	if (pinfo->data.dual_charger_support) {
		if (g_oppo_chip->dual_charger_support &&
				(!g_oppo_chip->slave_charger_enable && !em_mode)) {

			rc = bq25910_disable_charging();
			if (rc < 0) {
				chg_debug("disable sub charging fail\n");
			}

			rc = bq25910_suspend_charger();
			if (rc < 0) {
				chg_debug("disable sub charging fail\n");
			}
		}
	}

	//aicl_point_temp = g_oppo_chip->sw_aicl_point;
	if (g_oppo_chip->chg_ops->oppo_chg_get_pd_type) {
		if (g_oppo_chip->chg_ops->oppo_chg_get_pd_type() == true) {
			rc = oppo_pdc_get(&vbus_mv, &ibus_ma);
			if (rc >= 0 && ibus_ma >= 500 && ibus_ma < 3000 && value > ibus_ma) {
				value = ibus_ma;
				chg_debug("usb input max current limit=%d(pd)\n", value);
			}
		}
	}

	if (pinfo->data.dual_charger_support) {
		#ifdef VENDOR_EDIT
		/* xiejianglong@BSP.CHG.Basic,2020/7/20,add for 20625cake from 9V to 5V charge current drop */
		msleep(90);
		#endif
		chg_vol = battery_meter_get_charger_voltage();
		if (chg_vol > 7600) {
			aicl_point = 7600;
		} else {
			if (g_oppo_chip->batt_volt > 4100 )
				aicl_point = 4550;
			else
				aicl_point = 4500;
		}
	} else {
		if (g_oppo_chip->batt_volt > 4100 )
			aicl_point = 4550;
		else
			aicl_point = 4500;
	}

	if (value < 500) {
		i = 0;
		goto aicl_end;
	}

	mt6360_aicl_enable(false);

	i = 1; /* 500 */
	rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	msleep(90);
	chg_vol = battery_meter_get_charger_voltage();
	if (chg_vol < aicl_point) {
		chg_debug( "use 500 here\n");
		goto aicl_end;
	} else if (value < 900)
		goto aicl_end;

	i = 2; /* 900 */
	rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	msleep(90);
	chg_vol = battery_meter_get_charger_voltage();
	if (chg_vol < aicl_point) {
		i = i - 1;
		goto aicl_pre_step;
	} else if (value < 1200)
		goto aicl_end;

	i = 3; /* 1200 */
	rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	msleep(90);
	chg_vol = battery_meter_get_charger_voltage();
	if (chg_vol < aicl_point) {
		i = i - 1;
		goto aicl_pre_step;
	}

	i = 4; /* 1350 */
	rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	msleep(90);
	chg_vol = battery_meter_get_charger_voltage();
	if (chg_vol < aicl_point) {
		i = i - 2;
		goto aicl_pre_step;
	}

	i = 5; /* 1500 */
	rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	msleep(90);
	chg_vol = battery_meter_get_charger_voltage();
	if (chg_vol < aicl_point) {
		i = i - 3; //We DO NOT use 1.2A here
		goto aicl_pre_step;
	} else if (value < 1500) {
		i = i - 2; //We use 1.2A here
		goto aicl_end;
	} else if (value < 2000)
		goto aicl_end;

	i = 6; /* 2000 */
	rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	msleep(90);
	chg_vol = battery_meter_get_charger_voltage();
	if (chg_vol < aicl_point) {
		i = i - 1;
		goto aicl_pre_step;
	} else if (value < 2400)
		goto aicl_end;

	i = 7; /* 2400 */
        rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
        msleep(90);
        chg_vol = battery_meter_get_charger_voltage();
        if (chg_vol < aicl_point) {
                i = i - 1;
                goto aicl_pre_step;
        } else if (value < 3000)
                goto aicl_end;

	i = 8; /* 3000 */
	rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	msleep(90);
	chg_vol = battery_meter_get_charger_voltage();
	if (chg_vol < aicl_point) {
		i = i - 1;
		goto aicl_pre_step;
	} else if (value >= 3000)
		goto aicl_end;

aicl_pre_step:
	chg_debug("usb input max current limit aicl chg_vol=%d i[%d]=%d sw_aicl_point:%d aicl_pre_step\n", chg_vol, i, usb_icl[i], aicl_point);
	if (pinfo->data.dual_charger_support) {
		if (g_oppo_chip->dual_charger_support &&
				(g_oppo_chip->slave_charger_enable || em_mode)) {
			chg_debug("enable mt6360 and bq25910 for charging\n");
			rc = bq25910_enable_charging();
			if (rc < 0) {
				chg_debug("disable sub charging fail\n");
			}

			rc = bq25910_unsuspend_charger();
			if (rc < 0) {
				chg_debug("disable sub charging fail\n");
			}

			slave_cur = (usb_icl[i] * g_oppo_chip->slave_pct)/100;
			slave_cur -= slave_cur % 100;
			main_cur = usb_icl[i] - slave_cur;

			chg_debug("usb input max current limit aicl: master and salve input current: %d, %d\n",
					main_cur,slave_cur);
			bq25910_input_current_limit_write(slave_cur);
			rc = charger_dev_set_input_current(chg, main_cur * 1000 );
		} else {
			rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
		}
	} else {
		rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	}
	goto aicl_rerun;
aicl_end:		
	chg_debug("usb input max current limit aicl chg_vol=%d i[%d]=%d sw_aicl_point:%d aicl_end\n", chg_vol, i, usb_icl[i], aicl_point);

	if (pinfo->data.dual_charger_support) {
		if (g_oppo_chip->dual_charger_support &&
				(g_oppo_chip->slave_charger_enable || em_mode)) {
			rc = bq25910_enable_charging();
			if (rc < 0) {
				chg_debug("disable sub charging fail\n");
			}

			rc = bq25910_unsuspend_charger();
			if (rc < 0) {
				chg_debug("disable sub charging fail\n");
			}

			slave_cur = (usb_icl[i] * g_oppo_chip->slave_pct)/100;
			slave_cur -= slave_cur % 100;
			main_cur = usb_icl[i] - slave_cur;

			chg_debug("usb input max current limit aicl: master and salve input current: %d, %d\n",
					main_cur,slave_cur);
			bq25910_input_current_limit_write(slave_cur);
			rc = charger_dev_set_input_current(chg, main_cur * 1000 );
		} else {
			rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
		}
	} else {
		rc = charger_dev_set_input_current(chg, usb_icl[i] * 1000);
	}

	goto aicl_rerun;
aicl_rerun:
	mt6360_aicl_enable(true);
	return rc;

}

#define DELTA_MV        32
static int oppo_mt6360_float_voltage_write(int vfloat_mv)
{
	int rc = 0;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

	if (pinfo->data.dual_charger_support) {
		rc = charger_dev_set_constant_voltage(chg, vfloat_mv * 1000);
		if (rc < 0) {
			chg_debug("set float voltage:%d fail\n", vfloat_mv);
		}

		rc = bq25910_float_voltage_write(vfloat_mv + DELTA_MV);
		if (rc < 0) {
			chg_debug("set sub float voltage:%d fail\n", vfloat_mv);
		}
	} else {
		rc = charger_dev_set_constant_voltage(chg, vfloat_mv * 1000);
		if (rc < 0) {
			chg_debug("set float voltage:%d fail\n", vfloat_mv);
		}
	}

	return 0;
}

static int oppo_mt6360_set_termchg_current(int term_curr)
{
	int rc = 0;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;
	rc = charger_dev_set_eoc_current(chg, term_curr * 1000);
	if (rc < 0) {
		//chg_debug("set termchg_current fail\n");
	}
	return 0;
}

static int oppo_mt6360_enable_charging(void)
{
	int rc = 0;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

	rc = charger_dev_enable(chg, true);
	if (rc < 0) {
		chg_debug("enable charging fail\n");
	}

	return 0;
}

static int oppo_mt6360_disable_charging(void)
{
	int rc = 0;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}

	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

 	if (pinfo->data.dual_charger_support) {
		rc = charger_dev_enable(chg, false);
		if (rc < 0) {
			chg_debug("disable charging fail\n");
		}

		rc = bq25910_disable_charging();
		if (rc < 0) {
			chg_debug("disable sub charging fail\n");
		}
	} else {
		rc = charger_dev_enable(chg, false);
		if (rc < 0) {
			chg_debug("disable charging fail\n");
		}
	}

	return 0;
}

static int oppo_mt6360_check_charging_enable(void)
{
	return mt6360_check_charging_enable();
}

static int oppo_mt6360_suspend_charger(void)
{
	struct oppo_chg_chip *chip = g_oppo_chip;
	int rc = 0;

	chip->chg_ops->input_current_write(500);
		msleep(50);

	rc = mt6360_suspend_charger(true);
	if (rc < 0) {
		chg_debug("suspend charger fail\n");
	}

	return 0;
}

static int oppo_mt6360_unsuspend_charger(void)
{
	int rc = 0;

	rc = mt6360_suspend_charger(false);
	if (rc < 0) {
		chg_debug("unsuspend charger fail\n");
	}

	return 0;
}

static int oppo_mt6360_set_rechg_voltage(int rechg_mv)
{
	int rc = 0;

	rc = mt6360_set_rechg_voltage(rechg_mv);
	if (rc < 0) {
		chg_debug("set rechg voltage fail:%d\n", rechg_mv);
	}
	return 0;
}

static int oppo_mt6360_reset_charger(void)
{
	int rc = 0;

	rc = mt6360_reset_charger();
	if (rc < 0) {
		chg_debug("reset charger fail\n");
	}
	return 0;
}

static int oppo_mt6360_registers_read_full(void)
{
	bool full = false;
	int rc = 0;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;
	rc = charger_dev_is_charging_done(chg, &full);
	if (rc < 0) {
		chg_debug("registers read full  fail\n");
		full = false;
	} else {
		//chg_debug("registers read full\n");
	}

	return full;
}

static int oppo_mt6360_otg_enable(void)
{
	return 0;
}

static int oppo_mt6360_otg_disable(void)
{
	return 0;
}

static int oppo_mt6360_set_chging_term_disable(void)
{
	int rc = 0;

	rc = mt6360_set_chging_term_disable(true);
	if (rc < 0) {
		chg_debug("disable chging_term fail\n");
	}
	return 0;
}

static bool oppo_mt6360_check_charger_resume(void)
{
	return true;
}

static int oppo_mt6360_get_chg_current_step(void)
{
	return 100;
}

int mt_power_supply_type_check(void)
{
	int charger_type = POWER_SUPPLY_TYPE_UNKNOWN;

	switch(mt_get_charger_type()) {
	case CHARGER_UNKNOWN:
		break;
	case STANDARD_HOST:
		charger_type = POWER_SUPPLY_TYPE_USB;
		break;
	case CHARGING_HOST:
		charger_type = POWER_SUPPLY_TYPE_USB_CDP;
		break;
	case NONSTANDARD_CHARGER:
	case STANDARD_CHARGER:	
	case APPLE_2_1A_CHARGER:
	case APPLE_1_0A_CHARGER:
	case APPLE_0_5A_CHARGER:
		charger_type = POWER_SUPPLY_TYPE_USB_DCP;
		break;
	default:
		break;
	}
	chg_debug("charger_type[%d]\n", charger_type);
	return charger_type;
}
bool oppo_mt_get_vbus_status(void)
{
	bool vbus_status = false;
	static bool pre_vbus_status = false;
	int ret = 0;

	ret = mt6360_get_vbus_rising();
	if (ret < 0) {
		if (g_oppo_chip && g_oppo_chip->unwakelock_chg == 1
				&& g_oppo_chip->charger_type != POWER_SUPPLY_TYPE_UNKNOWN) {
			printk(KERN_ERR "[OPPO_CHG][%s]: unwakelock_chg=1, use pre status\n", __func__);
			return pre_vbus_status;
		} else {
			return false;
		}
	}
	if (ret == 0)
		vbus_status = false;
	else
		vbus_status = true;
	pre_vbus_status = vbus_status;
	return vbus_status;
}

int oppo_battery_meter_get_battery_voltage(void)
{
	return 4000;
}

int get_rtc_spare_oppo_fg_value(void)
{
	return 0;
}

int set_rtc_spare_oppo_fg_value(int soc)
{
	return 0;
}

static void oppo_mt_power_off(void)
{
	struct tcpc_device *tcpc_dev = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	if (g_oppo_chip->ac_online != true) {
		if (tcpc_dev == NULL) {
			tcpc_dev = tcpc_dev_get_by_name("type_c_port0");
		}
		if (tcpc_dev) {
			if (!(tcpc_dev->pd_wait_hard_reset_complete)
					&& !oppo_mt_get_vbus_status()) {
				kernel_power_off();
			}
		}
	} else {
		printk(KERN_ERR "[OPPO_CHG][%s]: ac_online is true, return!\n", __func__);
	}
}

#ifdef CONFIG_OPPO_SHORT_C_BATT_CHECK
/* This function is getting the dynamic aicl result/input limited in mA.
 * If charger was suspended, it must return 0(mA).
 * It meets the requirements in SDM660 platform.
 */
static int oppo_mt6360_chg_get_dyna_aicl_result(void)
{
	int rc = 0;
	int aicl_ma = 0;
	struct charger_device *chg = NULL;

	if (!g_oppo_chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;
	rc = charger_dev_get_input_current(chg, &aicl_ma);
	if (rc < 0) {
		chg_debug("get dyna aicl fail\n");
		return 500;
	}
	return aicl_ma / 1000;
}
#endif

#ifdef CONFIG_OPPO_RTC_DET_SUPPORT
static int rtc_reset_check(void)
{
	int rc = 0;
	struct rtc_time tm;
	struct rtc_device *rtc;

	rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);
	if (rtc == NULL) {
		pr_err("%s: unable to open rtc device (%s)\n",
			__FILE__, CONFIG_RTC_HCTOSYS_DEVICE);
		return 0;
	}

	rc = rtc_read_time(rtc, &tm);
	if (rc) {
		pr_err("Error reading rtc device (%s) : %d\n",
			CONFIG_RTC_HCTOSYS_DEVICE, rc);
		goto close_time;
	}

	rc = rtc_valid_tm(&tm);
	if (rc) {
		pr_err("Invalid RTC time (%s): %d\n",
			CONFIG_RTC_HCTOSYS_DEVICE, rc);
		goto close_time;
	}

	if ((tm.tm_year == 70) && (tm.tm_mon == 0) && (tm.tm_mday <= 1)) {
		chg_debug(": Sec: %d, Min: %d, Hour: %d, Day: %d, Mon: %d, Year: %d  @@@ wday: %d, yday: %d, isdst: %d\n",
			tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon, tm.tm_year,
			tm.tm_wday, tm.tm_yday, tm.tm_isdst);
		rtc_class_close(rtc);
		return 1;
	}

	chg_debug(": Sec: %d, Min: %d, Hour: %d, Day: %d, Mon: %d, Year: %d  ###  wday: %d, yday: %d, isdst: %d\n",
		tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon, tm.tm_year,
		tm.tm_wday, tm.tm_yday, tm.tm_isdst);

close_time:
	rtc_class_close(rtc);
	return 0;
}
#endif /* CONFIG_OPPO_RTC_DET_SUPPORT */
//====================================================================//

static int oppo_chg_get_main_ibat(void)
{
	int ibat = 0;
	int ret = 0;
	struct charger_device *chg = NULL;
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		return -1;
	}

	chg = g_oppo_chip->chgic_mtk.oppo_info->chg1_dev;

	ret = charger_dev_get_ibat(chg, &ibat);
	if (ret < 0) {
		pr_err("[%s] get ibat fail\n", __func__);
		return -1;
	}

	return ibat / 1000;
}

//====================================================================//
#ifdef VENDOR_EDIT
//extern bool ignore_usb;
static void set_usbswitch_to_rxtx(struct oppo_chg_chip *chip)
{
	int ret = 0;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	//if (ignore_usb) {
	//	chg_err("ignore_usb is true, do not set_usbswitch_to_rxtx\n");
	//	return;
	//}

	gpio_direction_output(chip->normalchg_gpio.chargerid_switch_gpio, 1);
	ret = pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.charger_gpio_as_output2);
	if (ret < 0) {
		chg_err("failed to set pinctrl int\n");
	}
	chg_err("set_usbswitch_to_rxtx\n");
	return;
}

static void set_usbswitch_to_dpdm(struct oppo_chg_chip *chip)
{
	int ret = 0;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	gpio_direction_output(chip->normalchg_gpio.chargerid_switch_gpio, 0);
	ret = pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.charger_gpio_as_output1);
	if (ret < 0) {
		chg_err("failed to set pinctrl int\n");
		return;
	}
	chg_err("set_usbswitch_to_dpdm\n");
}
static bool chargerid_support = false;
static bool is_support_chargerid_check(void)
{
#ifdef CONFIG_OPPO_CHECK_CHARGERID_VOLT
	return chargerid_support;
#else
	return false;
#endif
}

int mt_get_chargerid_volt(void)
{
	int chargerid_volt = 0;
	int rc = 0;

	if (!pinfo->charger_id_chan) {
                chg_err("charger_id_chan NULL\n");
                return 0;
        }

	if (is_support_chargerid_check() == true) {
		rc = iio_read_channel_processed(pinfo->charger_id_chan, &chargerid_volt);
		if (rc < 0) {
			chg_err("read charger_id_chan fail, rc=%d\n", rc);
			return 0;
		}

		chargerid_volt = chargerid_volt * 1500 / 4096;
		chg_debug("chargerid_volt=%d\n", chargerid_volt);
	} else {
		chg_debug("is_support_chargerid_check=false!\n");
		return 0;
	}

	return chargerid_volt;
}

void mt_set_chargerid_switch_val(int value)
{
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	if (is_support_chargerid_check() == false)
		return;

	if (chip->normalchg_gpio.chargerid_switch_gpio <= 0) {
		chg_err("chargerid_switch_gpio not exist, return\n");
		return;
	}
	if (IS_ERR_OR_NULL(chip->normalchg_gpio.pinctrl)
			|| IS_ERR_OR_NULL(chip->normalchg_gpio.charger_gpio_as_output1)
			|| IS_ERR_OR_NULL(chip->normalchg_gpio.charger_gpio_as_output2)) {
		chg_err("pinctrl null, return\n");
		return;
	}

	if (value == 1) {
		set_usbswitch_to_rxtx(chip);
	} else if (value == 0) {
		set_usbswitch_to_dpdm(chip);
	} else {
		//do nothing
	}
	chg_debug("get_val=%d\n", gpio_get_value(chip->normalchg_gpio.chargerid_switch_gpio));

	return;
}

int mt_get_chargerid_switch_val(void)
{
	int gpio_status = 0;
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 0;
	}
	if (is_support_chargerid_check() == false)
		return 0;

	gpio_status = gpio_get_value(chip->normalchg_gpio.chargerid_switch_gpio);

	chg_debug("mt_get_chargerid_switch_val=%d\n", gpio_status);

	return gpio_status;
}

static int oppo_usb_switch_gpio_gpio_init(void)
{
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return -EINVAL;
	}

	chip->normalchg_gpio.pinctrl = devm_pinctrl_get(chip->dev);
	if (IS_ERR_OR_NULL(chip->normalchg_gpio.pinctrl)) {
		chg_err("get normalchg_gpio.chargerid_switch_gpio pinctrl fail\n");
		return -EINVAL;
	}

	chip->normalchg_gpio.charger_gpio_as_output1 =
			pinctrl_lookup_state(chip->normalchg_gpio.pinctrl,
			"charger_gpio_as_output_low");
	if (IS_ERR_OR_NULL(chip->normalchg_gpio.charger_gpio_as_output1)) {
		chg_err("get charger_gpio_as_output_low fail\n");
		return -EINVAL;
	}

	chip->normalchg_gpio.charger_gpio_as_output2 =
			pinctrl_lookup_state(chip->normalchg_gpio.pinctrl,
			"charger_gpio_as_output_high");
	if (IS_ERR_OR_NULL(chip->normalchg_gpio.charger_gpio_as_output2)) {
		chg_err("get charger_gpio_as_output_high fail\n");
		return -EINVAL;
	}

	pinctrl_select_state(chip->normalchg_gpio.pinctrl,
			chip->normalchg_gpio.charger_gpio_as_output1);

	return 0;
}

static int oppo_chg_chargerid_parse_dt(struct oppo_chg_chip *chip)
{
	int rc = 0;
	struct device_node *node = NULL;

	if (chip != NULL)
		node = chip->dev->of_node;

	if (chip == NULL || node == NULL) {
		chg_err("oppo_chip or device tree info. missing\n");
		return -EINVAL;
	}

	chargerid_support = of_property_read_bool(node, "qcom,chargerid_support");
	if (chargerid_support == false){
		chg_err("not support chargerid\n");
		//return -EINVAL;
	}

	chip->normalchg_gpio.chargerid_switch_gpio =
			of_get_named_gpio(node, "qcom,chargerid_switch-gpio", 0);
	if (chip->normalchg_gpio.chargerid_switch_gpio <= 0) {
		chg_err("Couldn't read chargerid_switch-gpio rc=%d, chargerid_switch-gpio:%d\n",
				rc, chip->normalchg_gpio.chargerid_switch_gpio);
	} else {
		if (gpio_is_valid(chip->normalchg_gpio.chargerid_switch_gpio)) {
			rc = gpio_request(chip->normalchg_gpio.chargerid_switch_gpio, "charging_switch1-gpio");
			if (rc) {
				chg_err("unable to request chargerid_switch-gpio:%d\n",
						chip->normalchg_gpio.chargerid_switch_gpio);
			} else {
				rc = oppo_usb_switch_gpio_gpio_init();
				if (rc)
					chg_err("unable to init chargerid_switch-gpio:%d\n",
							chip->normalchg_gpio.chargerid_switch_gpio);
			}
		}
		chg_err("chargerid_switch-gpio:%d\n", chip->normalchg_gpio.chargerid_switch_gpio);
	}

	return rc;
}
#endif /*VENDOR_EDIT*/
//====================================================================//


//====================================================================//
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for HW shortc */
static bool oppo_shortc_check_is_gpio(struct oppo_chg_chip *chip)
{
	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return false;
	}

	if (gpio_is_valid(chip->normalchg_gpio.shortc_gpio)) {
		return true;
	}

	return false;
}

static int oppo_shortc_gpio_init(struct oppo_chg_chip *chip)
{
	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return -EINVAL;
	}

	chip->normalchg_gpio.pinctrl = devm_pinctrl_get(chip->dev);

	chip->normalchg_gpio.shortc_active =
		pinctrl_lookup_state(chip->normalchg_gpio.pinctrl, "shortc_active");
	if (IS_ERR_OR_NULL(chip->normalchg_gpio.shortc_active)) {
		chg_err("get shortc_active fail\n");
		return -EINVAL;
	}

	pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.shortc_active);

	return 0;
}

#ifdef CONFIG_OPPO_SHORT_HW_CHECK	
static bool oppo_chg_get_shortc_hw_gpio_status(void)
{
	bool shortc_hw_status = 1;
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return shortc_hw_status;
	}

	if (oppo_shortc_check_is_gpio(chip) == true) {	
		shortc_hw_status = !!(gpio_get_value(chip->normalchg_gpio.shortc_gpio));
	}
	return shortc_hw_status;
}
#else /* CONFIG_OPPO_SHORT_HW_CHECK */
static bool oppo_chg_get_shortc_hw_gpio_status(void)
{
	bool shortc_hw_status = 1;

	return shortc_hw_status;
}
#endif /* CONFIG_OPPO_SHORT_HW_CHECK */

static int oppo_chg_shortc_hw_parse_dt(struct oppo_chg_chip *chip)
{
	int rc = 0;
	struct device_node *node = NULL;

        if (chip != NULL)
		node = chip->dev->of_node;

	if (chip == NULL || node == NULL) {
		chg_err("oppo_chip or device tree info. missing\n");
		return -EINVAL;
	}

	chip->normalchg_gpio.shortc_gpio = of_get_named_gpio(node, "qcom,shortc-gpio", 0);
	if (chip->normalchg_gpio.shortc_gpio <= 0) {
		chg_err("Couldn't read qcom,shortc-gpio rc=%d, qcom,shortc-gpio:%d\n",
				rc, chip->normalchg_gpio.shortc_gpio);
	} else {
		if (oppo_shortc_check_is_gpio(chip) == true) {
			rc = gpio_request(chip->normalchg_gpio.shortc_gpio, "shortc-gpio");
			if (rc) {
				chg_err("unable to request shortc-gpio:%d\n",
						chip->normalchg_gpio.shortc_gpio);
			} else {
				rc = oppo_shortc_gpio_init(chip);
				if (rc)
					chg_err("unable to init shortc-gpio:%d\n", chip->normalchg_gpio.ship_gpio);
			}
		}
		chg_err("shortc-gpio:%d\n", chip->normalchg_gpio.shortc_gpio);
	}

	return rc;
}
#endif /*VENDOR_EDIT*/
//====================================================================//


//====================================================================//
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for using gpio as shipmode stm6620 */
static bool oppo_ship_check_is_gpio(struct oppo_chg_chip *chip)
{
	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return false;
	}

	if (gpio_is_valid(chip->normalchg_gpio.ship_gpio))
		return true;

	return false;
}

static int oppo_ship_gpio_init(struct oppo_chg_chip *chip)
{
	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return -EINVAL;
	}

	chip->normalchg_gpio.pinctrl = devm_pinctrl_get(chip->dev);
	chip->normalchg_gpio.ship_active = 
		pinctrl_lookup_state(chip->normalchg_gpio.pinctrl, 
			"ship_active");

	if (IS_ERR_OR_NULL(chip->normalchg_gpio.ship_active)) {
		chg_err("get ship_active fail\n");
		return -EINVAL;
	}
	chip->normalchg_gpio.ship_sleep = 
			pinctrl_lookup_state(chip->normalchg_gpio.pinctrl, 
				"ship_sleep");
	if (IS_ERR_OR_NULL(chip->normalchg_gpio.ship_sleep)) {
		chg_err("get ship_sleep fail\n");
		return -EINVAL;
	}

	pinctrl_select_state(chip->normalchg_gpio.pinctrl,
		chip->normalchg_gpio.ship_sleep);

	return 0;
}

#define SHIP_MODE_CONFIG		0x40
#define SHIP_MODE_MASK			BIT(0)
#define SHIP_MODE_ENABLE		0
#define PWM_COUNT				5
static void smbchg_enter_shipmode(struct oppo_chg_chip *chip)
{
	int i = 0;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	if (oppo_ship_check_is_gpio(chip) == true) {
		chg_err("select gpio control\n");
		if (!IS_ERR_OR_NULL(chip->normalchg_gpio.ship_active) && !IS_ERR_OR_NULL(chip->normalchg_gpio.ship_sleep)) {
			pinctrl_select_state(chip->normalchg_gpio.pinctrl,
				chip->normalchg_gpio.ship_sleep);
			for (i = 0; i < PWM_COUNT; i++) {
				//gpio_direction_output(chip->normalchg_gpio.ship_gpio, 1);
				pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.ship_active);
				mdelay(3);
				//gpio_direction_output(chip->normalchg_gpio.ship_gpio, 0);
				pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.ship_sleep);
				mdelay(3);
			}
		}
		chg_err("power off after 15s\n");
	}
}
static void enter_ship_mode_function(struct oppo_chg_chip *chip)
{
	struct charger_device *chg = NULL;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	if (chip->enable_shipmode) {
		if (pinfo->data.dual_charger_support) {
			mt6360_enter_shipmode();
		} else {
			smbchg_enter_shipmode(chip);
		}
		printk(KERN_ERR "[OPPO_CHG][%s]: enter_ship_mode_function\n", __func__);
	}
}

static int oppo_chg_shipmode_parse_dt(struct oppo_chg_chip *chip)
{
	int rc = 0;
	struct device_node *node = NULL;

        if (chip != NULL)
        	node = chip->dev->of_node;

	if (chip == NULL || node == NULL) {
		chg_err("oppo_chip or device tree info. missing\n");
		return -EINVAL;
	}

	chip->normalchg_gpio.ship_gpio =
			of_get_named_gpio(node, "qcom,ship-gpio", 0);
	if (chip->normalchg_gpio.ship_gpio <= 0) {
		chg_err("Couldn't read qcom,ship-gpio rc = %d, qcom,ship-gpio:%d\n",
				rc, chip->normalchg_gpio.ship_gpio);
	} else {
		if (oppo_ship_check_is_gpio(chip) == true) {
			rc = gpio_request(chip->normalchg_gpio.ship_gpio, "ship-gpio");
			if (rc) {
				chg_err("unable to request ship-gpio:%d\n", chip->normalchg_gpio.ship_gpio);
			} else {
				rc = oppo_ship_gpio_init(chip);
				if (rc)
					chg_err("unable to init ship-gpio:%d\n", chip->normalchg_gpio.ship_gpio);
			}
		}
		chg_err("ship-gpio:%d\n", chip->normalchg_gpio.ship_gpio);
	}

	return rc;
}
#endif /* VENDOR_EDIT */
//====================================================================//

#ifdef VENDOR_EDIT
/* Baoquan.Lai@BSP.CHG.Basic, 2020/05/19, Add for chargeric temp checkout*/
#define CHARGER_25C_VOLT	2457//mv
static void oppo_get_chargeric_temp_volt(struct charger_data *pdata)
{
	int rc = 0;
	int chargeric_temp_volt = 0;

	if (!pdata) {
		printk(KERN_ERR "[OPPO_CHG][%s]: charger_data not ready!\n", __func__);
		return;
	}

	if (!pinfo->chargeric_temp_chan) {
		chg_err("chargeric_temp_chan NULL\n");
                return;
	}

	rc = iio_read_channel_processed(pinfo->chargeric_temp_chan, &chargeric_temp_volt);
        if (rc < 0) {
                chg_err("read chargeric_temp_chan volt failed, rc=%d\n", rc);
        }

	if (chargeric_temp_volt <= 0) {
                chargeric_temp_volt = CHARGER_25C_VOLT;
        }

	pdata->chargeric_temp_volt= chargeric_temp_volt * 1500 / 4096;

	chg_err("chargeric_temp_volt: %d\n", pdata->chargeric_temp_volt);

	return;
}

static void get_chargeric_temp(struct charger_data *pdata)
{
	int i = 0;

	if (!pdata) {
		printk(KERN_ERR "[OPPO_CHG][%s]: charger_data not ready!\n", __func__);
		return;
	}

	for (i = ARRAY_SIZE(con_volt_19165) - 1; i >= 0; i--) {
		if (con_volt_19165[i] >= pdata->chargeric_temp_volt)
			break;
		else if (i == 0)
			break;
	}
	pdata->chargeric_temp= con_temp_19165[i];

	chg_err("chargeric_temp: %d\n", pdata->chargeric_temp);

	return;
}
#endif

//====================================================================//
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2019/06/18, sjc Add for usbtemp */
static bool oppo_usbtemp_check_is_gpio(struct oppo_chg_chip *chip)
{
	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return false;
	}

	if (gpio_is_valid(chip->normalchg_gpio.dischg_gpio))
		return true;

	return false;
}

static bool oppo_usbtemp_check_is_support(void)
{
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return false;
	}

	if (get_boot_mode() == KERNEL_POWER_OFF_CHARGING_BOOT)
		return false;

	if (oppo_usbtemp_check_is_gpio(chip) == true)
		return true;

	chg_err("not support, return false\n");

	return false;
}

static int oppo_dischg_gpio_init(struct oppo_chg_chip *chip)
{
	if (!chip) {
		chg_err("oppo_chip not ready!\n");
		return -EINVAL;
	}

	chip->normalchg_gpio.pinctrl = devm_pinctrl_get(chip->dev);

	if (IS_ERR_OR_NULL(chip->normalchg_gpio.pinctrl)) {
		chg_err("get dischg_pinctrl fail\n");
		return -EINVAL;
	}

	chip->normalchg_gpio.dischg_enable = pinctrl_lookup_state(chip->normalchg_gpio.pinctrl, "dischg_enable");
	if (IS_ERR_OR_NULL(chip->normalchg_gpio.dischg_enable)) {
		chg_err("get dischg_enable fail\n");
		return -EINVAL;
	}

	chip->normalchg_gpio.dischg_disable = pinctrl_lookup_state(chip->normalchg_gpio.pinctrl, "dischg_disable");
	if (IS_ERR_OR_NULL(chip->normalchg_gpio.dischg_disable)) {
		chg_err("get dischg_disable fail\n");
		return -EINVAL;
	}

	pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.dischg_disable);

	return 0;
}

#define USB_20C		20//degreeC
#define USB_40C		40
#define USB_50C		50
#define USB_57C		57
#define USB_100C	100
#define USB_25C_VOLT	2457//900//mv
#define USB_50C_VOLT	450
#define USB_55C_VOLT	448
#define USB_60C_VOLT	327
#define VBUS_VOLT_THRESHOLD	3000//3V
#define RETRY_CNT_DELAY		5 //ms
#define MIN_MONITOR_INTERVAL	50//50ms
#define MAX_MONITOR_INTERVAL	200//200ms
#define VBUS_MONITOR_INTERVAL	3000//3s

static void oppo_get_usbtemp_volt(struct oppo_chg_chip *chip)
{
	int rc = 0;
	int usbtemp_volt = 0;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	if (!pinfo->usb_temp_v_l_chan
		|| !pinfo->usb_temp_v_r_chan) {
		chg_err("usb_temp_v_l_chan or usb_temp_v_r_chan NULL\n");
                return;
	}

	rc = iio_read_channel_processed(pinfo->usb_temp_v_l_chan, &usbtemp_volt);
        if (rc < 0) {
                chg_err("read usb_temp_v_l_chan volt failed, rc=%d\n", rc);
        }

	if (usbtemp_volt <= 0) {
                usbtemp_volt = USB_25C_VOLT;
        }
	chip->usbtemp_volt_l = usbtemp_volt * 1500 / 4096;

        rc = iio_read_channel_processed(pinfo->usb_temp_v_r_chan, &usbtemp_volt);
        if (rc < 0) {
                chg_err("read usb_temp_v_r_chan volt failed, rc=%d\n", rc);
        }

	if (usbtemp_volt <= 0) {
		usbtemp_volt = USB_25C_VOLT;
	}
	chip->usbtemp_volt_r = usbtemp_volt * 1500 / 4096;

       //chg_err("usbtemp_volt: %d, %d\n", chip->usbtemp_volt_r, chip->usbtemp_volt_l);

	return;
}

static void get_usb_temp(struct oppo_chg_chip *chip)
{
	int i = 0;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return;
	}

	for (i = ARRAY_SIZE(con_volt_19165) - 1; i >= 0; i--) {
		if (con_volt_19165[i] >= chip->usbtemp_volt_r)
			break;
		else if (i == 0)
			break;
	}
	chip->usb_temp_r = con_temp_19165[i];

	for (i = ARRAY_SIZE(con_volt_19165) - 1; i >= 0; i--) {
		if (con_volt_19165[i] >= chip->usbtemp_volt_l)
			break;
		else if (i == 0)
			break;
	}
	chip->usb_temp_l = con_temp_19165[i];

	//chg_err("usbtemp: %d, %d\n", chip->usb_temp_r, chip->usb_temp_l);

	return;
}

static bool oppo_chg_get_vbus_status(struct oppo_chg_chip *chip)
{
	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return false;
	}

	return chip->charger_exist;
}

static void usbtemp_callback(unsigned long a)
{
	struct oppo_chg_chip *chip = g_oppo_chip;

		pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.dischg_disable);
		oppo_clear_usb_status(USB_TEMP_HIGH);
		chg_err("usbtemp exit after 4hours \n");
		}

static void usbtemp_exit_init (void)
{
	init_timer(& usbtemp_timer);
	usbtemp_timer.expires = jiffies + ( 14400 * HZ);    // 4 hours
	usbtemp_timer.function = &usbtemp_callback ;
	usbtemp_timer.data = ((unsigned long)0);
	}

#define RETRY_COUNT		3
static int oppo_usbtemp_monitor_main(void *data)
{
	int i = 0;
	int delay = 0;
	int vbus_volt = 0;
	int count_r = 0;
	int count_l = 0;
	int time_count = 0;
	static bool dischg_flag = false;
	static bool limit_current_flag = false;
	static int count = 0;
	static int last_usb_temp_r = 25;
	static int last_usb_temp_l = 25;
	static bool init_flag = true;
	struct oppo_chg_chip *chip = g_oppo_chip;

	while (!kthread_should_stop() && dischg_flag == false) {
		if (oppo_get_chg_powersave() == true) {
			delay = 1000 * 30;//30S
			goto check_again;
		}
		
		usbtemp_exit_init();
		oppo_get_usbtemp_volt(chip);
		get_usb_temp(chip);

		if (init_flag == true) {
			init_flag = false;
			if (oppo_chg_get_vbus_status(chip) == false
					&& oppo_chg_get_otg_online() == false) {
				delay = MIN_MONITOR_INTERVAL * 20;
				goto check_again;
			}
		}
		if (chip->usb_temp_r < USB_50C && chip->usb_temp_l < USB_50C)//get vbus when usbtemp < 50C
			vbus_volt = battery_meter_get_charger_voltage();
		else
			vbus_volt = 0;

		if (chip->usb_temp_r < USB_40C && chip->usb_temp_l < USB_40C) {
			delay = MAX_MONITOR_INTERVAL;
			time_count = 8;
		} else {
			delay = MIN_MONITOR_INTERVAL;
			time_count = 30;
		}

		if (chip->usb_temp_r < USB_50C && chip->usb_temp_l < USB_50C && vbus_volt < VBUS_VOLT_THRESHOLD)
			delay = VBUS_MONITOR_INTERVAL;

		if ((chip->usb_temp_r >= USB_57C && chip->usb_temp_r < USB_100C)
				|| (chip->usb_temp_l >= USB_57C && chip->usb_temp_l < USB_100C)) {
			for (i = 0; i < RETRY_COUNT; i++) {
				mdelay(RETRY_CNT_DELAY);
				oppo_get_usbtemp_volt(chip);
				get_usb_temp(chip);
				if (chip->usb_temp_r >= USB_57C)
					count_r++;
				if (chip->usb_temp_l >= USB_57C)
					count_l++;
			}

			if (count_r >= RETRY_COUNT || count_l >= RETRY_COUNT) {
				if (!IS_ERR_OR_NULL(chip->normalchg_gpio.dischg_enable)) {
					dischg_flag = true;
					chg_err("dischg enable...usb_temp[%d,%d], usb_volt[%d,%d]\n",
							chip->usb_temp_r, chip->usb_temp_l, chip->usbtemp_volt_r, chip->usbtemp_volt_l);
#ifndef CONFIG_HIGH_TEMP_VERSION
					oppo_set_usb_status(USB_TEMP_HIGH);
					if (oppo_chg_get_otg_online() == true) {
						oppo_set_otg_switch_status(false);
						usleep_range(10000, 11000);
					}
					if (oppo_vooc_get_fastchg_started() == true) {
						oppo_chg_set_chargerid_switch_val(0);
						oppo_vooc_switch_mode(NORMAL_CHARGER_MODE);
						oppo_vooc_reset_mcu();
						//msleep(20);//wait for turn-off fastchg MOS
					}
					//chip->chg_ops->charging_disable();
					usleep_range(10000, 11000);
					bq25910_disable_charging();
					chip->chg_ops->charger_suspend();
					usleep_range(10000, 11000);
					pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.dischg_enable);
					add_timer(&usbtemp_timer);
#endif
				}
			}

			count_r = 0;
			count_l = 0;
			count = 0;
		} else if (((chip->usb_temp_r  - chip->temperature/10) > 12 && chip->usb_temp_r >= USB_20C && chip->usb_temp_r < USB_100C)
				|| ((chip->usb_temp_l  - chip->temperature/10) > 12 && chip->usb_temp_l >= USB_20C && chip->usb_temp_l < USB_100C)) {
			if (count <= time_count) {
				if (count == 0) {
					last_usb_temp_r = chip->usb_temp_r;
					last_usb_temp_l = chip->usb_temp_l;
				}

				if (((chip->usb_temp_r - last_usb_temp_r) >= 3 )
						|| ((chip->usb_temp_l - last_usb_temp_l) >= 3 )) {
					for (i = 0; i < RETRY_COUNT; i++) {
						mdelay(RETRY_CNT_DELAY);
						oppo_get_usbtemp_volt(chip);
						get_usb_temp(chip);
						if ((chip->usb_temp_r - last_usb_temp_r) >= 3)
							count_r++;
						if ((chip->usb_temp_l - last_usb_temp_l) >= 3)
							count_l++;
					}

					if (count_r >= RETRY_COUNT || count_l >= RETRY_COUNT) {
						if (!IS_ERR_OR_NULL(chip->normalchg_gpio.dischg_enable)) {
							dischg_flag = true;
							chg_err("dischg enable...current_usb_temp[%d,%d], last_usb_temp[%d,%d], count[%d]\n",
									chip->usb_temp_r, chip->usb_temp_l, last_usb_temp_r, last_usb_temp_l, count);
#ifndef CONFIG_HIGH_TEMP_VERSION
							oppo_set_usb_status(USB_TEMP_HIGH);
							if (oppo_chg_get_otg_online() == true) {
								oppo_set_otg_switch_status(false);
								usleep_range(10000, 11000);
							}
							if (oppo_vooc_get_fastchg_started() == true) {
								oppo_chg_set_chargerid_switch_val(0);
								oppo_vooc_switch_mode(NORMAL_CHARGER_MODE);
								oppo_vooc_reset_mcu();
								//msleep(20);//wait for turn-off fastchg MOS
							}
							//chip->chg_ops->charging_disable();
							usleep_range(10000, 11000);
							bq25910_disable_charging();
							chip->chg_ops->charger_suspend();
							usleep_range(10000, 11000);
							pinctrl_select_state(chip->normalchg_gpio.pinctrl, chip->normalchg_gpio.dischg_enable);
							add_timer(&usbtemp_timer);
#endif
						}
					}

					count_r = 0;
					count_l = 0;
				}

				count++;
				if (count > time_count) {
					count = 0;
				}
			}
			msleep(delay);
		}  else {
check_again:
			count = 0;
			last_usb_temp_r = chip->usb_temp_r;
			last_usb_temp_l = chip->usb_temp_l;
			msleep(delay);
			wait_event_interruptible(oppo_usbtemp_wq,
				oppo_chg_get_vbus_status(chip) == true /*|| oppo_chg_get_otg_online() == true*/ );
		}
	}

	return 0;
}

static void oppo_usbtemp_thread_init(void)
{
	oppo_usbtemp_kthread =
			kthread_run(oppo_usbtemp_monitor_main, 0, "usbtemp_kthread");
	if (IS_ERR(oppo_usbtemp_kthread)) {
		chg_err("failed to cread oppo_usbtemp_kthread\n");
	}
}

void oppo_wake_up_usbtemp_thread(void)
{
	if (oppo_usbtemp_check_is_support() == true) {
		wake_up_interruptible(&oppo_usbtemp_wq);
	}
}

static int oppo_chg_usbtemp_parse_dt(struct oppo_chg_chip *chip)
{
	int rc = 0;
	struct device_node *node = NULL;

	if (chip)
		node = chip->dev->of_node;
	if (node == NULL) {
		chg_err("oppo_chip or device tree info. missing\n");
		return -EINVAL;
	}

	chip->normalchg_gpio.dischg_gpio = of_get_named_gpio(node, "qcom,dischg-gpio", 0);
	if (chip->normalchg_gpio.dischg_gpio <= 0) {
		chg_err("Couldn't read qcom,dischg-gpio rc=%d, qcom,dischg-gpio:%d\n",
				rc, chip->normalchg_gpio.dischg_gpio);
	} else {
		if (oppo_usbtemp_check_is_support() == true) {
			rc = gpio_request(chip->normalchg_gpio.dischg_gpio, "dischg-gpio");
			if (rc) {
				chg_err("unable to request dischg-gpio:%d\n",
						chip->normalchg_gpio.dischg_gpio);
			} else {
				rc = oppo_dischg_gpio_init(chip);
				if (rc)
					chg_err("unable to init dischg-gpio:%d\n",
							chip->normalchg_gpio.dischg_gpio);
			}
		}
		chg_err("dischg-gpio:%d\n", chip->normalchg_gpio.dischg_gpio);
	}

	return rc;
}

static bool oppo_mtk_hv_flashled_check_is_gpio()
{
	if (gpio_is_valid(mtkhv_flashled_pinctrl.chgvin_gpio) && gpio_is_valid(mtkhv_flashled_pinctrl.pmic_chgfunc_gpio)) {
		return true;
	} else {
		chg_err("[%s] fail\n", __func__);
		return false;
	}
}
static int oppo_mtk_hv_flashled_dt(struct oppo_chg_chip *chip)
{
	int rc = 0;
	struct device_node *node = NULL;

	if (chip != NULL)
		node = chip->dev->of_node;

	if (chip == NULL || node == NULL) {
		chg_err("[%s] oppo_chip or device tree info. missing\n", __func__);
		return -EINVAL;
	}

	mtkhv_flashled_pinctrl.hv_flashled_support = of_property_read_bool(node, "qcom,hv_flashled_support");
	if (mtkhv_flashled_pinctrl.hv_flashled_support == false) {
		chg_err("[%s] hv_flashled_support not support\n", __func__);
		return -EINVAL;
	}

	mtkhv_flashled_pinctrl.chgvin_gpio = of_get_named_gpio(node, "qcom,chgvin", 0);
	mtkhv_flashled_pinctrl.pmic_chgfunc_gpio = of_get_named_gpio(node, "qcom,pmic_chgfunc", 0);
	if (mtkhv_flashled_pinctrl.chgvin_gpio <= 0 || mtkhv_flashled_pinctrl.pmic_chgfunc_gpio <= 0) {
		chg_err("read dts fail %d %d\n", mtkhv_flashled_pinctrl.chgvin_gpio, mtkhv_flashled_pinctrl.pmic_chgfunc_gpio);
	} else {
		if (oppo_mtk_hv_flashled_check_is_gpio() == true) {
			rc = gpio_request(mtkhv_flashled_pinctrl.chgvin_gpio, "chgvin");
			if (rc ) {
				chg_err("unable to request chgvin:%d\n",
						mtkhv_flashled_pinctrl.chgvin_gpio);
			} else {
				mtkhv_flashled_pinctrl.pinctrl= devm_pinctrl_get(chip->dev);
				//chgvin
				mtkhv_flashled_pinctrl.chgvin_enable =
					pinctrl_lookup_state(mtkhv_flashled_pinctrl.pinctrl, "chgvin_enable");
				if (IS_ERR_OR_NULL(mtkhv_flashled_pinctrl.chgvin_enable)) {
					chg_err("get chgvin_enable fail\n");
					return -EINVAL;
				}

				mtkhv_flashled_pinctrl.chgvin_disable =
					pinctrl_lookup_state(mtkhv_flashled_pinctrl.pinctrl, "chgvin_disable");
				if (IS_ERR_OR_NULL(mtkhv_flashled_pinctrl.chgvin_disable)) {
					chg_err("get chgvin_disable fail\n");
					return -EINVAL;
				}
				pinctrl_select_state(mtkhv_flashled_pinctrl.pinctrl, mtkhv_flashled_pinctrl.chgvin_enable);


				rc = gpio_request(mtkhv_flashled_pinctrl.chgvin_gpio, "pmic_chgfunc");
				if (rc ) {
					chg_err("unable to request pmic_chgfunc:%d\n",
							mtkhv_flashled_pinctrl.chgvin_gpio);
				} else {

					//pmic_chgfunc
					mtkhv_flashled_pinctrl.pmic_chgfunc_enable =
						pinctrl_lookup_state(mtkhv_flashled_pinctrl.pinctrl, "pmic_chgfunc_enable");
					if (IS_ERR_OR_NULL(mtkhv_flashled_pinctrl.pmic_chgfunc_enable)) {
						chg_err("get pmic_chgfunc_enable fail\n");
						return -EINVAL;
					}

					mtkhv_flashled_pinctrl.pmic_chgfunc_disable =
						pinctrl_lookup_state(mtkhv_flashled_pinctrl.pinctrl, "pmic_chgfunc_disable");
					if (IS_ERR_OR_NULL(mtkhv_flashled_pinctrl.pmic_chgfunc_disable)) {
						chg_err("get pmic_chgfunc_disable fail\n");
						return -EINVAL;
					}
					pinctrl_select_state(mtkhv_flashled_pinctrl.pinctrl, mtkhv_flashled_pinctrl.pmic_chgfunc_disable);
				}

			}
		}
		//chg_err("mtk_hv_flash_led:%d\n", chip->normalchg_gpio.shortc_gpio);
	}

	chg_err("mtk_hv_flash_led:%d\n", rc);
	return rc;

}
#endif /* VENDOR_EDIT */
//====================================================================//


//====================================================================//
static int oppo_chg_parse_custom_dt(struct oppo_chg_chip *chip)
{
	int rc = 0;	
	if (chip == NULL) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return -EINVAL;
	}

	rc = oppo_chg_chargerid_parse_dt(chip);
	if (rc) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chg_chargerid_parse_dt fail!\n", __func__);
		return -EINVAL;
	}

	rc = oppo_chg_shipmode_parse_dt(chip);
	if (rc) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chg_shipmode_parse_dt fail!\n", __func__);
		return -EINVAL;
	}

	rc = oppo_chg_shortc_hw_parse_dt(chip);
	if (rc) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chg_shortc_hw_parse_dt fail!\n", __func__);
		return -EINVAL;
	}

	rc = oppo_chg_usbtemp_parse_dt(chip);
	if (rc) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chg_usbtemp_parse_dt fail!\n", __func__);
		return -EINVAL;
	}

	rc = oppo_mtk_hv_flashled_dt(chip);
	if (rc) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_mtk_hv_flashled_dt fail!\n", __func__);
		return -EINVAL;
	}
	return rc;
}
//====================================================================//


//====================================================================//
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for charging */
/************************************************/
/* Power Supply Functions
*************************************************/
static int mt_ac_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	int rc = 0;

	rc = oppo_ac_get_property(psy, psp, val);
	return rc;
}

static int mt_usb_prop_is_writeable(struct power_supply *psy,
	enum power_supply_property psp)
{
	int rc = 0;

	switch (psp) {
		case POWER_SUPPLY_PROP_WATER_DETECT_FEATURE:
			rc = 1;
			break;
		default:
			rc = oppo_usb_property_is_writeable(psy, psp);
	}
	return rc;
}

static int mt_usb_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	int rc = 0;

	switch (psp) {
		case POWER_SUPPLY_PROP_TYPEC_CC_ORIENTATION:
			if (pinfo != NULL && pinfo->tcpc != NULL) {
				if (tcpm_inquire_typec_attach_state(pinfo->tcpc) != TYPEC_UNATTACHED) {
					val->intval = (int)tcpm_inquire_cc_polarity(pinfo->tcpc) + 1;
				} else {
					val->intval = 0;
				}
				if (val->intval != 0)
					printk(KERN_ERR "[OPPO_CHG][%s]: cc[%d]\n", __func__, val->intval);
			} else {
				val->intval = 0;
			}
			break;
		case POWER_SUPPLY_PROP_TYPEC_SBU_VOLTAGE:
			val->intval = oppo_get_typec_sbu_voltage();
			break;
		case POWER_SUPPLY_PROP_WATER_DETECT_FEATURE:
			val->intval = oppo_get_water_detect();
			break;
		case POWER_SUPPLY_PROP_USB_STATUS:
			val->intval = oppo_get_usb_status();
			break;
		case POWER_SUPPLY_PROP_USBTEMP_VOLT_L:
			if (g_oppo_chip)
				val->intval = g_oppo_chip->usbtemp_volt_l;
			else
				val->intval = -ENODATA;
			break;
		case POWER_SUPPLY_PROP_USBTEMP_VOLT_R:
			if (g_oppo_chip)
				val->intval = g_oppo_chip->usbtemp_volt_r;
			else
				val->intval = -ENODATA;
			break;
		case POWER_SUPPLY_PROP_FAST_CHG_TYPE:
			if(((oppo_vooc_get_fastchg_started() == true) ||
				(oppo_vooc_get_fastchg_dummy_started() == true)) &&
				(g_oppo_chip->vbatt_num == 1) &&
				(oppo_vooc_get_fast_chg_type() == FASTCHG_CHARGER_TYPE_UNKOWN)){
				val->intval = CHARGER_SUBTYPE_FASTCHG_VOOC;
			} else {
				val->intval = oppo_vooc_get_fast_chg_type();
				if (val->intval == 0 && g_oppo_chip && g_oppo_chip->chg_ops->get_charger_subtype) {
					val->intval = g_oppo_chip->chg_ops->get_charger_subtype();
				}
			}
			break;

		default:
			rc = oppo_usb_get_property(psy, psp, val);
	}
	return rc;
}

static int mt_usb_set_property(struct power_supply *psy,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	int rc = 0;

	switch (psp) {
		case POWER_SUPPLY_PROP_WATER_DETECT_FEATURE:
			printk(KERN_ERR "[OPPO_CHG][%s]: oppo_set_water_detect[%d]\n", __func__, val->intval);
			if (val->intval == 0) {
				oppo_set_water_detect(false);
			} else {
				oppo_set_water_detect(true);
			}
			break;
		default:
			rc = oppo_usb_set_property(psy, psp, val);
	}
	return rc;
}

static int battery_prop_is_writeable(struct power_supply *psy,
	enum power_supply_property psp)
{
	int rc = 0;

	if (pinfo->data.dual_charger_support) {
		switch (psp) {
		case POWER_SUPPLY_PROP_EM_MODE:
			rc = 1;
			break;
		default :
			rc = oppo_battery_property_is_writeable(psy, psp);
			break;
		}
	} else {
		rc = oppo_battery_property_is_writeable(psy, psp);
	}

	return rc;
}

static int battery_set_property(struct power_supply *psy,
	enum power_supply_property psp, const union power_supply_propval *val)
{
	int rc = 0;

	if (pinfo->data.dual_charger_support) {
		switch (psp) {
		case POWER_SUPPLY_PROP_EM_MODE:
			if (val->intval > 0)
				em_mode = true;
			else
				em_mode = false;
			break;
		default:
			rc = oppo_battery_set_property(psy, psp, val);
			break;
		}
	} else {
		rc = oppo_battery_set_property(psy, psp, val);
	}

	return rc;
}

static int battery_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	int rc = 0;

	if (pinfo->data.dual_charger_support) {
		if (!g_oppo_chip) {
	                pr_err("%s, oppo_chip null\n", __func__);
	                return -1;
	        }

		switch (psp) {
		case POWER_SUPPLY_PROP_EM_MODE:
			val->intval = em_mode;
			break;
		case POWER_SUPPLY_PROP_SUB_CURRENT:
			if (g_oppo_chip->charger_exist) {
				val->intval = g_oppo_chip->icharging + oppo_chg_get_main_ibat();
			} else {
				val->intval = 0;
			}
			break;
		default:
			rc = oppo_battery_get_property(psy, psp, val);
			break;
		}
	} else {
		rc = oppo_battery_get_property(psy, psp, val);
	}

	return 0;
}

static enum power_supply_property mt_ac_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static enum power_supply_property mt_usb_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_OTG_SWITCH,
	POWER_SUPPLY_PROP_OTG_ONLINE,
	POWER_SUPPLY_PROP_TYPEC_CC_ORIENTATION,
	POWER_SUPPLY_PROP_TYPEC_SBU_VOLTAGE,
	POWER_SUPPLY_PROP_WATER_DETECT_FEATURE,
	POWER_SUPPLY_PROP_USB_STATUS,
	POWER_SUPPLY_PROP_USBTEMP_VOLT_L,
	POWER_SUPPLY_PROP_USBTEMP_VOLT_R,
	POWER_SUPPLY_PROP_FAST_CHG_TYPE,
};

static enum power_supply_property battery_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MIN,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_AUTHENTICATE,
	POWER_SUPPLY_PROP_CHARGE_TIMEOUT,
	POWER_SUPPLY_PROP_CHARGE_TECHNOLOGY,
	POWER_SUPPLY_PROP_FAST_CHARGE,
	POWER_SUPPLY_PROP_MMI_CHARGING_ENABLE,        /*add for MMI_CHG_TEST*/
#ifdef CONFIG_OPPO_CHARGER_MTK
	POWER_SUPPLY_PROP_STOP_CHARGING_ENABLE,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
	POWER_SUPPLY_PROP_CURRENT_MAX,
#endif
	POWER_SUPPLY_PROP_BATTERY_FCC,
	POWER_SUPPLY_PROP_BATTERY_SOH,
	POWER_SUPPLY_PROP_BATTERY_CC,
	POWER_SUPPLY_PROP_BATTERY_RM,
	POWER_SUPPLY_PROP_BATTERY_NOTIFY_CODE,
#ifdef CONFIG_OPPO_SMART_CHARGER_SUPPORT
	POWER_SUPPLY_PROP_COOL_DOWN,
#endif
	POWER_SUPPLY_PROP_ADAPTER_FW_UPDATE,
	POWER_SUPPLY_PROP_VOOCCHG_ING,
#ifdef CONFIG_OPPO_CHECK_CHARGERID_VOLT
	POWER_SUPPLY_PROP_CHARGERID_VOLT,
#endif
#ifdef CONFIG_OPPO_SHIP_MODE_SUPPORT
	POWER_SUPPLY_PROP_SHIP_MODE,
#endif
#ifdef CONFIG_OPPO_CALL_MODE_SUPPORT
	POWER_SUPPLY_PROP_CALL_MODE,
#endif
#ifdef CONFIG_OPPO_SHORT_C_BATT_CHECK
#ifdef CONFIG_OPPO_SHORT_USERSPACE
	POWER_SUPPLY_PROP_SHORT_C_LIMIT_CHG,
	POWER_SUPPLY_PROP_SHORT_C_LIMIT_RECHG,
	POWER_SUPPLY_PROP_CHARGE_TERM_CURRENT,
	POWER_SUPPLY_PROP_INPUT_CURRENT_SETTLED,
#else
	POWER_SUPPLY_PROP_SHORT_C_BATT_UPDATE_CHANGE,
	POWER_SUPPLY_PROP_SHORT_C_BATT_IN_IDLE,
	POWER_SUPPLY_PROP_SHORT_C_BATT_CV_STATUS,
#endif /*CONFIG_OPPO_SHORT_USERSPACE*/
#endif
#ifdef CONFIG_OPPO_SHORT_HW_CHECK
	POWER_SUPPLY_PROP_SHORT_C_HW_FEATURE,
	POWER_SUPPLY_PROP_SHORT_C_HW_STATUS,
#endif
#ifdef CONFIG_OPPO_SHORT_IC_CHECK
	POWER_SUPPLY_PROP_SHORT_C_IC_OTP_STATUS,
	POWER_SUPPLY_PROP_SHORT_C_IC_VOLT_THRESH,
	POWER_SUPPLY_PROP_SHORT_C_IC_OTP_VALUE,
#endif
#ifdef CONFIG_OPPO_CHIP_SOC_NODE
	POWER_SUPPLY_PROP_CHIP_SOC,
	POWER_SUPPLY_PROP_SMOOTH_SOC,
#endif
	POWER_SUPPLY_PROP_EM_MODE,
	POWER_SUPPLY_PROP_SUB_CURRENT,
};

static int oppo_power_supply_init(struct oppo_chg_chip *chip)
{
	int ret = 0;
	struct oppo_chg_chip *mt_chg = NULL;

	if (chip == NULL) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return -EINVAL;
	}
	mt_chg = chip;

	mt_chg->ac_psd.name = "ac";
	mt_chg->ac_psd.type = POWER_SUPPLY_TYPE_MAINS;
	mt_chg->ac_psd.properties = mt_ac_properties;
	mt_chg->ac_psd.num_properties = ARRAY_SIZE(mt_ac_properties);
	mt_chg->ac_psd.get_property = mt_ac_get_property;
	mt_chg->ac_cfg.drv_data = mt_chg;

	mt_chg->usb_psd.name = "usb";
	mt_chg->usb_psd.type = POWER_SUPPLY_TYPE_USB;
	mt_chg->usb_psd.properties = mt_usb_properties;
	mt_chg->usb_psd.num_properties = ARRAY_SIZE(mt_usb_properties);
	mt_chg->usb_psd.get_property = mt_usb_get_property;
	mt_chg->usb_psd.set_property = mt_usb_set_property;
	mt_chg->usb_psd.property_is_writeable = mt_usb_prop_is_writeable;
	mt_chg->usb_cfg.drv_data = mt_chg;
    
	mt_chg->battery_psd.name = "battery";
	mt_chg->battery_psd.type = POWER_SUPPLY_TYPE_BATTERY;
	mt_chg->battery_psd.properties = battery_properties;
	mt_chg->battery_psd.num_properties = ARRAY_SIZE(battery_properties);
	mt_chg->battery_psd.get_property = battery_get_property;
	mt_chg->battery_psd.set_property = battery_set_property;
	mt_chg->battery_psd.property_is_writeable = battery_prop_is_writeable;

	mt_chg->ac_psy = power_supply_register(mt_chg->dev, &mt_chg->ac_psd,
			&mt_chg->ac_cfg);
	if (IS_ERR(mt_chg->ac_psy)) {
		dev_err(mt_chg->dev, "Failed to register power supply ac: %ld\n",
			PTR_ERR(mt_chg->ac_psy));
		ret = PTR_ERR(mt_chg->ac_psy);
		goto err_ac_psy;
	}

	mt_chg->usb_psy = power_supply_register(mt_chg->dev, &mt_chg->usb_psd,
			&mt_chg->usb_cfg);
	if (IS_ERR(mt_chg->usb_psy)) {
		dev_err(mt_chg->dev, "Failed to register power supply usb: %ld\n",
			PTR_ERR(mt_chg->usb_psy));
		ret = PTR_ERR(mt_chg->usb_psy);
		goto err_usb_psy;
	}

	mt_chg->batt_psy = power_supply_register(mt_chg->dev, &mt_chg->battery_psd,
			NULL);
	if (IS_ERR(mt_chg->batt_psy)) {
		dev_err(mt_chg->dev, "Failed to register power supply battery: %ld\n",
			PTR_ERR(mt_chg->batt_psy));
		ret = PTR_ERR(mt_chg->batt_psy);
		goto err_battery_psy;
	}

	chg_err("%s OK\n", __func__);
	return 0;

err_battery_psy:
	power_supply_unregister(mt_chg->usb_psy);
err_usb_psy:
	power_supply_unregister(mt_chg->ac_psy);
err_ac_psy:

	return ret;
}
#endif /* VENDOR_EDIT */
//====================================================================//


//====================================================================//
#ifdef VENDOR_EDIT
/* Jianchao.Shi@PSW.BSP.CHG.Basic, 2018/12/10, sjc Add for OTG switch */
void oppo_set_otg_switch_status(bool value)
{
	if (pinfo != NULL && pinfo->tcpc != NULL) {
		printk(KERN_ERR "[OPPO_CHG][%s]: otg switch[%d]\n", __func__, value);
		tcpm_typec_change_role(pinfo->tcpc, value ? TYPEC_ROLE_DRP : TYPEC_ROLE_SNK);
	}
}
EXPORT_SYMBOL(oppo_set_otg_switch_status);
#endif /* VENDOR_EDIT */
//====================================================================//


//====================================================================//
#ifdef VENDOR_EDIT
/* Jianchao.Shi@PSW.BSP.CHG.Basic, 2019/07/05, sjc Add for mmi status */
int oppo_chg_get_mmi_status(void)
{
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		printk(KERN_ERR "[OPPO_CHG][%s]: oppo_chip not ready!\n", __func__);
		return 1;
	}
	if (chip->mmi_chg == 0)
		printk(KERN_ERR "[OPPO_CHG][%s]: mmi_chg[%d]\n", __func__, chip->mmi_chg);
	return chip->mmi_chg;
}
EXPORT_SYMBOL(oppo_chg_get_mmi_status);
#endif /* VENDOR_EDIT */
//====================================================================//


//====================================================================//
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2019/01/11, sjc Add for PD */
#define VBUS_9V	9000
#define VBUS_5V	5000
#define IBUS_2A	2000
#define IBUS_3A	3000
bool oppo_chg_get_pd_type(void)
{
	if (pinfo != NULL) {
		if (pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK ||
			pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK_PD30 ||
			pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK_APDO)
			return true;
		//return mtk_pdc_check_charger(pinfo);
	}
	return false;
}

int oppo_mt6360_pd_setup(void)
{
	int vbus_mv = VBUS_5V;
	int ibus_ma = IBUS_2A;
	int ret = -1;
	struct adapter_power_cap cap;
	struct oppo_chg_chip *chip = g_oppo_chip;
	int i;

	cap.nr = 0;
	cap.pdp = 0;
	for (i = 0; i < ADAPTER_CAP_MAX_NR; i++) {
		cap.max_mv[i] = 0;
		cap.min_mv[i] = 0;
		cap.ma[i] = 0;
		cap.type[i] = 0;
		cap.pwr_limit[i] = 0;
	}

	printk(KERN_ERR "pd_type: %d\n", pinfo->pd_type);

	if (!chip->calling_on && !chip->camera_on && chip->charger_volt < 6500 && chip->soc < 90
		&& chip->temperature <= 410 && chip->cool_down_force_5v == false) {
		if (pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK_APDO) {
			adapter_dev_get_cap(pinfo->pd_adapter, MTK_PD_APDO, &cap);
			for (i = 0; i < cap.nr; i++) {
				printk(KERN_ERR "PD APDO cap %d: mV:%d,%d mA:%d type:%d pwr_limit:%d pdp:%d\n", i,
					cap.max_mv[i], cap.min_mv[i], cap.ma[i],
					cap.type[i], cap.pwr_limit[i], cap.pdp);
			}

		for (i = 0; i < cap.nr; i++) {
			if (cap.min_mv[i] <= VBUS_9V && VBUS_9V <= cap.max_mv[i]) {
				vbus_mv = VBUS_9V;
				ibus_ma = cap.ma[i];
				if (ibus_ma > IBUS_2A)
					ibus_ma = IBUS_2A;
				break;
			}
		}
	} else if (pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK
		|| pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK_PD30) {
		adapter_dev_get_cap(pinfo->pd_adapter, MTK_PD, &cap);
		for (i = 0; i < cap.nr; i++) {
			printk(KERN_ERR "PD cap %d: mV:%d,%d mA:%d type:%d\n", i,
				cap.max_mv[i], cap.min_mv[i], cap.ma[i], cap.type[i]);
		}

		for (i = 0; i < cap.nr; i++) {
			if (VBUS_9V <= cap.max_mv[i]) {
				vbus_mv = cap.max_mv[i];
				ibus_ma = cap.ma[i];
				if (ibus_ma > IBUS_2A)
					ibus_ma = IBUS_2A;
				break;
			}
		}
	} else {
		vbus_mv = VBUS_5V;
		ibus_ma = IBUS_2A;
	}

		printk(KERN_ERR "PD request: %dmV, %dmA\n", vbus_mv, ibus_ma);
		ret = oppo_pdc_setup(&vbus_mv, &ibus_ma);
	} else {
		if (chip->charger_volt > 7500 &&
			(chip->calling_on || chip->camera_on || chip->soc >= 90 || chip->batt_volt >= 4450
			|| chip->temperature > 410 || chip->cool_down_force_5v == true)) {
			vbus_mv = VBUS_5V;
			ibus_ma = IBUS_3A;

			printk(KERN_ERR "PD request: %dmV, %dmA\n", vbus_mv, ibus_ma);
			ret = oppo_pdc_setup(&vbus_mv, &ibus_ma);
		}
	}

	return ret;
}


/*Lukaili@BSP.CHG.Basic, 2020/06/06, Add for compatible adapter pd && qc*/
int oppo_chg_enable_hvdcp_detect(void);
int oppo_chg_set_pd_config(void)
{
	return oppo_mt6360_pd_setup();
}

int oppo_chg_enable_qc_detect(void)
{
	return oppo_chg_enable_hvdcp_detect();
}
#endif /* VENDOR_EDIT */


#ifdef VENDOR_EDIT
/* LiYue@BSP.CHG.Basic, 2019/10/30, Add for fast_chg_type */
int oppo_chg_get_charger_subtype(void)
{
	if (!pinfo)
		return CHARGER_SUBTYPE_DEFAULT;

	if (pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK ||
		pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK_PD30 ||
		pinfo->pd_type == MTK_PD_CONNECT_PE_READY_SNK_APDO)
		return CHARGER_SUBTYPE_PD;
		
	if (mt6360_get_hvdcp_type() == POWER_SUPPLY_TYPE_USB_HVDCP)
		return CHARGER_SUBTYPE_QC;

	return CHARGER_SUBTYPE_DEFAULT;
}

int oppo_chg_set_qc_config(void)
{
	int ret = -1;
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		pr_err("oppo_chip is null\n");
		return -1;
	}

	if (!chip->calling_on && !chip->camera_on && chip->charger_volt < 6500 && chip->soc < 90
		&& chip->temperature <= 410 && chip->cool_down_force_5v == false) {
		printk(KERN_ERR "%s: set qc to 9V", __func__);
		mt6360_set_register(MT6360_PMU_DPDM_CTRL, 0x1F, 0x18);
		ret = 0;
	} else {
		if (chip->charger_volt > 7500 &&
			(chip->calling_on || chip->camera_on || chip->soc >= 90 || chip->batt_volt >= 4450
			|| chip->temperature > 410 || chip->cool_down_force_5v == true)) {
			printk(KERN_ERR "%s: set qc to 5V", __func__);
			mt6360_set_register(MT6360_PMU_DPDM_CTRL, 0x1F, 0x15);
			ret = 0;
		}
	}

	return ret;
}

int oppo_chg_enable_hvdcp_detect(void)
{
	mt6360_enable_hvdcp_detect();

	return 0;
}

static void mt6360_step_charging_work(struct work_struct *work)
{
	int tbat_normal_current = 0;
	int step_chg_current = 0;
	struct oppo_chg_chip *chip = g_oppo_chip;

	if (!chip) {
		pr_err("%s, oppo_chip null\n", __func__);
		return;
	}

	if (!pinfo) {
		pr_err("%s, pinfo null\n", __func__);
		return;
	}
	if (pinfo->data.dual_charger_support) {
		if (chip->tbatt_status == BATTERY_STATUS__NORMAL) {
			tbat_normal_current = oppo_chg_get_tbatt_normal_charging_current(chip);


				if (pinfo->step_status == STEP_CHG_STATUS_STEP1) {
					pinfo->step_cnt += 5;
					if (pinfo->step_cnt >= pinfo->data.step1_time) {
						pinfo->step_status = STEP_CHG_STATUS_STEP2;
						pinfo->step_cnt = 0;
					}
				} else if (pinfo->step_status == STEP_CHG_STATUS_STEP2) {
					pinfo->step_cnt += 5;
					if (pinfo->step_cnt >= pinfo->data.step2_time) {
						pinfo->step_status = STEP_CHG_STATUS_STEP3;
						pinfo->step_cnt = 0;
					}
				} else {
					 if (pinfo->step_status == STEP_CHG_STATUS_STEP3) {
						pinfo->step_cnt = 0;
					}
			}

			if (pinfo->step_status == STEP_CHG_STATUS_STEP1)
				step_chg_current = pinfo->data.step1_current_ma;
			else if (pinfo->step_status == STEP_CHG_STATUS_STEP2)
				step_chg_current = pinfo->data.step2_current_ma;
			else if (pinfo->step_status == STEP_CHG_STATUS_STEP3)
				step_chg_current = pinfo->data.step3_current_ma;
			else
				step_chg_current = 0;

			if (step_chg_current != 0) {
				if (tbat_normal_current >= step_chg_current) {
					pinfo->step_chg_current = step_chg_current;
				} else {
					pinfo->step_chg_current = tbat_normal_current;
				}
			} else {
				pinfo->step_chg_current = tbat_normal_current;
			}

			if (pinfo->step_status != pinfo->step_status_pre) {
				pr_err("%s, step status: %d, step charging current: %d\n", __func__, pinfo->step_status, pinfo->step_chg_current);
				oppo_mt6360_charging_current_write_fast(pinfo->step_chg_current);
				pinfo->step_status_pre = pinfo->step_status;
			}
		}

		schedule_delayed_work(&pinfo->step_charging_work, msecs_to_jiffies(5000));
	}

	return;
}

void oppo_chg_set_camera_on(bool val)
{
	if (!g_oppo_chip) {
		return;
	} else {
		g_oppo_chip->camera_on = val;
		if (g_oppo_chip->dual_charger_support) {
			if (g_oppo_chip->camera_on == 1 && g_oppo_chip->charger_exist) {
				if (g_oppo_chip->chg_ops->get_charger_subtype() == CHARGER_SUBTYPE_QC){
					oppo_chg_set_qc_config();
				} else if (g_oppo_chip->chg_ops->get_charger_subtype() == CHARGER_SUBTYPE_PD){
					oppo_mt6360_pd_setup();
				}
			}
		}

		if (mtkhv_flashled_pinctrl.hv_flashled_support) {
			chr_err("%s: bc1.2_done = %d camera_on %d \n", __func__, mtkhv_flashled_pinctrl.bc1_2_done, val);
			if (mtkhv_flashled_pinctrl.bc1_2_done) {
				if (val) {
					pinctrl_select_state(mtkhv_flashled_pinctrl.pinctrl, mtkhv_flashled_pinctrl.chgvin_disable);
				} else {
					pinctrl_select_state(mtkhv_flashled_pinctrl.pinctrl, mtkhv_flashled_pinctrl.chgvin_enable);
				}
			}
		}
	}
}
EXPORT_SYMBOL(oppo_chg_set_camera_on);
#endif /* VENDOR_EDIT */
//====================================================================//


//====================================================================//
struct oppo_chg_operations  mtk6360_chg_ops = {
	.dump_registers = oppo_mt6360_dump_registers,
	.kick_wdt = oppo_mt6360_kick_wdt,
	.hardware_init = oppo_mt6360_hardware_init,
	.charging_current_write_fast = oppo_mt6360_charging_current_write_fast,
	.set_aicl_point = oppo_mt6360_set_aicl_point,
	.input_current_write = oppo_mt6360_input_current_limit_write,
	.float_voltage_write = oppo_mt6360_float_voltage_write,
	.term_current_set = oppo_mt6360_set_termchg_current,
	.charging_enable = oppo_mt6360_enable_charging,
	.charging_disable = oppo_mt6360_disable_charging,
	.get_charging_enable = oppo_mt6360_check_charging_enable,
	.charger_suspend = oppo_mt6360_suspend_charger,
	.charger_unsuspend = oppo_mt6360_unsuspend_charger,
	.set_rechg_vol = oppo_mt6360_set_rechg_voltage,
	.reset_charger = oppo_mt6360_reset_charger,
	.read_full = oppo_mt6360_registers_read_full,
	.otg_enable = oppo_mt6360_otg_enable,
	.otg_disable = oppo_mt6360_otg_disable,
	.set_charging_term_disable = oppo_mt6360_set_chging_term_disable,
	.check_charger_resume = oppo_mt6360_check_charger_resume,
	.get_chg_current_step = oppo_mt6360_get_chg_current_step,
#ifdef CONFIG_OPPO_CHARGER_MTK
	.get_charger_type = mt_power_supply_type_check,
	.get_charger_volt = battery_meter_get_charger_voltage,
	.check_chrdet_status = oppo_mt_get_vbus_status,
	.get_instant_vbatt = oppo_battery_meter_get_battery_voltage,
	.get_boot_mode = (int (*)(void))get_boot_mode,
	.get_boot_reason = (int (*)(void))get_boot_reason,
	.get_chargerid_volt = mt_get_chargerid_volt,
	.set_chargerid_switch_val = mt_set_chargerid_switch_val ,
	.get_chargerid_switch_val  = mt_get_chargerid_switch_val,
	.get_rtc_soc = get_rtc_spare_oppo_fg_value,
	.set_rtc_soc = set_rtc_spare_oppo_fg_value,
	.set_power_off = oppo_mt_power_off,
	.get_charger_subtype = oppo_chg_get_charger_subtype,
	.usb_connect = mt_usb_connect,
	.usb_disconnect = mt_usb_disconnect,
#else /* CONFIG_OPPO_CHARGER_MTK */
	.get_charger_type = qpnp_charger_type_get,
	.get_charger_volt = qpnp_get_prop_charger_voltage_now,
	.check_chrdet_status = qpnp_lbc_is_usb_chg_plugged_in,
	.get_instant_vbatt = qpnp_get_prop_battery_voltage_now,
	.get_boot_mode = get_boot_mode,
	.get_rtc_soc = qpnp_get_pmic_soc_memory,
	.set_rtc_soc = qpnp_set_pmic_soc_memory,
#endif /* CONFIG_OPPO_CHARGER_MTK */

#ifdef CONFIG_OPPO_SHORT_C_BATT_CHECK
	.get_dyna_aicl_result = oppo_mt6360_chg_get_dyna_aicl_result,
#endif
	.get_shortc_hw_gpio_status = oppo_chg_get_shortc_hw_gpio_status,
#ifdef CONFIG_OPPO_RTC_DET_SUPPORT
	.check_rtc_reset = rtc_reset_check,
#endif
	.oppo_chg_get_pd_type = oppo_chg_get_pd_type,
	.oppo_chg_pd_setup = oppo_mt6360_pd_setup,
	.set_qc_config = oppo_chg_set_qc_config,
	.enable_qc_detect = oppo_chg_enable_hvdcp_detect,	//for qc 9v2a
};
//====================================================================//

/* Lukaili@BSP.CHG.Basic, 2020/06/06, Add for OTG */
bool oppo_otgctl_by_buckboost(void)
{
	if (!g_oppo_chip)
		return false;

	return g_oppo_chip->vbatt_num == 2;
}

void oppo_otg_enable_by_buckboost(void)
{
	if (!g_oppo_chip || !(g_oppo_chip->chg_ops->charging_disable) || !(g_oppo_chip->chg_ops->charging_disable))
		return;

	g_oppo_chip->chg_ops->charging_disable();
	g_oppo_chip->chg_ops->otg_enable();
}

void oppo_otg_disable_by_buckboost(void)
{
	if (!g_oppo_chip || !(g_oppo_chip->chg_ops->otg_disable))
		return;

	g_oppo_chip->chg_ops->otg_disable();
}


void oppo_gauge_set_event(int event)
{
	if (NULL != pinfo) {
		charger_manager_notifier(pinfo, event);
		chr_err("[%s] notify mtkfuelgauge event = %d\n", __func__, event);
	}
}
#endif /* VENDOR_EDIT */

static int mtk_charger_probe(struct platform_device *pdev)
{
	struct charger_manager *info = NULL;
	struct list_head *pos;
	struct list_head *phead = &consumer_head;
	struct charger_consumer *ptr;
	int ret;
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for charging*/
	struct oppo_chg_chip *oppo_chip;
#endif

	chr_err("%s: starts support \n", __func__);
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for charging*/
	oppo_chip = devm_kzalloc(&pdev->dev, sizeof(*oppo_chip), GFP_KERNEL);
	if (!oppo_chip)
		return -ENOMEM;

	oppo_chip->dev = &pdev->dev;
	oppo_chg_parse_svooc_dt(oppo_chip);
	if (oppo_chip->vbatt_num == 1) {
		if (oppo_gauge_check_chip_is_null()) {
			chg_err("[oppo_chg_init] gauge null, will do after bettery init.\n");
			return -EPROBE_DEFER;
		}
		oppo_chip->chg_ops = &mtk6360_chg_ops;
	} else {
		if (oppo_gauge_check_chip_is_null() || oppo_vooc_check_chip_is_null()
				|| oppo_adapter_check_chip_is_null()) {
			chg_err("[oppo_chg_init] gauge || vooc || adapter null, will do after bettery init.\n");
			return -EPROBE_DEFER;
		}
		oppo_chip->chg_ops = (oppo_get_chg_ops());		//lkl modify for boost ic
		is_vooc_cfg = true;
		chg_err("%s is_vooc_cfg = %d\n", __func__, is_vooc_cfg);
	}
#endif /* VENDOR_EDIT */

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	pinfo = info;

	platform_set_drvdata(pdev, info);
	info->pdev = pdev;
	mtk_charger_parse_dt(info, &pdev->dev);

	oppo_step_charging_parse_dt(info, &pdev->dev);

#ifdef VENDOR_EDIT							//lkl need to make sure
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for charging*/
	oppo_chip->chgic_mtk.oppo_info = info;

	info->chg1_dev = get_charger_by_name("primary_chg");
	if (info->chg1_dev) {
		chg_err("found primary charger [%s]\n",
			info->chg1_dev->props.alias_name);
	} else {
		chg_err("can't find primary charger!\n");
	}
#endif
	mutex_init(&info->charger_lock);
	mutex_init(&info->charger_pd_lock);

#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for charging*/
	g_oppo_chip = oppo_chip;
	oppo_power_supply_init(oppo_chip);
	oppo_chg_parse_custom_dt(oppo_chip);
	oppo_chg_parse_charger_dt(oppo_chip);
	oppo_chip->chg_ops->hardware_init();
	oppo_chip->authenticate = oppo_gauge_get_batt_authenticate();
	oppo_chg_init(oppo_chip);
	oppo_chg_wake_update_work();
	if (get_boot_mode() != KERNEL_POWER_OFF_CHARGING_BOOT) {
		oppo_tbatt_power_off_task_init(oppo_chip);
	}
#endif

#ifdef VENDOR_EDIT
/*Baoquan.Lai@BSP.CHG.Basic, 2020/05/19, modefy for chargeric temp check*/
	pinfo->chargeric_temp_chan = iio_channel_get(oppo_chip->dev, "auxadc3-chargeric_temp");
        if (IS_ERR(pinfo->chargeric_temp_chan)) {
                chg_err("Couldn't get chargeric_temp_chan...\n");
                pinfo->chargeric_temp_chan = NULL;
        }
#endif
#ifdef VENDOR_EDIT
/*LiYue@BSP.CHG.Basic, 2019/07/04, modefy for usb connector temp check*/
	pinfo->charger_id_chan = iio_channel_get(oppo_chip->dev, "auxadc3-charger_id");
        if (IS_ERR(pinfo->charger_id_chan)) {
                chg_err("Couldn't get charger_id_chan...\n");
                pinfo->charger_id_chan = NULL;
        }

	pinfo->usb_temp_v_l_chan = iio_channel_get(oppo_chip->dev, "auxadc4-usb_temp_v_l");
	if (IS_ERR(pinfo->usb_temp_v_l_chan)) {
		chg_err("Couldn't get usb_temp_v_l_chan...\n");
		pinfo->usb_temp_v_l_chan = NULL;
	}

	pinfo->usb_temp_v_r_chan = iio_channel_get(oppo_chip->dev, "auxadc5-usb_temp_v_r");
	if (IS_ERR(pinfo->usb_temp_v_r_chan)) {
		chg_err("Couldn't get usb_temp_v_r_chan...\n");
		pinfo->usb_temp_v_r_chan = NULL;
	}
#endif

#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2019/06/18, sjc Add for usbtemp */
	if (oppo_usbtemp_check_is_support() == true)
		oppo_usbtemp_thread_init();
#endif
#ifdef VENDOR_EDIT
	/* LiYue@BSP.CHG.Basic, 2020/04/20, suspend mt6360 for svooc */
	if (is_vooc_project() == true)			//lkl add for vooc
		oppo_mt6360_suspend_charger();
#endif
	srcu_init_notifier_head(&info->evt_nh);
	ret = mtk_charger_setup_files(pdev);
	if (ret)
		chr_err("Error creating sysfs interface\n");

	info->pd_adapter = get_adapter_by_name("pd_adapter");
	if (info->pd_adapter)
		chr_err("Found PD adapter [%s]\n",
			info->pd_adapter->props.alias_name);
	else
		chr_err("*** Error : can't find PD adapter ***\n");
#ifdef VENDOR_EDIT
/*LuKaili@BSP.CHG.Basic, 2020/05/09, Add for detecting otg plug in/out*/
	pinfo->tcpc = tcpc_dev_get_by_name("type_c_port0");
	if (!pinfo->tcpc) {
		chr_err("%s get tcpc device type_c_port0 fail\n", __func__);
	}
#endif

	mtk_pdc_init(info);

	mutex_lock(&consumer_mutex);
	list_for_each(pos, phead) {
		ptr = container_of(pos, struct charger_consumer, list);
		ptr->cm = info;
		if (ptr->pnb != NULL) {
			srcu_notifier_chain_register(&info->evt_nh, ptr->pnb);
			ptr->pnb = NULL;
		}
	}
	mutex_unlock(&consumer_mutex);
#ifdef VENDOR_EDIT
/* LiYue@BSP.CHG.Basic, 2020/03/16, Add for step charging */
	INIT_DELAYED_WORK(&pinfo->step_charging_work, mt6360_step_charging_work);
#endif
	return 0;
}

static int mtk_charger_remove(struct platform_device *dev)
{
	return 0;
}

static void mtk_charger_shutdown(struct platform_device *dev)
{
	struct charger_manager *info = platform_get_drvdata(dev);
	int ret;

	if (mtk_pe20_get_is_connect(info) || mtk_pe_get_is_connect(info)) {
		if (info->chg2_dev)
			charger_dev_enable(info->chg2_dev, false);
		ret = mtk_pe20_reset_ta_vchr(info);
		if (ret == -ENOTSUPP)
			mtk_pe_reset_ta_vchr(info);
		pr_debug("%s: reset TA before shutdown\n", __func__);
	}
#ifdef VENDOR_EDIT
/* Jianchao.Shi@BSP.CHG.Basic, 2018/11/09, sjc Add for shipmode stm6620 */
	if (g_oppo_chip) {
		enter_ship_mode_function(g_oppo_chip);
	}
#endif	
}

static const struct of_device_id mtk_charger_of_match[] = {
	{.compatible = "mediatek,charger",},
	{},
};

MODULE_DEVICE_TABLE(of, mtk_charger_of_match);

struct platform_device charger_device = {
	.name = "charger",
	.id = -1,
};

static struct platform_driver charger_driver = {
	.probe = mtk_charger_probe,
	.remove = mtk_charger_remove,
	.shutdown = mtk_charger_shutdown,
	.driver = {
		   .name = "charger",
		   .of_match_table = mtk_charger_of_match,
	},
};

static int __init mtk_charger_init(void)
{
	return platform_driver_register(&charger_driver);
}
late_initcall(mtk_charger_init);

static void __exit mtk_charger_exit(void)
{
	platform_driver_unregister(&charger_driver);
}
module_exit(mtk_charger_exit);


MODULE_AUTHOR("LiYue<liyue1@oppo.com>");
MODULE_DESCRIPTION("OPPO Charger Driver");
MODULE_LICENSE("GPL");
