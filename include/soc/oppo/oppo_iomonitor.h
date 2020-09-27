#ifndef _OPPO_IOMONITOR_H_
#define _OPPO_IOMONITOR_H_

/* Add for calculate free memory. */
#include <linux/vmstat.h>
#include <asm/page.h>

/* Add for calculate free disk with f2fs. */
#include <linux/f2fs_fs.h>
#include <../../../fs/f2fs/f2fs.h>
#include <linux/types.h>
extern struct super_block *data_super_block_pointer;

/* Add for statistical IO time-consuming distribution. */
#include <linux/blkdev.h>
#include <linux/blk_types.h>
#include <linux/ktime.h>

/*
* io monitor entry type
* NUM_ENTRY_TYPE: int value
* STR_ENTRY_TYPE: str value
* STR_FREE_ENTRY_TYPE: str value, need free string mem
*/
#define NUM_ENTRY_TYPE 0
#define STR_ENTRY_TYPE 1
#define STR_FREE_ENTRY_TYPE 2
#define PRIVATE_ENTRY_TYPE 3
#define IO_WAIT_TIMEOUT 500
#define UX_IO_WAIT_TIMEOUT 300
#define IO_WAIT_LOW 200
#define IO_WAIT_HIGH 500
enum {
	IO_MONITOR_DELAY_SYNC = 0,
	IO_MONITOR_DELAY_NSYNC,
	IO_MONITOR_DELAY_BLOCK_NUM,
};

struct block_delay_data {
	/* sync = 0, nsync = 1 */
	u64 stage_one[2];
	u64 stage_two[2];
	u64 stage_thr[2];
	u64 stage_fou[2];
	u64 stage_fiv[2];
	u64 stage_six[2];
	u64 stage_sev[2];
	u64 stage_eig[2];
	u64 stage_nin[2];
	u64 stage_ten[2];
	u64 max_delay[2];
	u64 cnt[2];
	u64 total_delay[2];
};

enum {
	IO_MONITOR_DELAY_READ = 0,
	IO_MONITOR_DELAY_WRITE,
	IO_MONITOR_DELAY_OPTION_NUM,
};

enum {
	IO_MONITOR_DELAY_4K = 0,
	IO_MONITOR_DELAY_512K,
	IO_MONITOR_DELAY_DEVICE_NUM,
};

struct device_delay_data {
	/* read = 0, write = 1 */
	u64 stage_one[2];
	u64 stage_two[2];
	u64 stage_thr[2];
	u64 stage_fou[2];
	u64 stage_fiv[2];
	u64 max_delay[2];
	u64 cnt[2];
	u64 total_delay[2];
};

struct block_delay_data_ux {
	u64 stage_one;
	u64 stage_two;
	u64 stage_thr;
	u64 stage_fou;
	u64 stage_fiv;
	u64 stage_six;
	u64 stage_sev;
	u64 stage_eig;
	u64 stage_nin;
	u64 stage_ten;
	u64 max_delay;
	u64 cnt;
	u64 total_delay;
};

struct req_delay_data {
	struct block_delay_data_ux uxreq_block_para;
	struct block_delay_data req_block_para;
	/* 4K = 0, 512K = 1 */
	struct device_delay_data req_device_para[2];
};

struct cal_data_info{
	u64 user_read_data;
	u64 user_write_data;
	u64 kernel_read_data;
	u64 kernel_write_data;
	u64 dio_write_data;
};

struct cal_page_info{
	u64 page_in;
	u64 page_out;
};

struct fs_status_info {
	atomic64_t major_fpage;
	atomic64_t nfsync;
	u64 discard;
	u64 gc;
	u64 task_rw_data;
	u64 low_iowait;
	u64 high_iowait;
	atomic64_t dirty_page;
};

struct abnormal_info{
	int reason;
	pid_t pid;
};

enum iomointor_io_type{
	IO_WAIT,
	UX_IO_WAIT,
	FRAME_DROP,
	USER_TRIGGER
};

enum daily_data_type {
	USER_READ,
	USER_WRITE,
	KERNEL_READ,
	KERNEL_WRITE,
	DIO_WRITE
};
extern struct fs_status_info fs_status;
extern void put_rw_bytes(enum daily_data_type type, struct file *file, ssize_t ret);
extern void pgpg_put_value(enum vm_event_item type, u64 delta);

#define PID_LENGTH 32
#define TASK_STATUS_LENGTH 4096
#define UID_SHOW_DAILY_LIMIT  300*1024*1024   //300M
#define TASK_SHOW_DAILY_LIMIT 10*1024*1024       //10M
struct task_io_info  {
	pid_t pid;
	pid_t tgid;
	uid_t uid;
	int state;
	u64 read_bytes;
	u64 write_bytes;
	u64 all_read_bytes;
	u64 all_write_bytes;
	u64 fsync;
	char comm[TASK_COMM_LEN];
	u64 time;
};
struct disk_info  {
	unsigned int score;
	unsigned int free; /* blocks */
	block_t blocks[9]; /* 4..8K |8..16K | 16..32K |32..64K | 64..128K |128..256K | 256..512K | 512K..1M | 1M+ */
};
struct inter_disk_data {
	unsigned int len;
	char *buf;
};

extern void add_pid_to_list(struct task_struct *task, size_t bytes, bool opt);
extern int exit_uid_find_or_register(struct task_struct *task);
extern void free_all_uid_entry(void);
extern block_t of2fs_seg_freefrag(struct f2fs_sb_info *sbi,
				unsigned int segno, block_t* blocks, unsigned int n);

int abnormal_handle(enum iomointor_io_type reason, pid_t pid);

struct proc_dir_entry *create_uid_proc(struct proc_dir_entry *parent);


//extern void iomonitor_get_disk_info(struct super_block *sb, void *arg);

#endif /* _OPPO_IOMONITOR_H_*/
