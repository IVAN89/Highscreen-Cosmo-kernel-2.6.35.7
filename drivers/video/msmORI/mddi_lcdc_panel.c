/*
#include "mach/../../cci_smem.h"
#include "mach/../../smd_private.h"

#ifdef CONFIG_FB_MSM_LCDC
// LCDC interface
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <mach/hardware.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/mach-types.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/pm_qos_params.h>
#include <mach/msm_reqs.h>

#include "msm_fb.h"

static int lcdc_probe(struct platform_device *pdev);
static int lcdc_remove(struct platform_device *pdev);

static int lcdc_off(struct platform_device *pdev);
static int lcdc_on(struct platform_device *pdev);

static struct platform_device *pdev_list[MSM_FB_MAX_DEV_LIST];
static int pdev_list_cnt;

static struct clk *pixel_mdp_clk; // drives the lcdc block in mdp
static struct clk *pixel_lcdc_clk; // drives the lcdc interface

static struct platform_driver lcdc_driver = {
	.probe = lcdc_probe,
	.remove = lcdc_remove,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		   .name = "lcdc",
		   },
};

static struct lcdc_platform_data *lcdc_pdata;

static int lcdc_off(struct platform_device *pdev)
{
	int ret = 0;

	ret = panel_next_off(pdev);

	clk_disable(pixel_mdp_clk);
	clk_disable(pixel_lcdc_clk);

	if (lcdc_pdata && lcdc_pdata->lcdc_power_save)
		lcdc_pdata->lcdc_power_save(0);

	if (lcdc_pdata && lcdc_pdata->lcdc_gpio_config)
		ret = lcdc_pdata->lcdc_gpio_config(0);

	pm_qos_update_requirement(PM_QOS_SYSTEM_BUS_FREQ , "lcdc",
				  PM_QOS_DEFAULT_VALUE);

	return ret;
}

static int lcdc_on(struct platform_device *pdev)
{
	int ret = 0;
	struct msm_fb_data_type *mfd;
	unsigned long panel_pixclock_freq, pm_qos_rate;

	mfd = platform_get_drvdata(pdev);
	panel_pixclock_freq = mfd->fbi->var.pixclock;

#ifdef CONFIG_MSM_NPA_SYSTEM_BUS
	pm_qos_rate = MSM_AXI_FLOW_MDP_LCDC_WVGA_2BPP;
#else
	if (panel_pixclock_freq > 65000000)
		// pm_qos_rate should be in Khz
		pm_qos_rate = panel_pixclock_freq / 1000 ;
	else
		pm_qos_rate = 65000;
#endif

	pm_qos_update_requirement(PM_QOS_SYSTEM_BUS_FREQ , "lcdc",
				  pm_qos_rate);
	mfd = platform_get_drvdata(pdev);

	ret = clk_set_rate(pixel_mdp_clk, mfd->fbi->var.pixclock);
	if (ret) {
		pr_err("%s: Can't set MDP LCDC pixel clock to rate %u\n",
			__func__, mfd->fbi->var.pixclock);
		goto out;
	}

	clk_enable(pixel_mdp_clk);
	clk_enable(pixel_lcdc_clk);

	if (lcdc_pdata && lcdc_pdata->lcdc_power_save)
		lcdc_pdata->lcdc_power_save(1);
	if (lcdc_pdata && lcdc_pdata->lcdc_gpio_config)
		ret = lcdc_pdata->lcdc_gpio_config(1);

	ret = panel_next_on(pdev);

out:
	return ret;
}

static int lcdc_probe(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct fb_info *fbi;
	struct platform_device *mdp_dev = NULL;
	struct msm_fb_panel_data *pdata = NULL;
	int rc;

	if (pdev->id == 0) {
		lcdc_pdata = pdev->dev.platform_data;
		return 0;
	}

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if (pdev_list_cnt >= MSM_FB_MAX_DEV_LIST)
		return -ENOMEM;

	mdp_dev = platform_device_alloc("mdp", pdev->id);
	if (!mdp_dev)
		return -ENOMEM;

	
	// link to the latest pdev
	
	mfd->pdev = mdp_dev;
	mfd->dest = DISPLAY_LCDC;

	
	// alloc panel device data
	
	if (platform_device_add_data
	    (mdp_dev, pdev->dev.platform_data,
	     sizeof(struct msm_fb_panel_data))) {
		printk(KERN_ERR "lcdc_probe: platform_device_add_data failed!\n");
		platform_device_put(mdp_dev);
		return -ENOMEM;
	}
	
	// data chain
	
	pdata = (struct msm_fb_panel_data *)mdp_dev->dev.platform_data;
	pdata->on = lcdc_on;
	pdata->off = lcdc_off;
	pdata->next = pdev;

	
	// get/set panel specific fb info
	
	mfd->panel_info = pdata->panel_info;

#ifdef MSMFB_FRAMEBUF_32
	if (mfd->index == 0)
		mfd->fb_imgType = MDP_RGBA_8888; // primary
	else
		mfd->fb_imgType = MDP_RGB_565;	// secondary
#else
	mfd->fb_imgType = MDP_RGB_565;
#endif

	fbi = mfd->fbi;
	fbi->var.pixclock = clk_round_rate(pixel_mdp_clk,
					mfd->panel_info.clk_rate);
	fbi->var.left_margin = mfd->panel_info.lcdc.h_back_porch;
	fbi->var.right_margin = mfd->panel_info.lcdc.h_front_porch;
	fbi->var.upper_margin = mfd->panel_info.lcdc.v_back_porch;
	fbi->var.lower_margin = mfd->panel_info.lcdc.v_front_porch;
	fbi->var.hsync_len = mfd->panel_info.lcdc.h_pulse_width;
	fbi->var.vsync_len = mfd->panel_info.lcdc.v_pulse_width;

	
	// set driver data
	
	platform_set_drvdata(mdp_dev, mfd);

	
	// register in mdp driver
	
	rc = platform_device_add(mdp_dev);
	if (rc)
		goto lcdc_probe_err;

	pdev_list[pdev_list_cnt++] = pdev;
		return 0;

lcdc_probe_err:
	platform_device_put(mdp_dev);
	return rc;
}

static int lcdc_remove(struct platform_device *pdev)
{
	pm_qos_remove_requirement(PM_QOS_SYSTEM_BUS_FREQ , "lcdc");
	return 0;
}

static int lcdc_register_driver(void)
{
	return platform_driver_register(&lcdc_driver);
}

static int __init lcdc_driver_init(void)
{

	pixel_mdp_clk = clk_get(NULL, "pixel_mdp_clk");
	if (IS_ERR(pixel_mdp_clk))
		pixel_mdp_clk = NULL;

	if (pixel_mdp_clk) {
		pixel_lcdc_clk = clk_get(NULL, "pixel_lcdc_clk");
		if (IS_ERR(pixel_lcdc_clk)) {
			printk(KERN_ERR "Couldnt find pixel_lcdc_clk\n");
			return -EINVAL;
		}
	} else {
		pixel_mdp_clk = clk_get(NULL, "mdp_lcdc_pclk_clk");
		if (IS_ERR(pixel_mdp_clk)) {
			printk(KERN_ERR "Couldnt find mdp_lcdc_pclk_clk\n");
			return -EINVAL;
		}

		pixel_lcdc_clk = clk_get(NULL, "mdp_lcdc_pad_pclk_clk");
		if (IS_ERR(pixel_lcdc_clk)) {
			printk(KERN_ERR "Couldnt find mdp_lcdc_pad_pclk_clk\n");
			return -EINVAL;
		}
	}

	pm_qos_add_requirement(PM_QOS_SYSTEM_BUS_FREQ , "lcdc",
			       PM_QOS_DEFAULT_VALUE);
	return lcdc_register_driver();
}
#endif // CONFIG_FB_MSM_LCDC

#ifdef CONFIG_FB_MSM_LCDC
// LCDC interface
#include "msm_fb.h"

static int lcdc_panel_on(struct platform_device *pdev)
{
	return 0;
}

static int lcdc_panel_off(struct platform_device *pdev)
{
	return 0;
}

static int __init lcdc_panel_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = lcdc_panel_probe,
	.driver = {
		.name   = "lcdc_panel",
	},
};

static struct msm_fb_panel_data lcdc_panel_data = {
	.on = lcdc_panel_on,
	.off = lcdc_panel_off,
};

static int lcdc_dev_id;

int lcdc_device_register(struct msm_panel_info *pinfo)
{
	struct platform_device *pdev = NULL;
	int ret;

	pdev = platform_device_alloc("lcdc_panel", ++lcdc_dev_id);
	if (!pdev)
		return -ENOMEM;

	lcdc_panel_data.panel_info = *pinfo;
	ret = platform_device_add_data(pdev, &lcdc_panel_data,
		sizeof(lcdc_panel_data));
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int __init lcdc_panel_init(void)
{
	return platform_driver_register(&this_driver);
}
#endif // CONFIG_FB_MSM_LCDC
*/

//#ifdef CONFIG_FB_MSM_MDDI
// MDDI interface
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <mach/hardware.h>
#include <asm/io.h>

#include <asm/system.h>
#include <asm/mach-types.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include "msm_fb.h"
#include "mddihosti.h"
#include "mddihost.h"
#include <mach/gpio.h>
#include <mach/clk.h>

static int mddi_probe(struct platform_device *pdev);
static int mddi_remove(struct platform_device *pdev);

static int mddi_off(struct platform_device *pdev);
static int mddi_on(struct platform_device *pdev);

#ifdef CONFIG_PM
static int mddi_suspend(struct platform_device *pdev, pm_message_t state);
static int mddi_resume(struct platform_device *pdev);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mddi_early_suspend(struct early_suspend *h);
static void mddi_early_resume(struct early_suspend *h);
#endif

static struct platform_device *pdev_list[MSM_FB_MAX_DEV_LIST];
static int pdev_list_cnt;
static struct clk *mddi_clk;
static struct clk *mddi_pclk;
static struct mddi_platform_data *mddi_pdata;

static struct platform_driver mddi_driver = {
	.probe = mddi_probe,
	.remove = mddi_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
#ifdef CONFIG_PM
	.suspend = mddi_suspend,
	.resume = mddi_resume,
#endif
#endif
	.shutdown = NULL,
	.driver = {
		   .name = "mddi",
		   },
};

extern int int_mddi_pri_flag;

static int mddi_off(struct platform_device *pdev)
{
	int ret = 0;

	ret = panel_next_off(pdev);

	if (mddi_pdata && mddi_pdata->mddi_power_save)
		mddi_pdata->mddi_power_save(0);

	return ret;
}

static int mddi_on(struct platform_device *pdev)
{
	int ret = 0;
	u32 clk_rate;
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (mddi_pdata && mddi_pdata->mddi_power_save)
		mddi_pdata->mddi_power_save(1);

	clk_rate = mfd->fbi->var.pixclock;
	clk_rate = min(clk_rate, mfd->panel_info.clk_max);

	if (mddi_pdata &&
	    mddi_pdata->mddi_sel_clk &&
	    mddi_pdata->mddi_sel_clk(&clk_rate))
			printk(KERN_ERR
			  "%s: can't select mddi io clk targate rate = %d\n",
			  __func__, clk_rate);

	if (clk_set_min_rate(mddi_clk, clk_rate) < 0)
		printk(KERN_ERR "%s: clk_set_min_rate failed\n",
			__func__);

	ret = panel_next_on(pdev);

	return ret;
}

static int mddi_resource_initialized;

static int mddi_probe(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct platform_device *mdp_dev = NULL;
	struct msm_fb_panel_data *pdata = NULL;
	int rc;
	resource_size_t size ;
	u32 clk_rate;

	if ((pdev->id == 0) && (pdev->num_resources >= 0)) {
		mddi_pdata = pdev->dev.platform_data;

		size =  resource_size(&pdev->resource[0]);
		msm_pmdh_base =  ioremap(pdev->resource[0].start, size);

		MSM_FB_INFO("primary mddi base phy_addr = 0x%x virt = 0x%x\n",
				pdev->resource[0].start, (int) msm_pmdh_base);

		if (unlikely(!msm_pmdh_base))
			return -ENOMEM;

		if (mddi_pdata && mddi_pdata->mddi_power_save)
			mddi_pdata->mddi_power_save(1);

		mddi_resource_initialized = 1;
		return 0;
	}

	if (!mddi_resource_initialized)
		return -EPERM;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	if (pdev_list_cnt >= MSM_FB_MAX_DEV_LIST)
		return -ENOMEM;

	mdp_dev = platform_device_alloc("mdp", pdev->id);
	if (!mdp_dev)
		return -ENOMEM;

	/*
	 * link to the latest pdev
	 */
	mfd->pdev = mdp_dev;
	mfd->dest = DISPLAY_LCD;

	/*
	 * alloc panel device data
	 */
	if (platform_device_add_data
	    (mdp_dev, pdev->dev.platform_data,
	     sizeof(struct msm_fb_panel_data))) {
		printk(KERN_ERR "mddi_probe: platform_device_add_data failed!\n");
		platform_device_put(mdp_dev);
		return -ENOMEM;
	}
	/*
	 * data chain
	 */
	pdata = mdp_dev->dev.platform_data;
	pdata->on = mddi_on;
	pdata->off = mddi_off;
	pdata->next = pdev;

	/*
	 * get/set panel specific fb info
	 */
	mfd->panel_info = pdata->panel_info;

#ifdef MSMFB_FRAMEBUF_32
	if (mfd->index == 0)
		mfd->fb_imgType = MDP_RGBA_8888; /* primary */
	else
		mfd->fb_imgType = MDP_RGB_565;	/* secondary */
#else
	mfd->fb_imgType = MDP_RGB_565;
#endif

	clk_rate = mfd->panel_info.clk_max;
	if (mddi_pdata &&
	    mddi_pdata->mddi_sel_clk &&
	    mddi_pdata->mddi_sel_clk(&clk_rate))
			printk(KERN_ERR
			  "%s: can't select mddi io clk targate rate = %d\n",
			  __func__, clk_rate);

	if (clk_set_max_rate(mddi_clk, clk_rate) < 0)
		printk(KERN_ERR "%s: clk_set_max_rate failed\n", __func__);
	mfd->panel_info.clk_rate = mfd->panel_info.clk_min;

	if (!mddi_client_type)
		mddi_client_type = mfd->panel_info.lcd.rev;
	else if (!mfd->panel_info.lcd.rev)
		printk(KERN_ERR
		"%s: mddi client is trying to revert back to type 1	!!!\n",
		__func__);

	/*
	 * set driver data
	 */
	platform_set_drvdata(mdp_dev, mfd);

	/*
	 * register in mdp driver
	 */
	rc = platform_device_add(mdp_dev);
	if (rc)
		goto mddi_probe_err;

	pdev_list[pdev_list_cnt++] = pdev;

#ifdef CONFIG_HAS_EARLYSUSPEND
	mfd->mddi_early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	mfd->mddi_early_suspend.suspend = mddi_early_suspend;
	mfd->mddi_early_suspend.resume = mddi_early_resume;
	register_early_suspend(&mfd->mddi_early_suspend);
#endif

	return 0;

mddi_probe_err:
	platform_device_put(mdp_dev);
	return rc;
}

static int mddi_pad_ctrl;
static int mddi_power_locked;

void mddi_disable(int lock)
{
	mddi_host_type host_idx = MDDI_HOST_PRIM;

	if (mddi_power_locked)
		return;

	if (lock)
		mddi_power_locked = 1;

	if (mddi_host_timer.function)
		del_timer_sync(&mddi_host_timer);

	mddi_pad_ctrl = mddi_host_reg_in(PAD_CTL);
	mddi_host_reg_out(PAD_CTL, 0x0);

	if (clk_set_min_rate(mddi_clk, 0) < 0)
		printk(KERN_ERR "%s: clk_set_min_rate failed\n", __func__);

	clk_disable(mddi_clk);
	if (mddi_pclk)
		clk_disable(mddi_pclk);
	disable_irq(INT_MDDI_PRI);

	if (mddi_pdata && mddi_pdata->mddi_power_save)
		mddi_pdata->mddi_power_save(0);
}

#ifdef CONFIG_PM
static int mddi_is_in_suspend;

static int mddi_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (mddi_is_in_suspend)
		return 0;

	mddi_is_in_suspend = 1;
	mddi_disable(0);
	return 0;
}

static int mddi_resume(struct platform_device *pdev)
{
	mddi_host_type host_idx = MDDI_HOST_PRIM;

	if (!mddi_is_in_suspend)
		return 0;

	mddi_is_in_suspend = 0;

	if (mddi_power_locked)
		return 0;

	enable_irq(INT_MDDI_PRI);
	clk_enable(mddi_clk);
	if (mddi_pclk)
		clk_enable(mddi_pclk);
	mddi_host_reg_out(PAD_CTL, mddi_pad_ctrl);

	if (mddi_host_timer.function)
		mddi_host_timer_service(0);

	return 0;
}
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mddi_early_suspend(struct early_suspend *h)
{
	pm_message_t state;
	struct msm_fb_data_type *mfd = container_of(h, struct msm_fb_data_type,
							mddi_early_suspend);

	state.event = PM_EVENT_SUSPEND;
	mddi_suspend(mfd->pdev, state);
}

static void mddi_early_resume(struct early_suspend *h)
{
	struct msm_fb_data_type *mfd = container_of(h, struct msm_fb_data_type,
							mddi_early_suspend);
	mddi_resume(mfd->pdev);
}
#endif

static int mddi_remove(struct platform_device *pdev)
{
	if (mddi_host_timer.function)
		del_timer_sync(&mddi_host_timer);

	iounmap(msm_pmdh_base);

	return 0;
}

static int mddi_register_driver(void)
{
	return platform_driver_register(&mddi_driver);
}

static int __init mddi_driver_init(void)
{
	int ret;

	mddi_clk = clk_get(NULL, "mddi_clk");
	if (IS_ERR(mddi_clk)) {
		printk(KERN_ERR "can't find mddi_clk \n");
		return PTR_ERR(mddi_clk);
	}
	clk_enable(mddi_clk);

	mddi_pclk = clk_get(NULL, "mddi_pclk");
	if (IS_ERR(mddi_pclk))
		mddi_pclk = NULL;
	else
		clk_enable(mddi_pclk);

	ret = mddi_register_driver();
	if (ret) {
		clk_disable(mddi_clk);
		clk_put(mddi_clk);
		if (mddi_pclk) {
			clk_disable(mddi_pclk);
			clk_put(mddi_pclk);
		}
		printk(KERN_ERR "mddi_register_driver() failed!\n");
		return ret;
	}

	mddi_init();

	return ret;
}
//#endif // CONFIG_FB_MSM_MDDI

static int __init mddi_lcdc_driver_init(void)
{
	int ret;
/*
	cci_smem_value_t *smem_cci_smem_value;
	smem_cci_smem_value = smem_alloc( SMEM_CCI_SMEM_VALUE, sizeof( cci_smem_value_t ));

	switch(smem_cci_smem_value->cci_project_id)
	{
		case PROJECT_ID_K4H:
		case PROJECT_ID_CAP6:
		case PROJECT_ID_CAP2:
#ifdef CONFIG_FB_MSM_MDDI
*/
				ret = mddi_driver_init();
				printk(KERN_INFO "[John] mddi_lcdc_driver_init: MDDI ret=%d\r\n", ret);
/*
#endif // CONFIG_FB_MSM_MDDI
				break;
		case PROJECT_ID_K4:
		case PROJECT_ID_CAP8:
		case PROJECT_ID_K5:
		default:
#ifdef CONFIG_FB_MSM_LCDC
				ret = lcdc_driver_init();
				ret = lcdc_panel_init();
				printk(KERN_INFO "[John] mddi_lcdc_driver_init: LCDC ret=%d\r\n", ret);
#endif // CONFIG_FB_MSM_LCDC
				break;
	}
*/
	return ret;
}

module_init(mddi_lcdc_driver_init);