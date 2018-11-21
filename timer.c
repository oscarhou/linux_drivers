#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/init.h>

//MODULE_LICENSE("GPL");
static struct hrtimer hr_timer;
static unsigned long const timer_interval_ns = 1e9;

enum hrtimer_restart hrtimer_callback(struct hrtimer* test)
{
	ktime_t currtime, interval;
	currtime = ktime_get();
	interval = ktime_set(0, timer_interval_ns);
	hrtimer_forward(test, currtime, interval);
	printk("yO YOU CALLING ME!?\n");
	return HRTIMER_RESTART;
}
static int __init init_module_test(void)
{
	ktime_t ktime;
	printk("initializing module\n");
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &hrtimer_callback;

	ktime = ktime_set(0, timer_interval_ns);
	hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);
	return 0;
}


static void  __exit exit_module_test(void)
{
	hrtimer_cancel(&hr_timer);
	printk("exiting module\n");
}

module_init(init_module_test);
module_exit(exit_module_test);
MODULE_LICENSE("GPL");

