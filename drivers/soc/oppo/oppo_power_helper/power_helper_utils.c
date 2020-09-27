/**********************************************************************************
* Description: add interface to save power log that in kernel to the file
* ------------------------------ Revision History: --------------------------------
* <version>           <date>                <author>                            <desc>
* Revision 1.0        2020-05-05        yangmingjin@realme.com                  Created
***********************************************************************************/
#include <linux/ktime.h>
#include <linux/kernel.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>

#include <asm/uaccess.h>
#include <linux/stat.h>
#include <asm/unistd.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <stdarg.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/power/power_helper.h>


#ifdef CONFIG_POWER_HELPER_SUPPORT

#define FIFO_SIZE    512*1024
#define LOG_BUF_CACHE_SIZE   50
#define LOG_BUF_MAX_SIZE     1024
#define LOG_BUF_SIZE     (LOG_BUF_MAX_SIZE-4)
#define PROC_FIFO    "power_wakeup_info"

enum buf_flag {
	FLAG_UNUSED,
	FLAG_START,
	FLAG_END,
};

struct log_helper {
	struct task_struct *log_show_thread;
	struct kfifo fifo;
	struct mutex read_lock;
	struct mutex write_lock;
	wait_queue_head_t read_queue;
	bool is_suspend;
	char buf[LOG_BUF_CACHE_SIZE][LOG_BUF_MAX_SIZE];
};

static struct log_helper *get_log_helper_instance(void)
{
	static struct log_helper *g_log_helper = NULL;
	int i = 0;

	if(g_log_helper != NULL)
		return g_log_helper;

	g_log_helper = (struct log_helper *)kmalloc(sizeof(struct log_helper), GFP_KERNEL);
	if(!g_log_helper)
	{
		pr_err("g_log_helper kmalloc failed\n");
	}

	for(i = 0; i < LOG_BUF_CACHE_SIZE; i++){
		g_log_helper->buf[i][LOG_BUF_MAX_SIZE - 1] = FLAG_UNUSED;//means not be used
		g_log_helper->buf[i][LOG_BUF_MAX_SIZE - 2] = '\n';
		g_log_helper->buf[i][LOG_BUF_MAX_SIZE - 3] = '\0';
	}

	g_log_helper->is_suspend = false;

	return g_log_helper;
}

static char* get_cache_buf(void)
{
	int i = 0;
	struct log_helper * log_helper_pri = get_log_helper_instance();

	for(i = 0; i < LOG_BUF_CACHE_SIZE; i++){
		if(log_helper_pri->buf[i][LOG_BUF_MAX_SIZE - 1] != FLAG_UNUSED)
			continue;
		else
			break;
	}

	if(i >= LOG_BUF_CACHE_SIZE)
		return NULL;

	log_helper_pri->buf[i][LOG_BUF_MAX_SIZE - 1] = FLAG_START;
	return log_helper_pri->buf[i];
}

extern int timekeeping_suspended;
int power_helper_get_utc_time(char *buf, int max_size)
{
	struct timespec ts;
	struct rtc_time tm;

	memset(&ts, 0, sizeof(struct timespec));
	memset(&tm, 0, sizeof(struct rtc_time));

	if(!timekeeping_suspended){
		getnstimeofday(&ts);
		rtc_time_to_tm(ts.tv_sec, &tm);
	}

	return snprintf(buf, max_size, "[%d-%02d-%02d %02d:%02d:%02d.%09lu UTC] ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
}

static void notify_to_user(void)
{
	struct log_helper * log_helper_pri = get_log_helper_instance();
	wake_up_interruptible(&log_helper_pri->read_queue);
}

void power_helper_save_to_buf(char *fmt, ...)
{
	int count_time = 0, count_str = 0;
	struct log_helper * log_helper_pri = NULL;
	char *log_buf = NULL;
	va_list argp;

	if(!check_power_helper_support())
		return;

	log_helper_pri = get_log_helper_instance();
	log_buf = get_cache_buf();

	if(NULL == log_buf)
		return;

	va_start(argp, fmt);
	count_time = power_helper_get_utc_time(log_buf, LOG_BUF_SIZE);
	if(count_time < 0){
		log_buf[LOG_BUF_MAX_SIZE - 1] = FLAG_UNUSED;
		va_end(argp);
		return;
	}
	count_str = vsnprintf(log_buf + count_time, LOG_BUF_SIZE - count_time, fmt, argp);
	if(count_str < 0){
		log_buf[LOG_BUF_MAX_SIZE - 1] = FLAG_UNUSED;
		va_end(argp);
		return;
	}
	va_end(argp);

	count_str = ((count_str > (LOG_BUF_SIZE - count_time - 1)) ? (LOG_BUF_SIZE - count_time - 1) : count_str);
	log_buf[count_time + count_str] = '\0';//force add str terminal
	log_buf[count_time + count_str + 1] = '\n';//end with \n,java reanLine needed
	log_buf[LOG_BUF_MAX_SIZE - 1] = FLAG_END;

	if(!timekeeping_suspended)
		wake_up_process(log_helper_pri->log_show_thread);
}
EXPORT_SYMBOL(power_helper_save_to_buf);

void power_helper_set_ap_suspend_state(bool suspend)
{
	struct log_helper * log_helper_pri = NULL;

	if(!check_power_helper_support())
		return;

	log_helper_pri = get_log_helper_instance();

	if(log_helper_pri->is_suspend != suspend)
	{
		log_helper_pri->is_suspend = suspend;
		power_helper_save_to_buf("%s", log_helper_pri->is_suspend ? "suspend entry" : "suspend exit");
	}
}

static int __ref log_show_thread_main(void *data)
{
	int rc = 0, i = 0;
	struct log_helper * log_helper_pri = get_log_helper_instance();
	char *buf_cache = NULL;

	while (!kthread_should_stop()) {
		set_current_state(TASK_RUNNING);

		for(i = 0; i < LOG_BUF_CACHE_SIZE; i++)
		{
			buf_cache = log_helper_pri->buf[i];
			if(log_helper_pri->buf[i][LOG_BUF_MAX_SIZE - 1] == FLAG_END){
				kfifo_in(&log_helper_pri->fifo, buf_cache, strlen(buf_cache) + 2);
				log_helper_pri->buf[i][LOG_BUF_MAX_SIZE - 1] = FLAG_UNUSED;
			}
		}

		if(!log_helper_pri->is_suspend)
			notify_to_user();

		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}
	return rc;
}

static ssize_t fifo_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	unsigned int copied;
	struct log_helper * log_helper_pri = get_log_helper_instance();

	if (mutex_lock_interruptible(&log_helper_pri->write_lock))
		return -ERESTARTSYS;

	ret = kfifo_from_user(&log_helper_pri->fifo, buf, count, &copied);

	mutex_unlock(&log_helper_pri->write_lock);

	return ret ? ret : copied;

}

static ssize_t fifo_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	unsigned int copied;
	struct log_helper * log_helper_pri = get_log_helper_instance();

	do {
		if (kfifo_is_empty(&log_helper_pri->fifo)) {
			if (file->f_flags & O_NONBLOCK)
				return -EAGAIN;

			ret = wait_event_interruptible(log_helper_pri->read_queue,
					!kfifo_is_empty(&log_helper_pri->fifo));
			if (ret)
				return ret;
		}

		if (mutex_lock_interruptible(&log_helper_pri->read_lock))
			return -ERESTARTSYS;

		ret = kfifo_to_user(&log_helper_pri->fifo, buf, count, &copied);
		mutex_unlock(&log_helper_pri->read_lock);

		if (ret)
			return ret;

		if (copied == 0 && (file->f_flags & O_NONBLOCK))
			return -EAGAIN;

	} while (copied == 0);

	return copied;
}

static const struct file_operations fifo_fops =
{
	.owner      = THIS_MODULE,
	.read       = fifo_read,
	.write      = fifo_write,
	.llseek     = noop_llseek,
};

int power_ultil_init(void)
{
	int rc = 0, ret;
	struct log_helper * log_helper_pri = get_log_helper_instance();

	log_helper_pri->log_show_thread = kthread_create(log_show_thread_main, (void *)log_helper_pri, "rm_power_log");
	if (!log_helper_pri->log_show_thread) {
		pr_err("Can't create log_show_thread\n");
		rc = -EPROBE_DEFER;
	}

	ret = kfifo_alloc(&log_helper_pri->fifo, FIFO_SIZE, GFP_KERNEL);
	if (ret) {
		pr_err("power_ultil_init kfifo_alloc error\n");
		return ret;
	}
	mutex_init(&log_helper_pri->read_lock);
	mutex_init(&log_helper_pri->write_lock);
	init_waitqueue_head(&log_helper_pri->read_queue);

	if (proc_create(PROC_FIFO, 0664, power_helper_get_proc_dir(), &fifo_fops) == NULL)
	{
		kfifo_free(&log_helper_pri->fifo);
		return -ENOMEM;
	}

	return rc;
}

void power_ultil_exit(void)
{
	kfree(get_log_helper_instance());
}
#else
int  power_helper_get_utc_time(char *buf, int max_size){return 0;}
void power_helper_save_to_buf(char *fmt, ...){}
void power_helper_set_ap_suspend_state(bool suspend){}
#endif
