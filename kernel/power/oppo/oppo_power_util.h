/* SPDX-License-Identifier: GPL-2.0 */
#ifndef OPPO_POWER_UTIL_H
#define OPPO_POWER_UTIL_H

#define WAKELOCK_BUFFER_SIZE 4096
struct oppo_util {
	struct delayed_work oppo_work;
	int polling_time;
	char *wakelock_buf;
};

extern void oppo_pm_get_active_wakeup_sources(char *pending_wakeup_source, size_t max);
#endif //end of OPPO_POWER_UTIL_H
