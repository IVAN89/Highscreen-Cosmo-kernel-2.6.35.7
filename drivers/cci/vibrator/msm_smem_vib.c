/*
 * Copyright (C) 2010 Compalcomm Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <../../../drivers/staging/android/timed_output.h>
#include <linux/module.h>
#include <linux/earlysuspend.h>
#include "../../../arch/arm/mach-msm/proc_comm.h"

/*
 * Use Compalcomm process command
 */
int call_vibrator_smem(int timeout)
{
	u32 duration = timeout;

	return msm_proc_comm (PCOM_CCI_VIBRATE, 0, &duration);
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	int rc;
	if (value >= 0) {
		rc = call_vibrator_smem(value);
		if (rc < 0) {
			printk(KERN_ERR "call_vibrator_smem failed rc=%d\n",rc);
		}
	}
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
void vibrator_early_suspend(struct early_suspend *h)
{
	call_vibrator_smem(0);
}

static struct early_suspend early_suspend = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = vibrator_early_suspend,
};
#endif

static struct timed_output_dev vibrator_dev = {
	.name = "vibrator",
	.get_time = vibrator_get_time,
	.enable = vibrator_enable,
};

static int __init msm_init_vibrator(void)
{
	int rc = 0;

	printk("vibrator init\n");
#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&early_suspend);
#endif
	rc = timed_output_dev_register(&vibrator_dev);
	if (rc) {
		printk(KERN_ERR "timed_output_dev register failed!\n");
	}
	return 0;
}

module_init(msm_init_vibrator);

MODULE_DESCRIPTION("timed output vibrator device");
MODULE_LICENSE("GPL");
