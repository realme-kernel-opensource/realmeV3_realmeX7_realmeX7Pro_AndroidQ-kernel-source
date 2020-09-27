#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/sched/clock.h>
#include <linux/oom.h>
#include <linux/uaccess.h>
#include <linux/mm.h>

static int show_uid_limit = 0;
static LIST_HEAD(user_task_head);

struct process_mem {
	char comm[TASK_COMM_LEN];
	int pid;
	int oom_score_adj;
	unsigned long rss;
	unsigned long swapents_ori;
	unsigned int uid;
	unsigned long ions;
};
static struct process_mem pmem[1024];

void update_user_tasklist(struct task_struct *tsk)
{
	if (!tsk)
		return;

	if (tsk->flags & PF_KTHREAD) {
		return;
	}
	list_add_tail_rcu(&tsk->user_tasks, &user_task_head);
}
EXPORT_SYMBOL_GPL(update_user_tasklist);

static int memory_monitor_show(struct seq_file *m, void *p)
{
	struct task_struct *ptsk, *tsk;
	struct list_head *list = NULL;
	unsigned long start = 0, rcu_lock_end = 0, end = 0, task_lock_start = 0, task_lock_end = 0, task_lock_time = 0;
	int record_tasks = 0, i = 0;
	unsigned long ptes, pmds;

	seq_printf(m, "-------------------------------------------------------\n");
	seq_printf(m, "memory monitor for ns\n");
	seq_printf(m, "-------------------------------------------------------\n");

	start = sched_clock();
	rcu_read_lock();

	seq_printf(m, "%-24s\t%-24s\t%-24s\t%-24s\t%-24s\t%-24s\t%-24s\n",
			"task_name", "pid", "uid", "oom_score_adj", "rss", "swapents_ori", "ion");
	list_for_each(list, &user_task_head) {
		tsk = container_of(list, struct task_struct, user_tasks);

		ptsk = find_lock_task_mm(tsk);
		if (!ptsk)
			continue;
		if (show_uid_limit > task_uid(ptsk).val) {
			task_unlock(ptsk);
			continue;
		}

		task_lock_start = sched_clock();
		pmem[record_tasks].pid = ptsk->pid;
		pmem[record_tasks].uid = task_uid(ptsk).val;
		memcpy(pmem[record_tasks].comm, ptsk->comm, TASK_COMM_LEN);
		pmem[record_tasks].oom_score_adj = ptsk->signal->oom_score_adj;
		pmem[record_tasks].rss = get_mm_rss(ptsk->mm) << 2;
		ptes = PTRS_PER_PTE * sizeof(pte_t) * atomic_long_read(&ptsk->mm->nr_ptes);
		pmds = PTRS_PER_PMD * sizeof(pmd_t) * mm_nr_pmds(ptsk->mm);
		pmem[record_tasks].rss += (ptes + pmds) >> 10;
		pmem[record_tasks].swapents_ori = get_mm_counter(ptsk->mm, MM_SWAPENTS) << 2;
		task_unlock(ptsk);
		pmem[record_tasks].ions = ((unsigned long)atomic64_read(&tsk->ions)) >> 10;

		record_tasks++;
		if (record_tasks >= 1024)
			break;
		task_lock_end = sched_clock();
		if (task_lock_end - task_lock_start > task_lock_time)
			task_lock_time = task_lock_end - task_lock_start;

	}

	rcu_read_unlock();
	rcu_lock_end = sched_clock();
	for (i = 0; i < record_tasks; i++) {
		seq_printf(m, "%-24s\t%-24d\t%-24u\t%-24hd\t%-24lu\t%-24lu\t%-24lu\n",
				(char *)(pmem[i].comm), pmem[i].pid, pmem[i].uid,
				pmem[i].oom_score_adj, pmem[i].rss,
				pmem[i].swapents_ori, pmem[i].ions);
	}
	end = sched_clock();
	seq_printf(m, "read time: %lu ns, rcu_lock: %lu, task_lock_max : %lu, record_tasks = %d\n",
			end - start, rcu_lock_end - start, task_lock_time, record_tasks);

	seq_printf(m, "-------------------------------------------------------\n");

	return 0;
}

static int memory_monitor_open(struct inode *inode, struct file *file)
{
	return single_open(file, memory_monitor_show, NULL);
}

static ssize_t memory_monitor_write(struct file *file, const char __user *buff, size_t len, loff_t *ppos)
{
	char write_data[16] = {0};

	if (copy_from_user(&write_data, buff, len)) {
		pr_err("memory_monitor_fops write error.\n");
		return -EFAULT;
	}

	if (kstrtoint(write_data, 10, &show_uid_limit)) {
		pr_err("memory_monitor_fops kstrtoul error.\n");
		return -EFAULT;
	}

	return len;
}

static const struct file_operations memory_monitor_fops = {
	.open       = memory_monitor_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release    = single_release,
	.write      = memory_monitor_write,
};

static int __init memory_monitor_init(void)
{
	struct proc_dir_entry *pentry;

	pentry = proc_create("memory_monitor", S_IRWXUGO, NULL, &memory_monitor_fops);
	if(!pentry) {
		pr_err("create  proc memory_monitor failed.\n");
		return -1;
	}
	return 0;
}
device_initcall(memory_monitor_init);
