/* drivers/input/touchscreen/msm_touch.c
 *
 * Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
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

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/io.h>
//#include "mach/../../cci_smem.h"
#include "mach/../../smd_private.h"
#include <mach/msm_touch.h>
#include <linux/irq.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <asm/mach-types.h>
#include <mach/vreg.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <mach/rpc_server_handset.h>
#include "../../../arch/arm/mach-msm/smd_private.h"
#define TS_PENUP_TIMEOUT_MS 50

struct ts {
	struct input_dev *input;
	struct timer_list timer;
	struct timer_list error_monitor;
	int irq;
	unsigned int x_max;
	unsigned int y_max;
	unsigned int x_min;
	unsigned int y_min;
	struct work_struct work;
	spinlock_t	lock;
};

struct st1232_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
	int irq;
	atomic_t irq_disable;
	bool has_relative_report;
	struct hrtimer timer;
	struct work_struct  ts_event_work;
	uint16_t max[2];
	int snap_state[2][2];
	int snap_down_on[2];
	int snap_down_off[2];
	int snap_up_on[2];
	int snap_up_off[2];
	int snap_down[2];
	int snap_up[2];
	uint32_t flags;
	int reported_finger_count;
	int8_t sensitivity_adjust;
	int (*power)(int on);
	struct early_suspend early_suspend;
       int prev_touches;
	bool isp_enabled;
};

struct input_dev *kpdev;
struct ts *ts_point;
int tp_version=1;
static u8  tpversion;
static u8  keyregister,keypress;
static bool touchedge=0;
int TOUCH_GPIO = 94;
bool  BKL_EN=1;
int ts_width;
struct st1232_ts_data *st1232_ts;	
static u32 last_x;
static u32 last_y;
static struct early_suspend ts_early;

static void st1232_ts_timer(unsigned long arg);
static irqreturn_t st1232_ts_interrupt(int irq, void *dev_id);
int st1232_i2c_read(struct i2c_client *client, int reg, u8 *buf, int count);

extern int cci_in_CTS;  // by cci - for low memory killer
static void st1232_ts_update_pen_state(struct st1232_ts_data *ts, int x, int y, int pressure)
{
	int i=0;/*
	if(NULL!=msm_7k_handset_get_input_dev())
		kpdev =(struct input_dev *) msm_7k_handset_get_input_dev();
	else
	{
		printk("[bing]msm_7k_handset_get_input_dev=NULL \r\n");
		goto err;
	}*/
	//printk("[Bing][%s] x=%d, y=%d, p=%d keyregister=%d ,tpversion=%d \r\n", __func__, x, y, pressure,keyregister,tpversion);
	//printk("[bing]kpdev=%d ,KEY_HOME=%d KEY_SEARCH=%d KEY_BACK=%d KEY_MENU=%d ,pressure=%d\r\n",kpdev,KEY_HOME,KEY_SEARCH,KEY_BACK,KEY_MENU,pressure);
	if(y<=480)
	{	
        	for (i = 0; i < st1232_ts->prev_touches; i++) {
        		input_report_abs(st1232_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
        		input_report_abs(st1232_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
        		input_mt_sync(st1232_ts->input_dev);
        	}
        	st1232_ts->prev_touches = 0;
        	input_sync(st1232_ts->input_dev);
	}

		 if(tpversion>=1) //Herman
		{
			if(keyregister==1)
			{	
				input_report_key(st1232_ts->input_dev, KEY_HOME, pressure);	
				//printk("[bing]KEY_HOME,keyregister=%d \r\n",keyregister);
			}
			else if(keyregister==2)
			{	
				input_report_key(st1232_ts->input_dev, KEY_SEARCH, pressure);
				//printk("[bing]KEY_SEARCH,keyregister=%d \r\n",keyregister);
			}
			else if(keyregister==4)
			{	
				input_report_key(st1232_ts->input_dev, KEY_BACK, pressure);		
				//printk("[bing]KEY_BACK,keyregister=%d \r\n",keyregister);
			}
			else if(keyregister==8)
			{
				input_report_key(st1232_ts->input_dev, KEY_MENU, pressure);
				//printk("[bing]KEY_MENU,keyregister=%d \r\n",keyregister);
			}
        		input_sync(st1232_ts->input_dev);
		}	
 		else if(tpversion==0)
 		{
 	 			if(y>490)			
 	 			{
					if(x>0&&x<=80)
					{	
						input_report_key(st1232_ts->input_dev, KEY_HOME, pressure);	
						//printk("[bing]KEY_HOME \r\n");
					}
					else if(x>80&&x<=150)
					{
						input_report_key(st1232_ts->input_dev, KEY_SEARCH, pressure);
						//printk("[bing]KEY_SEARCH \r\n");
					}
					else if(x>150&&x<=260)
					{	
						input_report_key(st1232_ts->input_dev, KEY_BACK, pressure);		
						//printk("[bing]KEY_BACK \r\n");
					}
					else if(x>260)
					{
						input_report_key(st1232_ts->input_dev, KEY_MENU, pressure);
						//printk("[bing]KEY_MENU \r\n");
					}
				}
          		  input_sync(st1232_ts->input_dev);
		}
//err:
	//printk("st1232_ts_update_pen_state\r\n");
}
static void st1232_ts_timer(unsigned long arg)
{
	//printk("[Bing][%s]\r\n", __func__);
	st1232_ts_update_pen_state(st1232_ts, last_x, last_y, 0);
	touchedge=0;
	keyregister=0;
}

static irqreturn_t st1232_ts_interrupt(int irq, void *dev_id)
{
	//printk("[Bing]st1232_ts_interrupt\r\n");

	atomic_dec(&st1232_ts->irq_disable);
	disable_irq_nosync(st1232_ts->irq);
	schedule_work(&st1232_ts->ts_event_work);
	return IRQ_HANDLED;
}

int st1232_i2c_read(struct i2c_client *client, int reg, u8 *buf, int count)
{
	int rc = 0;
	int ret = 0;

	buf[0] = reg;
	rc = i2c_master_send(client, buf, 1);
	if (rc != 1) {
		dev_err(&client->dev,"%s FAILED: read of register %d\n",__func__, reg);
		ret = -1;
		goto failed;
	}
	//memset(buf,0x00,count);
	rc = i2c_master_recv(client, buf, count);
	if (rc != count) {
		dev_err(&client->dev,"%s FAILED: read %d bytes from reg %d\n", __func__,count, reg);
		ret = -1;
        goto failed;
	}

	return ret;
failed:
    //reset_TP_state();
	return ret;
}

static void report_data(struct st1232_ts_data *ts, u32 x, u32 y, int pressure)
{
	input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, pressure);
//	input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR, ts->dd->finger_size);
	input_mt_sync(ts->input_dev);
}

static void st1232_ts_irq_worker(struct work_struct *work)
{
       u8 buf[8],buf1[16];
       u32 x_st1,y_st1,x_st2,y_st2;
       int ret,rc,press1=0, press2=0;
	if(tp_version==1)
	{
		buf1[0]=0x0;
		rc=i2c_master_send(st1232_ts->client, buf1,1);
		if(rc!=1)
			printk("[Bing]st1232  i2c_master_send  error\r\n");
		
		rc= i2c_master_recv(st1232_ts->client, buf1,16);
		if(rc!=16)
			printk("[Bing]st1232  i2c_master_recv  error\r\n");
		
		printk("[Bing]st1232 probe:buf[0]=%d\r\n", buf1[0]);
		printk("[Bing]st1232 probe:buf[1]=%d\r\n", buf1[1]);
		printk("[Bing]st1232 probe:buf[2]=%d\r\n", buf1[2]);
		printk("[Bing]st1232 probe:buf[3]=%d\r\n", buf1[3]);
		printk("[Bing]st1232 probe:buf[4]=%d\r\n", buf1[4]);
		printk("[Bing]st1232 probe:buf[5]=%d\r\n", buf1[5]);
		printk("[Bing]st1232 probe:buf[6]=%d\r\n", buf1[6]);
		printk("[Bing]st1232 probe:buf[7]=%d\r\n", buf1[7]);
		printk("[Bing]st1232 probe:buf[8]=%d\r\n", buf1[8]);
		printk("[Bing]st1232 probe:buf[9]=%d\r\n", buf1[9]);
		printk("[Bing]st1232 probe:buf[10]=%d\r\n", buf1[10]);
		printk("[Bing]st1232 probe:buf[11]=%d\r\n", buf1[11]);
		printk("[Bing]st1232 probe:buf[12]=%d\r\n", buf1[12]);
		printk("[Bing]st1232 probe:buf[13]=%d\r\n", buf1[13]);
		printk("[Bing]st1232 probe:buf[14]=%d\r\n", buf1[14]);
		printk("[Bing]st1232 probe:buf[15]=%d\r\n", buf1[15]);

		tpversion=buf1[0];
		buf[0]=0x10;
		rc = i2c_master_send(st1232_ts->client, buf,1);
		if(rc!=1)
			printk("[Bing]st1232  i2c_master_send  error\r\n");
		tp_version=0;		
	}
	buf[0]=0x10;
	rc = i2c_master_send(st1232_ts->client, buf,1);
	if(rc!=1)
		printk("[Bing]st1232  i2c_master_send  error\r\n");

	ret = i2c_master_recv(st1232_ts->client, buf, 8);
	if (BKL_EN==1)
	{
             if( (buf[2]&0x80) && (buf[5]&0x80))
	     {
                    press1 = 1;
                    press2 = 1;
              }
              else if( (buf[2]&0x80))
                    press1 = 1;
              else if( (buf[5]&0x80))
                     press2 = 1;
	      x_st1 = ((buf[2]&0x70)<<4)|buf[3];
	      y_st1 = (((buf[2]&0x07)<<8)|buf[4]);
	      x_st2 = ((buf[5]&0x70)<<4)|buf[6];
	      y_st2 = (((buf[5]&0x07)<<8)|buf[7]);
	    // printk("[bing]press1=%d  press2=%d x_st1=%d  y_st1=%d\r\n",press1,press2,x_st1,y_st1);


            if(press1 && press2) 
	    {
                last_x = x_st1;
                last_y = y_st1;               
                if(y_st1<=480) { //finger 1 is in touch area
                    	report_data(st1232_ts, x_st1, y_st1, 1 );
                     
                     if(y_st2<=480){
                   	    report_data(st1232_ts, x_st2, y_st2, 1 );
                        st1232_ts->prev_touches = 2;
                     }
                     else
                        st1232_ts->prev_touches = 1;

                    input_sync(st1232_ts->input_dev);                     
                 }
                 else { // buttom 4...
                    st1232_ts_update_pen_state(st1232_ts,x_st1, y_st1, 1 );                    
                  }
                  
            }
            else if(press1)
	    {		

                if(y_st1<=480)
		{ //finger 1 is in touch area
                    	report_data(st1232_ts, x_st1, y_st1, 1 );
                     if(st1232_ts->prev_touches == 2){
                        input_report_abs(st1232_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
                	input_report_abs(st1232_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
                	input_mt_sync(st1232_ts->input_dev);
                     }
                     st1232_ts->prev_touches = 1;
		     touchedge=1;
                     input_sync(st1232_ts->input_dev);
                 }
               	 else if(tpversion==0)
                 {

			//printk("[bing]touchedge=%d y_st1=%d\r\n",touchedge,y_st1);				 				 
                  	if(y_st1>480)
                  	{
     				if(touchedge==1)
				{
	               		    	report_data(st1232_ts, x_st1, y_st1, 0 );		
					st1232_ts_update_pen_state(st1232_ts, last_x, last_y, 0);			
					printk("[bing]trigger edge=%d\r\n",touchedge);
					touchedge=0;
				}
				else
	 	             		 st1232_ts_update_pen_state(st1232_ts,x_st1, y_st1, 1 );
                 	}
                 }
                last_x = x_st1;
                last_y = y_st1;               
		//printk("[bing]x_st1,y_st1=(%d %d)  trigger edge=%d\r\n",x_st1,y_st1,touchedge);				 
            }
            else if(press2)
	    {				
                last_x = x_st2;
                last_y = y_st2;               
                if(y_st2<=480)
		{ //finger 1 is in touch area
                    	report_data(st1232_ts, x_st2, y_st2, 1 );
                     if(st1232_ts->prev_touches == 2)
		     {
                        input_report_abs(st1232_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
                	input_report_abs(st1232_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
                	input_mt_sync(st1232_ts->input_dev);
                     }
                     st1232_ts->prev_touches = 1;
                    input_sync(st1232_ts->input_dev);

                 }
                 else
                    st1232_ts_update_pen_state(st1232_ts,x_st2, y_st2, 1 );                    
            }
	    else
	    {
        	 if(tpversion>=1) //Herman
              	 {
    			keypress=buf[1];
			if(keypress!=0)	
				keyregister=keypress;
        		if(keypress!=0)
        		{
	    			//printk("[Bing]keyregister=%d \r\n",buf[1]);
				x_st1=y_st1=-1;
				st1232_ts_update_pen_state(st1232_ts,x_st1,y_st1, 1);		
        		}
                  }
	     }		

	//		int mod_timer(struct timer_list *timer, unsigned long expires)

        	mod_timer(&st1232_ts->timer,jiffies + msecs_to_jiffies(TS_PENUP_TIMEOUT_MS));
	}
	/* kick pen up timer - to make sure it expires again(!) */
	enable_irq(st1232_ts->irq);
	atomic_inc(&st1232_ts->irq_disable);
}

static int __devinit st1232_ts_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{	
	struct input_dev *input_dev;
	int ret = 0;

	printk("[Bing][%s]\r\n", __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk("[Bing][%s] st1232_ts_probe: need I2C_FUNC_I2C\r\n", __func__);
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	ret = gpio_request(93,"touch_int");
	if(ret < 0)
		printk("fail to request gpio93 for touch_int! error = %d\n",ret);
	
	 printk("[Bing]After %s: TOUCH_GPIO93 = %d\n",__func__,gpio_get_value(93) );
	gpio_set_value(93, 1);
	ret = gpio_request(TOUCH_GPIO,"touch_int");
	if(ret < 0)
		printk("fail to request gpio for touch_int! error = %d\n",ret);
	 printk("[Bing]Before %s: TOUCH_GPIO94 = %d\n",__func__,gpio_get_value(TOUCH_GPIO) );
	 printk("[Bing]After %s: TOUCH_GPIO94 = %d\n",__func__,gpio_get_value(TOUCH_GPIO) );
   	 gpio_direction_output(TOUCH_GPIO, 1);
	#if 0
  printk("%s: TOUCH_GPIO = %d\n",__func__,gpio_get_value(TOUCH_GPIO) );
  mdelay(20);
  vreg_ruim = vreg_get(NULL, "ruim");
	vreg_disable(vreg_ruim);
	mdelay(20);
	vreg_set_level(vreg_ruim,2600);
	vreg_enable(vreg_ruim);
	mdelay(20);
	vreg_disable(vreg_ruim);
	mdelay(20);
	vreg_enable(vreg_ruim);
	mdelay(200);
	#endif
	st1232_ts = (struct st1232_ts_data *)kzalloc(sizeof(struct st1232_ts_data), GFP_KERNEL);
	st1232_ts->input_dev = input_allocate_device();
	if (!st1232_ts->input_dev || !st1232_ts)
	{
		ret = -ENOMEM;
		goto err_input;
	}
	
	INIT_WORK(&st1232_ts->ts_event_work, st1232_ts_irq_worker);
           //init 
        st1232_ts->prev_touches = 0;

	st1232_ts->client = client;
	st1232_ts->irq = MSM_GPIO_TO_INT(TOUCH_GPIO);
	st1232_ts->isp_enabled = false;
	atomic_set(&st1232_ts->irq_disable, 1);
	i2c_set_clientdata(client, st1232_ts);
	input_dev = st1232_ts->input_dev;
	input_dev->name = "st1232-ts";
	input_dev->phys = "msm_touch/input0";
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0002;
	input_dev->id.version = 0x0100;
	input_dev->dev.parent = &st1232_ts->client->dev;
	

/*	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);
	input_dev->absbit[BIT_WORD(ABS_MISC)] = BIT_MASK(ABS_MISC);
        input_dev->keybit[BIT_WORD(BTN_TOUCH)] |= BIT_MASK(BTN_TOUCH);
*/
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(ABS_MISC, input_dev->absbit);

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);
	/*
	input_set_capability(input_dev,EV_KEY, KEY_HOME);
	input_set_capability(input_dev,EV_KEY, KEY_SEARCH);
	input_set_capability(input_dev,EV_KEY, KEY_BACK);
	input_set_capability(input_dev,EV_KEY, KEY_MENU);*/

		
/*        
	set_bit(EV_SYN, st1232_ts->input_dev->evbit);
	set_bit(EV_ABS, st1232_ts->input_dev->evbit);
	set_bit(EV_KEY, st1232_ts->input_dev->evbit);
	set_bit(ABS_MT_TOUCH_MAJOR, st1232_ts->input_dev->keybit);
	set_bit(KEY_BACK, st1232_ts->input_dev->keybit);
	set_bit(0xE5, st1232_ts->input_dev->keybit);
	set_bit(KEY_SEARCH, st1232_ts->input_dev->keybit);		
*/
//#if 0
	input_set_capability(input_dev,EV_KEY, KEY_HOME);
	input_set_capability(input_dev,EV_KEY, KEY_SEARCH);
	input_set_capability(input_dev,EV_KEY, KEY_BACK);
	input_set_capability(input_dev,EV_KEY, KEY_MENU);	
//#endif
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,0, 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR,0, 320, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, 320, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, 480, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	//printk("[Bing][%s] st1232_ts->input_dev = 0x%x\r\n", __func__, st1232_ts->input_dev);
	ret = input_register_device(st1232_ts->input_dev);
	if (ret)
	{
		printk("[Bing][%s] input_register_device(st1232_ts->input_dev) fail\r\n", __func__);
		goto err_register;		
	}
	input_set_drvdata(st1232_ts->input_dev, st1232_ts);
	setup_timer(&st1232_ts->timer,st1232_ts_timer,st1232_ts);
	ret = request_irq(st1232_ts->irq, st1232_ts_interrupt, IRQF_TRIGGER_LOW, "touchscreen", NULL);
	if (ret < 0) {
		dev_err(&st1232_ts->client->dev, "Failed to register interrupt\n");
		goto err_irq;
	}

	return 0;
	
err_register:
	input_free_device(st1232_ts->input_dev);
err_irq:
	input_unregister_device(st1232_ts->input_dev);
	st1232_ts->input_dev = NULL;	
err_input:
	return ret;
err_check_functionality_failed:
	return ret;
}

static int st1232_ts_remove(struct i2c_client *client)
{
	printk("[Bing][%s]\r\n", __func__);
	return 0;
}

void st1232_early_suspend(struct early_suspend *h)
{
	int rc;
	u8 buf[2];
	buf[0]=0x2;
	buf[1]=0xA;
	printk("[Bing]st1232_early_suspend,gp4=0v\r\n");
	rc = i2c_master_send(st1232_ts->client, buf,2);
	if(rc!=2)
	{
		printk("[Bing]st1232_early_suspend: i2c_master_send 0x2,0xA error\r\n");
	}
	buf[0]=0x10;
	rc = i2c_master_send(st1232_ts->client, buf,1);
	if(rc!=1)
	{
		printk("[Bing]st1232_early_suspend: i2c_master_send 0x10 error\r\n");
	}
}

void st1232_later_resume(struct early_suspend *h)
{
	int ret;
	ret = gpio_request(93,"touch_int");
	if(ret < 0)
		printk("fail to request gpio93 for touch_int! error = %d\n",ret);	
	 gpio_set_value(93, 0);
         mdelay(20); 
	 gpio_set_value(93, 1);	 
	 printk("[Bing] %s: TOUCH_GPIO93 = %d\n",__func__,gpio_get_value(93) );
}

/*
static int st1232_ts_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk("[Bing]st1232_ts_suspend\r\n");	
	return 0;
}

static int st1232_ts_resume(struct platform_device *pdev)
{
	printk("[Bing]st1232_ts_resume\r\n");	
	return 0;
}
*/
static const struct i2c_device_id st1232_ts_id[] = {
	{ "st1232-ts", 0 },
	{ }
};

static struct i2c_driver st1232_ts_driver = {
	.probe		= st1232_ts_probe,
	.remove		= st1232_ts_remove,
	//.suspend 		 = st1232_ts_suspend,
	//.resume		=  st1232_ts_resume,
	.id_table	= st1232_ts_id,
	.driver = {
	.name	= "st1232-ts",
	},
};

static int __init ts_init(void)
{	
	u8 buf[8];
	int ret,rc;
	struct vreg *vreg_gp4 = NULL;
	u32 x_st,y_st;

		printk("[Bing]set gp4=2.6v\r\n");
		vreg_gp4 = vreg_get(0, "gp4");
		rc = vreg_set_level(vreg_gp4, 2600);
		if (rc) 
		{
			printk("[Bing]%s: vreg set gp4 level failed (%d)\n",	__func__, rc);
		}

		rc = vreg_enable(vreg_gp4);
		if (rc) 
		{
			printk("[Bing]%s: vreg enable gp4 failed (%d)\n", __func__, rc);
		}
		ret = i2c_add_driver(&st1232_ts_driver);
		ts_early.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
		ts_early.suspend = st1232_early_suspend;
		ts_early.resume = st1232_later_resume;
		register_early_suspend(&ts_early);	
		printk("[Bing][%s]K4H/CAP6 project, ret = %d\r\n", __func__, ret);

		//ret = gpio_request(TOUCH_GPIO,"touch_int");	
		//if(ret < 0)
		//	printk("fail to request gpio for touch_int! error = %d\n",ret);
		// printk("[Bing] %s: TOUCH_GPIO = %d\n",__func__,gpio_get_value(TOUCH_GPIO) );
		 //gpio_direction_output(TOUCH_GPIO, 1);
	 	printk("[Bing] %s: TOUCH_GPIO = %d\n",__func__,gpio_get_value(TOUCH_GPIO) );		
		ret = i2c_master_recv(st1232_ts->client, buf, 8);
			y_st = ((buf[2]&0x70)<<4)|buf[3];
			x_st = 289 - (((buf[2]&0x07)<<8)|buf[4]);
			printk("[Bing]init x_st=%d, y_st=%d\r\n", x_st, y_st);


	return ret;
}
module_init(ts_init);
MODULE_DESCRIPTION("MSM Touch Screen driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:msm_touchscreen");
