#ifndef __REALME_POWER_HELPER_UTILS_H__
#define __REALME_POWER_HELPER_UTILS_H__
	int  power_helper_get_utc_time(char *buf, int max_size);
	void power_helper_save_to_buf(char *fmt, ...);
	void power_helper_set_ap_suspend_state(bool suspend);
#endif
