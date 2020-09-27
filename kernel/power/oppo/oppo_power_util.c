// SPDX-License-Identifier: GPL-2.0
/*
 * kernel/power/oppo/oppo_util.c
 *
 * oppo power debug feature
 *
 * */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include "oppo_power_util.h"

struct oppo_util oppo_util;
static int polling_time = 60000;
module_param_named(polling_time, polling_time, int, S_IRUGO|S_IWUSR);

ssize_t wakelock_printk(char *buf)
{
	oppo_pm_get_active_wakeup_sources(buf, WAKELOCK_BUFFER_SIZE);
	return 0;
}

void oppo_power_polling_fn(struct work_struct *work)
{
	ssize_t wlock_size = 0;
	oppo_util.polling_time = polling_time;
	wlock_size = wakelock_printk(oppo_util.wakelock_buf);
	pr_info("[OPPO_POWER]: %s", oppo_util.wakelock_buf);
	schedule_delayed_work(&(oppo_util.oppo_work),
							msecs_to_jiffies(oppo_util.polling_time));
}

static int __init oppo_util_init(void)
{
	oppo_util.polling_time = polling_time;
	oppo_util.wakelock_buf = (char *)kzalloc(WAKELOCK_BUFFER_SIZE, GFP_KERNEL);
	INIT_DELAYED_WORK(&(oppo_util.oppo_work),
				oppo_power_polling_fn);
	schedule_delayed_work(&(oppo_util.oppo_work),
							msecs_to_jiffies(oppo_util.polling_time));
	return 0;
}

static void __exit oppo_util_exit(void)
{
	kfree(oppo_util.wakelock_buf);
}
module_init(oppo_util_init);
module_exit(oppo_util_exit);
