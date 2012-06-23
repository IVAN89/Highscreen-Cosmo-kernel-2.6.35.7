#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_cameraGIGABYTE.h>
#include <mach/gpio.h>
#include <mach/cameraGIGABYTE.h>
#include "s5k5cag.h"
#include <mach/vreg.h>

#include <mach/pmic.h>
//Camera Power
#ifdef CONFIG_MACH_MSM7X25_QRD
static struct vreg *vreg_CAM_gp3;      //power 1.5v
static struct vreg *vreg_CAM_gp6;      //power 2.8v
static struct vreg *vreg_CAM_wlan;     //power 2.8v
#endif

#ifdef CONFIG_MACH_MSM7X27_SKU5
static struct vreg *vreg_CAM_gp6;      //power 2.8v
#endif

#define CAM_DBG

#define GPIO_0 0
#define GPIO_1 1
#define GPIO_PULL_HIGH 1
#define GPIO_PULL_LOW  0

struct s5k5cag_work {
   struct work_struct work;
};
static struct s5k5cag_work *s5k5cag_sensorw;
static struct i2c_client    *s5k5cag_client;
static DECLARE_WAIT_QUEUE_HEAD(s5k5cag_wait_queue);
DEFINE_MUTEX(s5k5cag_mutex);
static u8 s5k5cag_i2c_buf[4];
static u8 s5k5cag_counter = 0;

static int is_autoflash =0;
struct __s5k5cag_ctrl 
{
	const struct msm_camera_sensor_info *sensordata;
	int sensormode;
	uint fps_divider; /* init to 1 * 0x00000400 */
	uint pict_fps_divider; /* init to 1 * 0x00000400 */
	u16 curr_step_pos;
	u16 curr_lens_pos;
	u16 init_curr_lens_pos;
	u16 my_reg_gain;
	u16 my_reg_line_count;
	enum msm_s_resolution prev_res;
	enum msm_s_resolution pict_res;
	enum msm_s_resolution curr_res;
	enum msm_s_test_mode  set_test;
};
static struct __s5k5cag_ctrl *s5k5cag_ctrl;
//static u32 exp_gain_tbl[3][2];

static int s5k5cag_i2c_remove(struct i2c_client *client);
static int s5k5cag_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id);

static int s5k5cag_i2c_txdata(u16 saddr,u8 *txdata,int length)
{
	struct i2c_msg msg[] = {
		{
			.addr  = saddr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};
	if (i2c_transfer(s5k5cag_client->adapter, msg, 1) < 0)	return -EIO;
	else return 0;
}

static int s5k5cag_i2c_write(unsigned short saddr, unsigned int waddr,
	unsigned short bdata,u8 trytimes)
{
   int rc = -EIO;
   s5k5cag_counter = 0;
   s5k5cag_i2c_buf[0] = (waddr & 0xFF00)>>8;
   s5k5cag_i2c_buf[1] = (waddr & 0x00FF);
   s5k5cag_i2c_buf[2] = (bdata & 0xFF00)>>8;
   s5k5cag_i2c_buf[3] = (bdata & 0x00FF);
   
   while ( (s5k5cag_counter<trytimes) &&(rc != 0) )
   {
      rc = s5k5cag_i2c_txdata(saddr, s5k5cag_i2c_buf, 4);
      if (rc < 0)
      {
      	s5k5cag_counter++;
      	printk(KERN_ERR "***Tom i2c_write_w failed,i2c addr=0x%x, command addr = 0x%x, val = 0x%x, s=%d, rc=%d!\n",saddr,waddr, bdata,s5k5cag_counter,rc);
      	msleep(4);
      }
      else 
	  {
		//printk(KERN_ERR "--CAMERA--i2c_write_w ok!\n");
	  }
   }
   return rc;
}
/*
static int s5k5cag_i2c_write_table(u32 p[][2],u8 trytimes)
{
	int i=0,rc=0;
	for(;i<(sizeof(p)/sizeof(p[0])/2);i++)
	{
		rc = s5k5cag_i2c_write(s5k5cag_client->addr,p[i][0],p[i][1],trytimes);
		if (rc < 0) return rc;
	}
	return rc;
}
*/
static int s5k5cag_i2c_rxdata(unsigned short saddr,
	unsigned char *rxdata, int length)
{
	struct i2c_msg msgs[] = {
	{
		.addr   = saddr,
		.flags = 0,
		.len   = 2,
		.buf   = rxdata,
	},
	{
		.addr   = saddr,
		.flags = I2C_M_RD,
		.len   = length,
		.buf   = rxdata,
	},
	};

	if (i2c_transfer(s5k5cag_client->adapter, msgs, 2) < 0) {
		CDBG("s5k5cag_i2c_rxdata failed!\n");
		printk(KERN_ERR "s5k5cag_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t s5k5cag_i2c_read(unsigned short   saddr,
	unsigned int raddr, unsigned int *rdata)
{
	int rc = 0;
	unsigned char buf[2];
	printk(KERN_ERR "+s5k5cag_i2c_read\n");
	memset(buf, 0, sizeof(buf));

	buf[0] = (raddr & 0xFF00)>>8;
	buf[1] = (raddr & 0x00FF);

	rc = s5k5cag_i2c_rxdata(saddr, buf, 2);
	if (rc < 0)	return rc;
	*rdata = buf[0] << 8 | buf[1];

	if (rc < 0)	CDBG("s5k5cag_i2c_read failed!\n");

	printk(KERN_ERR "-s5k5cag_i2c_read\n");
	return rc;
}


static void camera_power_onoff(u8 v)
{
#ifdef CONFIG_MACH_MSM7X25_QRD
	if (v==1)
	{
		printk(KERN_ERR "--CAMERA-- power on~!!\n");
		vreg_enable(vreg_CAM_wlan);
		msleep(1);
		vreg_enable(vreg_CAM_gp3);
		msleep(1);
		vreg_enable(vreg_CAM_gp6);
		msleep(5);
	}
	else
	{
		printk(KERN_ERR "--CAMERA-- power off~!!\n");
		vreg_disable(vreg_CAM_gp6);
		msleep(1);
		vreg_disable(vreg_CAM_gp3);
		msleep(1);
		vreg_disable(vreg_CAM_wlan);
		msleep(1);
	}
#endif

#ifdef CONFIG_MACH_MSM7X27_SKU5
	if (v==1)
	{
		printk(KERN_ERR "--CAMERA-- power on~!!\n");
		gpio_set_value(GPIO_1, GPIO_PULL_HIGH);
		msleep(1);
		vreg_enable(vreg_CAM_gp6);
		msleep(1);
		gpio_set_value(GPIO_0, GPIO_PULL_HIGH);
		msleep(5);
	}
	else
	{
		printk(KERN_ERR "--CAMERA-- power off~!!\n");
		gpio_set_value(GPIO_0, GPIO_PULL_LOW);
		msleep(1);
		vreg_disable(vreg_CAM_gp6);
		msleep(1);
		gpio_set_value(GPIO_1, GPIO_PULL_LOW);
		msleep(1);
	}
#endif

}

static int s5k5cag_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
   /*--There is do nothing in this functin--*/
   
   //u32 device_id=0;
   printk(KERN_ERR "--CAMERA-- %s (Start...)\n",__func__);
   
   // (1) set Camera PMIC and power on
   //vreg_CAM_gp3 = vreg_get(NULL, "gp3");
   //vreg_CAM_gp6 = vreg_get(NULL, "gp6");
   //vreg_CAM_wlan = vreg_get(NULL, "wlan");
   
   /* --- can Move to AMSS -------------- */
   //vreg_set_level(vreg_CAM_wlan, 2800);
   //vreg_set_level(vreg_CAM_gp3,  1500);
   //vreg_set_level(vreg_CAM_gp6,  2800);
   /* ----------------------------------- */   
 
   // (2) config pwd and rest pin
   printk(KERN_ERR "--CAMERA-- %s : sensor_pwd_pin=%d, sensor_reset_pin=%d\n",__func__,data->sensor_pwd,data->sensor_reset);
   //gpio_request(data->sensor_pwd, "s5k5cag");
   //gpio_request(data->sensor_reset, "s5k5cag");
   //gpio_direction_output(data->sensor_pwd, 0);
   //gpio_direction_output(data->sensor_reset, 0);

   // (3) Set Clock = 48 MHz
   //msm_camio_clk_rate_set(24000000);
   //mdelay(5);
   //msm_camio_camif_pad_reg_reset();
   //mdelay(5);

   // (4) Power On
   //camera_power_onoff(1);
   //gpio_set_value(data->sensor_pwd, 1);
   //mdelay(1);
   //gpio_set_value(data->sensor_reset, 1);
  // mdelay(5);

   // (5) Into Stand by mode
   //s5k5cag_i2c_write(s5k5cag_client->addr,REG_MODE_SELECT,MODE_SELECT_SW_STANDBY,10);
   
   // (6) Read Device ID
   //s5k5cag_i2c_read(s5k5cag_client->addr, 0x0156, &device_id);
   
   //printk(KERN_ERR "--CAMERA-- %s ok , device id=0x%x\n",__func__,device_id);
   return 0;
}

static int s5k5cag_setting(enum msm_s_reg_update rupdate,enum msm_s_setting rt)
{
	int rc = -EINVAL;
	//for original init setting
	//int len;
	int i = 0;
	printk(KERN_ERR "--CAMERA-- %s (Start...), rupdate=%d \n",__func__,rupdate);

	//len = sizeof(preview_s5k5cag_reg)/sizeof(preview_s5k5cag_reg[0]);
	
	switch (rupdate)
	{
		case S_UPDATE_PERIODIC:
			break; /* UPDATE_PERIODIC */
		case S_REG_INIT:
			printk(KERN_ERR "--CAMERA-- S_REG_INIT (Start)\n");
/*
			//set register setting
			for(i = 0;i<len;i++)
			{
			   rc = s5k5cag_i2c_write(s5k5cag_client->addr, \
			        preview_s5k5cag_reg[i][0], preview_s5k5cag_reg[i][1],10);
			  // udelay(500);
			}	
*/

			for(i = 0; ; i++)
			{
			    if(priview_arr[i].cnt == -1)
			    {
			        break;
			    }
			    if(priview_arr[i].cnt == -2)
			    {
			         msleep(100);
			        continue;
			    }
			    rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, priview_arr[i].data, priview_arr[i].cnt);
			} 


			/* reset fps_divider */
			s5k5cag_ctrl->fps_divider = 1 * 0x0400;
			printk(KERN_ERR "--CAMERA-- S_REG_INIT (End)\n");
			break; /* case REG_INIT: */
		default:
			break;
	} /* switch (rupdate) */

	printk(KERN_ERR "--CAMERA-- %s (End), rupdate=%d \n",__func__,rupdate);

	return rc;
}

static int s5k5cag_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int rc = -ENOMEM;
	printk(KERN_ERR "--CAMERA-- %s\n",__func__);

	s5k5cag_ctrl = kzalloc(sizeof(struct __s5k5cag_ctrl), GFP_KERNEL);
	if (!s5k5cag_ctrl)
	{
		printk(KERN_ERR "--CAMERA-- kzalloc s5k5cag_ctrl error !!\n");
		kfree(s5k5cag_ctrl);
		return rc;
	}
	s5k5cag_ctrl->fps_divider = 1 * 0x00000400;
	s5k5cag_ctrl->pict_fps_divider = 1 * 0x00000400;
	s5k5cag_ctrl->set_test = S_TEST_OFF;
	s5k5cag_ctrl->prev_res = S_QTR_SIZE;
	s5k5cag_ctrl->pict_res = S_FULL_SIZE;
	
	if (data) s5k5cag_ctrl->sensordata = data;

	/* enable mclk = 24 MHz first */
	msm_camio_clk_rate_set(24000000);
	//mdelay(20);

	msm_camio_camif_pad_reg_reset();
	//mdelay(20);

	camera_power_onoff(1);
	msleep(1);
	gpio_set_value(data->sensor_pwd, 1);
	msleep(1);
	gpio_set_value(data->sensor_reset, 1);
	msleep(1);
	
	if (s5k5cag_ctrl->prev_res == S_QTR_SIZE)
		rc = s5k5cag_setting(S_REG_INIT, S_RES_PREVIEW);
	else
		rc = s5k5cag_setting(S_REG_INIT, S_RES_CAPTURE);

	if (rc < 0)
	{
		printk(KERN_ERR "--CAMERA-- %s : s5k5cag_setting failed. rc = %d\n",__func__,rc);
		kfree(s5k5cag_ctrl);
		return rc;
	}

	printk(KERN_ERR "--CAMERA--re_init_sensor ok!!\n");
	return rc;
}

static int s5k5cag_sensor_release(void)
{
	printk(KERN_ERR "--CAMERA--s5k5cag_sensor_release!!\n");

	mutex_lock(&s5k5cag_mutex);

	camera_power_onoff(0);
	
	gpio_set_value(s5k5cag_ctrl->sensordata->sensor_reset, 0);
	
	//gpio_free(s5k5cag_ctrl->sensordata->sensor_reset);
	//gpio_free(s5k5cag_ctrl->sensordata->sensor_pwd);

	kfree(s5k5cag_ctrl);
	s5k5cag_ctrl = NULL;

	mutex_unlock(&s5k5cag_mutex);
	return 0;
}


			       
static const struct i2c_device_id s5k5cag_i2c_id[] = {
	{"s5k5cag", 0},{}
};

static int s5k5cag_i2c_remove(struct i2c_client *client)
{
   return 0;
}


static int s5k5cag_init_client(struct i2c_client *client)
{
   /* Initialize the MSM_CAMI2C Chip */
   init_waitqueue_head(&s5k5cag_wait_queue);
   return 0;
}
/*
static int s5k5cag_raw_snapshot_config(int mode)
{
	int rc;
	rc = s5k5cag_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
	if (rc < 0)	return rc;

	s5k5cag_ctrl->curr_res = s5k5cag_ctrl->pict_res;
	s5k5cag_ctrl->sensormode = mode;
	return rc;
}
*/
static int s5k5cag_set_sensor_mode(int mode, int res)
{
	int rc = -EINVAL;
	int len2;
	int len3;
	int i =0;
	int32_t tmp2;

	printk(KERN_ERR "--CAMERA-- s5k5cag_set_sensor_mode mode = %d, res = %d\n", mode, res);

	len2 = sizeof(capture_tbl_1)/sizeof(capture_tbl_1[0]);

	len3 = sizeof(preview_tbl_1)/sizeof(preview_tbl_1[0]);
	
	switch (mode)
	{
		case SENSOR_PREVIEW_MODE:
			printk(KERN_ERR "--CAMERA-- SENSOR_PREVIEW_MODE\n");
	      		if(is_autoflash == 1)
	              {
	                  pmic_flash_led_set_current(0);
	              }	
			
			for(i = 0;i<len3;i++)
			{
					rc = s5k5cag_i2c_write(s5k5cag_client->addr,
					     preview_tbl_1[i][0],
					     preview_tbl_1[i][1],
					     10);
					if (rc < 0)	return rc;     
			}
			break;
		case SENSOR_SNAPSHOT_MODE:
			printk(KERN_ERR "--CAMERA-- SENSOR_SNAPSHOT_MODE\n");

    	    if(is_autoflash == 1)
			{
    	        //s5k5cag_i2c_write(s5k5cag_client->addr,0x002c,0x7000,10);
   			    //s5k5cag_i2c_write(s5k5cag_client->addr,0x002e ,0x124E,10);
   			    //s5k5cag_i2c_read(s5k5cag_client->addr, 0x0f12, &tmp);

			    s5k5cag_i2c_write(s5k5cag_client->addr,0x002c,0x7000,10);
   			    s5k5cag_i2c_write(s5k5cag_client->addr,0x002e ,0x2448 ,10);
   			    s5k5cag_i2c_read(s5k5cag_client->addr, 0x0f12, &tmp2);
				
    	         //printk(KERN_ERR "--CAMERA-- Threshold : %x %d\n",tmp,tmp);
			     printk(KERN_ERR "--CAMERA-- NormBr : %x %d\n",tmp2,tmp2);
    	                  if(tmp2 >= 200)
    	                  {
    	                      pmic_flash_led_set_current(0);
    	                  }else{
    	                      pmic_flash_led_set_current(240);
    	                  }
    	    }
						
				for(i = 0;i<len2;i++)
				{
					rc = s5k5cag_i2c_write(s5k5cag_client->addr,
					     		capture_tbl_1[i][0],
					     		capture_tbl_1[i][1],
					     		10);
					if (rc < 0)	return rc;     
				}
			
			break;
		case SENSOR_RAW_SNAPSHOT_MODE:
			printk(KERN_ERR "--CAMERA-- SENSOR_RAW_SNAPSHOT_MODE\n");
			//rc = s5k5cag_raw_snapshot_config(mode);
			break;
		default:
			printk(KERN_ERR "--CAMERA--s5k5cag_set_sensor_mode  DEFAULT\n");
			break;
	}
	return rc;
}

static int s5k5cag_set_wb_oem(uint8_t param)
{
	printk(KERN_ERR "s5k5cag_set_wb_oem %d\r\n",param);

	s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
	switch (param)
	{
		case CAMERA_WB_AUTO:
			printk(KERN_ERR "CAMERA_WB_AUTO %d\r\n",param);
			//R Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04D2,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x067F,10);
			break;
		case CAMERA_WB_CLOUDY_DAYLIGHT: //4500K
			printk(KERN_ERR "CAMERA_WB_CLOUDY_DAYLIGHT %d\r\n",param);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04D2,10);		
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0677,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A0,10); 	//Rgain 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0128,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A4,10);		//G Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0100,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A8,10);		//B Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x01AB,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);			
			break;
		case CAMERA_WB_DAYLIGHT://D65
			printk(KERN_ERR "CAMERA_WB_DAYLIGHT %d\r\n",param);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04D2,10);		//R Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0677,10); 

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A0,10); 	//Rgain 1.42
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0166,10);   
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A4,10);		//G Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0100,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A8,10);		//B Gain 1.27
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0145,10);   
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);
			break;
		case CAMERA_WB_INCANDESCENT: //TL84
			printk(KERN_ERR "CAMERA_WB_INCANDESCENT %d\r\n",param);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04D2,10);		
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0677,10); 

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A0,10); 		//Rgain1.16
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0126,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A4,10);		//G Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0100,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A8,10);		//B Gain 1.74
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x01C5,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);
			break;		
		case CAMERA_WB_FLUORESCENT://CWF
			printk(KERN_ERR "CAMERA_WB_FLUORESCENT %d\r\n",param);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04D2,10);		
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0677,10); 

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A0,10); 		//Rgain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0145,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A4,10);		//G Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0100,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A8,10);		//B Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x01EB,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);
			break;		
		/*case CAMERA_WB_TUNGSTEN://Alight
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04D2,10);		
			s5k5cag_i2c_write(0x0F12, 0x0677,10); 

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A0,10); 		//Rgain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0100,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A4,10);		//G Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0100,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04A8,10);		//B Gain
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x021E,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);
			break;*/
/*		case CAMERA_WB_MANUAL:
		    // TODO
			break;		*/
		default:
			return 0;			
	}	

	return 1;

}

static int s5k5cag_antibanding(int para)
{    
    int32_t tmp;	
    int tmp2;

    printk(KERN_ERR "s5k5cag_antibanding %d\r\n",para);

    s5k5cag_i2c_write(s5k5cag_client->addr,0x002c,0x7000,10);
    s5k5cag_i2c_write(s5k5cag_client->addr,0x002e ,0x04D2,10);
    s5k5cag_i2c_read(s5k5cag_client->addr, 0x0f12, &tmp);
    
    printk(KERN_ERR "s5k5cag_antibanding tmp = %d %x\r\n", tmp, tmp);
    
    tmp2 = tmp & 0x000F;
    
    printk(KERN_ERR "s5k5cag_antibanding tmp = %d %x\r\n", tmp2, tmp2);    
	
	switch (para)
	{
		case CAMERA_ANTIBANDING_50HZ:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04D2,10);
			if(tmp2 == 0xF)
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x065F,10);
			else
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0657,10);
			
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04BA,10);		//50
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04BC,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10); 
			break;
		case CAMERA_ANTIBANDING_60HZ:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04D2,10);
			if(tmp2 == 0xF)
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x065F,10);
			else
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0657,10);

			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04BA,10);		//60
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0002,10); 
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x04BC,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10); 
			break;
		default:
			return 0;
	}
	return 0;

}

static long s5k5cag_set_effect(int mode,int para)
{
	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		/* Context A Special Effects */
		printk(KERN_ERR "--CAMERA-- %s ...SENSOR_PREVIEW_MODE\n",__func__);
		break;

	case SENSOR_SNAPSHOT_MODE:
		/* Context B Special Effects */
		printk(KERN_ERR "--CAMERA-- %s ...SENSOR_SNAPSHOT_MODE\n",__func__);
		break;

	default:
		break;
	}
    printk(KERN_ERR "s5k5cag_set_effect %d\r\n",para);
    s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);	
	switch (para)
	{	
		case CAMERA_EFFECT_OFF:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x021E,10);	
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);  				   
			break;		
		case CAMERA_EFFECT_MONO:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x021E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0001,10);  
			break;		
		case CAMERA_EFFECT_SEPIA:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x021E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0004,10);    
			break;		
		case CAMERA_EFFECT_NEGATIVE:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x021E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0003,10);  
			break;
	//unused function
	/*	case CAM_EFFECT_ENC_SEPIAGREEN:
			break;					
		case CAM_EFFECT_ENC_SEPIABLUE:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x021E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0004,10);  				   
			break;								
		case CAM_EFFECT_ENC_GRAYINV:
			
			break;								
		case CAM_EFFECT_ENC_COPPERCARVING:
	
			break;								
		case CAM_EFFECT_ENC_BLUECARVING:
	    	
			break;								
		case CAM_EFFECT_ENC_CONTRAST:

			break;						
		case CAM_EFFECT_ENC_SKETCH:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x021E,10);	
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0005,10);  
			break;
		case CAM_EFFECT_ENC_EMBOSSMENT:	
		case CAM_EFFECT_ENC_BLACKBOARD:
		case CAM_EFFECT_ENC_WHITEBOARD:
		case CAM_EFFECT_ENC_JEAN:
		case CAM_EFFECT_ENC_OIL:			*/
		default:
			printk(KERN_ERR "--CAMERA--unsupport effect %d\r\n",para);
	}
	return 0;
}

static int s5k5cag_set_saturation(int8_t para)
{

	//printk(KERN_ERR " ****** filanto s5k5cag_set_saturation");

    printk(KERN_ERR "s5k5cag_set_saturation %d\r\n",para);
	//s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
	switch (para)
	{
	   case CAMERA_SATURATION_LV0:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV0\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFF9B,10);
		break;
	   
	   case CAMERA_SATURATION_LV1:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV1\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFFB4,10);
		break;
	   
	   case CAMERA_SATURATION_LV2:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV2\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFFCD,10);
		break;
	   
	   case CAMERA_SATURATION_LV3:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV3\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFFE6,10);
		break;
	   
	   case CAMERA_SATURATION_LV4:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV4\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
		break;
	   
	   case CAMERA_SATURATION_LV5:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV5\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0019,10);
		break;
	   
	   case CAMERA_SATURATION_LV6:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV6\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0032,10);
		break;
	   
	   case CAMERA_SATURATION_LV7:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV7\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x004B,10);
		break;
	   
	   case CAMERA_SATURATION_LV8:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SATURATION_LV8\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0210,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0064,10);
       default:
       return 0;
	}

    return 0;

}
static int s5k5cag_set_sharpness(int8_t para)
{

	//printk(KERN_ERR " ****** filanto s5k5cag_set_sharpness");
  
    printk(KERN_ERR "s5k5cag_set_sharpness %d\r\n",para);
	//s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
	switch (para)
	{
	   case CAMERA_SHARPNESS_LV0:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV0\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFF9B,10);
		break;
	   
	   case CAMERA_SHARPNESS_LV1:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV1\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFFB4,10);
		break;
	   
	   case CAMERA_SHARPNESS_LV2:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV2\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFFE6,10);
		break;
	   
	   case CAMERA_SHARPNESS_LV3:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV3\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
		break;
	   
	   case CAMERA_SHARPNESS_LV4:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV4\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0019,10);
		break;
	   
	   case CAMERA_SHARPNESS_LV5:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV5\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0032,10);
		break;
	   
	   case CAMERA_SHARPNESS_LV6:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV6\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x004B,10);
		break;
	   
	   case CAMERA_SHARPNESS_LV7:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV7\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0064,10);
		break;
	   
	   case CAMERA_SHARPNESS_LV8:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SHARPNESS_LV8\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x0212,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x007D,10);
       default:
       return 0;
	}

  return 0;
}
static int s5k5cag_set_contrast(int8_t para)
{

	//printk(KERN_ERR " ****** filanto s5k5cag_contrast");
  
    printk(KERN_ERR "s5k5cag_set_contrast %d\r\n",para);
	//s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
	
	switch (para)
	{
	   case CAMERA_CONTRAST_LV0:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV0\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
		    s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFF9B,10);
		break;
	   
	   case CAMERA_CONTRAST_LV1:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV1\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFFB4,10);
		break;
	   
	   case CAMERA_CONTRAST_LV2:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV2\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFFCD,10);
		break;
	   
	   case CAMERA_CONTRAST_LV3:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV3\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0xFFE6,10);
		break;
	   
	   case CAMERA_CONTRAST_LV4:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV4\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
		break;
	   
	   case CAMERA_CONTRAST_LV5:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV5\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0019,10);
		break;
	   
	   case CAMERA_CONTRAST_LV6:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV6\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0032,10);
		break;
	   
	   case CAMERA_CONTRAST_LV7:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV7\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x004B,10);
		break;
	   
	   case CAMERA_CONTRAST_LV8:
	   #ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_CONTRAST_LV8\n");
		#endif
		    s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020E,10);
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0064,10);
		break;
       default:
       return 0; 		
	}

	return 0;

}

static int s5k5cag_set_exposure_compensation(int8_t para)
{
    printk(KERN_ERR "s5k5cag_set_exposure_compensation %d\r\n",para);
	s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
	switch (para)
	{
		case CAMERA_EXPOSURE_COMPENSATION_LV4:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D-(4*12),10);  					
			break;		
	/*	case CAM_EV_NEG_3_3:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D-(3*12),10);  				   
			break;				*/
		case CAMERA_EXPOSURE_COMPENSATION_LV3:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D-(2*12),10);  				   
			break;				
	/*	case CAM_EV_NEG_1_3:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D-(1*12),10);  				   
			break;				*/
		case CAMERA_EXPOSURE_COMPENSATION_LV2:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D,10);       		 
			break;				
/*		case CAM_EV_POS_1_3:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D+(1*14),10);  				
			break;				*/
		case CAMERA_EXPOSURE_COMPENSATION_LV1:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D+(2*14),10);  				   
			break;				
/*		case CAM_EV_POS_3_3:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D+(3*14),10);  					
			break;*/
		case CAMERA_EXPOSURE_COMPENSATION_LV0:
			s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0x020C,10);	//#TVAR_ae_BrAve //ae target
			s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x003D+(4*14),10);  				   
			break;
		default:
			return 0;
	}	
	
	return 0;	
}

static long s5k5cag_set_exposure_mode(int mode)
{
	long rc = 0;
    #ifdef CAM_DBG
    printk(KERN_ERR "--CAMERA-- %s ...(Start)\n",__func__);
    #endif
    #ifdef CAM_DBG
    printk(KERN_ERR "--CAMERA-- %s ...mode = %d\n",__func__ , mode);
    #endif

	switch (mode)
	{
		    case CAMERA_SETAE_AVERAGE:

				#ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SETAE_AVERAGE\n");
				#endif
				s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0X0F7E,10);	 //ae weight  //
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);		
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);	
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);	
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);

				break;
		    case CAMERA_SETAE_CENWEIGHT:
				#ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SETAE_CENWEIGHT\n");
				#endif
				s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0X0F7E,10);	 //ae weight  // 
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);	
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);	
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0201,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0102,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0202,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0202,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0202,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0202,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0201,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0202,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0202,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0102,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0201,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0202,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0202,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0102,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);

				break;
			case CAMERA_SETAE_SPOTMETERING:
				#ifdef CAM_DBG
				printk(KERN_ERR "--CAMERA--CAMERA_SETAE_SPOTMETERING\n");
				#endif
				s5k5cag_i2c_write(s5k5cag_client->addr,0xFCFC, 0xD000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0028, 0x7000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x002A, 0X0F7E,10);	 //ae weight  //
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0f01,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x010f,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0f01,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x010f,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0101,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);
				s5k5cag_i2c_write(s5k5cag_client->addr,0x0F12, 0x0000,10);

		    	break;
		    default:
				printk(KERN_ERR "--CAMERA--ERROR COMMAND OR NOT SUPPORT\n");
				break;
  	}

	#ifdef CAM_DBG
    printk(KERN_ERR "--CAMERA-- %s ...(End)\n",__func__);
	#endif
	return rc;
}

static int s5k5cag_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	long  rc = 0;
	if (copy_from_user(&cdata,(void *)argp,sizeof(struct sensor_cfg_data))) 
		return -EFAULT;

	printk(KERN_ERR "--CAMERA-- %s %d\n",__func__,cdata.cfgtype);
		
	mutex_lock(&s5k5cag_mutex);
	switch (cdata.cfgtype)
	{
		case CFG_SET_MODE:   // 0
			rc =s5k5cag_set_sensor_mode(cdata.mode, cdata.rs);
			break;
		case CFG_SET_EFFECT: // 1
    		printk(KERN_ERR "--CAMERA-- CFG_SET_EFFECT mode=%d, effect = %d !!\n",cdata.mode, cdata.cfg.effect);
			rc = s5k5cag_set_effect(cdata.mode, cdata.cfg.effect);
			break;
		case CFG_START:      // 2
			printk(KERN_ERR "--CAMERA-- CFG_START (Not Support) !!\n");
			// Not Support
			break;
		case CFG_PWR_UP:     // 3
			printk(KERN_ERR "--CAMERA-- CFG_PWR_UP (Not Support) !!\n");
			// Not Support
			break;
		case CFG_PWR_DOWN:   // 4
			printk(KERN_ERR "--CAMERA-- CFG_PWR_DOWN (Not Support) \n");
			//camera_power_onoff(0);
			break;
		case CFG_SET_DEFAULT_FOCUS:  // 06
			printk(KERN_ERR "--CAMERA-- CFG_SET_DEFAULT_FOCUS (Not Implement) !!\n");
			break;		
		case CFG_MOVE_FOCUS:     //  07
			printk(KERN_ERR "--CAMERA-- CFG_MOVE_FOCUS (Not Implement) !!\n");
			break;
        case CFG_SET_WB:
     		printk(KERN_ERR "--CAMERA-- CFG_SET_WB!!\n");
            s5k5cag_set_wb_oem(cdata.cfg.wb_val);
            rc = 0 ;
            break;
        case CFG_SET_ANTIBANDING:     //  17
     		printk(KERN_ERR "--CAMERA-- CFG_SET_ANTIBANDING antibanding = %d!!\n", cdata.cfg.antibanding);
     		rc = s5k5cag_antibanding(cdata.cfg.antibanding);
     		break;
        case CFG_SET_EXPOSURE_COMPENSATION:     //  12
     		printk(KERN_ERR "--CAMERA-- CFG_SET_EXPOSURE_COMPENSATION  !!\n");
     		rc = s5k5cag_set_exposure_compensation(cdata.cfg.exp_compensation);
     		break;
			
	   case CFG_SET_CONTRAST:     //  13
     		printk(KERN_ERR "--CAMERA-- CFG_SET_CONTRAST  !!\n");
     		rc = s5k5cag_set_contrast(cdata.cfg.contrast);
     		break;
        case CFG_SET_SHARPNESS:     //31
     		printk(KERN_ERR "--CAMERA-- CFG_SET_SHARPNESS  !!\n");
     		rc = s5k5cag_set_sharpness(cdata.cfg.sharpness);
     		break;
        case CFG_SET_SATURATION:     //30
     		printk(KERN_ERR "--CAMERA-- CFG_SET_SATURATION  !!\n");
     		rc = s5k5cag_set_saturation(cdata.cfg.saturation);
     		break;
		case CFG_SET_EXPOSURE_MODE:     //  15
			#ifdef CAM_DBG
			printk(KERN_ERR "--CAMERA-- CFG_SET_EXPOSURE_MODE !!\n");
			#endif
			rc = s5k5cag_set_exposure_mode(cdata.cfg.ae_mode);
			break;
        case CFG_SET_AUTOFLASH:
            printk(KERN_ERR "--CAMERA-- CFG_SET_AUTO_FLASH !\n");
            is_autoflash = cdata.cfg.is_autoflash;
            printk(KERN_ERR "[kylin] is autoflash %d\r\n",is_autoflash);
            rc = 0;
            break;
        default:
     		printk(KERN_ERR "--CAMERA-- %s: Command=%d (Not Implement)!!\n",__func__,cdata.cfgtype);
     		rc = -EINVAL;
     		break;	
	}
	mutex_unlock(&s5k5cag_mutex);
	return rc;	
}

static struct i2c_driver s5k5cag_i2c_driver = {
	.id_table = s5k5cag_i2c_id,
	.probe  = s5k5cag_i2c_probe,
	.remove = s5k5cag_i2c_remove,
	.driver = {
		.name = "s5k5cag",
	},
};


static int s5k5cag_sensor_probe(const struct msm_camera_sensor_info *info,struct msm_sensor_ctrl *s)
{
  int rc = -ENOTSUPP;

  printk(KERN_ERR "--CAMERA-- %s (Start...)\n",__func__);
  rc = i2c_add_driver(&s5k5cag_i2c_driver);
  if ((rc < 0 ) || (s5k5cag_client == NULL))
  {
  	printk(KERN_ERR "--CAMERA-- i2c_add_driver FAILS!!\n");
   	return rc;
  }

  rc = s5k5cag_probe_init_sensor(info);
  if (rc < 0)
  {
  	printk(KERN_ERR "--CAMERA--s5k5cag_probe_init_sensor Fail !!~~~~!!\n");
  	return rc;
  }
  s->s_init = s5k5cag_sensor_open_init;
  s->s_release = s5k5cag_sensor_release;
  s->s_config  = s5k5cag_sensor_config;
  s->s_camera_type = BACK_CAMERA_2D;
  s->s_mount_angle  = 90;
  
  printk(KERN_ERR "--CAMERA-- %s (End...)\n",__func__);
  return rc;
}

static void power_off(void)
{
   printk(KERN_ERR "--CAMERA-- %s ... (Start...)\n",__func__);

   camera_power_onoff(0);
   gpio_set_value(23, 0);
   gpio_set_value(89, 0);	

   printk(KERN_ERR "--CAMERA-- %s ... (End...)\n",__func__);
}

static int s5k5cag_i2c_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
    u32 device_id=0;

   printk(KERN_ERR "--CAMERA-- %s ... (Start...)\n",__func__);
   
   if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
   {
      printk(KERN_ERR "--CAMERA--i2c_check_functionality failed\n");
      return -ENOMEM;
   }
   
   s5k5cag_sensorw = kzalloc(sizeof(struct s5k5cag_work), GFP_KERNEL);
   if (!s5k5cag_sensorw)
   {
      printk(KERN_ERR "--CAMERA--kzalloc failed\n");
      return -ENOMEM;
   }
   
   i2c_set_clientdata(client, s5k5cag_sensorw);
   
   s5k5cag_init_client(client);
   s5k5cag_client = client;

   //power setting
   #ifdef CONFIG_MACH_MSM7X25_QRD
   vreg_CAM_gp3 = vreg_get(NULL, "gp3");
   vreg_CAM_gp6 = vreg_get(NULL, "gp6");
   vreg_CAM_wlan = vreg_get(NULL, "wlan");
   #endif

   #ifdef CONFIG_MACH_MSM7X27_SKU5
   vreg_CAM_gp6 = vreg_get(NULL, "gp6");
   vreg_set_level(vreg_CAM_gp6,  2850);
   #endif

   // (2) config pwd and rest pin
   //printk(KERN_ERR "--CAMERA-- %s : sensor_pwd_pin=%d, sensor_reset_pin=%d\n",__func__,data->sensor_pwd,data->sensor_reset);
   gpio_request(23, "s5k5cag");	//GPIO23 <- power down pin
   gpio_request(89, "s5k5cag");	//GPIO89 <-reset pin
   gpio_direction_output(23, 0);	//disable power down pin
   gpio_direction_output(89, 0);	//disable reset pin

   // (3) Set Clock = 24 MHz
   msm_camio_clk_rate_set(24000000);
   //mdelay(5);
   //msm_camio_camif_pad_reg_reset();
   //mdelay(5);

   // (4) Power On
   camera_power_onoff(1);
   gpio_set_value(23, 1);	//enable power down pin 
   msleep(1);
   gpio_set_value(89, 1);	//reset camera reset pin
   msleep(5);
   
   // (6) Read Device ID, s5k5cag chip id register address 0x00000040.
   s5k5cag_i2c_write(s5k5cag_client->addr,0x002c,0x0000,10);
   s5k5cag_i2c_write(s5k5cag_client->addr,0x002e,0x0040,10);
   s5k5cag_i2c_read(s5k5cag_client->addr, 0x0f12, &device_id);
   //s5k5cag chip id is 0x05ca
   if(device_id != 0x05ca)
   {
	printk(KERN_ERR "--CAMERA-- %s ok , device id error\r\n",__func__);
	power_off();
	return -1;
   }
   else printk(KERN_INFO "--CAMERA-- %s ok , device id=0x%x\n",__func__,device_id);

   power_off();
  
   printk("--CAMERA-- %s ... (End...)\n",__func__);
   
   return 0;
}

static int __s5k5cag_probe(struct platform_device *pdev)
{
   return msm_camera_drv_start(pdev, s5k5cag_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
    .probe = __s5k5cag_probe,
    .driver = {
        .name = "msm_camera_s5k5cag",
        .owner = THIS_MODULE,
    },
};

static int __init s5k5cag_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

module_init(s5k5cag_init);
