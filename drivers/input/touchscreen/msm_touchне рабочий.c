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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/io.h>
#include "mach/../../cci_smem.h"
#include "mach/../../smd_private.h"
#include <mach/msm_touch.h>
#include <linux/irq.h>
#include <asm/gpio.h>
#include <mach/gpio.h>
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
#include <linux/wakelock.h>
#include <linux/leds.h>
#include <linux/msm_rpcrouter.h>
#include <mach/msm_rpcrouter.h> 
#include <linux/proc_fs.h>
//systronix
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include "../../staging/android/logger.h"

#define ENABLE_DEBUG 0

#if (ENABLE_DEBUG == 1)
	#define DEBUG(format,args...) 	{ \
				printk(KERN_INFO "@[msm_touch.c][%d] %s(): " format, __LINE__, __func__, ##args); \
			}
	#define DEBUG3(format,args...) 	{}
#elif (ENABLE_DEBUG == 2)
	#define DEBUG(format,args...) 	{ \
				printk(KERN_INFO "@[msm_touch.c]" format, ##args); \
			}
	#define DEBUG3(format,args...)	{}
#elif (ENABLE_DEBUG == 3)
	#define DEBUG(format,args...)		{}
	#define DEBUG3(format,args...) 	{ \
				printk(KERN_INFO "" format, ##args); \
			}
#else
	#define DEBUG(format,args...)		{}
	#define DEBUG3(format,args...)	{}
#endif

enum {
	MSM_TS_DEBUG_INTERRUPT = 1U << 0,
};

static int debug_mask = 0;
module_param_named(
	debug_mask, debug_mask,
	int, S_IRUGO | S_IWUSR | S_IWGRP
);

#if 0
#define	FUNC_ENTER	printk("@[msm_touch][%d] %s:+\r\n", __LINE__, __func__)
#define	FUNC_LEAVE	printk("@[msm_touch][%d] %s:-\r\n", __LINE__, __func__)
#else
#define	FUNC_ENTER	{}
#define	FUNC_LEAVE	{}
#endif

/* HW register map */
#define TSSC_CTL_REG      0x100
#define TSSC_OPN_REG      0x104
#define TSSC_SI_REG       0x108
#define TSSC_STATUS_REG   0x10C
#define TSSC_AVG12_REG    0x110
#define TSSC_AVG34_REG    0x114
#define TSSC_SAMPLE_1_1_REG    0x118
#define TSSC_SAMPLE_1_2_REG    0x11C
#define TSSC_SAMPLE_1_3_REG    0x120
#define TSSC_SAMPLE_1_4_REG    0x124
#define TSSC_SAMPLE_1_5_REG    0x128
#define TSSC_SAMPLE_1_6_REG    0x12C
#define TSSC_SAMPLE_1_7_REG    0x130
#define TSSC_SAMPLE_1_8_REG    0x134
#define TSSC_SAMPLE_2_1_REG    0x138
#define TSSC_SAMPLE_2_2_REG    0x13C
#define TSSC_SAMPLE_2_3_REG    0x140
#define TSSC_SAMPLE_2_4_REG    0x144
#define TSSC_SAMPLE_2_5_REG    0x148
#define TSSC_SAMPLE_2_6_REG    0x14C
#define TSSC_SAMPLE_2_7_REG    0x150
#define TSSC_SAMPLE_2_8_REG    0x154

/* status bits */
#define TSSC_STS_OPN_SHIFT 0x6
#define TSSC_STS_OPN_BMSK  0x1C0
#define TSSC_STS_NUMSAMP_SHFT 0x1
#define TSSC_STS_NUMSAMP_BMSK 0x3E
#define TSSC_STS_ERROR_BMSK 0x1

/* CTL bits */
#define TSSC_CTL_EN		(0x1 << 0)
#define TSSC_CTL_SW_RESET	(0x1 << 2)
#define TSSC_CTL_MASTER_MODE	(0x3 << 3)
#define TSSC_CTL_AVG_EN		(0x1 << 5)
#define TSSC_CTL_DEB_EN		(0x1 << 6)
#define TSSC_CTL_DEB_12_MS	(0x2 << 7)	/* 1.2 ms */
#define TSSC_CTL_DEB_16_MS	(0x3 << 7)	/* 1.6 ms */
#define TSSC_CTL_DEB_2_MS	(0x4 << 7)	/* 2 ms */
#define TSSC_CTL_DEB_3_MS	(0x5 << 7)	/* 3 ms */
#define TSSC_CTL_DEB_4_MS	(0x6 << 7)	/* 4 ms */
#define TSSC_CTL_DEB_6_MS	(0x7 << 7)	/* 6 ms */
#define TSSC_CTL_INTR_FLAG1	(0x1 << 10)
#define TSSC_CTL_DATA		(0x1 << 11)
#define TSSC_CTL_INTR_FLAG2	(0x1 << 12)
#define TSSC_CTL_SSBI_CTRL_EN	(0x1 << 13)

#define X_MAX_K4	820
#define X_MIN_K4	170
#define Y_MAX_K4	910
#define Y_MIN_K4	100
#define X_MAX_PENGLITE	772
#define X_MIN_PENGLITE	254
#define Y_MAX_PENGLITE	845
#define Y_MIN_PENGLITE	253
/*#define X_MAX_PENGLITE	778
#define X_MIN_PENGLITE	260
#define Y_MAX_PENGLITE	847
#define Y_MIN_PENGLITE	238*/
#define X_MAX_K4_DVT1	780
#define X_MIN_K4_DVT1	240
#define Y_MAX_K4_DVT1	846
#define Y_MIN_K4_DVT1	258
#define X_MAX_K5	977
#define X_MIN_K5	55
#define Y_MAX_K5	937
#define Y_MIN_K5	73
#define X_MAX_K5_DVT1_2	847
#define X_MIN_K5_DVT1_2	170
#define Y_MAX_K5_DVT1_2	747
#define Y_MIN_K5_DVT1_2	266

#define P_MAX	256
#define TSSC_REG(reg) (virt + TSSC_##reg##_REG)

/* OPN bits */
#define	TOUCH_RESOLUTION_12_BIT 0
#define TSSC_OPN_8_BIT_RESOLUTION1	(0x0 << 0)
#define TSSC_OPN_10_BIT_RESOLUTION1	(0x1 << 0)
#define TSSC_OPN_12_BIT_RESOLUTION1	(0x2 << 0)
#define TSSC_OPN_8_BIT_RESOLUTION2	(0x0 << 2)
#define TSSC_OPN_10_BIT_RESOLUTION2	(0x1 << 2)
#define TSSC_OPN_12_BIT_RESOLUTION2	(0x2 << 2)
#define TSSC_OPN_8_BIT_RESOLUTION3	(0x0 << 4)
#define TSSC_OPN_10_BIT_RESOLUTION3	(0x1 << 4)
#define TSSC_OPN_12_BIT_RESOLUTION3	(0x2 << 4)
#define TSSC_OPN_8_BIT_RESOLUTION4	(0x0 << 6)
#define TSSC_OPN_10_BIT_RESOLUTION4	(0x1 << 6)
#define TSSC_OPN_12_BIT_RESOLUTION4	(0x2 << 6)
#define TSSC_OPN_1_NUM_SAMPLE1	(0x0 << 8)
#define TSSC_OPN_4_NUM_SAMPLE1	(0x1 << 8)
#define TSSC_OPN_8_NUM_SAMPLE1	(0x2 << 8)
#define TSSC_OPN_16_NUM_SAMPLE1	(0x3 << 8)
#define TSSC_OPN_1_NUM_SAMPLE2	(0x0 << 10)
#define TSSC_OPN_4_NUM_SAMPLE2	(0x1 << 10)
#define TSSC_OPN_8_NUM_SAMPLE2	(0x2 << 10)
#define TSSC_OPN_16_NUM_SAMPLE2	(0x3 << 10)
#define TSSC_OPN_1_NUM_SAMPLE3	(0x0 << 12)
#define TSSC_OPN_4_NUM_SAMPLE3	(0x1 << 12)
#define TSSC_OPN_8_NUM_SAMPLE3	(0x2 << 12)
#define TSSC_OPN_16_NUM_SAMPLE3	(0x3 << 12)
#define TSSC_OPN_1_NUM_SAMPLE4	(0x0 << 14)
#define TSSC_OPN_4_NUM_SAMPLE4	(0x1 << 14)
#define TSSC_OPN_8_NUM_SAMPLE4	(0x2 << 14)
#define TSSC_OPN_16_NUM_SAMPLE4	(0x3 << 14)
#define TSSC_OPN_OPERATION1	(0x1 << 16)
#define TSSC_OPN_OPERATION2	(0x2 << 20)
#define TSSC_OPN_OPERATION3	(0x3 << 24)
#define TSSC_OPN_OPERATION4	(0x4 << 28)

#define TSSC_OPN_RESOLUTION1_BMSK (0x3 << 0)
#define TSSC_OPN_RESOLUTION2_BMSK (0x3 << 2)
#define TSSC_OPN_RESOLUTION3_BMSK (0x3 << 4)
#define TSSC_OPN_RESOLUTION4_BMSK (0x3 << 6)

#define	TS_TSSC_WORK_FUNC_MASK		(0x0000FFFF)
#define	TS_TSSC_WORK_FUNC_SHIFT		(0)
#define	TS_UPDATE_PEN_STATE_MASK	(0x000F0000)
#define	TS_UPDATE_PEN_STATE_SHIFT	(16)
#define	TS_TIMER_MASK							(0x00F00000)
#define	TS_TIMER_SHIFT						(20)

/* Sample rate in Hz */
#define TSSC_SAMPLE_RATE 800//350

/* control reg's default state */
#define TSSC_CTL_STATE	  ( \
		TSSC_CTL_DEB_6_MS | \
		TSSC_CTL_DEB_EN | \
		TSSC_CTL_AVG_EN | \
		TSSC_CTL_MASTER_MODE | \
		TSSC_CTL_EN)

#if TOUCH_RESOLUTION_12_BIT
#define	TSSC_OPN_STATE	( \
		TSSC_OPN_12_BIT_RESOLUTION1 | \
		TSSC_OPN_12_BIT_RESOLUTION2 | \
		TSSC_OPN_16_NUM_SAMPLE1	| \
		TSSC_OPN_16_NUM_SAMPLE2	| \
		TSSC_OPN_OPERATION1			| \
		TSSC_OPN_OPERATION2)
#else// TOUCH_RESOLUTION_10_BIT
#define	TSSC_OPN_STATE	( \
		TSSC_OPN_10_BIT_RESOLUTION1 | \
		TSSC_OPN_10_BIT_RESOLUTION2 | \
		TSSC_OPN_10_BIT_RESOLUTION3 | \
		TSSC_OPN_10_BIT_RESOLUTION4 | \
		TSSC_OPN_16_NUM_SAMPLE1	| \
		TSSC_OPN_16_NUM_SAMPLE2	| \
		TSSC_OPN_4_NUM_SAMPLE3	| \
		TSSC_OPN_4_NUM_SAMPLE4	| \
		TSSC_OPN_OPERATION1			| \
		TSSC_OPN_OPERATION2			| \
		TSSC_OPN_OPERATION3			| \
		TSSC_OPN_OPERATION4)
#endif

#define	TSSC_SI_STATE	(1000 / TSSC_SAMPLE_RATE)

#define TSSC_NUMBER_OF_OPERATIONS 4
#define TS_PENUP_TIMEOUT_MS 120
#define TS_ERROR_MONITOR_TIMEOUT_S 5
#define TS_SAMPLE_POINT_TOLERANCE 100

#define TS_DRIVER_NAME "msm_touchscreen"
#define	MAX_CHANNELS			32
struct ts {
	struct input_dev *input;
	struct timer_list timer;
	struct timer_list error_monitor;
	int irq;
//	atomic_t irq_disable;	
	unsigned int x_max;
	unsigned int y_max;
	unsigned int x_min;
	unsigned int y_min;
	struct work_struct work;
//	struct work_struct  ts_event_work;
	spinlock_t	lock;
};
/*
struct sitronix_ts_priv {
	struct i2c_client *client;
	struct input_dev *input;
	struct work_struct work;
	struct mutex mutex;
	int irq;
	bool isp_enabled;
	bool autotune_result;
	bool always_update;
	char I2C_Offset;
	char I2C_Length;
	char I2C_RepeatTime;

};*/
struct st1232_ts_data {
	//uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input;	
	struct work_struct work;	
	struct input_dev *input_dev;
	int irq;
	atomic_t irq_disable;
	struct hrtimer timer;
	struct timer_list proximity_timer;
	struct work_struct  ts_event_work;
//	struct early_suspend early_suspend;
       int prev_touches;
	 //sitronix  
	struct mutex mutex;
	bool isp_enabled;
	bool autotune_result;
	bool always_update;
	char I2C_Offset;
	char I2C_Length;
	char I2C_RepeatTime;
	bool Can_update;
	int  fw_revision;
	int  struct_version;	
	int  data_mode;	
};

static void __iomem *virt;
#define TSSC_REG(reg) (virt + TSSC_##reg##_REG)
#define	MAX_CHANNELS			32
#define CONFIG_PARAM_MAX_CHANNELS	32
#define MUTUAL_CALIBRATION_BASE_SIZE	256
#define X_MAX	2976 //320
#define Y_MAX	3250 //480
#define X_MIN	1094 //0
#define Y_MIN	812  //0
 #define OOT_MAX	3
#define	PRINT_COUNTER						0
#define	PRINT_X_Y_VALUE					0
#define	REPORT_POINT_LOCATION		0
#define	X_Y_AXIS_DECREASE				1
#define	X_Y_AXIS_QUEUE					0
#define	FEATURE_TSSC_D1_FILTER	1
#define	TCXO_SHUTDOWN_WORKAROUND		0
static DEFINE_SPINLOCK(acct_lock);
static DEFINE_SPINLOCK(acct_lock1);

struct input_dev *kpdev;
struct ts *ts_point;
unsigned long timecounter[2],diff_time; 
unsigned long int en_irq=0,dis_irq=0;
static unsigned short report_value = 200;
static bool finger_move=0,CanUpdate=0,burnFw=0,release_press_flag=0;
static int last_key=0;
static int x[3],y[3],index=0,index_jiffies=0;
int tp_version=1;
static int press1=0, press2=0;
static bool screen_flag=1,bak_p1=0,bak_p2=0,receive_phone=0,phone_ap=0,run_one=1;
static u8  tpversion,org_tpversion=0,t_ver;
static u32 g_rev,org_rev=0,t_rev;
static int bak_x=0,bak_y=0;
static u8  keyregister,keypress;
static int interrupt_counter=0;
static bool touchedge=0,key_pressed=0,keylocation=0,key_location=0,Adjustsensitivity=0,runone=0;
int TOUCH_GPIO = 94;
bool  BKL_EN=1;
int ts_width;
int led_flag=0,press_point=0;
static struct early_suspend ts_early;
struct st1232_ts_data *st1232_ts;	
static int project_id;
static int hw_id;
	static u32 last_x;
	static u32 last_y;
	static u32 last_x2;
	static u32 last_y2;	
	static u32 last_key_x;
	static u32 last_key_y;		
static u32 ts_last_x;
static u32 ts_last_y;
	static u32 last_x_axis;
	static u32 last_y_axis;
	static u32 x_axis;
	static u32 y_axis;
	static u32 z1_axis;
	//static u32 z2_axis;
	static u32 x_axis_avg_raw;
	static u32 y_axis_avg_raw;
	static u32 z1_axis_avg_raw;
	static u32 z2_axis_avg_raw;
	
#if X_Y_AXIS_QUEUE
	static u32 x_axis_avg_raw_queue_index;
	static u32 y_axis_avg_raw_queue_index;
	static u32 x_axis_avg_raw_queue[3];
	static u32 y_axis_avg_raw_queue[3];
#endif
	
	static u32 x_OOT_buf[OOT_MAX];
	static u32 y_OOT_buf[OOT_MAX];
	static u32 x_axis_raw_buf[16];
	static u32 y_axis_raw_buf[16];
	static u32 coord_left;
	static u32 coord_right;
	static u32 coord_top;
	static u32 coord_bottom;
	static int calibrate_left_diff;
	static int calibrate_right_diff;
	static int calibrate_top_diff;
	static int calibrate_bottom_diff;	
	static u32 calibrated_left 		= 0;
	static u32 calibrated_right 	= 0;
	static u32 calibrated_top 		= 0;
	static u32 calibrated_bottom 	= 0;
	static u32 x_diff;
	static u32 y_diff;
	static u32 default_x_diff;
	static u32 default_y_diff;
	static u32 debug_flag 		= 0;
	static u32 debug_message 	= 0;
	static u32 tssc_reg_addr 	= 0;
	static u32 tssc_reg_data 	= 0;
	static u32 tssc_reg_read 	= 0;
	static u32 tssc_reg_write = 0;
	static int pen_down 			= 0;
	static int pen_OOR 				= 0;
	static int counter_OOT 		= 0;
	static int touch_recalibration			= 0;
	static int touch_calibration_enable	= 1;
	static int set_highest_priority	= 0;
	static u32 x_y_axis_coord_restore		= 1;
	static int down_edge=0;
	static struct wake_lock tpwakelock,tpwakelock_idle;
	
extern int vbatt;
extern int batt_capacity;
//	static cci_hw_id_type saved_cci_hw_id 		= HW_ID_INVALID;
extern int rpc_mpp_config_led_state(int index, int mode, int on_time, int off_time_1, int off_time_2, int level);
static void st1232_ts_timer(unsigned long arg);
static void st1232_ts_timer(unsigned long arg);
static irqreturn_t st1232_ts_interrupt(int irq, void *dev_id);
int st1232_i2c_read(struct i2c_client *client, int reg, u8 *buf, int count);
void config_gpio_table(uint32_t *table, int len);
void tp_reset(void);
void tp_reset_and_retry(void);
int tp_reset_and_check(struct i2c_client *client);
#if PRINT_X_Y_VALUE
	static int counter_TSSC	=	0;
#endif
	static struct workqueue_struct *TSSC_wq,*MT_wq;
#if PRINT_COUNTER
unsigned long ini_m1=0, end_m1=0;
unsigned long ini_m2=0, end_m2=0;
unsigned long ini_timer=0, end_timer=0;
#endif
static struct input_dev *fpdev;
static int distance = 0;
static int enablep = 0;

module_param_named(
	touch_calibration_enable, touch_calibration_enable,
	int, S_IRUGO | S_IWUSR | S_IWGRP
);

void tssc_hardware_init(void);
static void ts_TSSC_work_func(struct work_struct *work);
void restore_default_coord(void);
void ts_dump_tssc_register(void);
void read_x_y_axis_raw_value_2_buffer(void);
void add_x_y_axis_avg_raw_to_queue(u32 x, u32 y);
int Is_meet_fixed_pointer_filter(int x_diff, int y_diff);
int Is_out_of_range(int x, int y, int z1);
int Is_out_of_tolerance(int x, int y);
int	x_point_correction(int x);
int	y_point_correction(int y);
static void ts_update_pen_state(struct ts *ts, int x, int y, int pressure);
static uint32_t gpio_table[] = {
	GPIO_CFG(94, 0, 0, 0, 0),
	//GPIO_CFG(94, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),
	};

#if FEATURE_TSSC_D1_FILTER
typedef struct {
    int x;
    int y;
} point;

static int dist(point *p1, point *p2);
static point **sort(point *p1, point *p2, point *p3);
static point pbwa(point points[]);
#endif /* #ifdef FEATURE_TSSC_D1_FILTER */

#if TCXO_SHUTDOWN_WORKAROUND
// for Tcxo shutdown
static u32 bak_lx;
static u32 bak_ly;
#endif

#define SITRONIX_ISP_ENABLED
#ifdef SITRONIX_ISP_ENABLED
#if 0
struct st1232_ts_data {
	struct i2c_client *client;
	struct input_dev *input;
	struct work_struct work;
	struct mutex mutex;
	int irq;
	bool isp_enabled;
};
#endif //0

struct config_param_v0{
	u8 	reserve1[3];		
	u8	x_chs;				// Number of X channels.
	u8	y_chs;				// Number of Y channels.								
	u16	x_res;				// X resolution. [6:5]
	u16	y_res;				// Y resolution. [8:7]
	u8	ch_map[MAX_CHANNELS];		// Channle mapping table.

	u8	data_threshold_shift_x;		// Data threshold shift. [41]
	u8	data_threshold_offset_x;	// Data threshold offset.[42]

	u8	pt_threshold_shift;		// Point threshold shift. [43]
	u8	pt_threshold_offset;		// Point threshold offset. [44]
	u8      reserve2[5];
	
	u8 	cnt[MAX_CHANNELS];
	u8	offset[MAX_CHANNELS];
	u16	baseline[MAX_CHANNELS];	
	
	u8	k_chs;
	u8 	reserve4[11];
	
	u8	wake_threshold_shift;		
	u8	data_threshold_shift_y;		// Data threshold shift.  [191]
	u8	data_threshold_offset_y;	// Data threshold offset. [192]
	u8	data_threshold_shift_k;		// Data threshold shift.  [193]
	u8	data_threshold_offset_k;	// Data threshold offset. [194] 
	
	u8	peak_threshold_shift_x;		// Data threshold shift. 	[195]
	u8	peak_threshold_offset_x;	// Data threshold offset.	[196]
	u8	peak_threshold_shift_y;		// Data threshold shift.	[197]
	u8	peak_threshold_offset_y;	// Data threshold offset.	[198]
	
	u8	mutual_threshold_shift;		// Data threshold shift.	[199]
	u8	mutual_threshold_offset;	// Data threshold offset.	[200]
	
	//Filter
	u8	filter_rate;		// [201]
	u16	filter_range_1;	// [202]
	u16	filter_range_2;	// [203]
	
	u8	reserve5[299];
} __attribute__ ((packed)); 


struct config_param_v1{
	u8	reserve1[3];
	u8	x_chs;			           	 //BCB:x_chs, Number of X channels.
	u8	y_chs;			       	//BCB:y_chs, Number of Y channels.
	u8	k_chs;				       //BCB:k_chs, Number of Key channels.
	u16	x_res;				       //BCB:x_res,x report resolution
	u16	y_res;			            	//BCB:y_res,y report resolution
	u8	ch_map[MAX_CHANNELS];		//BCB:ch_map,Channle mapping table.
	
	u8	reserve2[15];
	
	u8	data_threshold_shift_x;		    //BCB:Daimond_data_threshold_shift_X,Data threshold shift.
	u8	data_threshold_offset_x;	    //BCB:Daimond_data_threshold_offset_X,Data threshold offset.
	u8	pt_threshold_shift;		        //BCB:pt_threshold_shift_XY,Point threshold shift.
	u8	pt_threshold_offset;		    //BCB:pt_threshold_offset_XY,Point threshold offset.
	u16	pt_threshold;			        //BCB:pt_threshold,Point threshold.
	u8	reserve3;
	u8	data_threshold_shift_y;		    //BCB:data_threshold_shift_Y,Data threshold shift.
	u8	data_threshold_offset_y;	    //BCB:data_threshold_offset_Y,Data threshold offset.
	u8	data_threshold_shift_k;		    //BCB:pt_threshold_shift_K,Data threshold shift.
	u8	data_threshold_offset_k;	    //BCB:pt_threshold_offset_K,Data threshold offset.
	u8	peak_threshold_shift_x;		    //BCB:Daimond_peak_shift_X,Data threshold shift.
	u8	peak_threshold_offset_x;	    //BCB:Daimond_peak_offset_X,Data threshold offset.
	u8	peak_threshold_shift_y;		    //BCB:Daimond_peak_shift_Y,Data threshold shift.
	u8	peak_threshold_offset_y;	    //BCB:Daimond_peak_offset_Y,Data threshold offset.
	u8	mutual_threshold_shift;		    //BCB:Daimond_mutual_threshold_shift,Data threshold shift.
	u8	mutual_threshold_offset;	//BCB:Daimond_mutual_threshold_offset,Data threshold offset.
	u8	filter_rate;                    	//BCB:filter_rate,
	u16	filter_range_1;                 	//BCB:filter_range1,
	u16	filter_range_2;	              //BCB:filter_range2,
	//
	u8  Bar_X_RAW;                      	//BCB:Bar_X_RAW,
	u8  Bar_X_Raw_2_Peak;             	//BCB:Bar_X_Raw_2_Peak,
	u8  Bar_X_Delta;                    	//BCB:Bar_X_Delta,
	u16 Bar_Y_Delta_2_Peak;           	//BCB:Bar_Y_Delta_2_Peak,
	u8  Border_Offset_X;                	//BCB:Border_Offset_X,
	u8  Border_Offset_Y;                	//BCB:Border_Offset_Y,
	
	u8	reserve[42];
	u8	cnt[CONFIG_PARAM_MAX_CHANNELS];
	u8	offset[CONFIG_PARAM_MAX_CHANNELS];
	u16	baseline[CONFIG_PARAM_MAX_CHANNELS];
	u8	mutual_baseline[MUTUAL_CALIBRATION_BASE_SIZE];		//used to store the mutual calibration baseline in no touch
} __attribute__ ((packed)); 

#define SITRONIX_MT_ENABLED

#define STX_TS_MAX_RES_SHIFT	(11)
#define STX_TS_MAX_VAL		((1 << STX_TS_MAX_RES_SHIFT) - 1)

#define ST1232_FLASH_SIZE	0x3C00
#define ST1232_ISP_PAGE_SIZE	0x200
#define ST1232_ROM_PARAM_ADR 0x3E00

// ISP command.
#define STX_ISP_ERASE		0x80
#define STX_ISP_SEND_DATA	0x81
#define STX_ISP_WRITE_FLASH	0x82
#define STX_ISP_READ_FLASH	0x83
#define STX_ISP_RESET		0x84
#define STX_ISP_READY		0x8F

typedef struct {
	u8	y_h		: 3,
		reserved	: 1,
		x_h		: 3,
		valid		: 1;
	u8	x_l;
	u8	y_l;
} xy_data_t;

typedef struct {
	u8	fingers		: 3,
		gesture		: 5;
	u8	keys;
	xy_data_t	xy_data[2];
} stx_report_data_t;

static u8 isp_page_buf[ST1232_ISP_PAGE_SIZE];

static int sitronix_ts_get_fw_version(struct i2c_client *client, u32 *ver)
{
	char buf[1];
	int ret;

	buf[0] = 0x0;	//Set Reg. address to 0x0 for reading FW version.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	//Read 1 byte FW version from Reg. 0x0 set previously.
	if ((ret = i2c_master_recv(client, buf, 1)) != 1)
		return -EIO;

	*ver = (u32) buf[0];

	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	return 0;
}

static int sitronix_ts_get_fw_revision(struct i2c_client *client, u32 *rev)
{
	char buf[4];
	int ret;
	printk("[bing]sitronix_ts_get_fw_revision\r\n");

	buf[0] = 0xC;	//Set Reg. address to 0x0 for reading FW version.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	//Read 1 byte FW version from Reg. 0x0 set previously.
	if ((ret = i2c_master_recv(client, buf, 4)) != 4)
		return -EIO;

	*rev = ((u32)buf[3]);
	*rev |= (((u32)buf[2]) << 8);
	*rev |= (((u32)buf[1]) << 16);
	*rev |= (((u32)buf[0]) << 24);

	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	return 0;

}

static int sitronix_ts_set_autotune_en(struct i2c_client *client)
{
	char buf[2], device_ctrl;
	int ret; 
	if(CanUpdate) {
	printk("AutoTune enable\n");
	buf[0] = 0x02;	// Set Reg. address to 0x02 for reading Device Control
	if((ret = i2c_master_send(client, buf, 1)) != 1)
	{
		printk("[FW]sitronix_ts_set_autotune_en1\r\n");		
		goto io_err;
	}	
	// Read 1 byte Device Control from Reg. 0x02
	if((ret = i2c_master_recv(client, &device_ctrl, 1)) != 1)
	{
		printk("[FW]sitronix_ts_set_autotune_en2\r\n");		
		goto io_err;
	}	
	device_ctrl |= 0x80;
	
        buf[0] = 0x2;        //Set Reg. address to 0x2 for enable autotune 
	buf[1] = device_ctrl;
	if ((ret = i2c_master_send(client, buf, 2)) != 2)
	{
		printk("[FW]sitronix_ts_set_autotune_en3\r\n");		
		goto io_err;
	}	
	mdelay(100);

        // Check autotune enable bit 
	
	/*
        count = 0;         
        // Read 1 byte Device Control from Reg. 0x02 
        do { 
                msleep(5); 
                if((ret = i2c_master_recv(client, buf, 1)) != 1) 
                        goto io_err; 
                count++; 
        } while((!(buf[0] & 0x80)) && (count <= 50000)); 
        
        if ( count > 50000 ) 
                printk("Enable Autotune Timeout!\n"); 
	*/
	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
	{
		printk("[FW]sitronix_ts_set_autotune_en4\r\n");		
		goto io_err;
	}	
	}

	return 0;
io_err:
	printk("[DEBUG_MSG] TEST!\n"); 
	led_flag=1;
	printk("[FW]sitronix_ts_set_autotune_en\r\n");

	WARN_ON(true);
		return -EIO;
	}

static int sitronix_ts_get_status(struct i2c_client *client)
{
	char buf[2], status;
	int ret;
	buf[0] = 0x01;	// Set Reg. address to 0x01 for reading Device Status
	if((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;
	
	// Read 1 byte Device Control from Reg 0x01
	if((ret = i2c_master_recv(client, &status, 1)) != 1)
		return -EIO;

	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	printk(".");
	return status;
	
}

static int sitronix_ts_get_resolution(struct i2c_client *client, u16 *x_res, u16 *y_res)
{
	char buf[3];
	int ret;
	printk("[bing]sitronix_ts_get_resolution\r\n");
	buf[0] = 0x4;	//Set Reg. address to 0x4 for reading XY resolution.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	//Read 3 byte XY resolution from Reg. 0x4 set previously.
	if ((ret = i2c_master_recv(client, buf, 3)) != 3)
		return -EIO;

	*x_res = ((buf[0] & 0xF0) << 4) | buf[1];
	*y_res = ((buf[0] & 0x0F) << 8) | buf[2];

	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	return 0;

}

static int sitronix_ts_set_resolution(struct i2c_client *client, u16 x_res, u16 y_res)
{
	char buf[4];
	int ret = 0;

	buf[0] = 0x4;	//Set Reg. address to 0x4 for reading XY resolution.
	buf[1] = ((u8)((x_res & 0x0700) >> 4)) | ((u8)((y_res & 0x0700) >> 8));
	buf[2] = ((u8)(x_res & 0xFF));
	buf[3] = ((u8)(y_res & 0xFF));
	if ((ret = i2c_master_send(client, buf, 4)) != 4)
		return -EIO;

	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	return 0;
}

static int sitronix_ts_open(struct input_dev *dev)
{
	struct st1232_ts_data *priv = input_get_drvdata(dev);

	enable_irq(priv->irq);
        ++en_irq;
	return 0;
}

static void sitronix_ts_close(struct input_dev *dev)
{
	struct st1232_ts_data *priv = input_get_drvdata(dev);

	disable_irq(priv->irq);
}

static int st1232_isp_erase(struct i2c_client *client, u8 page_num)
{
	u8 data[8];
	u8 i , done;
	u32 retry;
	data[0] = STX_ISP_ERASE;
	data[1] = 0x0;
	data[2] = page_num;
	printk("st1232_isp_erase\n");

	printk("[FW]st1232_isp_erase2:%d\r\n", data[0]);		

	if (i2c_master_send(client, data, sizeof(data)) != sizeof(data)) {
		dev_err(&client->dev, "%s(%u): ISP erase page(%u) failed!\n",
						__FUNCTION__, __LINE__, (unsigned int)page_num);
		led_flag=1;
		printk("[FW]st1232_isp_erase1\r\n");		
		return -EIO;
	}

	if (i2c_master_recv(client, data, sizeof(data)) != sizeof(data) || data[0] != STX_ISP_READY) {
		dev_err(&client->dev, "%s(%u): ISP read READY failed! (%02x)\n", __FUNCTION__, __LINE__, data[0]);
		led_flag=1;
		printk("[FW]st1232_isp_erase2\r\n");		
	        //return -EIO;
	}

	/*
	done = 0 ; 
	retry = 0;
	while((!done) && (retry < 500)) {
		i2c_master_recv(client, data, sizeof(data));
		if(data[0] == STX_ISP_READY)
			done = 1;
		else 
			retry++;
		mdelay(5);
	}
	
	if(retry >= 500)
		return -EIO;
	*/
	return 0;
}

static int st1232_isp_reset(struct i2c_client *client)
{
	u8 data[8];

	data[0] = STX_ISP_RESET;

	if (i2c_master_send(st1232_ts->client, data, sizeof(data)) != sizeof(data)) {
		dev_err(&client->dev, "%s(%u): ISP reset chip failed!\n", __FUNCTION__, __LINE__);
		return -EIO;
	}

	mdelay(200);
	printk("*************************************** ISP reset ok! ***********************************************\n");
	return 0;
}

static int st1232_jump_to_isp(struct i2c_client *client)
{
	int i;
	u8 signature[] = "STX_FWUP";
	u8 buf[2];
	u8 buf4[8];
	int ret;

	for (i = 0; i < strlen(signature); i++) {
		buf[0] = 0x0;
		buf[1] = signature[i];
		if (i2c_master_send(st1232_ts->client, buf, 2) != 2) {
			dev_err(&client->dev, "%s(%u): Unable to write ISP STX_FWUP!\n", __FUNCTION__, __LINE__);
			led_flag=1;	
			printk("[FW]st1232_jump_to_isp\r\n");		

			return -EIO;
		}
		msleep(100);
	}
	if(!gpio_get_value(94)) {
		printk("*************************************** gpio 93 = 0! ***********************************************\n");
		i2c_master_recv(client, buf, sizeof(buf));
	}
	
	printk("*************************************** Jump to ISP ok! ***********************************************\n");

	return 0;
}

static int st1232_isp_read_page(struct i2c_client *client, char *page_buf, u8 page_num)
{
	u8 data[8];
	u32 rlen;

	memset(data, 0, sizeof(data));
	memset(page_buf, 0, ST1232_ISP_PAGE_SIZE);
	data[0] = STX_ISP_READ_FLASH;
	data[2] = page_num;
	if (i2c_master_send(client, data, sizeof(data)) != sizeof(data)) {
		dev_err(&client->dev, "%s(%u): ISP read flash failed!\n", __FUNCTION__, __LINE__);
		led_flag=1;		
		printk("[FW]st1232_isp_read_page1\r\n");				
		return -EIO;
	}
	rlen = 0;
	while (rlen < ST1232_ISP_PAGE_SIZE) {
		if (i2c_master_recv(client, (page_buf+rlen), sizeof(data)) != sizeof(data)) {
			dev_err(&client->dev, "%s(%u): ISP read data failed!\n", __FUNCTION__, __LINE__);
			led_flag=1;
			printk("[FW]st1232_isp_read_page2\r\n");		
			
			return -EIO;
		}
		rlen += 8;
	}


	return ST1232_ISP_PAGE_SIZE;
}

static u16 st1232_isp_cksum(char *page_buf)
{
	u16 cksum = 0;
	int i;
	for (i = 0; i < ST1232_ISP_PAGE_SIZE; i++) {
		cksum += (u16) page_buf[i];
	}
	return cksum;
}

static int st1232_isp_write_page(struct i2c_client *client, char *page_buf, u8 page_num)
{
	u8 data[8];
	int wlen;
	u32 len;
	u16 cksum;

	if (st1232_isp_erase(client, page_num) < 0) {
		return -EIO;
	}
	printk("[bing]st1232_isp_write_page\r\n");

	cksum = st1232_isp_cksum(page_buf);

	data[0] = STX_ISP_WRITE_FLASH;
	data[2] = page_num;
	data[4] = (cksum & 0xFF);
	data[5] = ((cksum & 0xFF) >> 8);

	if (i2c_master_send(client, data, sizeof(data)) != sizeof(data)) {
		dev_err(&client->dev, "%s(%u): ISP write page failed!\n", __FUNCTION__, __LINE__);
		led_flag=1;		
		printk("[FW]st1232_isp_write_page1\r\n");		

		return -EIO;
	}

	data[0] = STX_ISP_SEND_DATA;
	wlen = ST1232_ISP_PAGE_SIZE;
	len = 0;
	while (wlen>0) {
		len = (wlen < 7) ? wlen : 7;
		memcpy(&data[1], page_buf, len);

		if (i2c_master_send(client, data, sizeof(data)) != sizeof(data)) {
			dev_err(&client->dev, "%s(%u): ISP send data failed!\n", __FUNCTION__, __LINE__);
			led_flag=1;	
			printk("[FW]st1232_isp_write_page2\r\n");		
			
			return -EIO;
		}

		wlen -= 7;
		page_buf += 7;
	}

	if (i2c_master_recv(client, data, sizeof(data)) != sizeof(data) || data[0] != STX_ISP_READY) {
		dev_err(&client->dev, "%s(%u): ISP read READY failed!\n", __FUNCTION__, __LINE__);
		led_flag=1;		
		printk("[FW]st1232_isp_write_page3\r\n");		
		
		return -EIO;
	}

	return ST1232_ISP_PAGE_SIZE;
}

static int st1232_isp_read_flash(struct i2c_client *client, char *buf, loff_t off, size_t count)
{
	u32 page_num, page_off;
	u32 len = 0, rlen = 0;

	page_num = off / ST1232_ISP_PAGE_SIZE;
	page_off = off % ST1232_ISP_PAGE_SIZE;
	printk("[bing]st1232_isp_read_flash\r\n");

	if (page_off) {

		if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return len;

		len -= page_off;

		len = (count > len ? len : count);

		memcpy(buf, (isp_page_buf + page_off), len);
		buf += len;
		count -= len;
		rlen += len;
		page_num++;
	}

	while (count) {
		if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return len;

		len = (count > len ? len : count);

		memcpy(buf, isp_page_buf, len);

		buf += len;
		count -= len;
		rlen += len;
		page_num++;
	}

	return rlen;
}

static ssize_t st1232_flash_read(struct kobject *kobj, struct bin_attribute *attr,
				  char *buf, loff_t off, size_t count)
{
	struct i2c_client *client = kobj_to_i2c_client(kobj);
	int rc;

	dev_dbg(&client->dev, "%s(%u): buf=%p, off=%lli, count=%zi)\n", __FUNCTION__, __LINE__, buf, off, count);
	printk("[bing]st1232_flash_read\r\n");

	if (off >= ST1232_FLASH_SIZE)
		return 0;

	if (off + count > ST1232_FLASH_SIZE)
		count = ST1232_FLASH_SIZE - off;

	rc = st1232_isp_read_flash(client, buf, off, count);

	if (rc < 0)
		return -EIO;

	return rc;
}

static int st1232_isp_write_flash(struct i2c_client *client, char *buf, loff_t off, size_t count)
{
	u8 page_num, page_off;
	u32 len, wlen = 0;
	int rc;

	page_num = off / ST1232_ISP_PAGE_SIZE;
	page_off = off % ST1232_ISP_PAGE_SIZE;
	printk("[bing]st1232_isp_write_flash\r\n");

	if (page_off) {

		// Start RMW.
		// Read page.
		if ((rc = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return rc;

		len = ST1232_ISP_PAGE_SIZE - page_off;
		// Modify data.
		memcpy((isp_page_buf+page_off), buf, len);

		// Write back page.
		st1232_isp_write_page(client, isp_page_buf, page_num);

		buf += len;
		count -= len;
		wlen += len;
	}

	while (count) {
		if (count >= ST1232_ISP_PAGE_SIZE) {
			len = ST1232_ISP_PAGE_SIZE;
			memcpy(isp_page_buf, buf, len);

			if ((rc = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return rc;

			buf += len;
			count -= len;
			wlen += len;
		} else {
			// Start RMW.
			// Read page.
			if ((rc = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
				return rc;

			len = count;
			// Modify data.
			memcpy(isp_page_buf, buf, len);

			// Write back page.
			if ((rc = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return rc;

			buf += len;
			count -= len;
			wlen += len;
		}
		page_num++;
	}

	return wlen;
}

static ssize_t st1232_flash_write(struct kobject *kobj, struct bin_attribute *attr,
				   char *buf, loff_t off, size_t count)
{
	struct i2c_client *client = kobj_to_i2c_client(kobj);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	int rc;
	//printk("[bing]st1232_flash_write:priv->Can_update=%d\r\n",priv->Can_update);

	if(priv->Can_update) {

	printk("[bing]st1232_flash_write\r\n");

	dev_dbg(&client->dev, "%s(%u): buf=%p, off=%lli, count=%zi)\n", __FUNCTION__, __LINE__, buf, off, count);

	if (off >= ST1232_FLASH_SIZE)
		return -ENOSPC;

	if (off + count > ST1232_FLASH_SIZE)
		count = ST1232_FLASH_SIZE - off;

	
	rc = st1232_isp_write_flash(client, buf, off, count);

	if (rc < 0)
		return -EIO;

	return rc;
	} else {
		return count;
	}
}

static struct bin_attribute st1232_flash_bin_attr = {
	.attr = {
		.name = "flash",
		.mode = S_IRUGO | S_IWUSR,
	},
	.size = ST1232_FLASH_SIZE,
	.read = st1232_flash_read,
	.write = st1232_flash_write,
};

static ssize_t st1232_panel_config_read(struct kobject *kobj, struct bin_attribute *attr,
				  char *buf, loff_t off, size_t count)
{
	return count;
}


static ssize_t st1232_panel_config_write(struct kobject *kobj, struct bin_attribute *attr,
				   char *buf, loff_t off, size_t count)
{
	struct i2c_client *client = kobj_to_i2c_client(kobj);
	int i;
	u8 page_num , retry;
	u8 read_buf[ST1232_ISP_PAGE_SIZE];
	u32 len;
	if(CanUpdate) {
		page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
		dev_dbg(&client->dev, "%s(%u): buf=%p, off=%lli, count=%zi)\n", __FUNCTION__, __LINE__, buf, off, count);
	
		printk("input panel config\n");
		for(i = 0 ; i < 32 ; i++) {
			printk("%02x %02x %02x %02x %02x %02x %02x %02x ", buf[i*16], buf[i*16+1], buf[i*16+2], buf[i*16+3], buf[i*16+4], buf[i*16+5], buf[i*16+6], buf[i*16+7]);
			printk("%02x %02x %02x %02x %02x %02x %02x %02x\n", buf[i*16+8], buf[i*16+9], buf[i*16+10], buf[i*16+11], buf[i*16+12], buf[i*16+13], buf[i*16+14], buf[i*16+15]);
		}
		//buf[511] = 0x3e;
		//Write back
		if ((len = st1232_isp_write_page(client, buf, page_num)) < 0)
		{
	   			//led_flag=1;
				//printk("[FW]st1232_panel_config_write\r\n");		
				
				//return -EIO;
	    	}	
		st1232_isp_read_page(client,read_buf,page_num);
		retry = 0;
		for(i = 0 ; i < ST1232_ISP_PAGE_SIZE ; i++) 
		{
			if(buf[i] != read_buf[i])
			{
				retry = 1;
				printk("[FW]st1232_panel_config_write retry\r\n");
				break;
			}
		}
		if(retry)
		{
			if ((len = st1232_isp_write_page(client, buf, page_num)) < 0)
			{
	   			led_flag=1;
				printk("[FW]st1232_panel_config_write\r\n");		
				
				return -EIO;
	    		}
		}
	}
	return count;
}

static struct bin_attribute st1232_panel_bin_attr = {
	.attr = {
		.name = "panel_config",
		.mode = S_IRUGO | S_IWUSR,
	},
	.size = ST1232_ISP_PAGE_SIZE,
	.read = st1232_panel_config_read,
	.write = st1232_panel_config_write,
};

static ssize_t sitronix_ts_isp_ctrl_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	printk("[bing]sitronix_ts_isp_ctrl_show\r\n");

	return sprintf(buf, "%d\n", priv->isp_enabled);
}

static ssize_t sitronix_ts_isp_ctrl_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	u8 buf4[8];
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	int enabled = 0;
	if(priv->Can_update) {
	sscanf(buf, "%x", &enabled);
	printk("[bing]sitronix_ts_isp_ctrl_store:priv->isp_enabled=%d  enabled=%d priv->Can_update=%d\r\n",priv->isp_enabled,enabled,priv->Can_update);
	
	if (priv->isp_enabled && !enabled) {
		//ISP Reset.
		priv->isp_enabled = false;
		st1232_isp_reset(client);
	} else if (!priv->isp_enabled && enabled) {
		//Jump to ISP.
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	}
	return count;
}

static DEVICE_ATTR(isp_ctrl, 0644, sitronix_ts_isp_ctrl_show, sitronix_ts_isp_ctrl_store);

static ssize_t sitronix_ts_revision_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u32 rev;
	int err;

	if ((err = sitronix_ts_get_fw_revision(client, &rev))) {
		dev_err(&client->dev, "Unable to get FW revision!\n");
		return 0;
	}
	printk("[bing]sitronix_ts_revision_show=%d\r\n",rev);

	return sprintf(buf, "%u\n", rev);
}

static DEVICE_ATTR(revision, 0644, sitronix_ts_revision_show, NULL);

static ssize_t sitronix_ts_struct_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 page_num;
	u32 len;
	
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);

	return sprintf(buf, "%u\n", isp_page_buf[1]);
}

static DEVICE_ATTR(struct_version, 0644, sitronix_ts_struct_version_show, NULL);

/*
 * sitronix data threshold show & store
 */
static ssize_t sitronix_ts_data_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 x_shift, y_shift, k_shift;
	u8 x_offset, y_offset, k_offset;
	u8 page_num;
	u32 len;

	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	printk("[bing] sitronix_ts_data_threshold_show\r\n");

	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		x_shift = pROM_v0->data_threshold_shift_x;
		x_offset = pROM_v0->data_threshold_offset_x;
		y_shift = pROM_v0->data_threshold_shift_y;
		y_offset = pROM_v0->data_threshold_offset_y;
		k_shift = pROM_v0->data_threshold_shift_k;
		k_offset = pROM_v0->data_threshold_offset_k;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		x_shift = pROM_v1->data_threshold_shift_x;
		x_offset = pROM_v1->data_threshold_offset_x;
		y_shift = pROM_v1->data_threshold_shift_y;
		y_offset = pROM_v1->data_threshold_offset_y;
		k_shift = pROM_v1->data_threshold_shift_k;
		k_offset = pROM_v1->data_threshold_offset_k;
	}
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	
	return sprintf(buf, "%u %u %u %u %u %u\n", x_shift, x_offset, y_shift, y_offset, k_shift, k_offset);
}

static ssize_t sitronix_ts_data_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u32 x_shift, y_shift, k_shift;
	u32 x_offset, y_offset, k_offset;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	printk("[bing] data_threshold:Adjustsensitivity=%d\r\n",Adjustsensitivity);
	if(Adjustsensitivity==1)
	{
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	sscanf(buf, "%u %u %u %u %u %u", &x_shift, &x_offset, &y_shift, &y_offset, &k_shift, &k_offset);

	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;
	
	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		pROM_v0->data_threshold_shift_x = x_shift&0xFF;
		pROM_v0->data_threshold_offset_x = x_offset&0xFF;
		pROM_v0->data_threshold_shift_y = y_shift&0xFF;
		pROM_v0->data_threshold_offset_y = y_offset&0xFF;
		pROM_v0->data_threshold_shift_k = k_shift&0xFF;
		pROM_v0->data_threshold_offset_k = k_offset&0xFF;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		pROM_v1->data_threshold_shift_x = x_shift&0xFF;
		pROM_v1->data_threshold_offset_x = x_offset&0xFF;
		pROM_v1->data_threshold_shift_y = y_shift&0xFF;
		pROM_v1->data_threshold_offset_y = y_offset&0xFF;
		pROM_v1->data_threshold_shift_k = k_shift&0xFF;
		pROM_v1->data_threshold_offset_k = k_offset&0xFF;
	}
	//Write back
	if ((len = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	}
	return count;
}

static DEVICE_ATTR(data_threshold, 0644, sitronix_ts_data_threshold_show, sitronix_ts_data_threshold_store);

/*
 * sitronix point threshold show & store
 */
static ssize_t sitronix_ts_point_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 shift, offset;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	printk("[bing] sitronix_ts_point_threshold_show\r\n");

	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		shift = pROM_v0->pt_threshold_shift;
		offset = pROM_v0->pt_threshold_offset;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		shift = pROM_v1->pt_threshold_shift;
		offset = pROM_v1->pt_threshold_offset;
	}
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	return sprintf(buf, "%u %u\n", shift, offset);
}

static ssize_t sitronix_ts_point_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{


	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u32 shift, offset;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;

	timecounter[index_jiffies]=jiffies;
	printk("[bing]jiffies=%lu HZ=%lu\r\n",jiffies,HZ);
	printk("[bing]point_threshold:Adjustsensitivity=%d \r\n",Adjustsensitivity);

	if(index_jiffies==0)
		Adjustsensitivity=1;
	if(index_jiffies==1)
	{
		diff_time=timecounter[1]-timecounter[0];
		if(diff_time>400)
			Adjustsensitivity=1;
		else
			Adjustsensitivity=0;
		printk("[bing][%lu %lu],  [%lu %lu] [%lu %lu] Adjustsensitivity=%d\r\n",timecounter[1],timecounter[0],jiffies_to_msecs(timecounter[1]),jiffies_to_msecs(timecounter[0]),diff_time,jiffies_to_msecs(diff_time),Adjustsensitivity);
	}
	++index_jiffies;
	if(index_jiffies==2)
	{
		timecounter[0]=timecounter[1];
		--index_jiffies;
	}
	if(Adjustsensitivity==1)
	{
	CanUpdate=1;
	printk("[bing] point_threshold: CanUpdate=%d\r\n",CanUpdate);
	
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	sscanf(buf, "%u %u", &shift, &offset);
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;
	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		pROM_v0->pt_threshold_shift = shift&0xFF;
		pROM_v0->pt_threshold_offset = offset&0xFF;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		pROM_v1->pt_threshold_shift = shift&0xFF;
		pROM_v1->pt_threshold_offset = offset&0xFF;
	}
	//Write back
	if ((len = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	}
	return count;
	
}

static DEVICE_ATTR(point_threshold, 0644, sitronix_ts_point_threshold_show, sitronix_ts_point_threshold_store);

/*
 * sitronix peak threshold show & store
 */
static ssize_t sitronix_ts_peak_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 x_shift, y_shift;
	u8 x_offset, y_offset;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	printk("[bing] sitronix_ts_peak_threshold_show\r\n");

	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00)
	{
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		x_shift = pROM_v0->peak_threshold_shift_x;
		x_offset = pROM_v0->peak_threshold_offset_x;
		y_shift = pROM_v0->peak_threshold_shift_y;
		y_offset = pROM_v0->peak_threshold_offset_y;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		x_shift = pROM_v1->peak_threshold_shift_x;
		x_offset = pROM_v1->peak_threshold_offset_x;
		y_shift = pROM_v1->peak_threshold_shift_y;
		y_offset = pROM_v1->peak_threshold_offset_y;
	}
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);

	return sprintf(buf, "%u %u %u %u\n", x_shift, x_offset, y_shift, y_offset);
}

static ssize_t sitronix_ts_peak_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u32 x_shift, y_shift;
	u32 x_offset, y_offset;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	printk("[bing]peak_threshold: Adjustsensitivity=%d\r\n",Adjustsensitivity);
	if(Adjustsensitivity==1)
	{
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	sscanf(buf, "%u %u %u %u", &x_shift, &x_offset, &y_shift, &y_offset);
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		pROM_v0->peak_threshold_shift_x = x_shift&0xFF;
		pROM_v0->peak_threshold_offset_x = x_offset&0xFF;
		pROM_v0->peak_threshold_shift_y = y_shift&0xFF;
		pROM_v0->peak_threshold_offset_y = y_offset&0xFF;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		pROM_v1->peak_threshold_shift_x = x_shift&0xFF;
		pROM_v1->peak_threshold_offset_x = x_offset&0xFF;
		pROM_v1->peak_threshold_shift_y = y_shift&0xFF;
		pROM_v1->peak_threshold_offset_y = y_offset&0xFF;
	}
	
	//Write back
	if ((len = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	CanUpdate=0;
	printk("[bing] peak_threshold: CanUpdate=%d\r\n",CanUpdate);
	
	}
	return count;
}

static DEVICE_ATTR(peak_threshold, 0644, sitronix_ts_peak_threshold_show, sitronix_ts_peak_threshold_store);

/*
 * sitronix mutual threshold show & store
 */
static ssize_t sitronix_ts_mutual_threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 shift, offset;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00)
	{
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		shift = pROM_v0->mutual_threshold_shift;
		offset = pROM_v0->mutual_threshold_offset;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		shift = pROM_v1->mutual_threshold_shift;
		offset = pROM_v1->mutual_threshold_offset;
	}
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	
	return sprintf(buf, "%u %u\n", shift, offset);
}

static ssize_t sitronix_ts_mutual_threshold_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u32 shift, offset;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;

	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	sscanf(buf, "%u %u", &shift, &offset);
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		pROM_v0->mutual_threshold_shift = shift&0xFF;
		pROM_v0->mutual_threshold_offset = offset&0xFF;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		pROM_v1->mutual_threshold_shift = shift&0xFF;
		pROM_v1->mutual_threshold_offset = offset&0xFF;
	}
	
	//Write back
	if ((len = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	return count;
}

static DEVICE_ATTR(mutual_threshold, 0644, sitronix_ts_mutual_threshold_show, sitronix_ts_mutual_threshold_store);

/*
 * sitronix range filter show & store
 */
static ssize_t sitronix_ts_range_filter_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 rate;
	u16 range1,range2;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;

	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		range1 = ((pROM_v0->filter_range_1 & 0xFF)<<8) | (pROM_v0->filter_range_1>>8);
		range2 = ((pROM_v0->filter_range_2 & 0xFF)<<8) | (pROM_v0->filter_range_2>>8);
		rate = pROM_v0->filter_rate;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		range1 = ((pROM_v1->filter_range_1 & 0xFF)<<8) | (pROM_v1->filter_range_1>>8);
		range2 = ((pROM_v1->filter_range_2 & 0xFF)<<8) | (pROM_v1->filter_range_2>>8);
		rate = pROM_v1->filter_rate;
	}
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);

	return sprintf(buf, "%u %u %u\n", range1, range2, rate);
}

static ssize_t sitronix_ts_range_filter_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u32 range1,range2,rate;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;

	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	sscanf(buf, "%u %u %u", &range1, &range2, &rate);
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		pROM_v0->filter_range_1 = ((range1&0xFF)<<8) | ((range1>>8)&0xFF);
		pROM_v0->filter_range_2 = ((range2&0xFF)<<8) | ((range2>>8)&0xFF);
		pROM_v0->filter_rate = rate&0xFF;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		pROM_v1->filter_range_1 = ((range1&0xFF)<<8) | ((range1>>8)&0xFF);
		pROM_v1->filter_range_2 = ((range2&0xFF)<<8) | ((range2>>8)&0xFF);
		pROM_v1->filter_rate = rate&0xFF;
	}
	
	//Write back
	if ((len = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	return count;
}

static DEVICE_ATTR(range_filter, 0644 , sitronix_ts_range_filter_show, sitronix_ts_range_filter_store);

/*
 * sitronix barX filter show & store
 */
static ssize_t sitronix_ts_barX_filter_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 RAW , RAW_2Peak , Delta;
	u8 page_num;
	int len;
	struct config_param_v1 *pROM_v1;
	if(priv->struct_version != 0x00) {
		page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
		//Jump to ISP.
		if(!priv->isp_enabled) {
			priv->isp_enabled = true;
			st1232_jump_to_isp(client);
		}
	
		//Read data from ROM
		if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		RAW = pROM_v1->Bar_X_RAW;
		RAW_2Peak = pROM_v1->Bar_X_Raw_2_Peak;
		Delta = pROM_v1->Bar_X_Delta;
	
		//ISP Reset.
		priv->isp_enabled = false;
		st1232_isp_reset(client);

		return sprintf(buf, "%u %u %u\n", RAW, RAW_2Peak, Delta);
	} else {
		return sprintf(buf, "Not Support!!\n");
	}
}

static ssize_t sitronix_ts_barX_filter_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u32 RAW , RAW_2Peak , Delta;
	u8 page_num;
	int len;
	struct config_param_v1 *pROM_v1;
	
	if(priv->struct_version != 0x00) {
		page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
		sscanf(buf, "%u %u %u", &RAW, &RAW_2Peak, &Delta);
	
		//Jump to ISP.
		if(!priv->isp_enabled) {
			priv->isp_enabled = true;
			st1232_jump_to_isp(client);
		}
	
		//Read data from ROM
		if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		pROM_v1->Bar_X_RAW = RAW&0xFF;
		pROM_v1->Bar_X_Raw_2_Peak = RAW_2Peak&0xFF;
		pROM_v1->Bar_X_Delta = Delta&0xFF;
	
	
		//Write back
		if ((len = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;
	
		//ISP Reset.
		priv->isp_enabled = false;
		st1232_isp_reset(client);
	}		
	return count;
}

static DEVICE_ATTR(barX_filter, 0644 , sitronix_ts_barX_filter_show, sitronix_ts_barX_filter_store);


/*
 * sitronix barY filter show & store
 */
static ssize_t sitronix_ts_barY_filter_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u16 RAW_2Peak;
	u8  shift , offset;
	u8 page_num;
	int len;
	struct config_param_v1 *pROM_v1;
	
	if(priv->struct_version != 0x00) {
		page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
		//Jump to ISP.
		if(!priv->isp_enabled) {
			priv->isp_enabled = true;
			st1232_jump_to_isp(client);
		}
	
		//Read data from ROM
		if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;

		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		RAW_2Peak =( (pROM_v1->Bar_Y_Delta_2_Peak & 0xFF)<<8) | (pROM_v1->Bar_Y_Delta_2_Peak>>8);
		shift = pROM_v1->peak_threshold_shift_y;
		offset = pROM_v1->peak_threshold_offset_y;
	
		//ISP Reset.
		priv->isp_enabled = false;
		st1232_isp_reset(client);
	
		return sprintf(buf, "%u %u %u\n", RAW_2Peak, shift, offset);
	} else {
		return sprintf(buf, "Not Support!!\n");
	}
}

static ssize_t sitronix_ts_barY_filter_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u32 RAW_2Peak , shift , offset;
	u8 page_num;
	int len;
	struct config_param_v1 *pROM_v1;
	
	if(priv->struct_version != 0x00) {
		page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
		sscanf(buf, "%u %u %u", &RAW_2Peak, &shift, &offset);
	
		//Jump to ISP.
		if(!priv->isp_enabled) {
			priv->isp_enabled = true;
			st1232_jump_to_isp(client);
		}
	
		//Read data from ROM
		if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;

		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		pROM_v1->Bar_Y_Delta_2_Peak = ((RAW_2Peak&0xFF)<<8) | ((RAW_2Peak>>8)&0xFF);
		pROM_v1->peak_threshold_shift_y = shift&0xFF;
		pROM_v1->peak_threshold_offset_y = offset&0xFF;
	
	
		//Write back
		if ((len = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
					return -EIO;
	
		//ISP Reset.
		priv->isp_enabled = false;
		st1232_isp_reset(client);
	}
	return count;
}

static DEVICE_ATTR(barY_filter, 0644 , sitronix_ts_barY_filter_show, sitronix_ts_barY_filter_store);
/*
 * sitronix resolution show & store
 */
static ssize_t sitronix_ts_resolution_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u16 x_res, y_res;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	printk("[bing]sitronix_ts_resolution_show\r\n");

	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		x_res = ((pROM_v0->x_res&0xFF)<<8) | (pROM_v0->x_res>>8);
		y_res = ((pROM_v0->y_res&0xFF)<<8) | (pROM_v0->y_res>>8);
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		x_res = ((pROM_v1->x_res&0xFF)<<8) | (pROM_v1->x_res>>8);
		y_res = ((pROM_v1->y_res&0xFF)<<8) | (pROM_v1->y_res>>8);
	}
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	return sprintf(buf, "%u %u\n", x_res, y_res);
}

static ssize_t sitronix_ts_resolution_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u32 x_res, y_res;
	u8 page_num;
	int len;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	sscanf(buf, "%u%u", &x_res, &y_res);
	printk("[bing]sitronix_ts_resolution_store\r\n");

	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;

	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		pROM_v0->x_res = ((x_res&0xFF)<<8) | ((x_res>>8)&0xFF);
		pROM_v0->y_res = ((y_res&0xFF)<<8) | ((y_res>>8)&0xFF);
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		pROM_v1->x_res = ((x_res&0xFF)<<8) | ((x_res>>8)&0xFF);
		pROM_v1->y_res = ((y_res&0xFF)<<8) | ((y_res>>8)&0xFF);
	}
	
	//Write back
	if ((len = st1232_isp_write_page(client, isp_page_buf, page_num)) < 0)
				return -EIO;
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	return count;
}

static DEVICE_ATTR(resolution, 0644 , sitronix_ts_resolution_show, sitronix_ts_resolution_store);

static ssize_t sitronix_ts_channel_num_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 page_num;
	u8 x_chs , y_chs , k_chs;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
		x_chs = pROM_v0->x_chs;
		y_chs = pROM_v0->y_chs;
		k_chs = pROM_v0->k_chs;
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
		x_chs = pROM_v1->x_chs;
		y_chs = pROM_v1->y_chs;
		k_chs = pROM_v1->k_chs;
	}
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	return sprintf(buf, "%u %u %u\n", x_chs, y_chs, k_chs);
}
static DEVICE_ATTR(channel_num, 0644, sitronix_ts_channel_num_show, NULL);

static ssize_t sitronix_ts_point_report_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	char Point_buf[8];
	int i ,ret;

	for(i = 0 ; i < 20 ; i++) {
		Point_buf[0] = 0x10;	//Set Reg. address to 0x10 for reading point report.
		if ((ret = i2c_master_send(client, Point_buf, 1)) != 1) {
			printk("I2C Send Data Fail\n");
			return 0;
		}

		//Read 8 byte point data from Reg. 0x10 set previously.
		if ((ret = i2c_master_recv(client, Point_buf, 8)) != 8) {
			printk("I2C Read Data Fail\n");
			return 0;
		}
		
		printk("Buf[0] : 0x%02X\n", Point_buf[0]);
		printk("Buf[1] : 0x%02X\n", Point_buf[1]);
		printk("Buf[2] : 0x%02X\n", Point_buf[2]);
		printk("Buf[3] : 0x%02X\n", Point_buf[3]);
		printk("Buf[4] : 0x%02X\n", Point_buf[4]);
		printk("Buf[5] : 0x%02X\n", Point_buf[5]);
		printk("Buf[6] : 0x%02X\n", Point_buf[6]);
		printk("Buf[7] : 0x%02X\n", Point_buf[7]);
		msleep(100);
		
	}
	return 0;	
}
static DEVICE_ATTR(point_report, 0644, sitronix_ts_point_report_show, NULL);

static ssize_t sitronix_ts_para_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 page_num;
	int len , i;
	
	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
			return -EIO;
	
	for(i = 0 ; i < 32 ; i++) {
		printk("%02x %02x %02x %02x %02x %02x %02x %02x ",isp_page_buf[16*i], isp_page_buf[16*i+1], isp_page_buf[16*i+2], isp_page_buf[16*i+3], isp_page_buf[16*i+4], isp_page_buf[16*i+5], isp_page_buf[16*i+6], isp_page_buf[16*i+7]);
		printk("%02x %02x %02x %02x %02x %02x %02x %02x\n",isp_page_buf[16*i+8], isp_page_buf[16*i+9], isp_page_buf[16*i+10], isp_page_buf[16*i+11], isp_page_buf[16*i+12], isp_page_buf[16*i+13], isp_page_buf[16*i+14], isp_page_buf[16*i+15]);
	}
	
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	return 0;
}
static DEVICE_ATTR(para, 0644 , sitronix_ts_para_show, NULL);


static int sitronix_ts_autotune_result_check(struct device *dev, int count_low, int count_high, int offset_low, int offset_high, int base_low, int base_high)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 page_num;
	int check_num,len, i, baseline;
	struct config_param_v0 *pROM_v0;
	struct config_param_v1 *pROM_v1;
	priv->autotune_result = true;

	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;

	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
	{
			led_flag=1;
			printk("[FW]sitronix_ts_autotune_result_check\r\n");		

			return -EIO;
	}		
	if(priv->struct_version == 0x00) {
		pROM_v0 = (struct config_param_v0 *)(&isp_page_buf);
	
		check_num = pROM_v0->x_chs + pROM_v0->y_chs + pROM_v0->k_chs;
	
		for(i = 0 ; i < check_num ; i++) {
			//Check Cnt
			if(i < pROM_v0->x_chs) {
				if(((pROM_v0->cnt[i] & 0x1F) < count_low) || ((pROM_v0->cnt[i] & 0x1F) > count_high)) {
					printk("invalid X[%d]_p1 = 0x%02X\n", i , pROM_v0->cnt[i] & 0x1F);
					priv->autotune_result = false;
				}
			} else if(i < pROM_v0->x_chs + pROM_v0->y_chs) {
				if(((pROM_v0->cnt[i] & 0x1F) < count_low) || ((pROM_v0->cnt[i] & 0x1F) > count_high)) {
					printk("invalid Y[%d]_p1 = 0x%02X\n", i-pROM_v0->x_chs , pROM_v0->cnt[i] & 0x1F);			
					priv->autotune_result = false;
				}
			} else {
				if(((pROM_v0->cnt[i] & 0x1F) < 0x06) || ((pROM_v0->cnt[i] & 0x1F) > 0x1E)) {
					printk("invalid K[%d]_p1 = 0x%02X\n", i-pROM_v0->x_chs-pROM_v0->y_chs , pROM_v0->cnt[i] & 0x1F);
					priv->autotune_result = false;
				}
			}
			//Check Offset
			if((pROM_v0->offset[i] < offset_low) || (pROM_v0->offset[i] > offset_high)) {
				if(i < pROM_v0->x_chs) {
					printk("invalid X[%d]_p2 = 0x%02X\n", i , pROM_v0->offset[i]);
					priv->autotune_result = false;
				} else if(i < pROM_v0->x_chs + pROM_v0->y_chs) {
					printk("invalid Y[%d]_p2 = 0x%02X\n", i-pROM_v0->x_chs , pROM_v0->offset[i]);
					priv->autotune_result = false;
				} else {
					printk("invalid K[%d]_p2 = 0x%02X\n", i-pROM_v0->x_chs-pROM_v0->y_chs , pROM_v0->offset[i]);
					priv->autotune_result = false;
				}
			}
			//Check Baseline 
			baseline = ((pROM_v0->baseline[i] & 0xFF) << 8) | ((pROM_v0->baseline[i] & 0xFF00) >> 8);
			if((baseline < base_low) || (baseline > base_high)) {
				if(i < pROM_v0->x_chs) {
					printk("invalid X[%d]_p3 = 0x%02X\n", i , baseline);
					priv->autotune_result = false;
				} else if(i < pROM_v0->x_chs + pROM_v0->y_chs) {
					printk("invalid Y[%d]_p3 = 0x%02X\n", i-pROM_v0->x_chs , baseline);
					priv->autotune_result = false;
				} else {
					printk("invalid K[%d]_p3 = 0x%02X\n", i-pROM_v0->x_chs-pROM_v0->y_chs , baseline);
					priv->autotune_result = false;
				}
			}
		}
		printk("Channel num : %u\n" , check_num);
	} else {
		pROM_v1 = (struct config_param_v1 *)(&isp_page_buf);
	
		check_num = pROM_v1->x_chs + pROM_v1->y_chs + pROM_v1->k_chs;
	for(i = 0 ; i < check_num ; i++) {
		//Check Cnt
			if(i < pROM_v1->x_chs) {
				if(((pROM_v1->cnt[i] & 0x1F) < count_low) || ((pROM_v1->cnt[i] & 0x1F) > count_high)) {
					printk("invalid X[%d]_p1 = 0x%02X\n", i , pROM_v1->cnt[i] & 0x1F);
				priv->autotune_result = false;
			}
			} else if(i < pROM_v1->x_chs + pROM_v1->y_chs) {
				if(((pROM_v1->cnt[i] & 0x1F) < count_low) || ((pROM_v1->cnt[i] & 0x1F) > count_high)) {
					printk("invalid Y[%d]_p1 = 0x%02X\n", i-pROM_v1->x_chs , pROM_v1->cnt[i] & 0x1F);			
				priv->autotune_result = false;
			}
		} else {
				if(((pROM_v1->cnt[i] & 0x1F) < 0x06) || ((pROM_v1->cnt[i] & 0x1F) > 0x1E)) {
					printk("invalid K[%d]_p1 = 0x%02X\n", i-pROM_v1->x_chs-pROM_v1->y_chs , pROM_v1->cnt[i] & 0x1F);
				priv->autotune_result = false;
			}
		}
		//Check Offset
			if((pROM_v1->offset[i] < offset_low) || (pROM_v1->offset[i] > offset_high)) {
				if(i < pROM_v1->x_chs) {
					printk("invalid X[%d]_p2 = 0x%02X\n", i , pROM_v1->offset[i]);
				priv->autotune_result = false;
				} else if(i < pROM_v1->x_chs + pROM_v1->y_chs) {
					printk("invalid Y[%d]_p2 = 0x%02X\n", i-pROM_v1->x_chs , pROM_v1->offset[i]);
				priv->autotune_result = false;
			} else {
					printk("invalid K[%d]_p2 = 0x%02X\n", i-pROM_v1->x_chs-pROM_v1->y_chs , pROM_v1->offset[i]);
				priv->autotune_result = false;
			}
		}
		//Check Baseline 
			baseline = ((pROM_v1->baseline[i] & 0xFF) << 8) | ((pROM_v1->baseline[i] & 0xFF00) >> 8);
		if((baseline < base_low) || (baseline > base_high)) {
				if(i < pROM_v1->x_chs) {
				printk("invalid X[%d]_p3 = 0x%02X\n", i , baseline);
				priv->autotune_result = false;
				} else if(i < pROM_v1->x_chs + pROM_v1->y_chs) {
					printk("invalid Y[%d]_p3 = 0x%02X\n", i-pROM_v1->x_chs , baseline);
				priv->autotune_result = false;
			} else {
					printk("invalid K[%d]_p3 = 0x%02X\n", i-pROM_v1->x_chs-pROM_v1->y_chs , baseline);
				priv->autotune_result = false;
			}
		}
	}	
		printk("Channel num : %u\n" , check_num);
	}	
	
	if(priv->always_update) {
		return priv->always_update;
	} else {
		return priv->autotune_result;
	}
}

static ssize_t sitronix_ts_autotune_en_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	char status;
	int count_low, count_high, offset_low, offset_high, base_low, base_high;
	int update = 0;	
	int enabled = 0;
	int wait = 0;
	if(CanUpdate) {


	priv->always_update = false;
	sscanf(buf, "%x %x 0x%02x 0x%02x 0x%02x 0x%02x %u %u",&enabled, &update, &count_low, &count_high, &offset_low, &offset_high, &base_low, &base_high);

	printk("Check parameter : count[%02X:%02X]\toffset[%02X:%02X]\tbaseline[%u:%u]\n", count_low, count_high, offset_low, offset_high, base_low, base_high);
	
	if(enabled) {
		priv->autotune_result = false;
		priv->always_update = update;
		sitronix_ts_set_autotune_en(client);
                mdelay(200);
		status = sitronix_ts_get_status(client);
		while(status && (count < 50000)) {
			status = sitronix_ts_get_status(client);
			wait++;
			if(wait % 16 == 15)
				printk("\n");
                        msleep(100); 
		}
                if(wait <= 50000) { 
			printk("\nAutoTune Done!\n");
			sitronix_ts_autotune_result_check(dev, count_low, count_high, offset_low, offset_high, base_low, base_high);
		} else {
			printk("\nAutoTune Timeout!\n");
		}
	}
	
	}
	return count;
}

static DEVICE_ATTR(autotune_en, 0644, NULL, sitronix_ts_autotune_en_store);

static ssize_t sitronix_ts_autotune_result_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	
	if(priv->always_update) {
		return sprintf(buf, "%u\n", priv->always_update);
	} else {
		return sprintf(buf, "%u\n", priv->autotune_result);
	}
}

static DEVICE_ATTR(autotune_result, 0644, sitronix_ts_autotune_result_show, NULL);

static ssize_t sitronix_ts_debug_check_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	char Point_buf[256];
	int i, j, ret;

	for(i = 0 ; i < priv->I2C_RepeatTime ; i++) {
		Point_buf[0] = priv->I2C_Offset;	//Set Reg. address to 0x10 for reading point report.
		if ((ret = i2c_master_send(client, Point_buf, 1)) != 1) {
			printk("I2C Send Data Fail\n");
			return 0;
		}

		//Read 8 byte point data from Reg. 0x10 set previously.
		if ((ret = i2c_master_recv(client, Point_buf, priv->I2C_Length)) != priv->I2C_Length) {
			printk("I2C Read Data Fail\n");
			return 0;
		}
		
		printk("===================debug report===================\n");
		for(j = 0 ; j < priv->I2C_Length ; j++) {
			printk("Buf[%d] : 0x%02X\n",j, Point_buf[j]);
		}		
		msleep(100);
	}
	return 0;	
}

static ssize_t sitronix_ts_debug_check_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	int Offset , Length , RepeatTime;

	sscanf(buf, "%u%u%u", &Offset, &Length, &RepeatTime);
	priv->I2C_Offset = Offset&0xFF;
	priv->I2C_Length = Length&0xFF;
	priv->I2C_RepeatTime = RepeatTime&0xFF;

	return count;
}
static DEVICE_ATTR(debug_check, 0644, sitronix_ts_debug_check_show, sitronix_ts_debug_check_store);

static ssize_t sitronix_ts_update_en_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
        u8 buf1[16],buf2[8],buf4[8];	
	int ret=0,rev,err,rc;
	char fwdata[25],fwdata1[1];
	struct file *fp,*fp1;
	mm_segment_t old_fs,old_fs1;

	printk("[BingReady ]updatefw\r\n");
	if(project_id==PROJECT_ID_CAP6)
	{
	
		if ((err = sitronix_ts_get_fw_revision(client, &rev)))
		{
			dev_err(&client->dev, "Unable to get FW revision!\n");		
		}

		org_rev=rev;

		buf1[0]=0x0;
		rc=i2c_master_send(st1232_ts->client, buf1,1);
		if(rc!=1)
			printk("[Bing]st1232  i2c_master_send  error\r\n");		
		rc= i2c_master_recv(st1232_ts->client, buf1,16);
		if(rc!=16)
			printk("[Bing]st1232  i2c_master_recv  error\r\n");
		
		printk("[Bing]updatefw :buf[0]=%d\r\n", buf1[0]);
		printk("[Bing]updatefw :buf[1]=%d\r\n", buf1[1]);
		printk("[Bing]updatefw :buf[2]=%d\r\n", buf1[2]);
		printk("[Bing]updatefw :buf[3]=%d\r\n", buf1[3]);
		printk("[Bing]updatefw :buf[4]=%d\r\n", buf1[4]);
		printk("[Bing]updatefw :buf[5]=%d\r\n", buf1[5]);
		printk("[Bing]updatefw :buf[6]=%d\r\n", buf1[6]);
		printk("[Bing]updatefw :buf[7]=%d\r\n", buf1[7]);
		printk("[Bing]updatefw :buf[8]=%d\r\n", buf1[8]);
		printk("[Bing]updatefw :buf[9]=%d\r\n", buf1[9]);
		printk("[Bing]updatefw :buf[10]=%d\r\n", buf1[10]);
		printk("[Bing]updatefw :buf[11]=%d\r\n", buf1[11]);
		printk("[Bing]updatefw :buf[12]=%d\r\n", buf1[12]);
		printk("[Bing]updatefw :buf[13]=%d\r\n", buf1[13]);
		printk("[Bing]updatefw :buf[14]=%d\r\n", buf1[14]);
		printk("[Bing]updatefw :buf[15]=%d\r\n", buf1[15]);
		org_tpversion=buf1[0];
		buf2[0]=0x10;
		rc = i2c_master_send(st1232_ts->client, buf2,1);
		if(rc!=1)
		{
			printk("[Bing]st1232  i2c_master_send  error_3\r\n");
			tp_reset();
		}	
/*
		if(rev>1431&&buf1[0]>=2)
			burnFw=1;
		else
		{
			CanUpdate=0;
			priv->Can_update=0;			
			burnFw=0;		
			return;
		}	*/
		if( !(hw_id==HW_ID_PVT||hw_id==HW_ID_PVT1)) 		
		{
			burnFw=0;
			CanUpdate=0;
			priv->Can_update=0;
			printk("[Bing]updatefw: This is not PVT, we can't update firmware\r\n");
			
			return; 
		}
		printk("[Bing]updatefw: rev=%d  vbatt=%d\r\n",rev,vbatt);
		//if(1)
		if( (hw_id==HW_ID_PVT||hw_id==HW_ID_PVT1)||burnFw==1 )		
		{
			mdelay(5000);
			printk("[bing]revision=%d vbatt=%d batt_capacity=%d\n",rev,vbatt,batt_capacity);
			if((vbatt>3450||batt_capacity>10)||burnFw==1)
			{
				printk("[bing]call nled\n");
				//rpc_mpp_config_led_state(0, 2,150,150,0,0);
				printk("[bing]Start to update touch firmware\n");
				priv->Can_update=1;
				CanUpdate=1;
				wake_lock(&tpwakelock_idle);
				printk("[bing]wake_lock\n");					
				memset(fwdata,0,sizeof(fwdata));
				burnFw=0;
				old_fs = get_fs();
				set_fs(KERNEL_DS);
				fp = filp_open("/data/touch_fw.txt", O_RDWR | O_CREAT, 0666);
				sprintf(fwdata,"R=%d V=%d C=%d\n",rev,vbatt,batt_capacity);
		   		if(IS_ERR(fp))
				    printk("[bing]Open file error...\n");
				 fp->f_op->write(fp,fwdata , 25 , &fp->f_pos);
  				 filp_close(fp,NULL);
				 set_fs(old_fs);
      				 ret=1;				 
 					 


				old_fs1 = get_fs();
				set_fs(KERNEL_DS);
				fp1 = filp_open("/data/updatefw_status", O_RDWR |  O_CREAT, 0666);
				sprintf(fwdata1,"0");
		   		if(IS_ERR(fp1))
				    printk("[bing]Open file error...\n");
				 fp1->f_op->write(fp1,fwdata1 , 1 , &fp1->f_pos);
  				 filp_close(fp1,NULL);
				 set_fs(old_fs1);					 
			}	
		}
		else
		{
			CanUpdate=0;
			priv->Can_update=0;
			ret=0;
		}	
	}	
	else
	{
		CanUpdate=0;	
		printk("[bing]This is not CAP6\n");
		priv->Can_update=0;
		ret=0;
	}	
	printk("[bing]Can_update=%d\n",priv->Can_update);
	
	return sprintf(buf, "%u\n", ret);	
}

static DEVICE_ATTR(update_enable, 0644, sitronix_ts_update_en_show, NULL);

static ssize_t sitronix_ts_update_fw(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	if( hw_id==HW_ID_PVT||hw_id==HW_ID_PVT1) 		
	{
	         burnFw=1;
	   	 CanUpdate=1;
		 priv->Can_update=1;
	}
	else
	{
		priv->Can_update=0;
		CanUpdate=0;
		burnFw=0;
	}	
	atomic_dec(&st1232_ts->irq_disable);
	disable_irq_nosync(st1232_ts->irq);
	dis_irq++;
	printk("[Bing]Disable touch irq in sitronix_ts_update_fw \r\n");		
	
	printk("[bing]sitronix_ts_update_fw:Can_update=%d  CanUpdate=%d  burnFw=%d\n",priv->Can_update,CanUpdate,burnFw);
	
	return sprintf(buf, "%u\n", priv->Can_update);	
}

static DEVICE_ATTR(update_fw, 0644, sitronix_ts_update_fw, NULL);


static ssize_t sitronix_ts_reset_fw(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int ret ,ver,err,rev;
	char fwdata1[5],test1[30],fwdata[1],test2[7];
	struct file *fp1,*fp;
	mm_segment_t old_fs1,old_fs;
	
	 ret=1;
	 
	if(CanUpdate==1)
	{
		wake_unlock(&tpwakelock_idle);
		printk("[bing]wake_unlock\n");
	}	
	//rpc_mpp_config_led_state(0, 1, 0, 0, 0, 0);	
	if(led_flag==1)
	{
		printk("[FW]update firmware but meet  i2c error!\n");
		//rpc_mpp_config_led_state(0, 0, 0, 0, 0, 0);
	}
	printk("[FW]led_flag=%d!\n",led_flag);

	//finish burn fw
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fp = filp_open("/data/updatefw_status", O_RDWR | O_CREAT, 0666);
	if(led_flag==1)
		sprintf(fwdata,"2");
	else if(CanUpdate==1)
		sprintf(fwdata,"1");		
	if(IS_ERR(fp))
	    printk("[bing]Open file error...\n");
	 fp->f_op->write(fp,fwdata , 1 , &fp->f_pos);
	 filp_close(fp,NULL);
	 set_fs(old_fs);	
	
	CanUpdate=0;
	burnFw=0;
	enable_irq(st1232_ts->irq);
	atomic_inc(&st1232_ts->irq_disable);
	++en_irq;
	
	if ((err = sitronix_ts_get_fw_version(client, &t_ver))) {
		printk("[Bing]Unable to get FW version...\r\n");
	} else {
		printk("[Bing]FW version=%X... irq_disable : %X\r\n",t_ver , atomic_read(&st1232_ts->irq_disable));

	}
	if ((err = sitronix_ts_get_fw_revision(client, &t_rev)))
	{
		dev_err(&client->dev, "Unable to get FW revision!\n");		
	}
	
	//write firmware version
	old_fs1 = get_fs();
	set_fs(KERNEL_DS);
	fp1 = filp_open("/data/show_touch_ver", O_RDWR | O_APPEND | O_CREAT, 0666);
	sprintf(test1,"R:%d, V:%d to R:%d V:%d(new)\n",org_rev,org_tpversion,t_rev,t_ver);
   	if(IS_ERR(fp1))
	    printk("[bing]Open file error...\n");
	 fp1->f_op->write(fp1,test1 , 30 , &fp1->f_pos);
  	 filp_close(fp1,NULL);
	 set_fs(old_fs1);	
	/*
	//#02#
	old_fs1 = get_fs();
	set_fs(KERNEL_DS);
	fp1 = filp_open("/data/touch_ver", O_RDWR | O_CREAT, 0666);
	sprintf(test2,"R:%d V:%d\n",rev,ver);
   	if(IS_ERR(fp1))
	    printk("[bing]Open file error...\n");
	 fp1->f_op->write(fp1,test2 ,7 , &fp1->f_pos);
  	 filp_close(fp1,NULL);
	 set_fs(old_fs1);	*/
	printk("[bing]ver=%d Enable touch irq: sitronix_ts_reset_fw=%d CanUpdate=%d\n",t_ver,ret,CanUpdate);
	return sprintf(buf, "%u\n", ret);	
}

static DEVICE_ATTR(reset_fw, 0644, sitronix_ts_reset_fw, NULL);


static int sitronix_ts_set_data_mode(struct i2c_client *client, u32 data_mode)
{
	char buf[2];
	int ret;

	//buf[0] = ((data_mode & 0x03) << 4 | 0x80);	//Set Reg. address to 0x2 for setting data mode.
	buf[0] = 0x02;
	buf[1] = ((data_mode & 0x03) << 4 | 0x08);	//Set Reg. address to 0x2 for setting data mode.
	if ((ret = i2c_master_send(client, buf, 2)) != 2)
		return -EIO;

	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	return 0;

}

static int sitronix_ts_read_raw_data(struct i2c_client *client, u8 *raw_data, u32 len)
{
	char buf[1];
	int ret;

	buf[0] = 0x40;	//Set Reg. address to 0x40 for reading raw data.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	//Read raw data of length len.
	if ((ret = i2c_master_recv(client, raw_data, len)) != len)
		return -EIO;

	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	return 0;

}

static ssize_t sitronix_ts_raw_data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	char data_buf[32*2];
	int i, ret;
	u32 raw, len;

	printk("st1232 data mode = %d\n" , priv->data_mode);
	if (priv->data_mode == 0)
		return 0;

	if ((ret = sitronix_ts_set_data_mode(client, priv->data_mode)))
		return 0;

	
	sitronix_ts_read_raw_data(client, data_buf, sizeof(data_buf));

	for(i = 0, len = 0; i < sizeof(data_buf); i += 2) {
		raw = ((((u32)data_buf[i]) << 8) | data_buf[i+1]);
		len += sprintf((buf + len), "%u ", raw);
	}

	/*
	if ((ret = sitronix_ts_set_data_mode(client, 0)))
		return 0;
	*/
	return len;
}

static ssize_t sitronix_ts_raw_data_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);

	sscanf(buf, "%u", &priv->data_mode);

	return count;
}
static DEVICE_ATTR(raw_data, 0644, sitronix_ts_raw_data_show, sitronix_ts_raw_data_store);

static ssize_t sitronix_ts_gpio_reset(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	
	printk("[Bing]sitronix_ts_gpio_reset\r\n");
	
	tp_reset_and_check(st1232_ts->client);
}

static DEVICE_ATTR(gpio_reset, 0644, sitronix_ts_gpio_reset, NULL);


static int sitronix_ts_set_proximity_en(struct i2c_client *client, u32 enable)
{
	char buf[2], data;
	int ret;

	// Get Device Control
	buf[0] = 0x2;
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	if ((ret = i2c_master_recv(client, buf, 1)) != 1)
		return -EIO;

	// Clear Proximity Bit
	data = buf[0];
	data &= 0xFB;
	
	// Write Device Control
	buf[0] = 0x02;
	buf[1] = ((enable & 0x01) << 2 | data);	
	if ((ret = i2c_master_send(client, buf, 2)) != 2)
		return -EIO;

	buf[0] = 0x10;	//Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
		return -EIO;

	return 0;
}

static ssize_t sitronix_ts_proximity_en_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	int proximity_en;

	sscanf(buf, "%u", &proximity_en);
	
	sitronix_ts_set_proximity_en(client, proximity_en);
	return count;
}
static DEVICE_ATTR(proximity_en, 0666, NULL, sitronix_ts_proximity_en_store);

static ssize_t sitronix_ts_tp_version(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);

	printk("[bing]tpversion=%x\r\n",tpversion);

	return sprintf(buf, "%d\n", tpversion);
}

static DEVICE_ATTR(tp_version, 0644, sitronix_ts_tp_version, NULL);

static struct attribute *sitronix_ts_attrs_v0[] = {
	&dev_attr_isp_ctrl.attr,
	&dev_attr_revision.attr,
	&dev_attr_struct_version.attr,
	&dev_attr_data_threshold.attr,
	&dev_attr_point_threshold.attr,
	&dev_attr_peak_threshold.attr,
	&dev_attr_mutual_threshold.attr,
	&dev_attr_range_filter.attr,
	&dev_attr_barX_filter.attr,
	&dev_attr_barY_filter.attr,
	&dev_attr_resolution.attr,
	&dev_attr_channel_num.attr,
	&dev_attr_para.attr,
	&dev_attr_autotune_en.attr,
	&dev_attr_autotune_result.attr,
	&dev_attr_point_report.attr,
	&dev_attr_debug_check.attr,
	&dev_attr_raw_data.attr,
	&dev_attr_update_enable.attr,
	&dev_attr_update_fw.attr,
	&dev_attr_reset_fw.attr,
	&dev_attr_gpio_reset.attr,
	&dev_attr_proximity_en.attr,
	&dev_attr_tp_version.attr,		
		
	NULL,
};



static struct attribute_group sitronix_ts_attr_group_v0 = {
	.name = "sitronix_ts_attrs",
	.attrs = sitronix_ts_attrs_v0,
};

static int sitronix_ts_create_sysfs_entry(struct i2c_client *client)
{
	//struct st1232_ts_data *priv = i2c_get_clientdata(client);
	int err;

	err = sysfs_create_group(&(client->dev.kobj), &sitronix_ts_attr_group_v0);
	if (err) {
		dev_dbg(&client->dev, "%s(%u): sysfs_create_group() failed!\n", __FUNCTION__, __LINE__);
	}
	
	err = sysfs_create_bin_file(&client->dev.kobj, &st1232_flash_bin_attr);
	if (err) {
		sysfs_remove_group(&(client->dev.kobj), &sitronix_ts_attr_group_v0);
		dev_dbg(&client->dev, "%s(%u): sysfs_create_bin_file() failed!\n", __FUNCTION__, __LINE__);
	}
	err = sysfs_create_bin_file(&client->dev.kobj, &st1232_panel_bin_attr);
	if (err) {
		sysfs_remove_group(&(client->dev.kobj), &sitronix_ts_attr_group_v0);
		sysfs_remove_bin_file(&(client->dev.kobj), &st1232_flash_bin_attr);
		dev_dbg(&client->dev, "%s(%u): sysfs_create_bin_file() failed!\n", __FUNCTION__, __LINE__);
	}
	return err;
}

static void sitronix_ts_destroy_sysfs_entry(struct i2c_client *client)
{
	//struct st1232_ts_data *priv = i2c_get_clientdata(client);
	sysfs_remove_bin_file(&client->dev.kobj, &st1232_panel_bin_attr);
	sysfs_remove_bin_file(&client->dev.kobj, &st1232_flash_bin_attr);
	sysfs_remove_group(&(client->dev.kobj), &sitronix_ts_attr_group_v0);

	return;
}

static int sitronix_ts_get_struct_version(struct i2c_client *client, u8 *rev)
{
	struct st1232_ts_data *priv = i2c_get_clientdata(client);
	u8 page_num;
	u32 len;
	
			printk("[Bing]sitronix_ts_get_struct_version \r\n");		

	page_num = ST1232_ROM_PARAM_ADR / ST1232_ISP_PAGE_SIZE;
	
	//Jump to ISP.
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	//Read data from ROM
	if ((len = st1232_isp_read_page(client, isp_page_buf, page_num)) < 0)
	{
				printk("[Bing]error \r\n");		
			return -EIO;
	}
	*rev = (u8)isp_page_buf[1];
			printk("[Bing]sitronix_ts_get_struct_version=%X \r\n",*rev);		

	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);

	return 0;
}

#if 0
static int sitronix_ts_internal_update(struct i2c_client *client)
{
	struct sitronix_ts_priv *priv = i2c_get_clientdata(client);
	int err = 1;
	int count, off;
	char buf[512];
	struct file *file = NULL;

	//Jump to ISP.
	printk("Jump to ISP\n");
	if(!priv->isp_enabled) {
		priv->isp_enabled = true;
		st1232_jump_to_isp(client);
	}
	
	// Update FW
	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);
	//file = filp_open("/media/mmcblk0p1/WinTek/touch_panel_T20_m0_QFN48.bin" , O_RDWR , 0644);
	file = filp_open("/system/flex/touch_panel_T20_m3_QFN40.bin" , O_RDWR , 0644);
	if(IS_ERR(file))
	{	
		printk("open file fail(%u)\n",PTR_ERR(file));
		//ISP Reset.
		priv->isp_enabled = false;
		st1232_isp_reset(client);
		return -1;
	}
	
	off = 0;
	
	while((count = file->f_op->read(file, (char *)buf , 512 , &file->f_pos)) > 0) {
		err = st1232_isp_write_flash(client, buf, off, count);
		if (err < 0) {
			printk("update fw fail\n");
			//ISP Reset.
			priv->isp_enabled = false;
			st1232_isp_reset(client);
			return err;
		}
		printk("Update FW : offset %d , length %d\n",off , count);
		off += count;
	}
	filp_close(file, NULL);
	set_fs(old_fs);
	printk("Update FW Finish\n");
	//ISP Reset.
	priv->isp_enabled = false;
	st1232_isp_reset(client);
	printk("ISP Reset\n");
	return err;

}
#endif
#endif //SITRONIX_ISP_ENABLED

extern int cci_in_CTS;  // by cci - for low memory killer
static ssize_t ts_left_show(struct device *dev, 
														struct device_attribute *attr, 
														char *buf)
{
	printk("[bing] show calibrated_left = %d\r\n", calibrated_left);
	return sprintf (buf, "%u\n", calibrated_left);
}

static ssize_t ts_left_store(	struct device *dev,
				      								struct device_attribute *attr,
				      								const char *buf, 
				      								size_t count)
{
	sscanf (buf, "%u", &calibrated_left);
	printk("[bing] store calibrated_left = %d\r\n", calibrated_left);
	touch_recalibration++;
	return count;
}

static DEVICE_ATTR(left, 0666, ts_left_show, ts_left_store);

static ssize_t ts_right_show(	struct device *dev, 
															struct device_attribute *attr, 
															char *buf)
{
	printk("[bing show calibrated_right = %d\r\n", calibrated_right);
	return sprintf (buf, "%u\n", calibrated_right);
}

static ssize_t ts_right_store(struct device *dev,
				      								struct device_attribute *attr,
				      								const char *buf, 
				      								size_t count)
{
	sscanf (buf, "%u", &calibrated_right);
	printk("[bing] store calibrated_right = %d\r\n", calibrated_right);
	touch_recalibration++;
	return count;
}

static DEVICE_ATTR(right, 0666, ts_right_show, ts_right_store);

static ssize_t ts_top_show(	struct device *dev, 
														struct device_attribute *attr, 
														char *buf)
{
	printk("[bing] show calibrated_top = %d\r\n", calibrated_top);
	return sprintf (buf, "%u\n", calibrated_top);
}

static ssize_t ts_top_store(struct device *dev,
				      							struct device_attribute *attr,
				      							const char *buf, 
				      							size_t count)
{
	sscanf (buf, "%u", &calibrated_top);
	printk("[bing] store calibrated_top = %d\r\n", calibrated_top);
	touch_recalibration++;
	return count;
}

static DEVICE_ATTR(top, 0666, ts_top_show, ts_top_store);

static ssize_t ts_bottom_show(struct device *dev, 
															struct device_attribute *attr, 
															char *buf)
{
	printk("[bing] show calibrated_bottom = %d\r\n", calibrated_bottom);
	return sprintf (buf, "%u\n", calibrated_bottom);
}

static ssize_t ts_bottom_store(	struct device *dev,
				      									struct device_attribute *attr,
				      									const char *buf, 
				      									size_t count)
{
	sscanf (buf, "%u", &calibrated_bottom);
	printk("[bing] store calibrated_bottom = %d\r\n", calibrated_bottom);
	touch_recalibration++;
	return count;
}

static DEVICE_ATTR(bottom, 0666, ts_bottom_show, ts_bottom_store);

static ssize_t x_axis_show(	struct device *dev, 
														struct device_attribute *attr, 
														char *buf)
{
	printk("[bing] show x_axis = %d\r\n", x_axis);
	return sprintf (buf, "%u\n", x_axis);
}

static ssize_t x_axis_store(struct device *dev,
				      							struct device_attribute *attr,
				      							const char *buf, 
				      							size_t count)
{
	sscanf (buf, "%u", &x_axis);
	printk("[bing] store x_axis = %d\r\n", x_axis);
	return count;
}

static DEVICE_ATTR(x_axis, 0664, x_axis_show, x_axis_store);

static ssize_t y_axis_show(	struct device *dev, 
														struct device_attribute *attr, 
														char *buf)
{
	printk("[bing] show y_axis = %d\r\n", y_axis);
	return sprintf (buf, "%u\n", y_axis);
}

static ssize_t y_axis_store(struct device *dev,
				      							struct device_attribute *attr,
				      							const char *buf, 
				      							size_t count)
{
	sscanf (buf, "%u", &y_axis);
	printk("[bing] store y_axis = %d\r\n", y_axis);
	return count;
}

static DEVICE_ATTR(y_axis, 0664, y_axis_show, y_axis_store);

static ssize_t last_x_show(	struct device *dev, 
														struct device_attribute *attr, 
														char *buf)
{
	printk("[bing] show last_x = %d\r\n", last_x);
	return sprintf (buf, "%u\n", last_x);
}

static DEVICE_ATTR(last_x, 0444, last_x_show, NULL);

static ssize_t last_y_show(	struct device *dev, 
														struct device_attribute *attr, 
														char *buf)
{
	printk("[bing] show last_y = %d\r\n", last_y);
	return sprintf (buf, "%u\n", last_y);
}

static DEVICE_ATTR(last_y, 0444, last_y_show, NULL);

static ssize_t ts_debug_flag_show(struct device *dev, 
															struct device_attribute *attr, 
															char *buf)
{
	printk("[bing] show debug_flag = %d\r\n", debug_flag);
	return sprintf (buf, "%u\n", debug_flag);
}

static ssize_t ts_debug_flag_store(	struct device *dev,
				      									struct device_attribute *attr,
				      									const char *buf, 
				      									size_t count)
{
	sscanf (buf, "%u", &debug_flag);
	printk("[bing] store debug_flag = %d\r\n", debug_flag);

	return count;
}

static DEVICE_ATTR(debug_flag, 0666, ts_debug_flag_show, ts_debug_flag_store);

static ssize_t ts_debug_message_show(struct device *dev, 
															struct device_attribute *attr, 
															char *buf)
{
	printk("[bing] show debug_message = 0x%x\r\n", debug_message);
	return sprintf (buf, "%x\n", debug_message);
}

static ssize_t ts_debug_message_store(	struct device *dev,
				      									struct device_attribute *attr,
				      									const char *buf, 
				      									size_t count)
{
	sscanf (buf, "%x", &debug_message);
	printk("[bing] store debug_message = 0x%x\r\n", debug_message);
	
	if (debug_message == 0)
	{
		ts_dump_tssc_register();
	}

	return count;
}

static DEVICE_ATTR(debug_message, 0666, ts_debug_message_show, ts_debug_message_store);

static ssize_t ts_tssc_reg_addr_show(struct device *dev, 
															struct device_attribute *attr, 
															char *buf)
{
	printk("[bing] show tssc_reg_addr = 0x%x\r\n", tssc_reg_addr);
	return sprintf (buf, "%x\n", tssc_reg_addr);
}

static ssize_t ts_tssc_reg_addr_store(	struct device *dev,
				      									struct device_attribute *attr,
				      									const char *buf, 
				      									size_t count)
{
	sscanf (buf, "%x", &tssc_reg_addr);
	printk("[bing] store tssc_reg_addr = 0x%x\r\n", tssc_reg_addr);

	return count;
}

static DEVICE_ATTR(tssc_reg_addr, 0666, ts_tssc_reg_addr_show, ts_tssc_reg_addr_store);

static ssize_t ts_tssc_reg_data_show(struct device *dev, 
															struct device_attribute *attr, 
															char *buf)
{
	printk("[bing] show tssc_reg_data = 0x%x\r\n", tssc_reg_data);
	return sprintf (buf, "%x\n", tssc_reg_data);
}

static ssize_t ts_tssc_reg_data_store(	struct device *dev,
				      									struct device_attribute *attr,
				      									const char *buf, 
				      									size_t count)
{
	sscanf (buf, "%x", &tssc_reg_data);
	printk("[bing] store tssc_reg_data = 0x%x\r\n", tssc_reg_data);

	return count;
}

static DEVICE_ATTR(tssc_reg_data, 0666, ts_tssc_reg_data_show, ts_tssc_reg_data_store);

static ssize_t ts_tssc_reg_read(struct device *dev, 
															struct device_attribute *attr, 
															char *buf)
{
	tssc_reg_read = readl(virt + tssc_reg_addr);
	printk("[bing tssc_reg[0x%x] = 0x%x\r\n", tssc_reg_addr, tssc_reg_read);
	return sprintf (buf, "%x\n", tssc_reg_read);
}

static DEVICE_ATTR(tssc_reg_read, 0666, ts_tssc_reg_read, NULL);

static ssize_t ts_tssc_reg_write(	struct device *dev,
				      									struct device_attribute *attr,
				      									const char *buf, 
				      									size_t count)
{
	sscanf (buf, "%x", &tssc_reg_write);
	writel(tssc_reg_addr, tssc_reg_write);
	printk("[bing] tssc_reg_write = 0x%x\r\n", tssc_reg_write);

	return count;
}

static DEVICE_ATTR(tssc_reg_write, 0666, NULL, ts_tssc_reg_write);

void tssc_hardware_init(void)
{
    /* save off the current operations (may have been set at runtime by panel
     * detection) */
    //uint32 opn = HWIO_TSSC_OPN_IN;
        /* Clear operations */
    writel(0x0, TSSC_REG(OPN));
    /* reset */
//    writel(TSSC_CTL_EN, TSSC_REG(CTL));
    writel(TSSC_CTL_EN | TSSC_CTL_SW_RESET, TSSC_REG(CTL));
//    msleep(10);	// Delay 10ms
    /* Enable the TSSC */
    writel(TSSC_CTL_STATE, TSSC_REG(CTL));
    /* Rev2.0 debounce logic bug fix - CR70897 */
    //HWIO_TSSC_TEST_1_OUT(HWIO_TSSC_TEST_1_GATE_DEBOUNCE_EN_BMSK);
    writel(TSSC_SI_STATE, TSSC_REG(SI));
    /* Finally, set valid operations which should kick everything off */
    writel(TSSC_OPN_STATE, TSSC_REG(OPN));
}
#if 0

static void ts_error_monitor(unsigned long arg)
{
	struct ts *ts = (struct ts *)arg;
	//printk("[bing]ts_error_monitor\r\n");

//	u32 status;

//	status = readl(TSSC_REG(STATUS));
//	if (status & TSSC_STS_ERROR_BMSK)
	{
//		printk("TOUCH: error occurred(%8x), reset TSSC\r\n", status);
//		debug_flag |= 0x02;
		tssc_hardware_init();
	}

	/* kick error monitor timer - to make sure it expires again(!) */
	//mod_timer(&ts->error_monitor,
//	jiffies + (TS_ERROR_MONITOR_TIMEOUT_S * HZ));
} // static void ts_error_monitor(unsigned long arg) END
#endif

static void st1232_ts_update_pen_state(struct st1232_ts_data *ts, int x, int y, int pressure)
{
	int i=0;
	if(NULL!=msm_7k_handset_get_input_dev())
		kpdev = msm_7k_handset_get_input_dev();
	else
	{
		printk("[bing]msm_7k_handset_get_input_dev=NULL \r\n");
		goto err;
	}
	
	if(debug_mask)
	    printk("[Bing][%s] x,y=(%d %d),last_xy=(%d %d)  p=%d  keyregister=%d ,tpversion=%d project_id=%d down_edge=%d #INT=%d\r\n", __func__, x, y, last_x,last_y,pressure,keyregister,tpversion,project_id,down_edge,interrupt_counter);
	//printk("[bing]kpdev=%d ,KEY_HOME=%d KEY_SEARCH=%d KEY_BACK=%d KEY_MENU=%d ,pressure=%d\r\n",kpdev,KEY_HOME,KEY_SEARCH,KEY_BACK,KEY_MENU,pressure);
	if(y<=down_edge&&y>=0)
	{	
		if(interrupt_counter==1||pressure==0)
			logger_write(0,2,"TP","update:x,y=(%d, %d) #INT=%d \n", x,y,interrupt_counter);	

        	for (i = 0; i < st1232_ts->prev_touches; i++) {
        		input_report_abs(st1232_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
        		input_report_abs(st1232_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
        		input_mt_sync(st1232_ts->input_dev);
        	}
        	st1232_ts->prev_touches = 0;
        	input_sync(st1232_ts->input_dev);
	}
	if(project_id==PROJECT_ID_K4H)
	  {
 		if(y>down_edge)
 	 	{
			if(x>0&&x<=80)
			{	
				input_report_key(kpdev, KEY_HOME, pressure);
				if(interrupt_counter==1||pressure==0)
					logger_write(0,2,"TP","KEY_HOME, Press=%d #INT=%d\n",pressure,interrupt_counter);				
				if(debug_mask)
				   printk("[bing]KEY_HOME \r\n");
			}
			else if(x>80&&x<=150)
			{
				input_report_key(kpdev, KEY_SEARCH, pressure);
				if(interrupt_counter==1||pressure==0)				
					logger_write(0,2,"TP","KEY_SEARCH, Press=%d #INT=%d\n",pressure,interrupt_counter);				
				if(debug_mask)				
				   printk("[bing]KEY_SEARCH \r\n");
			}
			else if(x>150&&x<=260)
			{	
				input_report_key(kpdev, KEY_BACK, pressure);						
				if(interrupt_counter==1||pressure==0)
					logger_write(0,2,"TP","KEY_BACK, Press=%d #INT=%d\n",pressure,interrupt_counter);				
				if(debug_mask)
				   printk("[bing]KEY_BACK \r\n");
			}
			else if(x>260)
			{
				input_report_key(kpdev, KEY_MENU, pressure);				
				if(interrupt_counter==1||pressure==0)				
					logger_write(0,2,"TP","KEY_MENU, Press=%d #INT=%d\n",pressure,interrupt_counter);				
				if(debug_mask)
				   printk("[bing]KEY_MENU \r\n");
			}
			last_key_x=x;
			last_key_y=y;			
 	 	}
        	input_sync(kpdev);		
	    }	
	else
	{
		 if(tpversion>=1)
		{
			if(keyregister==1)
			{	
				input_report_key(kpdev, KEY_HOME, pressure);	
				if(debug_mask)
				   printk("[bing]KEY_HOME,keyregister=%d \r\n",keyregister);
			}
			else if(keyregister==2)
			{	
				input_report_key(kpdev, KEY_SEARCH, pressure);
				if(debug_mask)
				   printk("[bing]KEY_SEARCH,keyregister=%d \r\n",keyregister);
			}
			else if(keyregister==4)
			{	
				input_report_key(kpdev, KEY_BACK, pressure);		
				if(debug_mask)
				   printk("[bing]KEY_BACK,keyregister=%d \r\n",keyregister);
			}
			else if(keyregister==8)
			{
				input_report_key(kpdev, KEY_MENU, pressure);
				if(debug_mask)
				   printk("[bing]KEY_MENU,keyregister=%d \r\n",keyregister);
			}
        		input_sync(kpdev);
		}	
 		else if(tpversion==0)
 		{
	   		  if(project_id==PROJECT_ID_CAP6||project_id==PROJECT_ID_CAP2)
	  		  {
 	 			if(y>down_edge)			
 	 			{
					if(x>0&&x<=80)
					{	
						input_report_key(kpdev, KEY_HOME, pressure);	
						if(debug_mask)
						   printk("[bing]KEY_HOME \r\n");
					}
					else if(x>80&&x<=150)
					{
						input_report_key(kpdev, KEY_SEARCH, pressure);
						if(debug_mask)
						   printk("[bing]KEY_SEARCH \r\n");
					}
					else if(x>150&&x<=260)
					{	
						input_report_key(kpdev, KEY_BACK, pressure);		
						if(debug_mask)
						   printk("[bing]KEY_BACK \r\n");
					}
					else if(x>260)
					{
						input_report_key(kpdev, KEY_MENU, pressure);
						if(debug_mask)
						   printk("[bing]KEY_MENU \r\n");
					}
				}
 	  		  }	
          		  input_sync(kpdev);
		}
	}
	
err:
	return;	
}
static void st1232_ts_timer(unsigned long arg)
{
     if(release_press_flag==0)
     {
	if(debug_mask)
	   printk("[Bing][%s]bak_p1,p2=(%d %d)  last_x,y=(%d %d)  last_x2,y2=(%d %d) press_point=%d  keyregister=%d  p=(%d %d)  #INT=%d\r\n", __func__,bak_p1,bak_p2,last_x,last_y,last_x2,last_y2,keyregister,press_point,press1,press2,interrupt_counter);
	logger_write(0,2,"TP","Releaser:last_x,y=(%d,%d) #INT=%d \n", last_x,last_y,interrupt_counter);	

	if(bak_p1==1)
		st1232_ts_update_pen_state(st1232_ts, last_x, last_y, 0);
	if(bak_p2==1)
		st1232_ts_update_pen_state(st1232_ts, last_x2, last_y2, 0);	
	if(keyregister!=0)
		st1232_ts_update_pen_state(st1232_ts, 0, 0, 0);	
	
	touchedge=0;
	keylocation=0;
	keyregister=0;
	interrupt_counter=0;
	index=0;
	press1=0;
	press2=0;
	runone=0;
	press_point=0;
        release_press_flag=1;
     }	
}
static void proximity_timer(unsigned long arg)
{
   //	printk("[Bing][%s]proximity_timer:  receive_phone=%d phone_ap=%d\r\n", __func__,receive_phone,phone_ap);
        input_report_abs(fpdev, ABS_DISTANCE, (receive_phone==1)?0:1);
	input_sync(fpdev);    		    	   
	if(enablep)
  		mod_timer(&st1232_ts->proximity_timer,jiffies + msecs_to_jiffies(report_value));   
}

static void release_press(void)
{
	release_press_flag=1;
	if(debug_mask)
	   printk("[Bing][%s]bak_p1,p2=(%d %d)  last_x,y=(%d %d)  last_x2,y2=(%d %d) press_point=%d  keyregister=%d  p=(%d %d)  #INT=%d\r\n", __func__,bak_p1,bak_p2,last_x,last_y,last_x2,last_y2,keyregister,press_point,press1,press2,interrupt_counter);
	logger_write(0,2,"TP","Releaser:last_x,y=(%d,%d) #INT=%d \n", last_x,last_y,interrupt_counter);	
	if(bak_p1==1)
		st1232_ts_update_pen_state(st1232_ts, last_x, last_y, 0);
	if(bak_p2==1)
		st1232_ts_update_pen_state(st1232_ts, last_x2, last_y2, 0);	
	if(keyregister!=0)
		st1232_ts_update_pen_state(st1232_ts, 0, 0, 0);	
	
	bak_p1=0;
	bak_p2=0;
	touchedge=0;
	keylocation=0;
	keyregister=0;
	interrupt_counter=0;
	index=0;
	press1=0;
	press2=0;
	runone=0;
	press_point=0;
}
static void ts_timer(unsigned long arg)
{
	struct ts *ts = (struct ts *)arg;
	//if (!(BKL_EN==0&&screen_flag==1))
	if (BKL_EN==1)
	{
	if (debug_message)
	{
		switch((debug_message & TS_TIMER_MASK) >> TS_TIMER_SHIFT)
		{
			case 1:
				printk("pen_down:%d\r\n", pen_down);
			break;
			case 2:
			break;
			default:
			break;
		}
	}
	
#if PRINT_COUNTER
	end_timer = jiffies;
	if (jiffies_to_msecs(end_timer - ini_timer) > TS_PENUP_TIMEOUT_MS)
	{
		printk("TOUCH: out:%d m%d\r\n", jiffies_to_msecs(end_timer - ini_timer), __LINE__);
	}
	ini_timer = jiffies;
#endif

	counter_OOT = 0;
#if PRINT_X_Y_VALUE
	counter_TSSC = 0;
#endif
	//if (pen_down)
	{

#if TCXO_SHUTDOWN_WORKAROUND
		// for Tcxo shutdown
		if( bak_lx > 0 || bak_ly > 0 ) 
		{
			ts_update_pen_state(ts, bak_lx, bak_ly, z1_axis);
		}
		// for Tcxo shutdown
		bak_lx = 0;
		bak_ly = 0;
#endif
		key_pressed=0;
		last_key=0;
		ts_update_pen_state(ts, ts_last_x, ts_last_y, 0);
		//cci_in_CTS = 0;  // by cci - for low memory killer
		interrupt_counter=0;
		pen_down = 0;
		touchedge=0;
		key_pressed=0;
	        last_x_axis=0;
 		last_y_axis=0;
		bak_x=0;
		bak_y=0;
		finger_move=0;
		last_key=0;
		//printk("[bing]Timer  penup:touchedge=%d ts_last_x,y=(%d %d)  coord_right/left=(%d %d)",touchedge,ts_last_x,ts_last_y,coord_right,coord_left);
	}
    }


} // static void ts_timer(unsigned long arg) END
static void ts_TSSC_work_func(struct work_struct *work)
{

	u32 avg12, avg34, lx, ly;
	u32 num_op, num_samp;
	u32 status, ctl;
	u32	dbg_msg_ts_tssc_work_func;

#if	X_Y_AXIS_QUEUE
#else
	#if	FEATURE_TSSC_D1_FILTER
	static int xbuf_D1[2] = {0};
	static int ybuf_D1[2] = {0};
	#endif
#endif
	struct ts *ts = container_of(work, struct ts, work);

	FUNC_ENTER;

	spin_lock(&acct_lock);	
	//printk("[Bing] interrupt_counter=%d\r\n",interrupt_counter);	


	if (BKL_EN==0)
	{
		//printk("[Bing] work_func:BKL_EN=%d\r\n",BKL_EN);	
		goto out;
	}
	/*if (!set_highest_priority)
	{
		// Set to a high priority
// by cci
//		set_user_nice(current, -10);
		struct sched_param s;
		struct cred *new = prepare_creds();
		cap_raise(new->cap_effective, CAP_SYS_NICE);
		commit_creds(new);
		s.sched_priority = 70;
		if ((sched_setscheduler(current, SCHED_FIFO, &s)) < 0)
			printk("[CCI] sched_setscheduler failed for Touch Thread !!\n");
		set_highest_priority = 1;
	}
*/
	#if	FEATURE_TSSC_D1_FILTER
	if (pen_down == 0)
	{
		xbuf_D1[0] = xbuf_D1[1] = ybuf_D1[0] = ybuf_D1[1] = 0;
	}
	#endif

	pen_down++;

	status = readl(TSSC_REG(STATUS));
	avg12 = readl(TSSC_REG(AVG12));
	avg34 = readl(TSSC_REG(AVG34));
	ctl = readl(TSSC_REG(CTL));

	x_axis_avg_raw = avg12 & 0xFFFF;
	y_axis_avg_raw = avg12 >> 16;
	z1_axis_avg_raw = avg34 & 0xFFFF;
	z2_axis_avg_raw = avg34 >> 16;
	if( debug_mask)
		printk("[Bing1] x_axis_avg_raw=%d  y_axis_avg_raw=%d z1_axis_avg_raw=%d\r\n",x_axis_avg_raw,y_axis_avg_raw,z1_axis_avg_raw);	
	if ((pen_down % 100) == 0)
	{
		/* kick error monitor timer - to make sure it expires again(!) */
//		mod_timer(&ts->error_monitor,
	//	jiffies + (TS_ERROR_MONITOR_TIMEOUT_S * HZ));
	}

	if ((x_axis_avg_raw == 0) || (y_axis_avg_raw == 0))
	{
		//printk("TOUCH: (x_axis_avg_raw == 0) || (y_axis_avg_raw == 0)!\r\n");
		goto out;
	}

	if (debug_flag != 1)
	{	
#if X_Y_AXIS_QUEUE
		add_x_y_axis_avg_raw_to_queue(x_axis_avg_raw, y_axis_avg_raw);
		if (x_axis_avg_raw_queue_index < 3)
		{
		 	//printk("TOUCH: x_axis_avg_raw_queue_index < 3!\r\n");
		 	goto out;
		}
		else
		{
			x_axis = x_axis_avg_raw_queue[1];
			y_axis = y_axis_avg_raw_queue[1];
		}
#else
		#if	FEATURE_TSSC_D1_FILTER
		{
			point points[3] = {{x_axis_avg_raw, y_axis_avg_raw}, {xbuf_D1[0] == 0 ? x_axis_avg_raw : xbuf_D1[0], ybuf_D1[0] == 0 ? y_axis_avg_raw : ybuf_D1[0]},
                         {xbuf_D1[1], ybuf_D1[1]}};
			point res = pbwa(points);
			xbuf_D1[1] = xbuf_D1[0];
			ybuf_D1[1] = ybuf_D1[0];
        
			xbuf_D1[0] = x_axis_avg_raw;
			ybuf_D1[0] = y_axis_avg_raw;

			x_axis = res.x;
			y_axis = res.y;
			//printk("[bing3] x_axis=%d y_axis=%d\r\n",x_axis,y_axis);
			
		}
		#else
		{
			x_axis = x_axis_avg_raw;
			y_axis = y_axis_avg_raw;
		}
		#endif
#endif
#if 0
		if (z1_axis_avg_raw)
		{
			u32 x_axis_tmp;
			u32 y_axis_tmp;
			
			if (x_axis_avg_raw > coord_right)
			{
				x_axis_tmp = coord_right;
			}
			else
			{
				x_axis_tmp = x_axis_avg_raw;
			}
			
			if (y_axis_avg_raw > coord_bottom)
			{
				y_axis_tmp = coord_bottom;
			}
			else
			{
				y_axis_tmp = y_axis_avg_raw;
			}
			
			if (!x_diff) x_diff = 1;
			if (!y_diff) y_diff = 1;
			
			#if 0
			z1_axis = (((coord_right - x_axis_tmp) * 234)  / x_diff) + 
								(((coord_bottom - y_axis_tmp) * 280) / y_diff) +
								z1_axis_avg_raw	+
								z2_axis_avg_raw;
			#else
			z1_axis = (x_axis_avg_raw * ((z2_axis_avg_raw * 102 / z1_axis_avg_raw) - 102)) / 906;
			//z1_axis = ((x_axis_avg_raw * z2_axis_avg_raw) / z1_axis_avg_raw);
			//printk("z:%d, %d\r\n", z1_axis, 330 - z1_axis);
			z1_axis = 320 - z1_axis;
			#endif

			if (z1_axis <= 0)
				z1_axis = 0;

			if (z1_axis >= 255)
				z1_axis = 255;
		}
		else
		{
			z1_axis = 0;
		}
#endif		
	}
	if (touch_calibration_enable)
	{
		if ((calibrated_left != 0) 	&&
				(calibrated_right != 0) &&
				(calibrated_top != 0) 	&&
				(calibrated_bottom != 0)&&
				(touch_recalibration == 4))
		{
			//printk("[bing2](%d %d %d %d) touch_recalibration=%d\r\n", coord_left,coord_right,coord_top,coord_bottom,touch_recalibration);
			calibrate_left_diff=calibrated_left-coord_right;//785-782
			calibrate_right_diff=calibrated_right-coord_left;//247-250
			calibrate_top_diff=calibrated_top-coord_bottom;//853-856
			calibrate_bottom_diff=calibrated_bottom-coord_top;//237-233
			
			coord_left  = calibrated_right+10;
			coord_right = calibrated_left-10;
			coord_top   = calibrated_bottom;
			coord_bottom= calibrated_top-10;
					
			//printk("[bing3]calibrated:(%d %d %d %d) diff=(%d %d %d %d) touch_recalibration=%d\r\n", coord_left,coord_right,coord_top,coord_bottom,calibrate_left_diff,calibrate_right_diff,calibrate_top_diff,calibrate_bottom_diff,touch_recalibration);
			
			//printk("[bing4]ts->x_max=%d,  ts->y_max=%d\r\n", ts->x_max, ts->y_max);
			
			x_diff = coord_right - coord_left;
			y_diff = coord_bottom - coord_top;
			ts->x_max = x_diff; 			//X_MAX - X_MIN;
			ts->y_max = y_diff; 			//Y_MAX - Y_MIN;
			
			//printk("[bing5]new ,ts->x_max=%d,  ts->y_max=%d\r\n", ts->x_max, ts->y_max);
			
			touch_recalibration = 0;
			x_y_axis_coord_restore = 0;
		}
	}
	else
	{
		if (!x_y_axis_coord_restore)
		{
			restore_default_coord();
		
			x_diff = coord_right - coord_left;
			y_diff = coord_bottom - coord_top;
			ts->x_max = x_diff; 			//X_MAX - X_MIN;
			ts->y_max = y_diff; 			//Y_MAX - Y_MIN;
			x_y_axis_coord_restore = 1;
			//printk("[bing]test ts->x_max=%d, ts->y_max=%d\r\n", ts->x_max, ts->y_max);
			
		}
	}
	//printk("[bing3a]calibrated:(%d %d %d %d) diff=(%d %d %d %d) touch_recalibration=%d\r\n", coord_left,coord_right,coord_top,coord_bottom,calibrate_left_diff,calibrate_right_diff,calibrate_top_diff,calibrate_bottom_diff,touch_recalibration);
	
	if (debug_message)
	{
		dbg_msg_ts_tssc_work_func = (debug_message & TS_TSSC_WORK_FUNC_MASK) >> TS_TSSC_WORK_FUNC_SHIFT;
		
		if ((dbg_msg_ts_tssc_work_func & 0x1) == 0x1)
			printk("x:%d, y:%d, z:%d, z1:%d, z2:%d\r\n", x_axis_avg_raw, y_axis_avg_raw, z1_axis, z1_axis_avg_raw, z2_axis_avg_raw);
		
		if ((dbg_msg_ts_tssc_work_func & 0x2) == 0x2)
			printk("x^:%d, y^:%d\r\n", x_axis, y_axis);
			
		if ((dbg_msg_ts_tssc_work_func & 0x4) == 0x4)
		{
			#if	FEATURE_TSSC_D1_FILTER
			printk("xbuf_D1[0]=%d, xbuf_D1[1]=%d\r\n", xbuf_D1[0], xbuf_D1[1]);
			printk("ybuf_D1[0]=%d, ybuf_D1[1]=%d\r\n", ybuf_D1[0], ybuf_D1[1]);
			#endif
		}

		if ((dbg_msg_ts_tssc_work_func & 0x8) == 0x8)
			read_x_y_axis_raw_value_2_buffer();
		
		if ((dbg_msg_ts_tssc_work_func & 0x10) == 0x10)
			ts_dump_tssc_register();
	}

	/* For pen down make sure that the data just read is still valid.
	 * The DATA bit will still be set if the ARM9 hasn't clobbered
	 * the TSSC. If it's not set, then it doesn't need to be cleared
	 * here, so just return.
	 */
	if (!(ctl & TSSC_CTL_DATA))
	{
		printk("TOUCH: TSSC_CTL_DATA = 0!\r\n");
		goto out;
	}

	/* Data has been read, OK to clear the data flag */
	writel(TSSC_CTL_STATE, TSSC_REG(CTL));

	/* Valid samples are indicated by the sample number in the status
	 * register being the number of expected samples and the number of
	 * samples collected being zero (this check is due to ADC contention).
	 */
	num_op = (status & TSSC_STS_OPN_BMSK) >> TSSC_STS_OPN_SHIFT;
	num_samp = (status & TSSC_STS_NUMSAMP_BMSK) >> TSSC_STS_NUMSAMP_SHFT;
	
	//printk("@[msm_touch.c][%d] num_op:%d, num_samp:%d\r\n", __LINE__, num_op, num_samp);
	//printk("@[msm_touch.c][%d] error:%d \r\n", __LINE__, (status & TSSC_STS_ERROR_BMSK));

	if ((num_op == TSSC_NUMBER_OF_OPERATIONS) && (num_samp == 0))
	{
		/* TSSC can do Z axis measurment, but driver doesn't support
		 * this yet.
		 */

		/*
		 * REMOVE THIS:
		 * These x, y co-ordinates adjustments will be removed once
		 * Android framework adds calibration framework.
		 */
		 //printk("[bingaaa] x_axis=%d, y_axis=%d \r\n", x_axis, y_axis);

		 if ( !Is_meet_fixed_pointer_filter(4,8)&&interrupt_counter>2
//		 	  || (pen_down <= 2)
		 	  )
		 {
		 		// printk("TOUCH: Out of range(%d %d %d)!\r\n", x_axis, y_axis, z1_axis);
		 		goto out;
		 }
		 
#if REPORT_POINT_LOCATION
		{
			u32 x, y;
			
			if (x_diff == 0) x_diff = 1;
			if (y_diff == 0) y_diff = 1;
			
			x = ((x_axis - coord_left) * ts->x_max) / x_diff;
			y = ((y_axis - coord_top ) * ts->y_max) / y_diff;
			
			lx = ts->x_max - x;
			ly = ts->y_max - y;
			
			//printk("x_diff:%d, y_diff:%d\r\n", x_diff, y_diff);
			//printk("coord_left:%d, coord_right:%d, coord_top:%d, coord_bottom:%d\r\n", coord_left, coord_right, coord_top, coord_bottom);
			//printk("x:%d, y:%d\r\n", x, y);
			//printk("ts->x_max:%d, ts->y_max:%d\r\n", ts->x_max, ts->y_max);
			//printk("[bing333] lx=%u, ly=%u \r\n", lx, ly);

		}
#else // QCT original report method
		
		if (x_diff == 0) x_diff = 1;
		if (y_diff == 0) y_diff = 1;
		
#if X_Y_AXIS_DECREASE
		if (touch_calibration_enable)
		{
			//printk("[bing6]touch_calibration_enable:  lx=%u, ly=%u x_axis=%d y_axis=%d \r\n", lx, ly,x_axis,y_axis);
			if(coord_right>x_axis)
				lx = ((coord_right - x_axis) * default_x_diff)/ x_diff;
			else
				lx = 0;
			if(coord_bottom>y_axis)
				ly = ((coord_bottom - y_axis) * default_y_diff) / y_diff;	
			else
				ly = 0;			
			//printk("[bing7] lx=%u, ly=%u  coord_right=%d coord_bottom=%d coord_top=%d coord_left=%d default_x,y_diff=(%d %d) x,y_diff=(%d %d)\r\n", lx, ly,coord_right,coord_bottom,coord_top,coord_left,default_x_diff,default_y_diff,x_diff,y_diff);
			
		}
		else
		{
			if(coord_right>x_axis)
				lx = coord_right - x_axis;				
			else
				lx = 0;
			if(coord_bottom>y_axis)
				ly = coord_bottom - y_axis;
			else
				ly = 0;
			
			//printk("[bing7] lx=%u, ly=%u xy_axis=(%d %d) coord_right=%d coord_bottom=%d default_x,y_diff=(%d %d) x,y_diff=(%d %d)\r\n", lx, ly,x_axis,y_axis,coord_right,coord_bottom,default_x_diff,default_y_diff,x_diff,y_diff);
			
		}
#else
		lx = x_axis;
		ly = y_axis;
#endif
		ts_last_x = lx;
		ts_last_y = ly;

		//printk("[bing9]ts_last_x,y=(%d %d)\r\n",ts_last_x,ts_last_y);

		//printk("[bing8] lx, ly=(%d %d),  x_axis,y_axis=(%d %d)  (%d  %d %d %d) trigger edge=%d\r\n",lx,ly,x_axis,y_axis,coord_right,coord_left,coord_bottom,coord_top,touchedge);
		if( debug_mask)
 	 	    printk("[bing]y_axis=%d coord_top=%d finger_move=%d key_location=%d key_pressed=%d\r\n",y_axis,coord_top, finger_move,key_location,key_pressed);

 	 	if(y_axis> coord_top)
		{
			touchedge=1;
			if(key_location==1)
				finger_move=1;
		}			
		if (y_axis< coord_top)//key location
		{
			key_location=1;
			if(touchedge==1)
			{
				//printk("[bing9]touchedge=%d  xy_axis=(%d %d) coord_top=%d\r\n",touchedge,x_axis,y_axis,coord_top);			
				ts_update_pen_state(ts, x_axis, y_axis, 0);
				touchedge=0;
			}
		}


 	 	//printk("[bing]y_axis=%d coord_top=%d finger_move=%d key_location=%d\r\n",y_axis,coord_top, finger_move,key_location);
		if (finger_move)//out bottom  edge		
		{
			//printk("[bing]key_pressed=%d   bak_xy=(%d %d) \r\n",key_pressed,bak_x,bak_y);		
			if(key_pressed==1)
			{	
				last_key=0;
				ts_update_pen_state(ts, bak_x, bak_y, 0);
				key_pressed=0;
				finger_move=0;
			}
		}

#if TCXO_SHUTDOWN_WORKAROUND
		// for Tcxo shutdown
		if( pen_down == 1 )
		{
			bak_lx = lx;
			bak_ly = ly;
		}
		else if( pen_down > 1 ) 
		{
			if( bak_lx > 0 || bak_ly > 0 ) 
			{
				ts_update_pen_state(ts, bak_lx, bak_ly, 1);
				bak_lx = 0;
				bak_ly = 0;
				last_x_axis = x_axis;
				last_y_axis = y_axis;
			}
		}
#endif


#endif
		//printk("[bing](%d, %d) (%d, %d)  (%d, %d)\r\n", x_axis, y_axis,last_x_axis,last_y_axis, lx, ly);

#if PRINT_X_Y_VALUE
		counter_TSSC++;
		if ((counter_TSSC%10) == 1)
		{
			//printk("(%d, %d), (%d, %d), (%d, %d)\r\n", x_axis, y_axis, x, y, lx, ly);
			if (counter_TSSC > 1000)
				counter_TSSC = 0;
		}
#endif

		//if (!Is_out_of_tolerance(lx, ly) || (pen_down == 1))
		if (!Is_out_of_tolerance(lx, ly) || (counter_OOT >= OOT_MAX))
		{		//printk("[bingcc](%d, %d) (%d, %d)\r\n", x_axis, y_axis, lx, ly);

			if (counter_OOT >= OOT_MAX)
			{
				int i=0;
				for (i=0; i < OOT_MAX-1; i++)
				{
					//printk("TOUCH: ts_update_pen_state_OOT(%d %d %d)\r\n", x_OOT_buf[i], y_OOT_buf[i], z1_axis);
					ts_update_pen_state(ts, x_OOT_buf[i], y_OOT_buf[i], 1);
				}
			}
			
			counter_OOT = 0;
			pen_OOR = 0;
			ts_update_pen_state(ts, lx, ly, 1);
			last_x_axis = x_axis;
			last_y_axis = y_axis;
			
		}
		else
		{
			//printk("TOUCH: Is_out_of_tolerance(%d %d) %d, %d pen_down=%d counter_OOT=%d\r\n", lx, ly, last_x, last_y, pen_down, counter_OOT);
		}
		
#if PRINT_COUNTER		
		end_m1 = jiffies;
		if (jiffies_to_msecs(end_m1 - ini_m1) >= TS_PENUP_TIMEOUT_MS)
		{
			printk("TOUCH: %d m%d\r\n", jiffies_to_msecs(end_m1 - ini_m1), __LINE__);
		}
		ini_m1 = jiffies;
#endif
	} 
	else
	{
		counter_OOT = 0;
		pen_OOR++;
		//printk("(%d, %d) is out of range\r\n", x_axis, y_axis);
		printk("Ignored interrupt: {%d, %d}, op = %d, samp = %d\r\n",
						x_axis, y_axis, num_op, num_samp);
	}

out:

#if 0
	//printk("ctl:0x%8x\r\n", ctl);
	if ( ctl & TSSC_CTL_INTR_FLAG2 )
	{
		printk("@[msm_touch.c][%d] penup IRQ\r\n", __LINE__);
	}
#endif
	
#if 0
	if ( ctl & TSSC_CTL_INTR_FLAG2 )
	{
		pen_down = 0;
		ts_update_pen_state(ts, 0, 0, 0);
		printk("[msm_touch.c][%d] penup\r\n", __LINE__);
	}
	else if (pen_OOR == 1)
	{
		ts_update_pen_state(ts, 0, 0, 0);
		printk("[msm_touch.c][%d] penup of OOR\r\n", __LINE__);
	}
	else
	{
		printk("pen_OOR: %d\r\n", pen_OOR);
	}
#endif
	
	//printk("OPN:%8x\r\n", readl(TSSC_REG(OPN)));
#if	TOUCH_RESOLUTION_12_BIT
	if ((readl(TSSC_REG(OPN)) & TSSC_OPN_RESOLUTION1_BMSK) != 0x2)
#else	//TOUCH_RESOLUTION_10_BIT
	if ((readl(TSSC_REG(OPN)) & TSSC_OPN_RESOLUTION1_BMSK) != 0x1)
#endif
	{
		printk("[msm_touch.c] TSSC_OPN:0x%x\r\n", readl(TSSC_REG(OPN)));
		printk("[msm_touch.c] writel(TSSC_OPN_STATE, TSSC_REG(OPN))\r\n");
		writel(TSSC_OPN_STATE, TSSC_REG(OPN));
	}

	setup_timer(&ts->timer, ts_timer, (unsigned long)ts);

	/* kick pen up timer - to make sure it expires again(!) */
	mod_timer(&ts->timer,
	jiffies + msecs_to_jiffies(TS_PENUP_TIMEOUT_MS));
	spin_unlock(&acct_lock);
	enable_irq(ts->irq);

	FUNC_LEAVE;
}

void restore_default_coord(void)
{


	default_x_diff = coord_right - coord_left;
	default_y_diff = coord_bottom - coord_top;

	//printk("@[msm_touch.c][%d] coord_left:%d, coord_right:%d, coord_top:%d, coord_bottom:%d\r\n", __LINE__, coord_left, coord_right, coord_top, coord_bottom);
}

void ts_dump_tssc_register(void)
{
	int i;
	
	for (i = 0x100; i < 0x198; i = i + 4)
	{
		printk("[%3x]: %8x\r\n", i, readl(virt + i));
	}
}

void read_x_y_axis_raw_value_2_buffer(void)
{
	int i, count=0;
	u32	reg;
	
	for (i = 0x118; i < 0x138; i = i + 4)
	{
		reg = readl(virt + i);
		//printk("[%3x]: %8x\r\n", i, reg);
		
		x_axis_raw_buf[count++] = reg & 0xFFFF;
		x_axis_raw_buf[count++] = reg >> 16;
		printk("x[%d]: %d\r\n", count-2, x_axis_raw_buf[count-2]);
		printk("x[%d]: %d\r\n", count-1, x_axis_raw_buf[count-1]);
	}
	
	count = 0;
	for (i = 0x138; i < 0x158; i = i + 4)
	{
		reg = readl(virt + i);
		//printk("[%3x]: %8x\r\n", i, reg);
		
		y_axis_raw_buf[count++] = reg & 0xFFFF;
		y_axis_raw_buf[count++] = reg >> 16;
		printk("y[%d]: %d\r\n", count-2, y_axis_raw_buf[count-2]);
		printk("y[%d]: %d\r\n", count-1, y_axis_raw_buf[count-1]);
	}
}

#if FEATURE_TSSC_D1_FILTER
/*===========================================================================
FUNCTION dist

DESCRIPTION
  Calculate the distance between 2 points

DEPENDENCIES
  None

RETURN VALUE
  Distance

SIDE EFFECTS
  None

===========================================================================*/
static int dist(point *p1, point *p2)
{
    return abs(p1->x - p2->x) + abs(p1->y - p2->y);
}

/*===========================================================================
FUNCTION sort

DESCRIPTION
  Sort 3 points

DEPENDENCIES
  None

RETURN VALUE
  A point array of 2 which 2 points which has shortest distance.

SIDE EFFECTS
  None

===========================================================================*/
static point **sort(point *p1, point *p2, point *p3)
{
    static point *sorted[2];
    int d_1_2, d_1_3, d_2_3;

    d_1_2 = dist(p1, p2);
    d_1_3 = dist(p1, p3);
    d_2_3 = dist(p2, p3);

    if (d_1_2 < d_1_3) {
        if (d_1_2 < d_2_3) {
            sorted[0] = p1;
            sorted[1] = p2;
        } else {
            sorted[0] = p2;
            sorted[1] = p3;
        }
    } else {
        if (d_1_3 < d_2_3) {
            sorted[0] = p1;
            sorted[1] = p3;
        } else {
            sorted[0] = p2;
            sorted[1] = p3;
        }
    }
    return sorted;
}

/*===========================================================================
FUNCTION pbwa

DESCRIPTION
  Calculated a result point whith 3 input points

DEPENDENCIES
  None

RETURN VALUE
  A point.

SIDE EFFECTS
  None

===========================================================================*/
static point pbwa(point points[]) {
    point result;
    point **sorted = sort(&points[0], &points[1], &points[2]);
    result.x = (sorted[0]->x * 3 + sorted[1]->x) >> 2;
    result.y = (sorted[0]->y * 3 + sorted[1]->y) >> 2;
    return result;
}

#endif /* #ifdef FEATURE_TSSC_D1_FILTER */

#if X_Y_AXIS_QUEUE
void	add_x_y_axis_avg_raw_to_queue(u32 x, u32 y)
{
	if ((x == 0) || (y == 0))
	{
		printk("(x == 0) || (y == 0)\r\n");
		return;
	}
	
	if (x_axis_avg_raw_queue_index >= 3)
	{
			x_axis_avg_raw_queue[0] = x_axis_avg_raw_queue[1];
			x_axis_avg_raw_queue[1] = x_axis_avg_raw_queue[2];
			x_axis_avg_raw_queue[2] = x;
			y_axis_avg_raw_queue[0] = y_axis_avg_raw_queue[1];
			y_axis_avg_raw_queue[1] = y_axis_avg_raw_queue[2];
			y_axis_avg_raw_queue[2] = y;
			x_axis_avg_raw_queue_index++;
			y_axis_avg_raw_queue_index++;
	}
	else
	{
		x_axis_avg_raw_queue[x_axis_avg_raw_queue_index++] = x;
		y_axis_avg_raw_queue[y_axis_avg_raw_queue_index++] = y;
	}
}
#endif

int Is_meet_fixed_pointer_filter(int x_diff, int y_diff)
{

	//printk("TOUCH: Is_meet_fixed_pointer_filter = 0!  (%d %d) Last(%d %d)\r\n", x_axis, y_axis, last_x_axis, last_y_axis);

	if ((last_x_axis == 0) && (last_y_axis == 0))
	{
		return 1;
	}
	
	if ((last_x_axis == x_axis) && (last_y_axis == y_axis))
	{				
		return 1;
	}
	
	if (x_axis < last_x_axis)
	{
		if ((last_x_axis - x_axis) > x_diff)
		{
			return 1;
		}
		else
		{	
			//return 0;
		}
	}
	else
	{
		if ((x_axis - last_x_axis) > x_diff)
		{		
			return 1;
		}
		else
		{		
			//return 0;
		}
	}
	
	if (y_axis < last_y_axis)
	{
		if ((last_y_axis - y_axis) > y_diff)
		{		
			return 1;
		}
		else
		{			
			//return 0;
		}
	}
	else
	{
		if ((y_axis - last_y_axis) > y_diff)
		{			
			return 1;
		}
		else
		{			
			//return 0;
		}
	}
	return 0;
}

int Is_out_of_range(int x, int y, int z1)
{		
	//printk("[bing]x,y,z=(%d %d %d) (%d,%d,%d,%d)\r\n",x,y,z1,coord_left,coord_right,coord_top,coord_bottom);
/*	if(y> coord_top)
		touchedge=1;
	if ((x > coord_right) || (x < coord_left)||(y > coord_bottom) || (y < coord_top))
	{
		if(touchedge==1)
		{
			ts_update_pen_state(ts, ts_last_x, ts_last_y, 0);
			printk("[bing]trigger edge=%d\r\n",touchedge);
			touchedge=0;
		}
	}*/
	return 0;
}

int Is_out_of_tolerance(int x, int y)
{
	int x_tmp, y_tmp;
	
	//return 0;
	
	if ((last_x == 0) && (last_y == 0))
	{
		return 0;
	}
	
	if (x > last_x)
	{
		x_tmp = x - last_x;
	}
	else
	{
		x_tmp = last_x - x;
	}
	
	if (y > last_y)
	{
		y_tmp = y - last_y;
	}
	else
	{
		y_tmp = last_y - y;
	}
	
	if (x_tmp > TS_SAMPLE_POINT_TOLERANCE || y_tmp > TS_SAMPLE_POINT_TOLERANCE)
	{
		//printk("out of tolerance (%d, %d), dis(%d, %d)\r\n", x, y, x_tmp, y_tmp);
		
		x_OOT_buf[counter_OOT] = x;
		y_OOT_buf[counter_OOT] = y;
		
		//printk("x_OOT_buf[%d]: %d\r\n", counter_OOT, x_OOT_buf[counter_OOT]);
		//printk("y_OOT_buf[%d]: %d\r\n", counter_OOT, y_OOT_buf[counter_OOT]);
		//
		counter_OOT++;
		return 1;
	}
	else
	{
		counter_OOT = 0;
		return 0;
	}
}

int	x_point_correction(int x)
{
	return x;
}

int	y_point_correction(int y)
{
	
	return y;
	
	if (y>360)
	{
		return (y+1);
	}
	else if (y>240)
	{
		return (y+2);
	}
	else if (y>120)
	{
		return (y+4);
	}
	else
	{
		return (y+6);
	}
}

static void ts_update_pen_state(struct ts *ts, int x, int y, int pressure)
{
	int calibration_x,calibration_y;
	if(interrupt_counter==1)
		goto err;
	if(NULL!=msm_7k_handset_get_input_dev())
		kpdev = msm_7k_handset_get_input_dev();
	else
	{
		printk("[bing]msm_7k_handset_get_input_dev=NULL \r\n");
		goto err;
	}

	spin_lock(&acct_lock1);
	/*
	if (debug_message)
	{
		switch((debug_message & TS_UPDATE_PEN_STATE_MASK) >> TS_UPDATE_PEN_STATE_SHIFT)
		{
			case 1:
				printk("x_:%d, y_:%d\r\n", x, y);
			break;
			case 2:
			break;
			default:
			break;
		}
	}*/
	if(default_x_diff==x_diff&&default_y_diff==y_diff)
	{
		calibration_x=x;
		calibration_y=y;
	}
	else
	{
		calibration_x=x*x_diff / default_x_diff;
		calibration_y=y*y_diff / default_y_diff;
	}
	if( debug_mask)
	    printk("[bing]x,y=(%d,%d)   calibration_xy=(%d %d) last_key=%d  press=%d  #INT=%d\n",x,y,calibration_x,calibration_y,last_key,pressure,interrupt_counter);
	
	//printk("[bing]x,y=(%d %d) calibration_x=%d calibration_y=%d  x,y_diff=(%d %d) default_xy_diff=(%d %d) \r\n",x,y,calibration_x,calibration_y,x_diff,y_diff,default_x_diff,default_y_diff);
	if(project_id==PROJECT_ID_CAP8&&hw_id>=HW_ID_DVT1)
	{	      
			//printk("[bing]CAP8 DVT1\r\n");
			if(calibration_y<620 )
			{	
				if(BKL_EN==1)
				{
				screen_flag=1;
				input_report_abs(ts->input, ABS_X, x);
				input_report_abs(ts->input, ABS_Y, y);				
				input_report_abs(ts->input, ABS_PRESSURE, pressure);
				input_report_key(ts->input, BTN_TOUCH, !!pressure);
				input_sync(ts->input);
				}
				last_x = x_point_correction(x);
				last_y = y_point_correction(y);				
			}
			else
			{	screen_flag=0;
				if(BKL_EN==1)
				{						
				if(calibration_x<110)
				{						
					if(last_key!=KEY_HOME&&last_key!=0)
					   pressure=0;
					if(last_key==0)
					   last_key=KEY_HOME;
					input_report_key(kpdev, last_key,pressure);					
					last_key=KEY_HOME;
					//printk("[bing]KEY_HOME \r\n");
				}
				else if(calibration_x>=110&&calibration_x<264)
				{	
					if(last_key!=KEY_SEARCH&&last_key!=0)
					   pressure=0;
					if(last_key==0)
					   last_key=KEY_SEARCH;
					input_report_key(kpdev, last_key,pressure);					
					last_key=KEY_SEARCH;										
					//printk("[bing]KEY_SEARCH \r\n");
				}
				else if(calibration_x>=264&&calibration_x<415)
				{	
					if(last_key!=KEY_BACK&&last_key!=0)
					   pressure=0;
					if(last_key==0)
					   last_key=KEY_BACK;	
					input_report_key(kpdev, last_key,pressure);
					last_key=KEY_BACK;
					//printk("[bing]KEY_BACK \r\n");
				}
				else if(calibration_x>=415)
				{
					if(last_key!=KEY_MENU&&last_key!=0)
					    pressure=0;
					if(last_key==0)
					   last_key=KEY_MENU;
					input_report_key(kpdev, last_key,pressure);
					last_key=KEY_MENU;
					//printk("[bing]KEY_MENU \r\n");
				}
				
				bak_x=x;
				bak_y=y;
				key_pressed=1;				
				input_sync(kpdev);				
			}
	}
	}
	else if(project_id==PROJECT_ID_K4&&hw_id>=HW_ID_DVT1)
	{
	               // printk("[bing]K4:xy=(%d %d) calibration_x,y=(%d %d) key_pressed=%d last_key=%d\r\n",x,y,calibration_x,calibration_y,key_pressed,last_key);

			if(calibration_y<=590)
			{	
				if(BKL_EN==1)
				{
				input_report_abs(ts->input, ABS_X, x);
				input_report_abs(ts->input, ABS_Y, y);
				input_report_abs(ts->input, ABS_PRESSURE, pressure);
				input_report_key(ts->input, BTN_TOUCH, !!pressure);
				input_sync(ts->input);					
				}
			}
			else if(calibration_y>600)
			{	
				if(BKL_EN==1)
				{
				if(calibration_x>25&&calibration_x<120)
				{	
					if(last_key!=KEY_HOME&&last_key!=0)
					   pressure=0;
					if(last_key==0)
					   last_key=KEY_HOME;
					input_report_key(kpdev, last_key,pressure);					
					last_key=KEY_HOME;
					//printk("[bing]KEY_HOME \r\n");
				}
				else if(calibration_x>147&&calibration_x<=241)
				{	
					if(last_key!=KEY_SEARCH&&last_key!=0)
					   pressure=0;
					if(last_key==0)
					   last_key=KEY_SEARCH;
					input_report_key(kpdev, last_key,pressure);					
					last_key=KEY_SEARCH;										
					//printk("[bing]KEY_SEARCH \r\n");
				}
				else if(calibration_x>276&&calibration_x<=374)
				{	
					if(last_key!=KEY_BACK&&last_key!=0)
					   pressure=0;
					if(last_key==0)
					   last_key=KEY_BACK;	
					input_report_key(kpdev, last_key,pressure);
					last_key=KEY_BACK;
					//printk("[bing]KEY_BACK \r\n");
				}
				else if(calibration_x>405&&calibration_x<520)
				{
					if(last_key!=KEY_MENU&&last_key!=0)
					    pressure=0;
					if(last_key==0)
					   last_key=KEY_MENU;
					input_report_key(kpdev, last_key,pressure);
					last_key=KEY_MENU;
					//printk("[bing]KEY_MENU \r\n");
				}
				bak_x=x;
				bak_y=y;				
				key_pressed=1;
				input_sync(kpdev);		
				}
			}
 	}	 
	else if(project_id==PROJECT_ID_K5)
	{
			if(BKL_EN==1)
			{						
			//printk("[bing]K5\r\n");
			input_report_abs(ts->input, ABS_X, x_point_correction(x));
			input_report_abs(ts->input, ABS_Y, y_point_correction(y));
			input_report_abs(ts->input, ABS_PRESSURE, pressure);
			input_report_key(ts->input, BTN_TOUCH, !!pressure);
			input_sync(ts->input);
			}
	}
	else
	{	//printk("[bing]K4\r\n");
			if(calibration_y>=175)
			{		
				input_report_abs(ts->input, ABS_X, x_point_correction(x));
				input_report_abs(ts->input, ABS_Y, y_point_correction(y));
				input_report_abs(ts->input, ABS_PRESSURE, pressure);
				input_report_key(ts->input, BTN_TOUCH, !!pressure);
				input_sync(ts->input);
			}
			else
			{
				if(calibration_x>400&&calibration_x<=500)
				{	
					input_report_key(kpdev, KEY_HOME, pressure);	
					//printk("[bing]KEY_HOME \r\n");
				}
				else if(calibration_x>500&&calibration_x<=600)
				{	
					input_report_key(kpdev, KEY_SEARCH,pressure);
					//printk("[bing]KEY_SEARCH \r\n");
				}
				else if(calibration_x>600&&calibration_x<=700)
				{	
					input_report_key(kpdev, KEY_BACK,pressure);		
					//printk("[bing]KEY_BACK \r\n");
				}
				else if(calibration_x>700)
				{
					input_report_key(kpdev, KEY_MENU,pressure);
					//printk("[bing]KEY_MENU \r\n");
				}
				input_sync(kpdev);
			}
	}		
	//}
/*	else
	{
		//if (counter_OOT) return;			
		input_report_abs(ts->input, ABS_PRESSURE, 0);
		input_report_key(ts->input, BTN_TOUCH, 0);
		input_sync(ts->input);
		last_x_axis = 0;
		last_y_axis = 0;
		last_x = 0;
		last_y = 0;
#if X_Y_AXIS_QUEUE
		x_axis_avg_raw_queue_index = 0;
		y_axis_avg_raw_queue_index = 0;
#endif
		//printk("@[msm_touch.c][%d] pen up\r\n", __LINE__);
	}*/	
spin_unlock(&acct_lock1);

err:
	return;
}
static irqreturn_t ts_interrupt(int irq, void *dev_id)
{
	struct ts *ts = dev_id;

	if( debug_mask)
		//printk("touch interrupt...........\r\n");

	FUNC_ENTER;
	//printk("[bing]ts_interrupt\r\n");
	#if 0
	atomic_dec(&ts->irq_disable);
	disable_irq_nosync(ts->irq);
	schedule_work(&ts->ts_event_work);
#endif
	interrupt_counter++;
	disable_irq_nosync(ts->irq);
	del_timer(&ts->timer);
		queue_work(TSSC_wq, &ts->work);

	FUNC_LEAVE;
	return IRQ_HANDLED;
}
static irqreturn_t st1232_ts_interrupt(int irq, void *dev_id)
{
	interrupt_counter++;
        if(debug_mask)
            printk("[Bing]#INT=%d\r\n",interrupt_counter);		
	atomic_dec(&st1232_ts->irq_disable);
	disable_irq_nosync(st1232_ts->irq);
	++dis_irq;
	queue_work(MT_wq, &st1232_ts->ts_event_work);	
	return IRQ_HANDLED;
}

static int __devinit ts_probe(struct platform_device *pdev)
{
	int result;
	struct input_dev *input_dev;
	struct resource *res, *ioarea;
	struct ts *ts;
	unsigned int x_max, y_max, x_min, y_min, pressure_max;
	struct msm_ts_platform_data *pdata = pdev->dev.platform_data;

	FUNC_ENTER;

	/* The primary initialization of the TS Hardware
	 * is taken care of by the ADC code on the modem side
	 */
	ts = kzalloc(sizeof(struct ts), GFP_KERNEL);
	printk("[bing] ts=%p\r\n",ts);
	ts_point=ts;
	input_dev = input_allocate_device();
	if (!input_dev || !ts) {
		result = -ENOMEM;
		goto fail_alloc_mem;
	}
	printk("[bing]ts_probe\r\n");
//InitializeCriticalSection(m_CriticalSection);
//	INIT_WORK(&ts->ts_event_work, ts_TSSC_work_func);
	INIT_WORK(&ts->work, ts_TSSC_work_func);
#if 0
	TSSC_wq = create_singlethread_workqueue("TSSC_wq");
	
	if (!TSSC_wq)
	{
		printk("!TSSC_wq \r\n");
		result = -ENOMEM;
		goto fail_alloc_mem;
	}
	//set_highest_priority = 0;
#endif
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		result = -ENOENT;
		goto fail_alloc_mem;
	}
	ts->irq = platform_get_irq(pdev, 0);
	printk("[bing]ts->irq=%d\r\n",ts->irq);
	
	if (!ts->irq)
	{
		dev_err(&pdev->dev, "Could not get IORESOURCE_IRQ\n");
		result = -ENODEV;
		goto fail_alloc_mem;
	}
	else
	{
		DEBUG3("ts->irq: %d\r\n", ts->irq);
	}

	ioarea = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!ioarea) {
		dev_err(&pdev->dev, "Could not allocate io region\n");
		result = -EBUSY;
		goto fail_alloc_mem;
	}

	virt = ioremap(res->start, resource_size(res));
	if (!virt) {
		dev_err(&pdev->dev, "Could not ioremap region\n");
		result = -ENOMEM;
		goto fail_ioremap;
	}

	input_dev->name = TS_DRIVER_NAME;
	input_dev->phys = "msm_touch/input0";
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0002;
	input_dev->id.version = 0x0100;
	input_dev->dev.parent = &pdev->dev;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	input_dev->absbit[0] = BIT(ABS_X) | BIT(ABS_Y) | BIT(ABS_PRESSURE);
	input_dev->absbit[BIT_WORD(ABS_MISC)] = BIT_MASK(ABS_MISC);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	if (pdata) 
	{
		x_max = pdata->x_max ? : coord_right; //X_MAX;
		y_max = pdata->y_max ? : coord_bottom; //Y_MAX;
		//x_min = pdata->x_min ? : X_MIN;
		//y_min = pdata->y_min ? : Y_MIN;
		x_min = coord_left; //X_MIN;
		y_min = coord_top; //Y_MIN;
		pressure_max = pdata->pressure_max ? : P_MAX;
		//printk("@[msm_touch.c][%d] pdata->x_max=%u, pdata->y_max=%u, pdata->pressure_max=%u, x_max=%u, y_max=%u, pressure_max=%u\n",__LINE__, pdata->x_max, pdata->y_max, pdata->pressure_max, x_max, y_max, pressure_max);
	}
	else
	{		
		printk("[bing]cr:%d, cl:%d, cb:%d, ct:%d\r\n", calibrated_right, calibrated_left, calibrated_bottom, calibrated_top);

		if(project_id==PROJECT_ID_K4)
		{
			if(hw_id<=HW_ID_EVT2)
			{
				coord_right = X_MAX_K4;
				coord_bottom = Y_MAX_K4;
				coord_left = X_MIN_K4;
				coord_top = Y_MIN_K4;
			}	
			else
			{
				coord_right = X_MAX_K4_DVT1;
				coord_bottom = Y_MAX_K4_DVT1;
				coord_left = X_MIN_K4_DVT1;
				coord_top = Y_MIN_K4_DVT1;				
			}			
		}
		else if(project_id==PROJECT_ID_K5)
		{

			if(hw_id<=HW_ID_DVT1)
			{
				coord_right = X_MAX_K5;
				coord_bottom = Y_MAX_K5;
				coord_left = X_MIN_K5;
				coord_top = Y_MIN_K5;
			}
			else
			{
				coord_right = X_MAX_K5_DVT1_2;
				coord_bottom = Y_MAX_K5_DVT1_2;
				coord_left = X_MIN_K5_DVT1_2;
				coord_top = Y_MIN_K5_DVT1_2;
			}			
		}		
		else
		{
			coord_right = X_MAX_PENGLITE;
			coord_bottom = Y_MAX_PENGLITE;
			coord_left = X_MIN_PENGLITE;
			coord_top = Y_MIN_PENGLITE;
		}
		
		printk("[bing]r:%d, l:%d, b:%d, t:%d\r\n", coord_right, coord_left, coord_bottom, coord_top);
		default_x_diff = coord_right - coord_left;
		default_y_diff = coord_bottom - coord_top;		
		x_diff = coord_right - coord_left;
		y_diff = coord_bottom - coord_top;
		
#if REPORT_POINT_LOCATION
		x_max = 320;	//X_MAX;
		y_max = 480; 	//Y_MAX;
		x_min = 0;		//X_MIN;
		y_min = 0; 		//Y_MIN;
#else // QCT original report method

#if X_Y_AXIS_DECREASE
		x_max = x_diff; 			//X_MAX - X_MIN;
		y_max = y_diff; 			//Y_MAX - Y_MIN;
		x_min = 0; 						//X_MAX - X_MAX;
		y_min = 0; 						//Y_MAX - Y_MAX;
#else
		x_max = coord_right;	//X_MAX;
		y_max = coord_bottom; //Y_MAX;
		x_min = coord_left;		//X_MIN;
		y_min = coord_top; 		//Y_MIN;
#endif
#endif
		pressure_max = P_MAX;
		printk("@[msm_touch.c][%d] x_max=%u, y_max=%u, x_min=%u, y_min=%u, pressure_max=%u\n", __LINE__, x_max, y_max, x_min, y_min, pressure_max);
	}

	ts->x_max = x_max;
	ts->y_max = y_max;

	input_set_abs_params(input_dev, ABS_X, x_min, x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, y_min, y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, pressure_max, 0, 0);

	//printk("input_dev:0x%x\r\n", (unsigned int)input_dev);

	result = input_register_device(input_dev);
	if (result)
		goto fail_ip_reg;

	ts->input = input_dev;

	/* Data has been read, OK to clear the data flag */
	// Make sure we can receive touch interrupt
	// Aaron Chen @2009.12.02
	//tssc_hardware_init();

	TSSC_wq = create_singlethread_workqueue("TSSC_wq");
	printk("[Bing]TSSC_wq=%p\r\n",TSSC_wq);
	if (!TSSC_wq)
		printk("[bing]!TSSC_wq \r\n");
	setup_timer(&ts->timer, ts_timer, (unsigned long)ts);
//	setup_timer(&ts->error_monitor, ts_error_monitor, (unsigned long)ts);

	/* kick error monitor timer - to make sure it expires again(!) */
//	mod_timer(&ts->error_monitor,
	//jiffies + (TS_ERROR_MONITOR_TIMEOUT_S * HZ));

	result = request_irq(ts->irq, ts_interrupt, IRQF_TRIGGER_RISING,
				 "touchscreen", ts);
	//enable_irq(ts->irq);

	printk("[bing][%d] msm_touch(%s): ts->irq:%d \r\n", __LINE__, __func__, ts->irq);
  
	if (result)
		goto fail_req_irq;

	platform_set_drvdata(pdev, ts);

	/* create device files */
	result = device_create_file(&pdev->dev, &dev_attr_left);
	if (result)
	{
		printk("failed to add left attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_right);
	if (result)
	{
		printk("failed to add right attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_top);
	if (result)
	{
		printk("failed to add top attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_bottom);
	if (result)
	{
		printk("failed to add bottom attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_last_x);
	if (result)
	{
		printk("failed to add last_x attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_last_y);
	if (result)
	{
		printk("failed to add last_y attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_x_axis);
	if (result)
	{
		printk("failed to add x_axis attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_y_axis);
	if (result)
	{
		printk("failed to add y_axis attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_debug_flag);
	if (result)
	{
		printk("failed to add debug_flag attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_debug_message);
	if (result)
	{
		printk("failed to add debug_message attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_tssc_reg_addr);
	if (result)
	{
		printk("failed to add tssc_reg_addr attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_tssc_reg_data);
	if (result)
	{
		printk("failed to add tssc_reg_data attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_tssc_reg_read);
	if (result)
	{
		printk("failed to add tssc_reg_read attribute\r\n");
	}
	
	result = device_create_file(&pdev->dev, &dev_attr_tssc_reg_write);
	if (result)
	{
		printk("failed to add tssc_reg_write attribute\r\n");
	}

	FUNC_LEAVE;
	return 0;

fail_req_irq:
	input_unregister_device(input_dev);
	input_dev = NULL;
fail_ip_reg:
	iounmap(virt);
fail_ioremap:
	release_mem_region(res->start, resource_size(res));
fail_alloc_mem:
	input_free_device(input_dev);
	kfree(ts);
	FUNC_LEAVE;
	return result;
}

static int __devexit ts_remove(struct platform_device *pdev)
{
	struct resource *res;
	struct ts *ts = platform_get_drvdata(pdev);

	free_irq(ts->irq, ts);
	del_timer_sync(&ts->timer);
	//del_timer_sync(&ts->error_monitor);
	//set_highest_priority = 0;
	input_unregister_device(ts->input);
	iounmap(virt);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));
	platform_set_drvdata(pdev, NULL);
	kfree(ts);

	return 0;
}

int ts_early_suspend(struct early_suspend *h)
{
	printk("[Bing]ts_early_suspend,: x_axis=%d y_axis=%d ts_point=%p\r\n",ts_last_x,ts_last_y,ts_point);
	ts_update_pen_state(ts_point, ts_last_x, ts_last_y, 0);
	BKL_EN=0;
	return 0;
}

int ts_later_resume(struct early_suspend *h)
{
	printk("[Bing]ts_later_resume,: x_axis=%d y_axis=%d ts_point=%p\r\n",ts_last_x,ts_last_y,ts_point);
	BKL_EN=1;
	tssc_hardware_init();
	ts_update_pen_state(ts_point, ts_last_x, ts_last_y, 0);
	ts_last_x=0;
	ts_last_y=0;
	printk("[Bing]ts_later_resume  end\r\n");
	return 0;
}
int ts_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk("[Bing]ts_suspend,: x_axis=%d y_axis=%d ts_point=%p\r\n",ts_last_x,ts_last_y,ts_point);	
	BKL_EN=0;	
	return 0;
}

int ts_resume(struct platform_device *pdev)
{
	pen_down = 0;
	touch_recalibration = 0;
	printk("[Bing]ts_resume,: x_axis=%d y_axis=%d ts_point=%p\r\n",ts_last_x,ts_last_y,ts_point);	
	BKL_EN=1;
	tssc_hardware_init();
	printk("[Bing]ts_resume  end\r\n");	
	return 0;
}

/* Add touch IOCTL, but it does not work now, remove it first.
   Aaron Chen 2010.02.22
static int ts_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int ts_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t ts_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
	int retval = 0;
	
	return retval;
}

static ssize_t ts_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int retval = 0;
	
	return retval;
}

#if 0
static unsigned int ts_poll(struct file *file, poll_table *wait)
{
	return 0;
}
#endif

static long ts_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	
	return retval;
}

static long ts_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	
	return retval;	
}

static const struct file_operations ts_fops = {
	.owner		= THIS_MODULE,
	.open			= ts_open,
	.release	= ts_release,
	.read			= ts_read,
	.write		= ts_write,
	//.poll			= ts_poll,
	.unlocked_ioctl	= ts_unlocked_ioctl,
	.compat_ioctl		= ts_compat_ioctl,
};
*/

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

#if 0
static int get_coordinates(struct i2c_client *client)
{
	u8 buf[8];
	u8 count;
	u32 x0, y0, x1, y1;
	stx_report_data_t *pdata;
	int ret = 0;
	count = 0; // Set point detected count to 0.
	printk("[TS]get_coordinates...\r\n");
	if (i2c_master_recv(st1232_ts->client,buf, 8)) // Read Coordinates from default Reg. address 0x10.
		goto err;
	pdata = (stx_report_data_t *) buf;
	if (pdata->fingers) 
	{
		if (pdata->xy_data[0].valid) 
		{
			x0 = pdata->xy_data[0].x_h << 8 | pdata->xy_data[0].x_l;
			y0 = pdata->xy_data[0].y_h << 8 | pdata->xy_data[0].y_l;
			(count)++;
		}
		if (pdata->xy_data[1].valid) 
		{
			x1 = pdata->xy_data[1].x_h << 8 | pdata->xy_data[1].x_l;
			y1 = pdata->xy_data[1].y_h << 8 | pdata->xy_data[1].y_l;
			(count)++;
		}
		printk("[TS]get_coordinates: x0=%u y0=%u x1=%u y1=%u\r\n",x0, y0, x1, y1);
	}
err:
	return ret;
}
#endif

static void report_data(struct st1232_ts_data *ts, u32 x, u32 y, int pressure)
{
//printk("[Bing]report_data x,y=(%d %d)  press=%d\r\n",x,y,pressure);

	if(interrupt_counter==1)
		logger_write(0,2,"TP","report_data:x,y=(%d,%d) pressure=%d #INT=%d \n", x,y,pressure,interrupt_counter);	
       if(debug_mask)	
		printk("[Bing]report_data x,y=(%d %d)  press=%d\r\n",x,y,pressure);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, pressure);
//	input_report_abs(ts->input, ABS_MT_WIDTH_MAJOR, ts->dd->finger_size);
	input_mt_sync(ts->input_dev);
}

static void st1232_ts_irq_worker(struct work_struct *work)
{

	char test[80],test1[20];
	struct file *file,*file1;
	mm_segment_t old_fs,old_fs1;
        u8 buf[8],buf1[1];
        u32 x_st1,y_st1,x_st2,y_st2;
        int ret,rc,k,end,err;
	   
 	if (!set_highest_priority)
	{
		//Set to a high priority
		//set_user_nice(current, -10);
		struct sched_param s;
		struct cred *new = prepare_creds();
		cap_raise(new->cap_effective, CAP_SYS_NICE);
		commit_creds(new);
		s.sched_priority = 70;
		if ((sched_setscheduler(current, SCHED_FIFO, &s)) < 0)
		      printk("[CCI] sched_setscheduler failed for Touch Thread !!\n");
		set_highest_priority = 1;
	}

	if(CanUpdate==1)
	{	
		printk("[Bing]st1232_ts_irq_worker:  CanUpdate=%d\r\n",CanUpdate);		 
		goto end;
	}	
	
	if(run_one==1)
	{
		//write firmware version
		memset(test1,0,sizeof(test1));
		
		//spin_lock_irqsave(&irq_lock, flags);
		if ((err = sitronix_ts_get_fw_version(st1232_ts->client, &t_ver))) {
			printk("[Bing]Unable to get FW version...\r\n");
		} else {
			printk("[Bing]FW version=%X\r\n",t_ver);
		}
		
		if ((err = sitronix_ts_get_fw_revision(st1232_ts->client, &t_rev)))
		{
			printk("[Bing]Unable to get FW revision \r\n");		 
			
		}
			
		old_fs1 = get_fs();
		set_fs(KERNEL_DS);
		file1 = filp_open("/data/touch_ver", O_RDWR | O_CREAT, 0666);
		sprintf(test1,"rev:%d, version:%x",t_rev,t_ver);
		printk("[Bing]t_rev=%d, t_ver=%d \r\n",t_rev,t_ver);		 
		
	   	if(IS_ERR(file1))
		    printk("[bing]Open file error...\n");
		 file1->f_op->write(file1,test1 , 20 , &file1->f_pos);
	  	 filp_close(file1,NULL);
		 set_fs(old_fs1);	
		 run_one=0;
		printk("[Bing]run_one=%d rev=%d tpversion=%d\r\n",run_one,g_rev,tpversion);		 
	} 
	memset(buf,0,sizeof(buf));
	memset(buf1,0,sizeof(buf1));
	
	press1=press2=0;
	buf1[0]=0x10;
	rc = i2c_master_send(st1232_ts->client, buf1,1);
	if(rc!=1)
	{
		printk("[Bing]st1232  i2c_master_send  error4\r\n");
		tp_reset();
		rc = i2c_master_send(st1232_ts->client, buf1,1);		
		if(rc!=1)
		{
			printk("[Bing]st1232  i2c_master_send  error4_1\r\n");
		        mod_timer(&st1232_ts->timer,jiffies + msecs_to_jiffies(TS_PENUP_TIMEOUT_MS));       			
			goto end;
		}	
	}	
	
	ret = i2c_master_recv(st1232_ts->client, buf, 8);
	if(ret!=8)
	{
		printk("[Bing]st1232  i2c_master_recv  error5\r\n");
		tp_reset();
		ret = i2c_master_recv(st1232_ts->client, buf, 8);
		if(ret!=8)
		{
			printk("[Bing]st1232  i2c_master_recv  error5_1\r\n");
		        mod_timer(&st1232_ts->timer,jiffies + msecs_to_jiffies(TS_PENUP_TIMEOUT_MS));       			
			goto end;
		}		
	}	
	
        mod_timer(&st1232_ts->timer,jiffies + msecs_to_jiffies(TS_PENUP_TIMEOUT_MS));       
      
              if( (buf[2]&0x80) && (buf[5]&0x80))
	      {
                    press1= press2=1;
		    runone=1;		
  		    release_press_flag=0;	
              }
              else if( (buf[2]&0x80))
              {     
              	    press1 = 1;
	            release_press_flag=0;
              }		 
              else if( (buf[5]&0x80))
              {
                     press2 = 1;
		     release_press_flag=0;
              }	
	      x_st1 = ((buf[2]&0x70)<<4)|buf[3];
	      y_st1 = (((buf[2]&0x07)<<8)|buf[4]);
	      x_st2 = ((buf[5]&0x70)<<4)|buf[6];
	      y_st2 = (((buf[5]&0x07)<<8)|buf[7]);
             // printk("[Bing]buf[0]>>3=%d  receive_phone=%d  phone_ap=%d \r\n",buf[0]>>3,receive_phone,phone_ap);	

	      if((buf[0]>>3)==0x0F)
	      {
		    receive_phone=1;          	    
	      }
	      else if((buf[0]>>3)==0x10)
	      {
	            receive_phone=0;			
	      }	 
	      if(debug_mask)
 	      {
 	            printk("[Bing]receive_phone=%d  phone_ap=%d dis_irq=%ld en_irq=%ld BKL_EN=%d\r\n",receive_phone,phone_ap,dis_irq,en_irq,BKL_EN);	
	       	    printk("[Bing]buf[0~7]=0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
		    printk("[bing]p1,p2=(%d %d) before_x,y=(%d %d) before_x2,y2=(%d %d)  runone=%d   #INT=%d\r\n",press1,press2,x_st1,y_st1,x_st2,y_st2,runone,interrupt_counter);			
	      }

              if(receive_phone==1&&phone_ap==1)
		  goto end;

	if (BKL_EN==1)			  
	//if (BKL_EN==1||(receive_phone==0&&phone_ap==1))
	{  	
	      if(press1==0)
		  x_st1=y_st1=0;
	      if(press2==0)
		  x_st2=y_st2=0;

	      if(index<3)
	      {
		 x[index]=x_st1;
		 y[index]=y_st1;
   		 ++index;
	      }		  
	      else
	      {
		  x[0]=x[1];
		  y[0]=y[1];		
		  x[1]=x[2];
		  y[1]=y[2];				  
		  x[2]=x_st1;
		  y[2]=y_st1;
	      }		

	       x_st1= x[0];
	       y_st1= y[0];

	      if(debug_mask)
	      {
		    printk("[bing] x,y=(%d %d) x2,y2=(%d %d)   x,y[%d]=(%d %d) #INT=%d\r\n",x_st1,y_st1,x_st2,y_st2,index,x[index], y[index],interrupt_counter);
	      }	
		  
	      if(press1==0&&press2==0&&buf[1]==0)
	      {
		     release_press();
		     goto end;
	      }	 
              else if(press1==0&&press2==1&&runone==1)
	      {
		    if(debug_mask)
  		        printk("[bing]Release press1: (p1,p2)=(%d %d), last_x,y=(%d %d) #INT=%d\n",press1,press2,last_x,last_y,interrupt_counter);	
		    st1232_ts_update_pen_state(st1232_ts, last_x, last_y, 0);
		    if(press_point==2)
		        st1232_ts_update_pen_state(st1232_ts, last_x2, last_y2, 0);
		    runone=0;
	      }	  

	     if(project_id==PROJECT_ID_K4H)	 
	     {
	     	if(x_st1>325||y_st1>600)
	      	{
        	    memset(test,0,sizeof(test));
		    for(k=0;k<8;k++)
			 printk("[Bing]buf[%d]=0x%02X\r\n",k,buf[k]);
		    printk("[bing]press1=%d  press2=%d x_st1=%d  y_st1=%d  x_st2=%d y_st2=%d #INT=%d\r\n",press1,press2,x_st1,y_st1,x_st2,y_st2,interrupt_counter);
     		    old_fs = get_fs();
		    set_fs(KERNEL_DS);
		    file = filp_open("/data/touch_data.txt", O_RDWR | O_APPEND | O_CREAT, 0666);
		    sprintf(test,"buf=(%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X) x=%d y=%d x2=%d y2=%d INT=%d\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],x_st1,y_st1,x_st2,y_st2,interrupt_counter);
   		    if(IS_ERR(file))
		        printk("[bing]Open file error...\n");
		    file->f_op->write(file,test , 80 , &file->f_pos);
  		    filp_close(file,NULL);
		    set_fs(old_fs);
                    tp_reset();
       	      	}
	      }
              else if(project_id==PROJECT_ID_CAP6)	 
	      {
		       if(tpversion>=1)
		       {
		       	     	if(x_st1>325||y_st1>490)
				{
					for(k=0;k<8;k++)
						printk("[Bing]buf[%d]=0x%02X\r\n",k,buf[k]);
		    			printk("[bing]press1=%d  press2=%d x_st1=%d  y_st1=%d  x_st2=%d y_st2=%d #INT=%d\r\n",press1,press2,x_st1,y_st1,x_st2,y_st2,interrupt_counter);
	     			}
		       	}	
		       else if(tpversion==0)
		       {
		       	     	if(x_st1>325||y_st1>600)
				{
					for(k=0;k<8;k++)
						printk("[Bing]buf[%d]=0x%02X\r\n",k,buf[k]);
					 printk("[bing]press1=%d  press2=%d x_st1=%d  y_st1=%d  x_st2=%d y_st2=%d #INT=%d\r\n",press1,press2,x_st1,y_st1,x_st2,y_st2,interrupt_counter);
	     			}
		       	}				   
	      }
             if(press1 && press2) 
	     {
		press_point=2;
		bak_p1=bak_p2=1;
	  	// printk("[bing]x,y=(%d %d)  x2,y2=(%d %d)   prev_touches=%d keylocation=%d \r\n",x_st1,y_st1,x_st2,y_st2,st1232_ts->prev_touches,keylocation);				 				 
				
                if(y_st1<=down_edge) 
		{   //finger 1 is in touch area
                     report_data(st1232_ts, x_st1, y_st1, 1 );                     
                     if(y_st2<=down_edge)
		     {
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
                last_x = x_st1;
                last_y = y_st1; 
                last_x2 = x_st2;
                last_y2 = y_st2;          				                 
            }
            else if(press1)
	    {		
	        press_point=1;
 		bak_p1=1;
		bak_p2=0;

                if(y_st1<=down_edge)
		{ 
		    //finger 1 is in touch area
                    report_data(st1232_ts, x_st1, y_st1, 1 );
  		    if(debug_mask)
			    printk("[bing]x,y=(%d %d) prev_touches=%d keylocation=%d touchedge=%d \r\n",x_st1,y_st1,st1232_ts->prev_touches,keylocation,touchedge);				 				 
						
                     if(st1232_ts->prev_touches == 2){					 	
                        input_report_abs(st1232_ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
                	input_report_abs(st1232_ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
                	input_mt_sync(st1232_ts->input_dev);
                     }
                     st1232_ts->prev_touches = 1;
                    
                     input_sync(st1232_ts->input_dev);
		     if(keylocation==1)
		     {
			st1232_ts_update_pen_state(st1232_ts, last_key_x, last_key_y, 0);			
		      }	
		     keylocation=0;	 			 
		     touchedge=1;
                 }
               	 else if(tpversion==0||project_id==PROJECT_ID_K4H)
                 {
                	if(debug_mask)
			    printk("[bing]touchedge=%d y_st1=%d keylocation=%d\r\n",touchedge,y_st1,keylocation);				 				 
                  	if(y_st1>down_edge)
                  	{
     				if(touchedge==1)
				{
	               		    	report_data(st1232_ts, x_st1, y_st1, 0 );		
					st1232_ts_update_pen_state(st1232_ts, last_x, last_y, 0);			
					printk("[bing]trigger edge=%d  (%d %d)\r\n",touchedge,x_st1,y_st1);
					touchedge=0;
				}
				else
				{
					keylocation=1;
	 	             	        st1232_ts_update_pen_state(st1232_ts,x_st1, y_st1, 1 );
                 	        }
                       }
                 }
                 last_x = x_st1;
                 last_y = y_st1;               
                 last_x2 = x_st2;
                 last_y2 = y_st2;           
	     }
            else if(press2)
	    {							
		 press_point=1;
	 	 bak_p1=0;
		 bak_p2=1;

  	        if(debug_mask)
			printk("[bing]x2,y2=(%d %d)  trigger edge=%d\r\n",x_st2,y_st2,touchedge);				 
				
                if(y_st2<=down_edge)
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
                last_x = x_st1;
                last_y = y_st1;     
                last_x2 = x_st2;
                last_y2 = y_st2;           
				 
            }
	    else
	    {
        	 if(tpversion>=1)
              	 {
    			keypress=buf[1];
			if(keypress!=0)	
			{
				keyregister=keypress;
		   		if(debug_mask)        		
		    			printk("[Bing]keyregister=%d \r\n",buf[1]);
				x_st1=y_st1=0;
				st1232_ts_update_pen_state(st1232_ts,x_st1,y_st1, 1);		
        		}
                  }
	     }		


	}	
end:	
        	//mod_timer(&st1232_ts->timer,jiffies + msecs_to_jiffies(TS_PENUP_TIMEOUT_MS));	
	/* kick pen up timer - to make sure it expires again(!) */
	enable_irq(st1232_ts->irq);
	atomic_inc(&st1232_ts->irq_disable);
	++en_irq;
}
static int call_touch_sensitive_write(struct file *file, const char *buffer,
                                 unsigned long count, void *data)
{
    char *buf;
    int BUFIndex;
            printk("[Bing] call_touch_sensitive_write \n");

    if (count < 1)
        return -EINVAL;

    buf = kmalloc(count, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    if (copy_from_user(buf, buffer, count))
    {
        kfree(buf);
        return -EFAULT;
    }

    BUFIndex = (int)simple_strtol(buf, NULL, 10);


    switch (BUFIndex)
    {
        case 1://Low Level
             printk("[Bing] Sensitive_Low \n");
	break;

        case 2://Mid Level
            printk("[Bing] Sensitive_Mid \n");
            break;

        case 3://High Level
            printk("[Bing] Sensitive_High \n");
            break;

    }
    kfree(buf);
    return count;

}

static ssize_t fakepenable_show(struct kobject *kobj, struct kobj_attribute *attr,
			 char *buf)
{
	return sprintf(buf, "%hu\n", enablep);
}

static ssize_t fakepenable_store(struct kobject *kobj, struct kobj_attribute *attr,
			  const char * buf, size_t n)
{

	unsigned short value;
        //printk("[Bing] fakepenable_store: report_value=%d \n",report_value);
	
	if (sscanf(buf, "%hu", &value) != 1 ||
	    (value != 0 && value != 1)) {
		printk(KERN_ERR "idle_sleep_store: Invalid value\n");
		return -EINVAL;
	}
	
	input_report_abs(fpdev, ABS_DISTANCE,(receive_phone==1)?0:1);
        input_sync(fpdev);                     
	enablep = value;
	if(enablep)
		mod_timer(&st1232_ts->proximity_timer,jiffies + msecs_to_jiffies(report_value));       
	return n;
}


static ssize_t fakepdata_show(struct kobject *kobj, struct kobj_attribute *attr,
			 char *buf)
{

	return sprintf(buf, "%hu\n", distance);
}

static ssize_t fakepdata_store(struct kobject *kobj, struct kobj_attribute *attr,
			  const char * buf, size_t n)
{

	unsigned short value;
      //  printk("[Bing] fakepdata_store: report_value=%d \n",report_value);
	
	if (sscanf(buf, "%hu", &value) != 1 ||
	    (value != 0 && value != 1)) {
		printk(KERN_ERR "%s: Invalid value\n", __func__);
		return -EINVAL;
	}
	
	distance = value;
	input_report_abs(fpdev, ABS_DISTANCE, distance);
        input_sync(fpdev);                     
	printk(KERN_ERR "%s: value = %d\n", __func__, distance);
	return n;
}

static ssize_t fakepwake_store(struct kobject *kobj, struct kobj_attribute *attr,
			  const char * buf, size_t n)
{
	unsigned short value;
        //printk("[Bing] fakepwake_store: report_value=%d \n",report_value);
	
	if (sscanf(buf, "%hu", &value) != 1 ||
	    (value != 0 && value != 1)) {
		printk(KERN_ERR "%s: Invalid value\n", __func__);
		return -EINVAL;
	}
	
	input_report_abs(fpdev, ABS_MISC, 0);
        input_sync(fpdev);                     
	printk(KERN_ERR "%s: value = %d\n", __func__, distance);
	return n;
}

static ssize_t fakeptimer_store(struct kobject *kobj, struct kobj_attribute *attr,
			  const char * buf, size_t n)
{

        //printk("[Bing] proximity_timer_report \n");


	/*
	if (sscanf(buf, "%hu", &value) != 1 ||
	    (value != 0 && value != 1)) {
		printk(KERN_ERR "%s: Invalid value\n", __func__);
		return -EINVAL;
	}*/
	sscanf(buf, "%hu", &report_value);
       // printk("[Bing] proximity_timer_report:value=%d  receive_phone=%d  phone_ap=%d distance=%d\n",report_value,receive_phone,phone_ap,distance);
	if(enablep)
	  	mod_timer(&st1232_ts->proximity_timer,jiffies + msecs_to_jiffies(report_value));       

	//if(distance==0)
		
     
	return n;
}

static struct kobj_attribute fakepdata_attr =
	__ATTR(data, 0666, fakepdata_show, fakepdata_store);

static struct kobj_attribute fakepenable_attr =
	__ATTR(enable, 0666, fakepenable_show, fakepenable_store);

static struct kobj_attribute fakepwake_attr =
	__ATTR(wake, 0666, NULL, fakepwake_store);

static struct kobj_attribute fakeptimer_attr =
	__ATTR(delay, 0666, NULL, fakeptimer_store);

static int call_receive_phone(struct file *file, const char *buffer,
                                 unsigned long count, void *data)
{
    char *buf;
    int BUFIndex;
    u8 buf1[1],buf2[2],buf3[1];
    int rc;
    if (count < 1)
        return -EINVAL;

    buf = kmalloc(count, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    if (copy_from_user(buf, buffer, count))
    {
        kfree(buf);
        return -EFAULT;
    }

    BUFIndex = (int)simple_strtol(buf, NULL, 10);

    buf1[0]=0x2;
    rc=i2c_master_send(st1232_ts->client, buf1,1);
    if(rc!=1)
    {
 	printk("[Bing]call_receive_phone  i2c_master_send  error1\r\n");		
        tp_reset();
    }
    rc= i2c_master_recv(st1232_ts->client, buf1,1);
    if(rc!=1)
    {
	printk("[Bing]call_receive_phone  i2c_master_recv  error2\r\n");
	tp_reset();
    }			
    printk("[Bing]call_receive_phone: 0x2=%d (before)\r\n", buf1[0]);


    switch (BUFIndex)
    {
        case 0://Low Level
             printk("[Bing] call_receive_phone: Disable phone application \n");			 
	     phone_ap=0;
	     buf2[0]=0x2;
	     buf2[1]=0x8;
             rc = i2c_master_send(st1232_ts->client, buf2,2);
	     if(rc!=2)
	     {
		printk("[Bing]call_receive_phone: i2c_master_send 0x2,0x8 error_1\r\n");
		tp_reset();					
	     }

	break;	
        case 1://Low Level
             printk("[Bing] call_receive_phone:  Enable phone application  \n");
	     phone_ap=1;
	     buf2[0]=0x2;
	     buf2[1]=0xC;
             rc = i2c_master_send(st1232_ts->client, buf2,2);
	     if(rc!=2)
	     {
		printk("[Bing]call_receive_phone: i2c_master_send 0x2,0xC error_2\r\n");
		tp_reset();					
	     }
		 
	break;
    }
    //read 0x2 data
    buf1[0]=0x2;
    rc=i2c_master_send(st1232_ts->client, buf1,1);
    if(rc!=1)
    {
 	printk("[Bing]call_receive_phone  i2c_master_send  error1\r\n");		
        tp_reset();
    }
    rc= i2c_master_recv(st1232_ts->client, buf1,1);
    if(rc!=1)
    {
	printk("[Bing]call_receive_phone  i2c_master_recv  error2\r\n");
	tp_reset();
    }			
    printk("[Bing]call_receive_phone: 0x2=%d (after)\r\n", buf1[0]);


    //set original address: 0x10	
    buf3[0]=0x10;
    rc = i2c_master_send(st1232_ts->client, buf3,1);
    if(rc!=1)
    {
	printk("[Bing]set original address error3\r\n");
  	tp_reset();
    }	

	
    kfree(buf);
    return count;

}
static int __devinit st1232_ts_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{	
	struct input_dev *input_dev;
	int ret = 0,rc,rev;
	u8 struct_ver;
	int err = -ENOMEM;
//for touch sensitivity
	struct proc_dir_entry *d_entry,*d_entry1;
        u8 buf[8],buf1[16];
	
	//Set Sensitive path " /proc/OJ_Sensitive "
	d_entry = create_proc_entry("touch_sensitive", S_IRWXU | S_IRWXO, NULL);
	if (d_entry)
	{
		d_entry->read_proc = NULL;
		d_entry->write_proc = call_touch_sensitive_write;
		d_entry->data = NULL;
	}
	else
	{
		printk("[Bing] Fail call_touch_sensitive_write  \n");
		remove_proc_entry("touch_sensitive", 0);
		return  -ENOMEM;
	}
	d_entry1 = create_proc_entry("receive_phone", S_IRWXU | S_IRWXO, NULL);
	if (d_entry1)
	{
		d_entry1->read_proc = NULL;
		d_entry1->write_proc = call_receive_phone;
		d_entry1->data = NULL;
	}
	else
	{
		printk("[Bing] Fail call_receive_phone  \n");
		remove_proc_entry("receive_phone", 0);
		return  -ENOMEM;
	}
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
	 printk("[Bing]Before TOUCH_GPIO94 = %d\n",gpio_get_value(TOUCH_GPIO) );
   	// gpio_direction_input(TOUCH_GPIO);
	 // gpio_set_value(TOUCH_GPIO, 1);
 	config_gpio_table(gpio_table, ARRAY_SIZE(gpio_table));

	 printk("[Bing]After TOUCH_GPIO94 = %d\n",gpio_get_value(TOUCH_GPIO) );
	 
	st1232_ts = kzalloc(sizeof(struct st1232_ts_data), GFP_KERNEL);
	st1232_ts->input_dev = input_allocate_device();

	fpdev = input_allocate_device();

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
	st1232_ts->Can_update = false;
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
	
	fpdev->name = "proximity";
	fpdev->phys = "msm_touch/input0";
	fpdev->id.bustype = BUS_VIRTUAL;
	fpdev->id.vendor = 0x0001;
	fpdev->id.product = 0x0002;
	fpdev->id.version = 0x0100;
	fpdev->dev.parent = &st1232_ts->client->dev;

	set_bit(EV_ABS, fpdev->evbit);
	input_set_abs_params(fpdev, ABS_DISTANCE, 0, 1, 0, 0);
	
	if(input_register_device(fpdev)){
		printk("[TOUCH] register Proximity FAILED\n");
	}
	sysfs_create_file(&fpdev->dev.kobj,&fakepdata_attr.attr);
	sysfs_create_file(&fpdev->dev.kobj,&fakepenable_attr.attr);
	sysfs_create_file(&fpdev->dev.kobj,&fakeptimer_attr.attr);


	printk("[TOUCH] register Proximity OK\n");

	set_bit(EV_ABS, input_dev->evbit);
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(ABS_MISC, input_dev->absbit);

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

#if 0
	input_set_capability(input_dev,EV_KEY, KEY_HOME);
	input_set_capability(input_dev,EV_KEY, KEY_SEARCH);
	input_set_capability(input_dev,EV_KEY, KEY_BACK);
	input_set_capability(input_dev,EV_KEY, KEY_MENU);	
#endif
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,0, 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR,0, 320, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, 320, 0, 0);
	if(project_id==PROJECT_ID_K4H)
		input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, 488, 0, 0);
	else		
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, 480, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, 255, 0, 0);
	//printk("[Bing][%s] st1232_ts->input_dev = 0x%x\r\n", __func__, st1232_ts->input_dev);

	MT_wq = create_singlethread_workqueue("MT_wq");
         if (!MT_wq)
	 {
        	printk("!MT_wq created failed\r\n");
	        goto err_input;
	 }
	
	if(project_id==PROJECT_ID_K4H)
	    down_edge=488;	
	else 	if(project_id==PROJECT_ID_CAP6)
	    down_edge=480;
	else
  	    down_edge=480;
	wake_lock_init(&tpwakelock, WAKE_LOCK_SUSPEND, "st1232-ts");
	wake_lock_init(&tpwakelock_idle, WAKE_LOCK_IDLE, "st1232-ts");

	ret = request_irq(st1232_ts->irq, st1232_ts_interrupt, IRQF_TRIGGER_LOW, "touchscreen", NULL);
	if (ret < 0) {
		dev_err(&st1232_ts->client->dev, "Failed to register interrupt\n");
		goto err_irq;
	}

	atomic_dec(&st1232_ts->irq_disable);
	disable_irq_nosync(st1232_ts->irq);
	++dis_irq;
	printk("[Bing]st1232 probe: Disable touch irq...\r\n");		

	cancel_delayed_work(&st1232_ts->ts_event_work); 
/*
	if((err = sitronix_ts_get_struct_version(client, &struct_ver))){
		printk("[Bing]Unable to get struct version=%X \r\n",struct_ver);		
		dev_err(&client->dev, "Unable to get struct version!\n");
	}else{
		printk("[Bing]st1232 probe: struct version=%X \r\n",struct_ver);
		dev_dbg(&client->dev, "%s(%u): struct version=%X\n", __FUNCTION__, __LINE__, struct_ver);
		st1232_ts->struct_version = struct_ver;
	}
	*/


	if ((err = sitronix_ts_get_fw_revision(client, &rev)))
	{
		dev_err(&client->dev, "Unable to get FW revision!\n");		
	}
	printk("[Bing][%s] revision=%d\r\n", __func__,rev);
	g_rev=rev;
	if(rev>=1595)
		st1232_ts->struct_version =	1;	
	else
		st1232_ts->struct_version =0;
	
	ret = input_register_device(st1232_ts->input_dev);
	if (ret)
	{
		printk("[Bing][%s] input_register_device(st1232_ts->input_dev) fail\r\n", __func__);
		goto err_register;		
	}
	input_set_drvdata(st1232_ts->input_dev, st1232_ts);
	setup_timer(&st1232_ts->timer, st1232_ts_timer, (unsigned long)st1232_ts);
	setup_timer(&st1232_ts->proximity_timer, proximity_timer, (unsigned long)st1232_ts);

	if (sitronix_ts_create_sysfs_entry(client) < 0) {
		dev_err(&client->dev, "Failed to create sitronix sysfs entry!\n");
		goto err_sysfs_entry;
	}

	buf1[0]=0x0;
	rc=i2c_master_send(st1232_ts->client, buf1,1);
	if(rc!=1)
	{
		printk("[Bing]st1232  i2c_master_send  error1\r\n");		
		tp_reset();
	}
	rc= i2c_master_recv(st1232_ts->client, buf1,16);
	if(rc!=16)
	{
		printk("[Bing]st1232  i2c_master_recv  error2\r\n");
		tp_reset();
	}			
	printk("[Bing]st1232 probe:st1232_ts->isp_enabled=%d\r\n",st1232_ts->isp_enabled);			
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
	{
		printk("[Bing]st1232  i2c_master_send  error3\r\n");
		tp_reset();
	}	

	




	// ToDo
	// Check Version
	// Check Power 
	//printk("[Bing]call sitronix_ts_internal_update\r\n");
	//sitronix_ts_internal_update(client);


	return 0;
	
err_sysfs_entry:
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
	sitronix_ts_destroy_sysfs_entry(client);
	return 0;
}

void st1232_early_suspend(struct early_suspend *h)
{
	printk("[Bing]st1232_early_suspend,en_irq=%ld dis_irq=%ld\r\n",en_irq,dis_irq);

}
void tp_reset(void)
{
	printk("[Bing]tp_reset\r\n");	
	gpio_set_value(93, 0);	
        mdelay(50); 
       	printk("[Bing] %s: tp_reset1:GPIO93 = %d\n",__func__,gpio_get_value(93) ); 		
	gpio_set_value(93, 1);
        mdelay(200); 	
       	printk("[Bing] %s: tp_reset2:GPIO93 = %d\n",__func__,gpio_get_value(93) ); 	
}

void tp_reset_and_retry(void)
{
	//printk("[Bing] tp_reset_and_retry\r\n");
	
	if (tp_reset_and_check(st1232_ts->client) != 0)
	{
		if (tp_reset_and_check(st1232_ts->client) != 0)
		{
			tp_reset_and_check(st1232_ts->client);
		}
	}
}

int tp_reset_and_check(struct i2c_client *client)
{
	char buf[2];
	char data_buf[32*2];
	u32 raw_data, len;
	int ret = 0, i, count = 0;
	
	//printk("[Bing] tp_reset_and_check\r\n");	
	
	gpio_set_value(93, 0);
    mdelay(5);
	gpio_set_value(93, 1);
    mdelay(150);
    
    // raw data check
	// set data mode
	buf[0] = 0x02;
	buf[1] = ((1) << 4 | 0x08); //Set Reg. address to 0x2 for setting data mode.
	if ((ret = i2c_master_send(client, buf, 2)) != 2)
	{
		printk("[Bing][%s] Set Reg. address to 0x2 for setting data mode fail.\r\n", __func__);
		goto _gpio_reset_end;
	}

	buf[0] = 0x40; //Set Reg. address to 0x40 for reading raw data.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
	{
		printk("[Bing][%s] Set Reg. address to 0x40 for reading raw data fail.\r\n", __func__);
		goto _gpio_reset_end;
	}

	//Read raw data of length len.
	len = sizeof(data_buf);
	if ((ret = i2c_master_recv(client,data_buf, len)) != len)
	{
		printk("[Bing][%s] Read raw data of length len [part 1] fail.\r\n", __func__);
		goto _gpio_reset_end;
	}
	
	mdelay(15);
	
	if ((ret = i2c_master_recv(client,data_buf, len)) != len)
	{
		printk("[Bing][%s] Read raw data of length len [part 2] fail.\r\n", __func__);
		goto _gpio_reset_end;
	}

	for(i = 0; i < len; i += 2) 
	{
		raw_data= ((((u32)data_buf[i]) << 8) | data_buf[i+1]);
		if (raw_data != 0)
		{
			printk("[Bing][%s] GPIO reset ok!!\r\n", __func__);
			break;
		}
		else
		{
			count++;
		}
		
		if (count > 30)
		{
			printk("[Bing][%s] GPIO reset fail!!\r\n", __func__);
			goto _gpio_reset_end;
		}
	}

	// set data mode
	buf[0] = 0x02;
	buf[1] = 0x08; //Set Reg. address to 0x2 for setting data mode.
	if ((ret = i2c_master_send(client, buf, 2)) != 2)
	{
		printk("[Bing][%s] Set Reg. address to 0x2 for setting data mode fail.\r\n", __func__);
		goto _gpio_reset_end;
	}

	#if 0
	buf[0] = 0x10; //Set Reg. address back to 0x10 for coordinates.
	if ((ret = i2c_master_send(client, buf, 1)) != 1)
	{
		printk("[Bing][%s] Set Reg. address to 0x2 for setting data mode fail.\r\n", __func__);
		goto _gpio_reset_end;
	}
	#endif
	
	return 0;
_gpio_reset_end:
	return -EIO;
}

void st1232_later_resume(struct early_suspend *h)
{
	u8 buf[1];
	int ret;
	receive_phone=0;		
	
	if(CanUpdate==1)
		return 1;
	else
	{
		//try to receive 1 byte
		ret = i2c_master_recv(st1232_ts->client, buf,1);
		if(ret!=1)
		{
			printk("[Bing]st1232_later_resume,  i2c_master_recv  error_a\r\n");
			tp_reset();
			ret = i2c_master_recv(st1232_ts->client, buf,1);
			if(ret!=1)
			{
				printk("[Bing]st1232_later_resume,  i2c_master_recv  error_b\r\n");
				tp_reset();
			}		
		}
		else
		        printk("[Bing] %s:buf[0]=%d  ,en_irq=%ld ,dis_irq=%ld\n",__func__,buf[0],en_irq,dis_irq);
		return 0;		
	}		
}


int st1232_ts_suspend(struct platform_device *pdev, pm_message_t state)
{

	int rc;
	u8 buf[2];
	buf[0]=0x2;
	buf[1]=0xA;

	printk("[Bing]st1232_ts_suspend, CanUpdate=%d isp_enabled=%d\r\n",CanUpdate,st1232_ts->isp_enabled);	

	if(CanUpdate==1)
		return 1;
	else
	{
	 	config_gpio_table(gpio_table, ARRAY_SIZE(gpio_table));	
		atomic_dec(&st1232_ts->irq_disable);
		disable_irq(st1232_ts->irq);
		++dis_irq;
		printk("[Bing]st1232_ts_suspend: disable_irq\r\n");
		
		if(!st1232_ts->isp_enabled) 
		{
			//power down
			rc = i2c_master_send(st1232_ts->client, buf,2);
			if(rc!=2)
			{
				printk("[Bing]st1232_ts_suspend: i2c_master_send 0x2,0xA error_1\r\n");
				tp_reset();
				rc = i2c_master_send(st1232_ts->client, buf,2);
				if(rc!=2)
				{
					printk("[Bing]st1232_ts_suspend: i2c_master_send 0x2,0xA error_2\r\n");
					tp_reset();					
				}
			}
			else				
				printk("[Bing]st1232_ts_suspend,power down touch chip\r\n");
		}
		return 0;		
	}
}

#if 0
int st1232_ts_resume(struct platform_device *pdev)
{
	int ret;
	u8 buf[2],buf1[1],buf2[1],power[2];
	char data_buf[32*2];
	u32 raw_data, len;
	int i, count = 0;

	printk("[Bing]st1232_ts_resume: CanUpdate=%d isp_enabled=%d receive_phone=%d\r\n",CanUpdate,st1232_ts->isp_enabled,receive_phone);

	//power on
	buf[0]=0x2;
	buf[1]=0x8;
	ret = i2c_master_send(st1232_ts->client, buf,2);
	if(ret != 2)
	{
		printk("[Bing] %s: i2c_master_send 0x2,0x8 error_1 and will tp_reset\r\n", __FUNCTION__);
	tp_reset();
		ret = i2c_master_send(st1232_ts->client, buf,2);
		if(ret != 2)
		{
			printk("[Bing] %s: i2c_master_send 0x2,0x8 error_2 and will tp_reset\r\n", __FUNCTION__);
			tp_reset();
		}
	}
	else
		printk("[Bing]st1232_ts_resume, re-power-on touch chip ok\r\n");	
	
	//set 0x10
	power[0]=0x2;
	ret = i2c_master_send(st1232_ts->client, power,1);
	if(ret != 1)
	{
		printk("[Bing] %s: i2c_master_send 0x10 error_1 and will tp_reset\r\n", __FUNCTION__);
		tp_reset();	
		ret = i2c_master_send(st1232_ts->client, power,1);
		if(ret != 1)
		{
			printk("[Bing] %s: i2c_master_send 0x10 error_2 and will tp_reset\r\n", __FUNCTION__);
			tp_reset();				
		}
	}
	else
		printk("[Bing]st1232_ts_resume,set 0x2\r\n");		

	//get power down bit
	ret = i2c_master_recv(st1232_ts->client, buf2, 1);
	if(ret!=1)
	{
		printk("[Bing]st1232_ts_resume, i2c_master_recv  error_a\r\n");	
	}
	else
	        printk("[Bing] %s:check power down bit:%d \n",__func__,buf2[0]);
	//check  power down
	if((buf2[0] & 0x2) == 0x2)
	{
		//power on
		printk("[Bing]st1232_ts_resume:touch isn't power-on, it will re-power on\r\n");		
		buf[0]=0x2;
		buf[1]=0x8;
		ret = i2c_master_send(st1232_ts->client, buf,2);
		if(ret != 2)
		{
			printk("[Bing] %s: i2c_master_send 0x2,0x8 error_1 and will tp_reset\r\n", __FUNCTION__);
			tp_reset();
			ret = i2c_master_send(st1232_ts->client, buf,2);
			if(ret != 2)
			{
				printk("[Bing] %s: i2c_master_send 0x2,0x8 error_2 and will tp_reset\r\n", __FUNCTION__);
				tp_reset();
			}
		}
		else
			printk("[Bing]st1232_ts_resume, re-power-on touch chip ok\r\n");	
	}
	else
		printk("[Bing]st1232_ts_resume, touch is power-on ok after reset\r\n");	

	// raw data check
	// set data mode
	buf[0] = 0x02;
	buf[1] = ((1) << 4 | 0x08); //Set Reg. address to 0x2 for setting data mode.
	if ((ret = i2c_master_send(st1232_ts->client, buf, 2)) != 2)
	{
		printk("[Bing][%s] Set Reg. address to 0x2 for setting data mode fail.\r\n", __func__);
		tp_reset();
	}

	buf[0] = 0x40; //Set Reg. address to 0x40 for reading raw data.
	if ((ret = i2c_master_send(st1232_ts->client, buf, 1)) != 1)
	{
		printk("[Bing][%s] Set Reg. address to 0x40 for reading raw data fail.\r\n", __func__);
		tp_reset();
	}

	//Read raw data of length len.
	len = sizeof(data_buf);
	if ((ret = i2c_master_recv(st1232_ts->client,data_buf, len)) != len)
	{
		printk("[Bing][%s] Read raw data of length len [part 1] fail.\r\n", __func__);
		tp_reset();
	}
	
	mdelay(15);
	
	if ((ret = i2c_master_recv(st1232_ts->client,data_buf, len)) != len)
	{
		printk("[Bing][%s] Read raw data of length len [part 2] fail.\r\n", __func__);
		tp_reset();
	}

	for(i = 0; i < len; i += 2) 
	{
		raw_data= ((((u32)data_buf[i]) << 8) | data_buf[i+1]);
		if (raw_data != 0)
		{
			printk("[Bing][%s] GPIO reset ok!!\r\n", __func__);
			break;
		}
		else
		{
			count++;
		}
		
		if (count > 30)
		{
			printk("[Bing][%s] GPIO reset fail!!\r\n", __func__);
	tp_reset();
		}
	}

	// set data mode
	buf[0] = 0x02;
	buf[1] = 0x08; //Set Reg. address to 0x2 for setting data mode.
	if ((ret = i2c_master_send(st1232_ts->client, buf, 2)) != 2)
	{
		printk("[Bing][%s] Set Reg. address to 0x2 for setting data mode fail.\r\n", __func__);
		tp_reset();
	}

	//set 0x10
	buf1[0]=0x10;
	ret = i2c_master_send(st1232_ts->client, buf1,1);
	if(ret != 1)
	{
		printk("[Bing] %s: i2c_master_send 0x10 error_3 and will tp_reset\r\n", __FUNCTION__);
		tp_reset();	
		ret = i2c_master_send(st1232_ts->client, buf1,1);
		if(ret != 1)
		{
			printk("[Bing] %s: i2c_master_send 0x10 error_4 and will tp_reset\r\n", __FUNCTION__);
			tp_reset();				
		}
	}
	else
		printk("[Bing]st1232_ts_resume,set 0x10\r\n");		

	 enable_irq(st1232_ts->irq);
	 atomic_inc(&st1232_ts->irq_disable);	
	 ++en_irq;
	 printk("[Bing] %s: Enable irq, GPIO93 = %d\n",__func__,gpio_get_value(93) );

	return 0;
}
#else
int st1232_ts_resume(struct platform_device *pdev)
{
	int ret;
	u8 buf[2],buf1[1],buf2[1],power[1];

	printk("[Bing]st1232_ts_resume: CanUpdate=%d isp_enabled=%d receive_phone=%d\r\n",CanUpdate,st1232_ts->isp_enabled,receive_phone);

	tp_reset_and_retry();
	
	//set 0x10
	power[0]=0x2;
	ret = i2c_master_send(st1232_ts->client, power,1);
	if(ret != 1)
	{
		printk("[Bing] %s: i2c_master_send 0x10 error_1 and will tp_reset\r\n", __FUNCTION__);
		tp_reset();	
		ret = i2c_master_send(st1232_ts->client, power,1);
		if(ret != 1)
		{
			printk("[Bing] %s: i2c_master_send 0x10 error_2 and will tp_reset\r\n", __FUNCTION__);
			tp_reset();				
		}
	}
	else
		printk("[Bing]st1232_ts_resume,set 0x2\r\n");		

	//get power down bit
	ret = i2c_master_recv(st1232_ts->client, buf2, 1);
	if(ret!=1)
	{
		printk("[Bing]st1232_ts_resume, i2c_master_recv  error_a\r\n");	
	}
	else
	        printk("[Bing] %s:check power down bit:%d \n",__func__,buf2[0]);
	//check  power down
	if((buf2[0] & 0x2) == 0x2)
	{
		//power on
		printk("[Bing]st1232_ts_resume:touch isn't power-on, it will re-power on\r\n");		
		buf[0]=0x2;
		buf[1]=0x8;
		ret = i2c_master_send(st1232_ts->client, buf,2);
		if(ret != 2)
		{
			printk("[Bing] %s: i2c_master_send 0x2,0x8 error_1 and will tp_reset\r\n", __FUNCTION__);
			tp_reset();
			ret = i2c_master_send(st1232_ts->client, buf,2);
			if(ret != 2)
			{
				printk("[Bing] %s: i2c_master_send 0x2,0x8 error_2 and will tp_reset\r\n", __FUNCTION__);
				tp_reset();
			}
		}
		else
			printk("[Bing]st1232_ts_resume, re-power-on touch chip ok\r\n");	
	}
	else
		printk("[Bing]st1232_ts_resume, touch is power-on ok after reset\r\n");	

	//set 0x10
	buf1[0]=0x10;
	ret = i2c_master_send(st1232_ts->client, buf1,1);
	if(ret != 1)
	{
		printk("[Bing] %s: i2c_master_send 0x10 error_3 and will tp_reset\r\n", __FUNCTION__);
		tp_reset();	
		ret = i2c_master_send(st1232_ts->client, buf1,1);
		if(ret != 1)
		{
			printk("[Bing] %s: i2c_master_send 0x10 error_4 and will tp_reset\r\n", __FUNCTION__);
			tp_reset();				
		}
	}
	else
		printk("[Bing]st1232_ts_resume,set 0x10\r\n");		

	 enable_irq(st1232_ts->irq);
	 atomic_inc(&st1232_ts->irq_disable);	
	 ++en_irq;
	 printk("[Bing] %s: Enable irq, GPIO93 = %d\n",__func__,gpio_get_value(93) );

	return 0;
}
#endif

static ssize_t ts_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int retval = 0;
	return retval;
}
static struct platform_driver ts_driver = {
	.probe		= ts_probe,
	.remove		= __devexit_p(ts_remove),
	.suspend  = ts_suspend,
	.resume		= ts_resume,
	.driver		= {
	.name = "msm_touchscreen",
		.owner = THIS_MODULE,
	},
};

static const struct i2c_device_id st1232_ts_id[] = {
	{ "st1232-ts", 0 },
	{ }
};

static struct i2c_driver st1232_ts_driver = {
	.probe		= st1232_ts_probe,
	.remove		= st1232_ts_remove,
	.suspend 		 = st1232_ts_suspend,
	.resume		=  st1232_ts_resume,
	.id_table	= st1232_ts_id,
	.driver = {
	.name	= "st1232-ts",
	},
};

static int __init ts_init(void)
{	
	u8 buf[8];
	int ret,rc;
	u32 x_st,y_st;	
	struct vreg *vreg_gp4 = NULL;
	cci_smem_value_t *smem_cci_smem_value = 0;
	smem_cci_smem_value = smem_alloc( SMEM_CCI_SMEM_VALUE, sizeof( cci_smem_value_t ) );
	project_id=smem_cci_smem_value->cci_project_id;
	hw_id=smem_cci_smem_value->cci_hw_id;
	printk("[Bing][%s]project_id =%d hw_id=%d\r\n", __func__,project_id,hw_id);
	if(project_id==PROJECT_ID_K4H||project_id==PROJECT_ID_CAP6||project_id==PROJECT_ID_CAP2)
	{
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
	 	//printk("[Bing] %s: TOUCH_GPIO = %d\n",__func__,gpio_get_value(TOUCH_GPIO) );		
		ret = i2c_master_recv(st1232_ts->client, buf, 8);
		 printk("[Bing] %s: ret= %d\n",__func__,ret);
			y_st = ((buf[2]&0x70)<<4)|buf[3];
			x_st = 289 - (((buf[2]&0x07)<<8)|buf[4]);
			printk("[Bing]init x_st=%d, y_st=%d\r\n", x_st, y_st);
	}
	else
	{
		ts_early.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
		ts_early.suspend = ts_early_suspend;
		ts_early.resume = ts_later_resume;
		register_early_suspend(&ts_early);			
		ret = platform_driver_register(&ts_driver);
		/*TSSC_wq = create_singlethread_workqueue("TSSC_wq");
		printk("[Bing]TSSC_wq=%p\r\n",TSSC_wq);
		
		if (!TSSC_wq)
		{
			printk("!TSSC_wq \r\n");
			return -ENOMEM;
		}*/
		printk("[Bing][%s]others, ret = %d\r\n", __func__, ret);
	}
	return ret;
}
module_init(ts_init);

static void __exit ts_exit(void)
{
	DEBUG("\r\n");
	if (TSSC_wq)
		destroy_workqueue(TSSC_wq);
	platform_driver_unregister(&ts_driver);
}
module_exit(ts_exit);

MODULE_DESCRIPTION("MSM Touch Screen driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:msm_touchscreen");
