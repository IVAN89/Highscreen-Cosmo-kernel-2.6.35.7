/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include "mach/../../cci_smem.h"
#include "mach/../../smd_private.h"
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "tcm9001md.h"

#include <linux/device.h> /* for vreg.h */
#include <mach/vreg.h>
#include <mach/msm_battery.h>

//#define T_CCI_CAMERA_INITIAL_SHORT_VERSION

#define TCM9001MD_REG_MODEL_ID   0x0000
#define TCM9001MD_MODEL_ID       0x1000

#define INFO_CHIPID_H          0x0000
#define INFO_CHIPID_L          0x0001

/* Frame Fotmat */
#define REG_FRAME_LENGTH_LINES_MSB    0x0340
#define REG_FRAME_LENGTH_LINES_LSB    0x0341
#define REG_LINE_LENGTH_PCK_MSB       0x0342
#define REG_LINE_LENGTH_PCK_LSB       0x0343

/* MSR setting */
/* Reserved registers */
#define REG_SHADE_CLK_ENABLE          0x30AC
#define REG_SEL_CCP                   0x30C4
#define REG_VPIX                      0x3024
#define REG_CLAMP_ON                  0x3015
#define REG_OFFSET                    0x307E

/* CDS timing settings */
/* Reserved registers */
#define REG_LD_START                  0x3000
#define REG_LD_END                    0x3001
#define REG_SL_START                  0x3002
#define REG_SL_END                    0x3003
#define REG_RX_START                  0x3004
#define REG_S1_START                  0x3005
#define REG_S1_END                    0x3006
#define REG_S1S_START                 0x3007
#define REG_S1S_END                   0x3008
#define REG_S3_START                  0x3009
#define REG_S3_END                    0x300A
#define REG_CMP_EN_START              0x300B
#define REG_CLP_SL_START              0x300C
#define REG_CLP_SL_END                0x300D
#define REG_OFF_START                 0x300E
#define REG_RMP_EN_START              0x300F
#define REG_TX_START                  0x3010
#define REG_TX_END                    0x3011
#define REG_STX_WIDTH                 0x3012
#define REG_TYPE1_AF_ENABLE           0x3130
#define DRIVER_ENABLED                0x0001
#define AUTO_START_ENABLED            0x0010
#define REG_NEW_POSITION              0x3131
#define REG_3152_RESERVED             0x3152
#define REG_315A_RESERVED             0x315A
#define REG_ANALOGUE_GAIN_CODE_GLOBAL_MSB 0x0204
#define REG_ANALOGUE_GAIN_CODE_GLOBAL_LSB 0x0205
#define REG_FINE_INTEGRATION_TIME         0x0200
#define REG_COARSE_INTEGRATION_TIME       0x0202
#define REG_COARSE_INTEGRATION_TIME_LSB   0x0203

/* Mode select register */
#define TCM9001MD_REG_MODE_SELECT      0x0100
#define TCM9001MD_MODE_SELECT_STREAM     0x01   /* start streaming */
#define TCM9001MD_MODE_SELECT_SW_STANDBY 0x00   /* software standby */
#define TCM9001MD_REG_SOFTWARE_RESET   0x0103
#define TCM9001MD_SOFTWARE_RESET         0x01
#define REG_TEST_PATTERN_MODE         0xB054

#define TCM9001MD_BRIGHTNESS_DEGREE		11

#define TCM9001MD_DEFAULT_CLOCK_RATE  24000000
//added by yhwang for dual sensors on CAP8 product
static int kernel_ini_tcm9001md_result = -EIO;
module_param_named(
	INI_TCM9001MD_RESULT, kernel_ini_tcm9001md_result,int, S_IRUGO | S_IWUSR | S_IWGRP);

struct reg_struct {
  uint8_t pre_pll_clk_div;               /* 0x0305 */
  uint8_t pll_multiplier_msb;            /* 0x0306 */
  uint8_t pll_multiplier_lsb;            /* 0x0307 */
  uint8_t vt_pix_clk_div;                /* 0x0301 */
  uint8_t vt_sys_clk_div;                /* 0x0303 */
  uint8_t op_pix_clk_div;                /* 0x0309 */
  uint8_t op_sys_clk_div;                /* 0x030B */
  uint8_t ccp_data_format_msb;           /* 0x0112 */
  uint8_t ccp_data_format_lsb;           /* 0x0113 */
  uint8_t x_output_size_msb;             /* 0x034C */
  uint8_t x_output_size_lsb;             /* 0x034D */
  uint8_t y_output_size_msb;             /* 0x034E */
  uint8_t y_output_size_lsb;             /* 0x034F */
  uint8_t x_even_inc;                    /* 0x0381 */
  uint8_t x_odd_inc;                     /* 0x0383 */
  uint8_t y_even_inc;                    /* 0x0385 */
  uint8_t y_odd_inc;                     /* 0x0387 */
  uint8_t binning_enable;                /* 0x3014 */
  uint8_t frame_length_lines_msb;        /* 0x0340 */
  uint8_t frame_length_lines_lsb;        /* 0x0341 */
  uint8_t line_length_pck_msb;           /* 0x0342 */
  uint8_t line_length_pck_lsb;           /* 0x0343 */
  uint8_t shade_clk_enable ;             /* 0x30AC */
  uint8_t sel_ccp;                       /* 0x30C4 */
  uint8_t vpix;                          /* 0x3024 */
  uint8_t clamp_on;                      /* 0x3015 */
  uint8_t offset;                        /* 0x307E */
  uint8_t ld_start;                      /* 0x3000 */
  uint8_t ld_end;                        /* 0x3001 */
  uint8_t sl_start;                      /* 0x3002 */
  uint8_t sl_end;                        /* 0x3003 */
  uint8_t rx_start;                      /* 0x3004 */
  uint8_t s1_start;                      /* 0x3005 */
  uint8_t s1_end;                        /* 0x3006 */
  uint8_t s1s_start;                     /* 0x3007 */
  uint8_t s1s_end;                       /* 0x3008 */
  uint8_t s3_start;                      /* 0x3009 */
  uint8_t s3_end;                        /* 0x300A */
  uint8_t cmp_en_start;                  /* 0x300B */
  uint8_t clp_sl_start;                  /* 0x300C */
  uint8_t clp_sl_end;                    /* 0x300D */
  uint8_t off_start;                     /* 0x300E */
  uint8_t rmp_en_start;                  /* 0x300F */
  uint8_t tx_start;                      /* 0x3010 */
  uint8_t tx_end;                        /* 0x3011 */
  uint8_t stx_width;                     /* 0x3012 */
  uint8_t reg_3152_reserved;             /* 0x3152 */
  uint8_t reg_315A_reserved;             /* 0x315A */
  uint8_t analogue_gain_code_global_msb; /* 0x0204 */
  uint8_t analogue_gain_code_global_lsb; /* 0x0205 */
  uint8_t fine_integration_time;         /* 0x0200 */
  uint8_t coarse_integration_time;       /* 0x0202 */
  uint32_t  size_h;
  uint32_t  blk_l;
  uint32_t  size_w;
  uint32_t  blk_p;
};

struct reg_struct tcm9001md_reg_pat[2] =  {
  {	/* Preview */
    0x06,  /* pre_pll_clk_div       REG=0x0305 */
    0x00,  /* pll_multiplier_msb    REG=0x0306 */
    0x88,  /* pll_multiplier_lsb    REG=0x0307 */
    0x0a,  /* vt_pix_clk_div        REG=0x0301 */
    0x01,  /* vt_sys_clk_div        REG=0x0303 */
    0x0a,  /* op_pix_clk_div        REG=0x0309 */
    0x01,  /* op_sys_clk_div        REG=0x030B */
    0x0a,  /* ccp_data_format_msb   REG=0x0112 */
    0x0a,  /* ccp_data_format_lsb   REG=0x0113 */
    0x05,  /* x_output_size_msb     REG=0x034C */
    0x10,  /* x_output_size_lsb     REG=0x034D */
    0x03,  /* y_output_size_msb     REG=0x034E */
    0xcc,  /* y_output_size_lsb     REG=0x034F */

    /* enable binning for preview */
    0x01,  /* x_even_inc             REG=0x0381 */
    0x01,  /* x_odd_inc              REG=0x0383 */
    0x01,  /* y_even_inc             REG=0x0385 */
    0x03,  /* y_odd_inc              REG=0x0387 */
    0x06,  /* binning_enable         REG=0x3014 */

    0x03,  /* frame_length_lines_msb        REG=0x0340 */
    0xde,  /* frame_length_lines_lsb        REG=0x0341 */
    0x0a,  /* line_length_pck_msb           REG=0x0342 */
    0xac,  /* line_length_pck_lsb           REG=0x0343 */
    0x81,  /* shade_clk_enable              REG=0x30AC */
    0x01,  /* sel_ccp                       REG=0x30C4 */
    0x04,  /* vpix                          REG=0x3024 */
    0x00,  /* clamp_on                      REG=0x3015 */
    0x02,  /* offset                        REG=0x307E */
    0x03,  /* ld_start                      REG=0x3000 */
    0x9c,  /* ld_end                        REG=0x3001 */
    0x02,  /* sl_start                      REG=0x3002 */
    0x9e,  /* sl_end                        REG=0x3003 */
    0x05,  /* rx_start                      REG=0x3004 */
    0x0f,  /* s1_start                      REG=0x3005 */
    0x24,  /* s1_end                        REG=0x3006 */
    0x7c,  /* s1s_start                     REG=0x3007 */
    0x9a,  /* s1s_end                       REG=0x3008 */
    0x10,  /* s3_start                      REG=0x3009 */
    0x14,  /* s3_end                        REG=0x300A */
    0x10,  /* cmp_en_start                  REG=0x300B */
    0x04,  /* clp_sl_start                  REG=0x300C */
    0x26,  /* clp_sl_end                    REG=0x300D */
    0x02,  /* off_start                     REG=0x300E */
    0x0e,  /* rmp_en_start                  REG=0x300F */
    0x30,  /* tx_start                      REG=0x3010 */
    0x4e,  /* tx_end                        REG=0x3011 */
    0x1E,  /* stx_width                     REG=0x3012 */
    0x08,  /* reg_3152_reserved             REG=0x3152 */
    0x10,  /* reg_315A_reserved             REG=0x315A */
    0x00,  /* analogue_gain_code_global_msb REG=0x0204 */
    0x80,  /* analogue_gain_code_global_lsb REG=0x0205 */
    0x02,  /* fine_integration_time         REG=0x0200 */
    0x03,  /* coarse_integration_time       REG=0x0202 */
		972,
		18,
		1296,
		1436
  },
  { /* Snapshot */
    0x06,  /* pre_pll_clk_div               REG=0x0305 */
    0x00,  /* pll_multiplier_msb            REG=0x0306 */
    0x88,  /* pll_multiplier_lsb            REG=0x0307 */
    0x0a,  /* vt_pix_clk_div                REG=0x0301 */
    0x01,  /* vt_sys_clk_div                REG=0x0303 */
    0x0a,  /* op_pix_clk_div                REG=0x0309 */
    0x01,  /* op_sys_clk_div                REG=0x030B */
    0x0a,  /* ccp_data_format_msb           REG=0x0112 */
    0x0a,  /* ccp_data_format_lsb           REG=0x0113 */
    0x0a,  /* x_output_size_msb             REG=0x034C */
    0x30,  /* x_output_size_lsb             REG=0x034D */
    0x07,  /* y_output_size_msb             REG=0x034E */
    0xa8,  /* y_output_size_lsb             REG=0x034F */

    /* disable binning for snapshot */
    0x01,  /* x_even_inc                    REG=0x0381 */
    0x01,  /* x_odd_inc                     REG=0x0383 */
    0x01,  /* y_even_inc                    REG=0x0385 */
    0x01,  /* y_odd_inc                     REG=0x0387 */
    0x00,  /* binning_enable                REG=0x3014 */

    0x07,  /* frame_length_lines_msb        REG=0x0340 */
    0xb6,  /* frame_length_lines_lsb        REG=0x0341 */
    0x0a,  /* line_length_pck_msb           REG=0x0342 */
    0xac,  /* line_length_pck_lsb           REG=0x0343 */
    0x81,  /* shade_clk_enable              REG=0x30AC */
    0x01,  /* sel_ccp                       REG=0x30C4 */
    0x04,  /* vpix                          REG=0x3024 */
    0x00,  /* clamp_on                      REG=0x3015 */
    0x02,  /* offset                        REG=0x307E */
    0x03,  /* ld_start                      REG=0x3000 */
    0x9c,  /* ld_end                        REG=0x3001 */
    0x02,  /* sl_start                      REG=0x3002 */
    0x9e,  /* sl_end                        REG=0x3003 */
    0x05,  /* rx_start                      REG=0x3004 */
    0x0f,  /* s1_start                      REG=0x3005 */
    0x24,  /* s1_end                        REG=0x3006 */
    0x7c,  /* s1s_start                     REG=0x3007 */
    0x9a,  /* s1s_end                       REG=0x3008 */
    0x10,  /* s3_start                      REG=0x3009 */
    0x14,  /* s3_end                        REG=0x300A */
    0x10,  /* cmp_en_start                  REG=0x300B */
    0x04,  /* clp_sl_start                  REG=0x300C */
    0x26,  /* clp_sl_end                    REG=0x300D */
    0x02,  /* off_start                     REG=0x300E */
    0x0e,  /* rmp_en_start                  REG=0x300F */
    0x30,  /* tx_start                      REG=0x3010 */
    0x4e,  /* tx_end                        REG=0x3011 */
    0x1E,  /* stx_width                     REG=0x3012 */
    0x08,  /* reg_3152_reserved             REG=0x3152 */
    0x10,  /* reg_315A_reserved             REG=0x315A */
    0x00,  /* analogue_gain_code_global_msb REG=0x0204 */
    0x80,  /* analogue_gain_code_global_lsb REG=0x0205 */
    0x02,  /* fine_integration_time         REG=0x0200 */
    0x03,  /* coarse_integration_time       REG=0x0202 */
		1960,
		14,
		2608,
		124
	}
};

struct tcm9001md_work {
	struct work_struct work;
};
static struct tcm9001md_work *tcm9001md_sensorw;
static struct i2c_client *tcm9001md_client;

struct tcm9001md_ctrl {
	const struct msm_camera_sensor_info *sensordata;

	int sensormode;
	uint32_t fps_divider; /* init to 1 * 0x00000400 */
	uint32_t pict_fps_divider; /* init to 1 * 0x00000400 */

	uint16_t curr_lens_pos;
	uint16_t init_curr_lens_pos;
	uint16_t my_reg_gain;
	uint32_t my_reg_line_count;

	enum msm_s_resolution prev_res;
	enum msm_s_resolution pict_res;
	enum msm_s_resolution curr_res;
	enum msm_s_test_mode  set_test;
};

struct tcm9001md_i2c_reg_conf {
	unsigned short waddr;
	unsigned char  bdata;
};
static DECLARE_WAIT_QUEUE_HEAD(tcm9001md_wait_queue);


// Capture Configuration Update setting
static CAM_REG_ADDR_VAL_TYPE	cam_new_configuration_capture_sync[]=
{
{REG_TC_GP_ActiveCapConfig,		0x0000},    //58MHz Sys Clock
{REG_TC_GP_NewConfigSync,		0x0001},
{REG_TC_GP_CapConfigChanged,	0x0001},
{REG_TC_GP_EnableCapture,		0x0001},
{REG_TC_GP_EnableCaptureChanged,	0x0001},
};

// Pewview Configuration Update setting
static CAM_REG_ADDR_VAL_TYPE	cam_new_configuration_preview_sync[]=
{
{REG_TC_GP_ActiveCapConfig,		0x0000},    //58MHz Sys Clock
{REG_TC_GP_NewConfigSync,		0x0001},
{REG_TC_GP_CapConfigChanged,	0x0001},
{REG_TC_GP_EnableCapture,		0x0000},
{REG_TC_GP_EnableCaptureChanged,	0x0001},
};

static uint16_t Brightness_Value[TCM9001MD_BRIGHTNESS_DEGREE] = {0x20, 0x30, 0x40, 0x50, 0x60, 0x70,0x80, 0x90, 0xA0, 0xB0, 0xC0};
static uint16_t Contrast_Value = 0x00; 
static uint16_t Saturation_Value = 0x64; 
static uint16_t Sharpness_Value = 0x32; 

static CAM_REG_ADDR_VAL_TYPE	cam_awb_auto[]=
{
{REG_TC_DBG_AutoAlgEnBits,	0x077F},		// #REG_TC_DBG_AutoAlgEnBits, AWB On
};

static CAM_REG_ADDR_VAL_TYPE	cam_wb_cloudy[]=
{
{REG_TC_DBG_AutoAlgEnBits,	0x0777},		// #REG_TC_DBG_AutoAlgEnBits, AWB Off
{REG_SF_USER_Rgain,			0x07A0},		// #REG_SF_USER_Rgain
{REG_SF_USER_RgainChanged,	0x0001}, 
{REG_SF_USER_Ggain,			0x0400},		// #REG_SF_USER_Ggain
{REG_SF_USER_GgainChanged,	0x0001},
{REG_SF_USER_Bgain,			0x0480},		// #REG_SF_USER_Bgain
{REG_SF_USER_BgainChanged,	0x0001},
};

static CAM_REG_ADDR_VAL_TYPE	cam_wb_horizon[]=
{
{REG_TC_DBG_AutoAlgEnBits,	0x0777},		// #REG_TC_DBG_AutoAlgEnBits, AWB Off
{REG_SF_USER_Rgain,			0x03B3},		// #REG_SF_USER_Rgain
{REG_SF_USER_RgainChanged,	0x0001}, 
{REG_SF_USER_Ggain,			0x0400},		// #REG_SF_USER_Ggain
{REG_SF_USER_GgainChanged,	0x0001},
{REG_SF_USER_Bgain,			0x0A28},		// #REG_SF_USER_Bgain
{REG_SF_USER_BgainChanged,	0x0001},
};

static CAM_REG_ADDR_VAL_TYPE	cam_wb_fluorescent[]=
{
{REG_TC_DBG_AutoAlgEnBits,	0x0777},		// #REG_TC_DBG_AutoAlgEnBits, AWB Off
{REG_SF_USER_Rgain,			0x05D1},		// #REG_SF_USER_Rgain
{REG_SF_USER_RgainChanged,	0x0001}, 
{REG_SF_USER_Ggain,			0x0400},		// #REG_SF_USER_Ggain
{REG_SF_USER_GgainChanged,	0x0001},
{REG_SF_USER_Bgain,			0x074C},		// #REG_SF_USER_Bgain
{REG_SF_USER_BgainChanged,	0x0001},
};

static CAM_REG_ADDR_VAL_TYPE	cam_wb_incandescent[]=
{
{REG_TC_DBG_AutoAlgEnBits,	0x0777},		// #REG_TC_DBG_AutoAlgEnBits, AWB Off
{REG_SF_USER_Rgain,			0x0433},		// #REG_SF_USER_Rgain
{REG_SF_USER_RgainChanged,	0x0001}, 
{REG_SF_USER_Ggain,			0x0400},		// #REG_SF_USER_Ggain
{REG_SF_USER_GgainChanged,	0x0001},
{REG_SF_USER_Bgain,			0x0828},		// #REG_SF_USER_Bgain
{REG_SF_USER_BgainChanged,	0x0001},
};

static CAM_REG_ADDR_VAL_TYPE	cam_wb_daylight[]=
{
{REG_TC_DBG_AutoAlgEnBits,	0x0777},		// #REG_TC_DBG_AutoAlgEnBits, AWB Off
{REG_SF_USER_Rgain,			0x06BB},		// #REG_SF_USER_Rgain
{REG_SF_USER_RgainChanged,	0x0001}, 
{REG_SF_USER_Ggain,			0x0400},		// #REG_SF_USER_Ggain
{REG_SF_USER_GgainChanged,	0x0001},
{REG_SF_USER_Bgain,			0x04D0},		// #REG_SF_USER_Bgain
{REG_SF_USER_BgainChanged,	0x0001},
};

static uint16_t cam_exposure_mode_center_weighted[]=
{
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0505},{0x0505},{0x0101},
{0x0101},{0x0505},{0x0505},{0x0101},
{0x0101},{0x0505},{0x0505},{0x0101},
{0x0101},{0x0505},{0x0505},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
};

static uint16_t cam_exposure_mode_spot_metering[]=
{
{0x0000},{0x0000},{0x0000},{0x0000},
{0x0000},{0x0000},{0x0000},{0x0000},
{0x0000},{0x0000},{0x0000},{0x0000},
{0x0000},{0x0500},{0x0005},{0x0000},
{0x0000},{0x0500},{0x0005},{0x0000},
{0x0000},{0x0000},{0x0000},{0x0000},
{0x0000},{0x0000},{0x0000},{0x0000},
{0x0000},{0x0000},{0x0000},{0x0000},
};

/* 
static CAM_REG_ADDR_VAL_TYPE	cam_wb_tungsten[]=
{
{0x700004D2, 0x0777},		// #REG_TC_DBG_AutoAlgEnBits, AWB Off
{0x700004A0, 0x0433},		// #REG_SF_USER_Rgain
{0x700004A2, 0x0001}, 
{0x700004A4, 0x0400},		// #REG_SF_USER_Ggain
{0x700004A6, 0x0001},
{0x700004A8, 0x0828},		// #REG_SF_USER_Bgain
{0x700004AA, 0x0001},
};
*/
static uint16_t cam_exposure_mode_frame_avg[]=
{
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
{0x0101},{0x0101},{0x0101},{0x0101},
};

static CAM_REG_ADDR_VAL_TYPE	cam_extra_mode_beach[]=
{
/*Color temperature of beach mode is above 5500K*/
{awbb_GridCorr_R_1__1_, 0x0000},
{awbb_GridCorr_R_2__1_, 0xFFDD},
{awbb_GridCorr_B_1__1_, 0x0000},
{awbb_GridCorr_B_2__1_, 0x0011},
};	

static CAM_REG_ADDR_VAL_TYPE	cam_extra_mode_night[]=
{
{lt_uMaxExp1, 0x3415},
{lt_uMaxExp2, 0xC350},
{evt1, 0xFFFF},
{evt1_lt_uMaxExp4, 0x86A0},
{0x70001682, 0x0003},
{lt_uCapMaxExp1, 0x3415},
{lt_uCapMaxExp2, 0xC350},
{evt1_lt_uCapMaxExp3, 0xFFFF},
{evt1_lt_uCapMaxExp4, 0x86A0},
{0x7000168A, 0x0003},
{lt_uMaxAnGain1, 0x0180},
{lt_uMaxAnGain2, 0x0250},
{evt1_lt_uMaxAnGain3, 0x0340},
{evt1_lt_uMaxAnGain4, 0x0820},
{lt_uMaxDigGain, 0x0200},
};	

static CAM_REG_ADDR_VAL_TYPE	cam_extra_mode_night_break[]=
{
{lt_uMaxExp1, 0x5DC0},
{0x70000532, 0x0000},
{lt_uMaxExp2, 0x6D60},
{0x70000536, 0x0000},
{evt1_lt_uMaxExp3, 0x9C40},
{0x7000167E, 0x0000},
{evt1_lt_uMaxExp4, 0xBB80},
{0x70001682, 0x0000},
{lt_uCapMaxExp1, 0x5DC0},
{0x7000053A ,0000},
{lt_uCapMaxExp2, 0x6D60},
{0x7000053E, 0x0000},
{evt1_lt_uCapMaxExp3, 0x9C40},
{0x70001686, 0x0000},
{evt1_lt_uCapMaxExp4, 0xBB80},
{0x7000168A, 0x0000},
{lt_uMaxAnGain1, 0x0150},
{lt_uMaxAnGain2, 0x0280},
{evt1_lt_uMaxAnGain3, 0x02A0},
{evt1_lt_uMaxAnGain4, 0x0800},
{lt_uMaxDigGain, 0x0100},
};	

static struct tcm9001md_ctrl *tcm9001md_ctrl;

DEFINE_MUTEX(tcm9001md_mutex);

void tcm9001md_power_enable(void)
{
       int rc;
       struct vreg *vreg_cam;
       
       
       printk("#### %s ####\n", __FUNCTION__);
       

       

       vreg_cam = vreg_get(0, "gp2");  //I/O used
       rc = vreg_set_level(vreg_cam,2600);
       if (rc)
               printk("#### vreg set gp2 level failed ####\n");
       rc = vreg_enable(vreg_cam);
       if (rc)
               printk("#### vreg enable gp2 level failed ####\n");

       mdelay(20);

       vreg_cam = vreg_get(0, "gp4"); //DCORE used
       rc = vreg_set_level(vreg_cam,1800);
       if (rc)
               printk("#### vreg set gp4 level failed ####\n");
       rc = vreg_enable(vreg_cam);
       if (rc)
               printk("#### vreg enable gp4 level failed ####\n");

       mdelay(20);

       vreg_cam = vreg_get(0, "wlan"); //ACORE used
       rc = vreg_set_level(vreg_cam,2800);
       if (rc)
               printk("#### vreg set wlan level failed ####\n");
       rc = vreg_enable(vreg_cam);
       if (rc)
               printk("#### vreg enable wlan level failed ####\n");

       mdelay(20);
       //Modify 2010/08/17 Battery device status update(on)
       cci_batt_device_status_update(CCI_BATT_DEVICE_ON_CAMERA_150,1);

}

void tcm9001md_power_disable(void)
{
    	int rc;
    	struct vreg *vreg_cam;

    	printk("#### %s ####\n", __FUNCTION__);

    	vreg_cam = vreg_get(0, "wlan");	//ACORE used
    	rc = vreg_set_level(vreg_cam,0);
    	if (rc)
    		printk("#### vreg set wlan level failed ####\n");
    	rc = vreg_disable(vreg_cam);
    	if (rc)
    		printk("#### vreg disable wlan level failed ####\n");

       mdelay(20);

    	vreg_cam = vreg_get(0, "gp4");	////VREG_5V used
    	rc = vreg_set_level(vreg_cam,0);
    	if (rc)
    		printk("#### vreg set gp4 level failed ####\n");
    	rc = vreg_disable(vreg_cam);
    	if (rc)
    		printk("#### vreg disable gp4 level failed ####\n");
	
       mdelay(20);

    	vreg_cam = vreg_get(0, "gp2");	//I/O used
    	rc = vreg_set_level(vreg_cam,0);
    	if (rc)
    		printk("#### vreg set gp2 level failed ####\n");
    	rc = vreg_disable(vreg_cam);
    	if (rc)
    		printk("#### vreg disable gp2 level failed ####\n");

       mdelay(20);
       //Modify 2010/08/17 Battery device status update(off)
       cci_batt_device_status_update(CCI_BATT_DEVICE_ON_CAMERA_150,0);
}

static int tcm9001md_i2c_rxdata(unsigned short saddr, unsigned char *rxdata,
	int length)
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

	if (i2c_transfer(tcm9001md_client->adapter, msgs, 2) < 0) {
		CDBG("tcm9001md_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t tcm9001md_i2c_txdata(unsigned short saddr,
	unsigned char *txdata, int length)
{
	struct i2c_msg msg[] = {
		{
		.addr  = saddr,
		.flags = 0,
		.len = length,
		.buf = txdata,
		},
	};

	if (i2c_transfer(tcm9001md_client->adapter, msg, 1) < 0) {
		CDBG("tcm9001md_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}

static int32_t tcm9001md_i2c_write_b(unsigned short saddr, unsigned short waddr,
	unsigned char bdata)
{
	int32_t rc = -EIO;
	unsigned char buf[2];

	memset(buf, 0, sizeof(buf));
	buf[0] = waddr;
	buf[1] = bdata;
	rc = tcm9001md_i2c_txdata(saddr, buf, 2);

	if (rc < 0)
		CDBG("i2c_write failed, addr = 0x%x, val = 0x%x!\n",
		waddr, bdata);

	return rc;
}

static int32_t camsensor_i2c_write_byte(unsigned short saddr, unsigned char bdata)
{
	int32_t rc = 0;
	rc = tcm9001md_i2c_write_b(tcm9001md_client->addr,
		saddr,
		bdata);
	return rc;
}

static int32_t tcm9001md_i2c_write_w(unsigned short saddr, unsigned short waddr,
	unsigned short wdata)
{
	int32_t rc = -EIO;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = (wdata & 0xFF00)>>8;
	buf[3] = (wdata & 0x00FF);

	rc = tcm9001md_i2c_txdata(saddr, buf, 4);

	if (rc < 0)
		CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
			waddr, wdata);

	return rc;
}

static int32_t camsensor_i2c_write_word(unsigned short saddr, unsigned short sdata)
{
	int32_t rc = 0;
	rc = tcm9001md_i2c_write_w(tcm9001md_client->addr,
		saddr,
		sdata);
	return rc;
}



static int32_t tcm9001md_i2c_read_w(unsigned short saddr, unsigned short raddr,
	unsigned short *rdata)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	buf[0] = (raddr & 0xFF00)>>8;
	buf[1] = (raddr & 0x00FF);

	rc = tcm9001md_i2c_rxdata(saddr, buf, 2);
	if (rc < 0)
		return rc;

	*rdata = buf[0] << 8 | buf[1];

	if (rc < 0)
		CDBG("tcm9001md_i2c_read failed!\n");

	return rc;
}

static int32_t camsensor_i2c_read_word(unsigned short saddr, unsigned short *sdata)
{
	int32_t rc = 0;
	rc = tcm9001md_i2c_read_w(tcm9001md_client->addr,
		saddr,
		sdata);
	return rc;
}

static int32_t camsensor_tcm9001md_i2c_write(uint16_t msb_reg, uint16_t lsb_reg, uint16_t data)
{
	int32_t rc = 0;

	rc = camsensor_i2c_write_word(0x0028, msb_reg);
	if(rc < 0)
	{
		return rc;
	 }
	rc = camsensor_i2c_write_word(0x002A, lsb_reg);
	if(rc < 0)
	{
		return rc;
 	}
	rc = camsensor_i2c_write_word(0x0F12, data);
	if(rc < 0)
	{
		return rc;
 	}
	
	return rc; 
}

static int32_t camsensor_tcm9001md_i2c_read(uint16_t msb_reg, uint16_t lsb_reg, uint16_t *data)
{
	int32_t rc = 0;

	rc = camsensor_i2c_write_word(0x002C, msb_reg);
	if(rc < 0)
	{
		return rc;
 	}	
	rc = camsensor_i2c_write_word(0x002E, lsb_reg);
	if(rc < 0)
	{
		return rc;
 	}
	rc = camsensor_i2c_read_word(0x0F12, data);
	if(rc < 0)
	{
		return rc;
 	}

	return rc; 
}

static int32_t camsensor_tcm9001md_sw_reset_reg_setup(void)
{
   int32_t  rc = 0;
   int32_t		ary_size=0;
   int			i;
   /*
   ary_size = sizeof(cam_sw_resets_setup)/sizeof(CAM_REG_ADDR_VAL_TYPE);
   for(i=0;i<ary_size;i++)
   {
	rc = camsensor_i2c_write_word(cam_sw_resets_setup[i].addr, cam_sw_resets_setup[i].val);
	if(rc < 0)
	{
         return rc;
 	}
   }
 */
   return rc;
}
static int tcm9001md_probe_init_done(const struct msm_camera_sensor_info *data)
{
	gpio_direction_output(data->sensor_pwd, 0);
	gpio_free(data->sensor_pwd);

	gpio_direction_output(data->sensor_reset, 0);
	gpio_free(data->sensor_reset);
	return 0;
}

static int tcm9001md_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int32_t  rc;
	uint16_t chipid = 0;

	rc = gpio_request(data->sensor_reset, "tcm9001md");
	printk("[camera] gpio_request reset=%d\n", rc);
	if (!rc)
		gpio_direction_output(data->sensor_reset, 1);
	else
		goto init_probe_fail;

	mdelay(30);

	rc = camsensor_i2c_read_word(INFO_CHIPID_L, &chipid);
	printk("[camera] camsensor_i2c_read_word=%d\n", rc);
	if (rc < 0)
		goto init_probe_fail;

	printk("[camera] chipid0x%X=0x%X\n", chipid, TCM9001MD_MODEL_ID);
	if (chipid != TCM9001MD_MODEL_ID) {
		CDBG("TCM9001MD wrong model_id = 0x%x\n", chipid);
		rc = -ENODEV;
		goto init_probe_fail;
	}

	goto init_probe_done;

init_probe_fail:
	/*while (1)
	{
               printk("#### [camera] init_probe_fail ####\n");
	}
*/
	tcm9001md_probe_init_done(data);
init_probe_done:
	return rc;
}

static int tcm9001md_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&tcm9001md_wait_queue);
	return 0;
}

static const struct i2c_device_id tcm9001md_i2c_id[] = {
	{ "tcm9001md", 0},
	{ }
};

static int tcm9001md_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	CDBG("tcm9001md_probe called!\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CDBG("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	tcm9001md_sensorw = kzalloc(sizeof(struct tcm9001md_work), GFP_KERNEL);
	if (!tcm9001md_sensorw) {
		CDBG("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, tcm9001md_sensorw);
	tcm9001md_init_client(client);
	tcm9001md_client = client;

	mdelay(50);

	CDBG("tcm9001md_probe successed! rc = %d\n", rc);
	return 0;

probe_failure:
	CDBG("tcm9001md_probe failed! rc = %d\n", rc);
	return rc;
}

static struct i2c_driver tcm9001md_i2c_driver = {
	.id_table = tcm9001md_i2c_id,
	.probe  = tcm9001md_i2c_probe,
	.remove = __exit_p(tcm9001md_i2c_remove),
	.driver = {
		.name = "tcm9001md",
	},
};

/*
static int32_t tcm9001md_test(enum msm_s_test_mode mo)
{
	int32_t rc = 0;

	if (mo == S_TEST_OFF)
		rc = 0;
	else
		rc = tcm9001md_i2c_write_b(tcm9001md_client->addr,
			REG_TEST_PATTERN_MODE, (uint16_t)mo);

	return rc;
*/
static int32_t tcm9001md_write_FlickerMode_60hz_reg(void)
{
   int32_t rc = 0;
#if 0
//#ifndef USE_CAMSENSOR_TOSHIBA_TCM9001MD  
   boolean	status;
   uint32_t ary_size=0;
   int			i;
   

   ary_size=sizeof(cam_flickermode_60hz)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      status=camsensor_i2c_write_byte(cam_flickermode_60hz[i].addr, 
		cam_flickermode_60hz[i].val);
      if(status==FALSE)
      {
         return FALSE;
      }
   }
//#endif
#endif
	return rc;
}

static int32_t tcm9001md_write_FlickerMode_50hz_reg(void)
{
   int32_t rc = 0;
#if 0
//#ifndef USE_CAMSENSOR_TOSHIBA_TCM9001MD  
   boolean	status;
   uint32_t ary_size=0;
   int			i;
   
   MSG_HIGH("camsensor_tcm9001md_write_FlickerMode_50hz_reg",0,0,0);
   ary_size=sizeof(cam_flickermode_50hz)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      status=camsensor_i2c_write_byte(cam_flickermode_50hz[i].addr, 
		cam_flickermode_50hz[i].val);
      if(status==FALSE)
      {
         return FALSE;
      }
   }
//#endif
#endif
	return rc;
}


static int32_t tcm9001md_write_AWB_reg(void)
{
   int32_t rc = 0;
   uint32_t		ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_awb_auto)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_tcm9001md_i2c_write((uint16_t)(cam_awb_auto[i].addr >> 16),
	  	(uint16_t)(cam_awb_auto[i].addr & 0xFFFF),
		cam_awb_auto[i].val);
      if(rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t tcm9001md_write_WB_cloudy_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_wb_cloudy)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_tcm9001md_i2c_write((uint16_t)((cam_wb_cloudy[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_wb_cloudy[i].addr & 0xFFFF),
		cam_wb_cloudy[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t tcm9001md_write_WB_horizon_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   /*
   ary_size=sizeof(cam_wb_horizon)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_tcm9001md_i2c_write((uint16_t)(cam_wb_horizon[i].addr >>16),
	  	(uint16_t)(cam_wb_horizon[i].addr & 0xFFFF),
		cam_wb_horizon[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   */
   
   return rc;
}

static int32_t tcm9001md_write_WB_fluorescent_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
  /*  
   ary_size=sizeof(cam_wb_fluorescent)/sizeof(CAM_REG_ADDR_VAL_TYPE);
 
   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_tcm9001md_i2c_write((uint16_t)(cam_wb_fluorescent[i].addr >>16),
	  	(uint16_t)(cam_wb_fluorescent[i].addr & 0xFFFF),
		cam_wb_fluorescent[i].val);
      if (rc < 0)
      {
         return rc;
      }
    
   }
   */
   return rc;
}

static int32_t tcm9001md_write_WB_incandescent_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   /*
   ary_size=sizeof(cam_wb_incandescent)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_tcm9001md_i2c_write((uint16_t)(cam_wb_incandescent[i].addr >>16),
	  	(uint16_t)(cam_wb_incandescent[i].addr & 0xFFFF),
		cam_wb_incandescent[i].val);
      if (rc < 0)
      {
         return rc;
      }
      
   }
   */
   return rc;
}

static int32_t tcm9001md_write_WB_daylight_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   /*
   ary_size=sizeof(cam_wb_daylight)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_tcm9001md_i2c_write((uint16_t)(cam_wb_daylight[i].addr >>16),
	  	(uint16_t)(cam_wb_daylight[i].addr & 0xFFFF),
		cam_wb_daylight[i].val);
      if (rc < 0)
      {
         return rc;
      }
     
   }
   */
   return rc;
}

static int32_t tcm9001md_write_bestshot_mode_beach_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   /*
   ary_size=sizeof(cam_extra_mode_beach)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_tcm9001md_i2c_write((uint16_t)((cam_extra_mode_beach[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_extra_mode_beach[i].addr & 0xFFFF),
		cam_extra_mode_beach[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   */
   return rc;
}

static int32_t tcm9001md_write_bestshot_mode_night_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   /*
   ary_size=sizeof(cam_extra_mode_night)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_tcm9001md_i2c_write((uint16_t)((cam_extra_mode_night[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_extra_mode_night[i].addr & 0xFFFF),
		cam_extra_mode_night[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   */
   return rc;
}

static int32_t tcm9001md_write_bestshot_mode_night_break_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   /*
   ary_size=sizeof(cam_extra_mode_night_break)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_tcm9001md_i2c_write((uint16_t)((cam_extra_mode_night_break[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_extra_mode_night_break[i].addr & 0xFFFF),
		cam_extra_mode_night_break[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   */
   return rc;
}


static int32_t camsensor_tcm9001md_new_config_capture_sync_reg_setup(void)
{

   int32_t rc = 0;
   uint32_t		ary_size=0;
   int			i;

   /*
   ary_size=sizeof(cam_new_configuration_capture_sync)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_tcm9001md_i2c_write((uint16_t)((cam_new_configuration_capture_sync[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_new_configuration_capture_sync[i].addr & 0xFFFF),
		cam_new_configuration_capture_sync[i].val);
      if(rc < 0)
      {
         return rc;
      }
   }
   */
   
   return rc;
}

static int32_t camsensor_tcm9001md_new_config_preview_sync_reg_setup(void)
{

   int32_t rc = 0;
   uint32_t		ary_size=0;
   int			i;

  /*
   ary_size=sizeof(cam_new_configuration_preview_sync)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_tcm9001md_i2c_write((uint16_t)((cam_new_configuration_preview_sync[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_new_configuration_preview_sync[i].addr & 0xFFFF),
		cam_new_configuration_preview_sync[i].val);
      if(rc < 0)
      {
         return rc;
      }
   }
   */
   
   return rc;
}
static int32_t camsensor_tcm9001md_camera_run(void)
{
	int32_t rc = 0;

	return rc;
}
static int32_t tcm9001md_setting(enum msm_s_reg_update rupdate,
	enum msm_s_setting rt)
{
	int32_t rc = 0;

       printk("#### %s(%d %d) ####\n", __FUNCTION__, rupdate, rt);

	switch (rupdate) {
	case S_UPDATE_PERIODIC:
	  /*lint -restore */
	      switch (rt)
	      {
	        case S_RES_CAPTURE:
	           rc = camsensor_tcm9001md_new_config_capture_sync_reg_setup();
	           if(rc < 0)
	           {
	              return rc;
	           }
	           mdelay(100);
	           break;
			   
	        case S_RES_PREVIEW:
	           rc = camsensor_tcm9001md_new_config_preview_sync_reg_setup();
	           if(rc < 0)
	           {
	              return rc;
	           }
	           mdelay(100);
	           break;
			
	        default:
	          return rc;
	      } /* rt */
		break; /* UPDATE_PERIODIC */

	case S_REG_INIT:
	if (rc = camsensor_i2c_write_byte(0x00,0x48)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x01,0x10)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x02,0xD8)) {break;} //alcint_sekiout[7:0];
	if (rc = camsensor_i2c_write_byte(0x03,0x00)) {break;} //alc_ac5060out/alc_les_modeout[2:0]/*/*/alcint_sekiout[9:;
	if (rc = camsensor_i2c_write_byte(0x04,0x20)) {break;} //alc_agout[7:0];
	if (rc = camsensor_i2c_write_byte(0x05,0x00)) {break;} //*/*/*/*/alc_agout[11:8];
	if (rc = camsensor_i2c_write_byte(0x06,0x2D)) {break;} //alc_dgout[7:0];
	if (rc = camsensor_i2c_write_byte(0x07,0x9D)) {break;} //alc_esout[7:0];
	if (rc = camsensor_i2c_write_byte(0x08,0x80)) {break;} //alc_okout//alc_esout[13:8];
	if (rc = camsensor_i2c_write_byte(0x09,0x45)) {break;} //awb_uiout[7:0];
	if (rc = camsensor_i2c_write_byte(0x0A,0x17)) {break;} //awb_uiout[15:8];
	if (rc = camsensor_i2c_write_byte(0x0B,0x05)) {break;} //awb_uiout[23:16];
	if (rc = camsensor_i2c_write_byte(0x0C,0x00)) {break;} //*/*/awb_uiout[29:24];
	if (rc = camsensor_i2c_write_byte(0x0D,0x39)) {break;} //awb_viout[7:0];
	if (rc = camsensor_i2c_write_byte(0x0E,0x56)) {break;} //awb_viout[15:8];
	if (rc = camsensor_i2c_write_byte(0x0F,0xFC)) {break;} //awb_viout[2316];
	if (rc = camsensor_i2c_write_byte(0x10,0x3F)) {break;} //*/*/awb_viout[29:24];
	if (rc = camsensor_i2c_write_byte(0x11,0xA1)) {break;} //awb_pixout[7:0];
	if (rc = camsensor_i2c_write_byte(0x12,0x28)) {break;} //awb_pixout[15:8];
	if (rc = camsensor_i2c_write_byte(0x13,0x03)) {break;} //*/*/*/*/*/awb_pixout[18:16];
	if (rc = camsensor_i2c_write_byte(0x14,0x85)) {break;} //awb_rgout[7:0];
	if (rc = camsensor_i2c_write_byte(0x15,0x80)) {break;} //awb_ggout[7:0];
	if (rc = camsensor_i2c_write_byte(0x16,0x6B)) {break;} //awb_bgout[7:0];
	if (rc = camsensor_i2c_write_byte(0x17,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x18,0x9C)) {break;} //LSTB/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x19,0x04)) {break;} //*/*/*/*/*/TSEL[2:0];
	if (rc = camsensor_i2c_write_byte(0x1A,0x90)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x1B,0x00)) {break;} //*/*/*/CKREF_DIV[4:0];
	if (rc = camsensor_i2c_write_byte(0x1C,0x00)) {break;} //CKVAR_SS0DIV[7:0];
	if (rc = camsensor_i2c_write_byte(0x1D,0x00)) {break;} //*/SPCK_SEL/*/EXTCLK_THROUGH/*/*/*/CKVAR_SS0DIV[8];
	if (rc = camsensor_i2c_write_byte(0x1E,0x8F)) {break;} //CKVAR_SS1DIV[7:0];
	if (rc = camsensor_i2c_write_byte(0x1F,0x00)) {break;} //MRCK_DIV[3:0]/*/*/*/CKVAR_SS1DIV[8];
	if (rc = camsensor_i2c_write_byte(0x20,0x80)) {break;} //VCO_DIV[1:0]/*/CLK_SEL[1:0]/AMON0SEL[1:0]/*;
	if (rc = camsensor_i2c_write_byte(0x21,0x0B)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x22,0x07)) {break;} //TBINV/RLINV//WIN_MODE//HV_INTERMIT[2:0];
	if (rc = camsensor_i2c_write_byte(0x23,0x96)) {break;} //H_COUNT[7:0];
	if (rc = camsensor_i2c_write_byte(0x24,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x25,0x42)) {break;} //V_COUNT[7:0];
	if (rc = camsensor_i2c_write_byte(0x26,0x00)) {break;} //*/*/*/*/*/V_COUNT[10:8];
	if (rc = camsensor_i2c_write_byte(0x27,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x28,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x29,0x83)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x2A,0x84)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x2B,0xAE)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x2C,0x21)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x2D,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x2E,0x04)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x2F,0x7D)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x30,0x19)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x31,0x88)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x32,0x88)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x33,0x09)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x34,0x6C)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x35,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x36,0x90)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x37,0x22)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x38,0x0B)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x39,0xAA)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x3A,0x0A)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x3B,0x84)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x3C,0x03)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x3D,0x10)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x3E,0x4C)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x3F,0x1D)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x40,0x34)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x41,0x05)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x42,0x12)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x43,0xB0)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x44,0x3F)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x45,0xFF)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x46,0x44)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x47,0x44)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x48,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x49,0xE8)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x4A,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x4B,0x9F)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x4C,0x9B)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x4D,0x2B)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x4E,0x53)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x4F,0x50)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x50,0x0E)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x51,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x52,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x53,0x04)) {break;} //TP_MODE[4:0]/TPG_DR_SEL/TPG_CBLK_SW/TPG_LINE_SW;
	if (rc = camsensor_i2c_write_byte(0x54,0x08)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x55,0x14)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x56,0x84)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x57,0x30)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x58,0x80)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x59,0x80)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x5A,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x5B,0x06)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x5C,0xF0)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x5D,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x5E,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x5F,0xB0)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x60,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x61,0x1B)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x62,0x4F)) {break;} //HYUKO_START[7:0];
	if (rc = camsensor_i2c_write_byte(0x63,0x04)) {break;} //VYUKO_START[7:0];
	if (rc = camsensor_i2c_write_byte(0x64,0x10)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x65,0x20)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x66,0x30)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x67,0x28)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x68,0x66)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x69,0xC0)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x6A,0x30)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x6B,0x30)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x6C,0x3F)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x6D,0xBF)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x6E,0xAB)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x6F,0x30)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x70,0x80)) {break;} //AGMIN_BLACK_ADJ[7:0];
	if (rc = camsensor_i2c_write_byte(0x71,0x90)) {break;} //AGMAX_BLACK_ADJ[7:0];
	if (rc = camsensor_i2c_write_byte(0x72,0x00)) {break;} //IDR_SET[7:0];
	if (rc = camsensor_i2c_write_byte(0x73,0x28)) {break;} //PWB_RG[7:0];
	if (rc = camsensor_i2c_write_byte(0x74,0x00)) {break;} //PWB_GRG[7:0];
	if (rc = camsensor_i2c_write_byte(0x75,0x00)) {break;} //PWB_GBG[7:0];
	if (rc = camsensor_i2c_write_byte(0x76,0x58)) {break;} //PWB_BG[7:0];
	if (rc = camsensor_i2c_write_byte(0x77,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x78,0x80)) {break;} //LSSC_SW/*/*/*/*/*/*/*/;
	if (rc = camsensor_i2c_write_byte(0x79,0x52)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x7A,0x4F)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x7B,0x90)) {break;} //LSSC_LEFT_RG[7:0];
	if (rc = camsensor_i2c_write_byte(0x7C,0x4D)) {break;} //LSSC_LEFT_GG[7:0];
	if (rc = camsensor_i2c_write_byte(0x7D,0x44)) {break;} //LSSC_LEFT_BG[7:0];
	if (rc = camsensor_i2c_write_byte(0x7E,0xC3)) {break;} //LSSC_RIGHT_RG[7:0];
	if (rc = camsensor_i2c_write_byte(0x7F,0x77)) {break;} //LSSC_RIGHT_GG[7:0];
	if (rc = camsensor_i2c_write_byte(0x80,0x67)) {break;} //LSSC_RIGHT_BG[7:0];
	if (rc = camsensor_i2c_write_byte(0x81,0x6D)) {break;} //LSSC_TOP_RG[7:0];
	if (rc = camsensor_i2c_write_byte(0x82,0x50)) {break;} //LSSC_TOP_GG[7:0];
	if (rc = camsensor_i2c_write_byte(0x83,0x3C)) {break;} //LSSC_TOP_BG[7:0];
	if (rc = camsensor_i2c_write_byte(0x84,0x78)) {break;} //LSSC_BOTTOM_RG[7:0];
	if (rc = camsensor_i2c_write_byte(0x85,0x4B)) {break;} //LSSC_BOTTOM_GG[7:0];
	if (rc = camsensor_i2c_write_byte(0x86,0x31)) {break;} //LSSC_BOTTOM_BG[7:0];
	if (rc = camsensor_i2c_write_byte(0x87,0x01)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x88,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x89,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x8A,0x40)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x8B,0x09)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x8C,0xE0)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x8D,0x20)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x8E,0x20)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x8F,0x20)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x90,0x20)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x91,0x80)) {break;} //ANR_SW/*/*/*/TEST_ANR/*/*/*;
	if (rc = camsensor_i2c_write_byte(0x92,0x30)) {break;} //AGMIN_ANR_WIDTH[7:0];
	if (rc = camsensor_i2c_write_byte(0x93,0x40)) {break;} //AGMAX_ANR_WIDTH[7:0];
	if (rc = camsensor_i2c_write_byte(0x94,0x40)) {break;} //AGMIN_ANR_MP[7:0];
	if (rc = camsensor_i2c_write_byte(0x95,0x80)) {break;} //AGMAX_ANR_MP[7:0];
	if (rc = camsensor_i2c_write_byte(0x96,0x80)) {break;} //DTL_SW/*/*/*/*/*/*/*/;
	if (rc = camsensor_i2c_write_byte(0x97,0x20)) {break;} //AGMIN_HDTL_NC[7:0];
	if (rc = camsensor_i2c_write_byte(0x98,0x68)) {break;} //AGMIN_VDTL_NC[7:0];
	if (rc = camsensor_i2c_write_byte(0x99,0xFF)) {break;} //AGMAX_HDTL_NC[7:0];
	if (rc = camsensor_i2c_write_byte(0x9A,0xFF)) {break;} //AGMAX_VDTL_NC[7:0];
	if (rc = camsensor_i2c_write_byte(0x9B,0x5C)) {break;} //AGMIN_HDTL_MG[7:0];
	if (rc = camsensor_i2c_write_byte(0x9C,0x28)) {break;} //AGMIN_HDTL_PG[7:0];
	if (rc = camsensor_i2c_write_byte(0x9D,0x40)) {break;} //AGMIN_VDTL_MG[7:0];
	if (rc = camsensor_i2c_write_byte(0x9E,0x28)) {break;} //AGMIN_VDTL_PG[7:0];
	if (rc = camsensor_i2c_write_byte(0x9F,0x00)) {break;} //AGMAX_HDTL_MG[7:0];
	if (rc = camsensor_i2c_write_byte(0xA0,0x00)) {break;} //AGMAX_HDTL_PG[7:0];
	if (rc = camsensor_i2c_write_byte(0xA1,0x00)) {break;} //AGMAX_VDTL_MG[7:0];
	if (rc = camsensor_i2c_write_byte(0xA2,0x00)) {break;} //AGMAX_VDTL_PG[7:0];
	if (rc = camsensor_i2c_write_byte(0xA3,0x80)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xA4,0x82)) {break;} //HCBC_SW/*/*/*/*/*/*/*/;
	if (rc = camsensor_i2c_write_byte(0xA5,0x38)) {break;} //AGMIN_HCBC_G[7:0];
	if (rc = camsensor_i2c_write_byte(0xA6,0x18)) {break;} //AGMAX_HCBC_G[7:0];
	if (rc = camsensor_i2c_write_byte(0xA7,0x98)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xA8,0x98)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xA9,0x98)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xAA,0x10)) {break;} //LMCC_BMG_SEL/LMCC_BMR_SEL/*/LMCC_GMB_SEL/LMCC_GMR_SEL/*/;
	if (rc = camsensor_i2c_write_byte(0xAB,0x5B)) {break;} //LMCC_RMG_G[7:0];
	if (rc = camsensor_i2c_write_byte(0xAC,0x00)) {break;} //LMCC_RMB_G[7:0];
	if (rc = camsensor_i2c_write_byte(0xAD,0x00)) {break;} //LMCC_GMR_G[7:0];
	if (rc = camsensor_i2c_write_byte(0xAE,0x00)) {break;} //LMCC_GMB_G[7:0];
	if (rc = camsensor_i2c_write_byte(0xAF,0x00)) {break;} //LMCC_BMR_G[7:0];
	if (rc = camsensor_i2c_write_byte(0xB0,0x48)) {break;} //LMCC_BMG_G[7:0];
	if (rc = camsensor_i2c_write_byte(0xB1,0xC2)) {break;} //GAM_SW[1:0]/*/CGC_DISP/TEST_AWBDISP/*/YUVM_AWBDISP_SW/YU;
	if (rc = camsensor_i2c_write_byte(0xB2,0x4D)) {break;} //*/R_MATRIX[6:0];
	if (rc = camsensor_i2c_write_byte(0xB3,0x10)) {break;} //*/B_MATRIX[6:0];
	if (rc = camsensor_i2c_write_byte(0xB4,0xC8)) {break;} //UVG_SEL/BRIGHT_SEL/*/TEST_YUVM_PE/NEG_YMIN_SW/PIC_EFFECT;
	if (rc = camsensor_i2c_write_byte(0xB5,0x5B)) {break;} //CONTRAST[7:0];
	if (rc = camsensor_i2c_write_byte(0xB6,0x47)) {break;} //BRIGHT[7:0];
	if (rc = camsensor_i2c_write_byte(0xB7,0x00)) {break;} //Y_MIN[7:0];
	if (rc = camsensor_i2c_write_byte(0xB8,0xFF)) {break;} //Y_MAX[7:0];
	if (rc = camsensor_i2c_write_byte(0xB9,0x69)) {break;} //U_GAIN[7:0];
	if (rc = camsensor_i2c_write_byte(0xBA,0x72)) {break;} //V_GAIN[7:0];
	if (rc = camsensor_i2c_write_byte(0xBB,0x78)) {break;} //SEPIA_US[7:0];
	if (rc = camsensor_i2c_write_byte(0xBC,0x90)) {break;} //SEPIA_VS[7:0];
	if (rc = camsensor_i2c_write_byte(0xBD,0x04)) {break;} //U_CORING[7:0];
	if (rc = camsensor_i2c_write_byte(0xBE,0x04)) {break;} //V_CORING[7:0];
	if (rc = camsensor_i2c_write_byte(0xBF,0xC0)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xC0,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xC1,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xC2,0x80)) {break;} //ALC_SW/ALC_LOCK/*/*/*/*/*/*/;
	if (rc = camsensor_i2c_write_byte(0xC3,0x14)) {break;} //MES[7:0];
	if (rc = camsensor_i2c_write_byte(0xC4,0x03)) {break;} //*/*/MES[13:8];
	if (rc = camsensor_i2c_write_byte(0xC5,0x00)) {break;} //MDG[7:0];
	if (rc = camsensor_i2c_write_byte(0xC6,0x74)) {break;} //MAG[7:0];
	if (rc = camsensor_i2c_write_byte(0xC7,0x80)) {break;} //AGCONT_SEL[1:0]/*/*/MAG[11:8];
	if (rc = camsensor_i2c_write_byte(0xC8,0x20)) {break;} //AG_MIN[7:0];
	if (rc = camsensor_i2c_write_byte(0xC9,0x06)) {break;} //AG_MAX[7:0];
	if (rc = camsensor_i2c_write_byte(0xCA,0x07)) {break;} //AUTO_LES_SW/AUTO_LES_MODE[2:0]/ALC_WEIGHT[1:0]/FLCKADJ[1;
	if (rc = camsensor_i2c_write_byte(0xCB,0xE7)) {break;} //ALC_LV[7:0];
	if (rc = camsensor_i2c_write_byte(0xCC,0x10)) {break;} //*/UPDN_MODE[2:0]/ALC_LV[9:8];
	if (rc = camsensor_i2c_write_byte(0xCD,0x0A)) {break;} //ALC_LVW[7:0];
	if (rc = camsensor_i2c_write_byte(0xCE,0x4F)) {break;} //L64P600S[7:0];
	if (rc = camsensor_i2c_write_byte(0xCF,0x03)) {break;} //*/ALC_VWAIT[2:0]/L64P600S[11:8];
	if (rc = camsensor_i2c_write_byte(0xD0,0x80)) {break;} //UPDN_SPD[7:0];
	if (rc = camsensor_i2c_write_byte(0xD1,0x20)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xD2,0x80)) {break;} //NEAR_SPD[7:0];
	if (rc = camsensor_i2c_write_byte(0xD3,0x30)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xD4,0x8A)) {break;} //AC5060/*/ALC_SAFETY[5:0];
	if (rc = camsensor_i2c_write_byte(0xD5,0x02)) {break;} //*/*/*/*/*/ALC_SAFETY2[2:0];
	if (rc = camsensor_i2c_write_byte(0xD6,0x4F)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xD7,0x08)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xD8,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xD9,0xFF)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xDA,0x01)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xDB,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xDC,0x14)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xDD,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xDE,0x80)) {break;} //AWB_SW/AWB_LOCK/*/AWB_TEST/*/*/HEXG_SLOPE_SEL/COLGATE_SE;
	if (rc = camsensor_i2c_write_byte(0xDF,0x80)) {break;} //WB_MRG[7:0];
	if (rc = camsensor_i2c_write_byte(0xE0,0x80)) {break;} //WB_MGG[7:0];
	if (rc = camsensor_i2c_write_byte(0xE1,0x80)) {break;} //WB_MBG[7:0];
	if (rc = camsensor_i2c_write_byte(0xE2,0x22)) {break;} //WB_RBMIN[7:0];
	if (rc = camsensor_i2c_write_byte(0xE3,0xF8)) {break;} //WB_RBMAX[7:0];
	if (rc = camsensor_i2c_write_byte(0xE4,0x80)) {break;} //HEXA_SW/*/COLGATE_RANGE[1:0]/*/*/*/COLGATE_OPEN;
	if (rc = camsensor_i2c_write_byte(0xE5,0x2C)) {break;} //*/RYCUTM[6:0];
	if (rc = camsensor_i2c_write_byte(0xE6,0x54)) {break;} //*/RYCUTP[6:0];
	if (rc = camsensor_i2c_write_byte(0xE7,0x28)) {break;} //*/BYCUTM[6:0];
	if (rc = camsensor_i2c_write_byte(0xE8,0x39)) {break;} //*/BYCUTP[6:0];
	if (rc = camsensor_i2c_write_byte(0xE9,0xE4)) {break;} //RBCUTL[7:0];
	if (rc = camsensor_i2c_write_byte(0xEA,0x3C)) {break;} //RBCUTH[7:0];
	if (rc = camsensor_i2c_write_byte(0xEB,0x81)) {break;} //SQ1_SW/SQ1_POL/*/*/*/*/*/YGATE_SW;
	if (rc = camsensor_i2c_write_byte(0xEC,0x37)) {break;} //RYCUT1L[7:0];
	if (rc = camsensor_i2c_write_byte(0xED,0x5A)) {break;} //RYCUT1H[7:0];
	if (rc = camsensor_i2c_write_byte(0xEE,0xDE)) {break;} //BYCUT1L[7:0];
	if (rc = camsensor_i2c_write_byte(0xEF,0x08)) {break;} //BYCUT1H[7:0];
	if (rc = camsensor_i2c_write_byte(0xF0,0x18)) {break;} //YGATE_L[7:0];
	if (rc = camsensor_i2c_write_byte(0xF1,0xFE)) {break;} //YGATE_H[7:0];
	if (rc = camsensor_i2c_write_byte(0xF2,0x00)) {break;} //*/*/IPIX_DISP_SW/*/*/*/*/;
	if (rc = camsensor_i2c_write_byte(0xF3,0x02)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xF4,0x02)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xF5,0x04)) {break;} //AWB_WAIT[7:0];
	if (rc = camsensor_i2c_write_byte(0xF6,0x00)) {break;} //AWB_SPDDLY[7:0];
	if (rc = camsensor_i2c_write_byte(0xF7,0x20)) {break;} //*/*/AWB_SPD[5:0];
	if (rc = camsensor_i2c_write_byte(0xF8,0x86)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xF9,0x00)) {break;} //*/*/*/*/*/*/*/*;
	if (rc = camsensor_i2c_write_byte(0xFA,0x41)) {break;} //MR_HBLK_START[7:0];
	if (rc = camsensor_i2c_write_byte(0xFB,0x50)) {break;} //*/MR_HBLK_WIDTH[6:0];
	if (rc = camsensor_i2c_write_byte(0xFC,0x0C)) {break;} //MR_VBLK_START[7:0];
	if (rc = camsensor_i2c_write_byte(0xFD,0x3C)) {break;} //*/*/MR_VBLK_WIDTH[5:0];
	if (rc = camsensor_i2c_write_byte(0xFE,0x50)) {break;} //PIC_FORMAT[3:0]/PINTEST_SEL[3:0];
	if (rc = camsensor_i2c_write_byte(0xFF,0x85)) {break;} //SLEEP/*/PARALLEL_OUTSW[1:0]/DCLK_POL/DOUT_CBLK_SW/*/AL;
	mdelay(300);
		break; /* case REG_INIT: */

	default:
		rc = -EINVAL;
		break;
	} /* switch (rupdate) */

//	printk("[Camera] test pattern\n");
//	rc = camsensor_i2c_write_word(0xB054, 0x0001);
	return rc;
}

static int tcm9001md_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int32_t  rc;

        printk("#### %s ####\n", __FUNCTION__);

	tcm9001md_ctrl = kzalloc(sizeof(struct tcm9001md_ctrl), GFP_KERNEL);
	if (!tcm9001md_ctrl) {
		CDBG("tcm9001md_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	tcm9001md_ctrl->fps_divider = 1 * 0x00000400;
	tcm9001md_ctrl->pict_fps_divider = 1 * 0x00000400;
	tcm9001md_ctrl->set_test = S_TEST_OFF;
	tcm9001md_ctrl->prev_res = S_QTR_SIZE;
	tcm9001md_ctrl->pict_res = S_FULL_SIZE;

	if (data)
		tcm9001md_ctrl->sensordata = data;


	/*******yhwang-temp******/
	rc = gpio_request(1, "tcm9001md");
	printk("[camera yhwang] tcm9001md gpio_request reset=%d\n", rc);
	if (!rc || rc == -EBUSY)
	{
		gpio_direction_output(1, 0);
		gpio_free(1);            
	}
	else printk(KERN_ERR "tcm9001md reset error: request gpio %d failed: "
			"%d\n", 1 , rc);
        
	rc = gpio_request(20, "tcm9001md");
	printk("[camera yhwang] tcm9001md gpio_request pwd low=%d\n", rc);
	if(!rc || rc == -EBUSY)
	{
		gpio_direction_output(20, 0);
		gpio_free(20);
	}
	else printk(KERN_ERR "tcm9001md pwd error: request gpio %d failed: "
			"%d\n", 20, rc);
	printk("[camera yhwang] ......\n");
	/*******************************/  

	/* pull down power-down */
	rc = gpio_request(data->sensor_pwd, "tcm9001md");
	printk("[camera] gpio_request pwd low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(data->sensor_pwd, 0);
	else printk(KERN_ERR "tcm9001md error: request gpio %d failed: "
			"%d\n", data->sensor_pwd, rc);

	tcm9001md_power_enable();

	/* pull up power-down */
	rc = gpio_request(data->sensor_pwd, "tcm9001md");
	printk("[camera] gpio_request pwd high=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(data->sensor_pwd, 1);
	else printk(KERN_ERR "tcm9001md error: request gpio %d failed: "
			"%d\n", data->sensor_pwd, rc);

	/* enable mclk first */
	msm_camio_clk_rate_set(TCM9001MD_DEFAULT_CLOCK_RATE);
	mdelay(100);

	/* pull down power-down */
	rc = gpio_request(data->sensor_pwd, "tcm9001md");
	printk("[camera] gpio_request pwd low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(data->sensor_pwd, 0);
	else printk(KERN_ERR "tcm9001md error: request gpio %d failed: "
			"%d\n", data->sensor_pwd, rc);
	mdelay(20);

	msm_camio_camif_pad_reg_reset();
	mdelay(20);

	rc = tcm9001md_probe_init_sensor(data);
	if (rc < 0)
		goto init_fail1;

	if (tcm9001md_ctrl->prev_res == S_QTR_SIZE)
		rc = tcm9001md_setting(S_REG_INIT, S_RES_PREVIEW);
	else
		rc = tcm9001md_setting(S_REG_INIT, S_RES_CAPTURE);

	if (rc < 0) {
		CDBG("tcm9001md_setting failed. rc = %d\n", rc);
		goto init_fail1;
	}

	goto init_done;

init_fail1:
	tcm9001md_probe_init_done(data);
	kfree(tcm9001md_ctrl);
init_done:
	return rc;
}

static int32_t tcm9001md_power_down(void)
{
	int32_t rc = 0;
	return rc;
}

static int tcm9001md_sensor_release(void)
{
	int rc = -EBADF;

	mutex_lock(&tcm9001md_mutex);

	tcm9001md_power_down();

	gpio_direction_output(tcm9001md_ctrl->sensordata->sensor_reset,
		0);
	gpio_free(tcm9001md_ctrl->sensordata->sensor_reset);

	gpio_direction_output(tcm9001md_ctrl->sensordata->sensor_pwd,
		0);
	gpio_free(tcm9001md_ctrl->sensordata->sensor_pwd);
	mdelay(1);

	tcm9001md_power_disable();

	kfree(tcm9001md_ctrl);
	tcm9001md_ctrl = NULL;

	CDBG("tcm9001md_release completed\n");

	mutex_unlock(&tcm9001md_mutex);
	return rc;
}

static void tcm9001md_get_pict_fps(uint16_t fps, uint16_t *pfps)
{
	/* input fps is preview fps in Q8 format */
	uint32_t divider;   /* Q10 */

/*
	divider = (uint32_t)
		((tcm9001md_reg_pat[S_RES_PREVIEW].size_h +
			tcm9001md_reg_pat[S_RES_PREVIEW].blk_l) *
		 (tcm9001md_reg_pat[S_RES_PREVIEW].size_w +
			tcm9001md_reg_pat[S_RES_PREVIEW].blk_p)) * 0x00000400 /
		((tcm9001md_reg_pat[S_RES_CAPTURE].size_h +
			tcm9001md_reg_pat[S_RES_CAPTURE].blk_l) *
		 (tcm9001md_reg_pat[S_RES_CAPTURE].size_w +
			tcm9001md_reg_pat[S_RES_CAPTURE].blk_p));
*/
	/* Verify PCLK settings and frame sizes. */
	*pfps = (uint16_t)(fps);
}

static uint16_t tcm9001md_get_prev_lines_pf(void)
{
	//return tcm9001md_reg_pat[S_RES_PREVIEW].size_h +
		//tcm9001md_reg_pat[S_RES_PREVIEW].blk_l;
		return 480;
}

static uint16_t tcm9001md_get_prev_pixels_pl(void)
{
	//return tcm9001md_reg_pat[S_RES_PREVIEW].size_w +
		//tcm9001md_reg_pat[S_RES_PREVIEW].blk_p;
		return 640;
}

static uint16_t tcm9001md_get_pict_lines_pf(void)
{
	//return tcm9001md_reg_pat[S_RES_CAPTURE].size_h +
		//tcm9001md_reg_pat[S_RES_CAPTURE].blk_l;
		return 480;
}

static uint16_t tcm9001md_get_pict_pixels_pl(void)
{
	//return tcm9001md_reg_pat[S_RES_CAPTURE].size_w +
		//tcm9001md_reg_pat[S_RES_CAPTURE].blk_p;
		return 640;
}

static uint32_t tcm9001md_get_pict_max_exp_lc(void)
{
	uint32_t snapshot_lines_per_frame;

	if (tcm9001md_ctrl->pict_res == S_QTR_SIZE)
		//snapshot_lines_per_frame =
		//tcm9001md_reg_pat[S_RES_PREVIEW].size_h +
		//tcm9001md_reg_pat[S_RES_PREVIEW].blk_l;
		snapshot_lines_per_frame =480;
	else
		//snapshot_lines_per_frame = 3961 * 3;
		snapshot_lines_per_frame = 480;

	return snapshot_lines_per_frame;
}

static uint32_t tcm9001md_style_tuning(void)
{
	int32_t rc = 0;
    /*
	rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_UserContrast & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserContrast & 0xFFFF), Contrast_Value);
 	if (rc < 0)
	{
		return rc;
	}	
 	rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_UserSaturation & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserSaturation & 0xFFFF), Saturation_Value);
	if (rc < 0)
	{
		return rc;
	}	
	rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_UserBrightness & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserBrightness & 0xFFFF), Sharpness_Value);
	if (rc < 0)
	{
		return rc;
	}	
*/
	return rc;  
}
static int32_t tcm9001md_set_brightness (int8_t brightness)
{
 uint16_t brightness_value = 0;
 int32_t rc = 0;
 uint32_t  brightness_offset;
 
 brightness_offset = (TCM9001MD_BRIGHTNESS_DEGREE-1)/2;
 brightness = brightness - brightness_offset; 
 
 switch(brightness)
 {
     case 5:
	brightness_value = Brightness_Value[10];
       break;
	   
    case 4:	
	brightness_value = Brightness_Value[9];
       break;	
	   
    case 3:
	brightness_value = Brightness_Value[8];
       break;
	   
    case 2:
	brightness_value = Brightness_Value[7];
       break;	
	   
    case 1:
	brightness_value = Brightness_Value[6];
       break;
	   
    case 0:	
	brightness_value = Brightness_Value[5];
       break;
	   
    case -1:
	brightness_value = Brightness_Value[4];
       break;
	   
    case -2:
	brightness_value = Brightness_Value[3];
       break;
	   
    case -3:
	brightness_value = Brightness_Value[2];
       break;	
	   
    case -4:
	brightness_value = Brightness_Value[1];
       break;
	   
    case -5:
	brightness_value = Brightness_Value[0];
       break;	   
	   
    default:
	brightness_value = Brightness_Value[5];
    	break;
 }
   rc=camsensor_i2c_write_byte(0xB6,brightness_value);
   //rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_UserBrightness & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserBrightness & 0xFFFF), brightness_value);
    if (rc < 0)
   {
      return rc;
   }	

  return rc;  
}


static int32_t tcm9001md_set_contrast (int8_t contrast)
{
#ifndef USE_CAMSENSOR_TOSHIBA_TCM9001MD  
	int32_t rc = 0;

	//rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_UserContrast & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserContrast & 0xFFFF), Contrast_Value);
 	if (rc < 0)
	{
		return rc;
	}	

	return rc;  
#endif
}

static int32_t tcm9001md_set_sharpness(int8_t sharpness)
{
#ifndef USE_CAMSENSOR_TOSHIBA_TCM9001MD  
   int32_t rc = 0;

#endif
  return rc;  
}
static int32_t tcm9001md_set_effect (int8_t effect)
{
 int32_t rc = 0;
    /*
	 switch(effect)
	 {
	    case CAMERA_EFFECT_OFF:
		rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0000);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
		break;
		
	    case CAMERA_EFFECT_NEGATIVE:
		rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0003);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
	       break;
		   
	    case CAMERA_EFFECT_SOLARIZE:
	       break;
		   
	    case CAMERA_EFFECT_SEPIA:
		rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0004);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
	       break;

	    case CAMERA_EFFECT_MONO:
		rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0001);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
	       break;

	    case CAMERA_EFFECT_AQUA:
		rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0005);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
	       break;

	     default:
		 break;	
	 }
*/
  return rc;  
}

static int32_t tcm9001md_set_antibanding (int8_t antibanding)
{
#ifndef USE_CAMSENSOR_TOSHIBA_TCM9001MD 
   int32_t rc = 0; 

//   MSG_HIGH("into tcm9001md_set_antibanding antiband=%d",antibanding,0,0);
   switch(antibanding)
   {
      case CAMERA_ANTIBANDING_60HZ:
         rc = tcm9001md_write_FlickerMode_60hz_reg();
         if (rc < 0)
         {
            return rc;
         } 
	  break;	 
      case CAMERA_ANTIBANDING_50HZ:
         rc = tcm9001md_write_FlickerMode_50hz_reg();
         if (rc < 0)
         {
            return rc;
         } 
	  break;	 
	default:
	   break;
   }
#endif  
  return rc;  
}
static int32_t tcm9001md_set_wb (int8_t wb)
{
	int32_t rc = 0;
#if 0
	switch(wb)
	{
		case CAMERA_WB_AUTO:

			rc = tcm9001md_write_AWB_reg();
			if (rc < 0)
			{
				return rc;
			}  	
         		break; 	  

		case CAMERA_WB_DAYLIGHT:
			rc = tcm9001md_write_WB_daylight_reg();
			if (rc < 0)
			{
         			return rc;
			}  	
			break; 	 

		case CAMERA_WB_CLOUDY_DAYLIGHT:
			rc = tcm9001md_write_WB_daylight_reg();
			if (rc < 0)
			{
         			return rc;
			}  	
			break; 	 

		case CAMERA_WB_FLUORESCENT:

			rc = tcm9001md_write_WB_fluorescent_reg();	
			if (rc < 0)
			{
				return rc;
			}  
			break; 	 
		case CAMERA_WB_INCANDESCENT:
			rc = tcm9001md_write_WB_incandescent_reg();	
			if (rc < 0)
			{
				return rc;
			}  
			break; 	 
/*
		case CAMERA_WB_HORIZON:
			rc = tcm9001md_write_WB_horizon_reg();
			if (rc < 0)
			{
				return rc;
			}  		  	  	
			break; 	 
*/
		default:

			rc = tcm9001md_write_AWB_reg();
			if (rc < 0)
			{
				return rc;
			}  				
			break; 	 
	}
#endif
	return rc;  
}

static int32_t tcm9001md_set_iso (int8_t iso)
{
	int32_t rc = 0;
    /*
	switch(iso)
	{
		case CAMERA_ISO_100:
			//set manual ISO, 0 for auto
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0001);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x0064);
		 	if (rc < 0)
			{
				return rc;
			}
			break;

		case CAMERA_ISO_200:
			//set manual ISO, 0 for auto
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0001);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x00C8);
		 	if (rc < 0)
			{
				return rc;
			}
			break;

		case CAMERA_ISO_400:
			//set manual ISO, 0 for auto
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0001);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x0190);
		 	if (rc < 0)
			{
				return rc;
			}
			break;

		case CAMERA_ISO_800:
			//set manual ISO, 0 for auto
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0001);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x0320);
		 	if (rc < 0)
			{
				return rc;
			}
			break;

		case CAMERA_ISO_AUTO:
		default:
			//set manual ISO, 0 for auto
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0000);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x0064);
		 	if (rc < 0)
			{
				return rc;
			}
			break;
	}

	//activate new ISO value
	rc = camsensor_tcm9001md_i2c_write((uint16_t)((REG_SF_USER_IsoChanged & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoChanged & 0xFFFF), 0x0001);
      */
	return rc;  
}

static int32_t tcm9001md_set_exposure_mode(int8_t exposure_mode)
{
	int32_t rc = 0;
	uint32_t		ary_size=0;
	int			i;
/*
	rc = camsensor_i2c_write_word(0x0028, (uint16_t)((ae_WeightTbl_16 & 0xFFFF0000)>>16));
	if(rc < 0)
	{
		return rc;
	 }
	rc = camsensor_i2c_write_word(0x002A, (uint16_t)(ae_WeightTbl_16 & 0xFFFF));
	if(rc < 0)
	{
		return rc;
 	}

	switch(exposure_mode)
	{
		case CAMERA_AEC_CENTER_WEIGHTED:
			ary_size = sizeof(cam_exposure_mode_center_weighted)/sizeof(uint16_t);
			for(i=0;i<ary_size;i++)
			{
				rc=camsensor_i2c_write_word(0x0F12, cam_exposure_mode_center_weighted[i]);
				if(rc < 0)
				{
         				return rc;
				}
			}
			break;

		case CAMERA_AEC_SPOT_METERING:
			ary_size = sizeof(cam_exposure_mode_spot_metering)/sizeof(uint16_t);
			for(i=0;i<ary_size;i++)
			{
				rc=camsensor_i2c_write_word(0x0F12, cam_exposure_mode_spot_metering[i]);
				if(rc < 0)
				{
         				return rc;
				}
			}
			break;

		case CAMERA_AEC_FRAME_AVERAGE:
		default:
			ary_size = sizeof(cam_exposure_mode_frame_avg)/sizeof(uint16_t);
			for(i=0;i<ary_size;i++)
			{
				rc=camsensor_i2c_write_word(0x0F12, cam_exposure_mode_frame_avg[i]);
				if(rc < 0)
				{
         				return rc;
				}
			}
			break;
	}
*/
	return rc;  
}
static int32_t tcm9001md_set_bestshot_mode (int8_t mode)
{
	int32_t rc = 0;
	uint32_t ary_size=0;
	int		i;

	 switch(mode)
	 {
		case CAMERA_BESTSHOT_NIGHT:
			rc = tcm9001md_write_bestshot_mode_night_reg();			
			if (rc < 0)
			{
				return rc;
			}	
		  	break;
		  	
		case CAMERA_BESTSHOT_SPORTS:
#if 0 
			ary_size = sizeof(cam_extra_mode_sports)/sizeof(CAM_REG_ADDR_VAL_TYPE);

			for(i=0;i<ary_size;i++)
			{
				rc = camsensor_i2c_write_byte(cam_extra_mode_sports[i].addr, cam_extra_mode_sports[i].val);
			
				if (rc < 0)
				{
         				return rc;
				}
			}
#endif
		  	break;	
		  	
		case CAMERA_BESTSHOT_BEACH:
			rc = tcm9001md_write_bestshot_mode_beach_reg();			
				if (rc < 0)
				{
         				return rc;
				}
			
		  	break;		
		  	
		case CAMERA_BESTSHOT_PORTRAIT:
#if 0
			ary_size = sizeof(cam_extra_mode_portrait)/sizeof(CAM_REG_ADDR_VAL_TYPE);

			for(i=0;i<ary_size;i++)
			{
				rc = camsensor_i2c_write_byte(cam_extra_mode_portrait[i].addr, cam_extra_mode_portrait[i].val);
			
				if (rc < 0)
				{
         				return rc;
				}
			}
#endif		
			tcm9001md_style_tuning();
		  	break;			

#if 0
		 case CAMERA_BESTSHOT_NORMAL:
			rc = camsensor_i2c_write_byte(0x3014, 0x2c);
			if (rc < 0)
 			{
 				return rc;
 			}	

			rc = camsensor_i2c_write_byte(0x3015, 0x31);
			if (rc < 0)
 			{
 				return rc;
 			}	
		  	break;
#endif			
			
		default:
#if 0
			rc = camsensor_i2c_write_byte(0x3014, 0x2c);
			if (rc < 0)
 			{
 				return rc;
 			}	

			rc = camsensor_i2c_write_byte(0x3015, 0x12);
			if (rc < 0)
 			{
 				return rc;
 			}	
#endif			
			break;	
	 }

	if(mode != CAMERA_BESTSHOT_NIGHT)
	{
		rc = tcm9001md_write_bestshot_mode_night_break_reg();
		if (rc < 0)
 		{
 			return rc;
 		}	
	}
	
	return rc;  
}

static int32_t tcm9001md_set_fps(struct fps_cfg *fps)
{
	/* input is new fps in Q10 format */
	int32_t rc = 0;
	enum msm_s_setting setting;

	tcm9001md_ctrl->fps_divider = fps->fps_div;

	if (tcm9001md_ctrl->sensormode == SENSOR_PREVIEW_MODE)
		setting = S_RES_PREVIEW;
	else
		setting = S_RES_CAPTURE;

  rc = tcm9001md_i2c_write_b(tcm9001md_client->addr,
		REG_FRAME_LENGTH_LINES_MSB,
		(((tcm9001md_reg_pat[setting].size_h +
			tcm9001md_reg_pat[setting].blk_l) *
			tcm9001md_ctrl->fps_divider / 0x400) & 0xFF00) >> 8);
	if (rc < 0)
		goto set_fps_done;

  rc = tcm9001md_i2c_write_b(tcm9001md_client->addr,
		REG_FRAME_LENGTH_LINES_LSB,
		(((tcm9001md_reg_pat[setting].size_h +
			tcm9001md_reg_pat[setting].blk_l) *
			tcm9001md_ctrl->fps_divider / 0x400) & 0x00FF));

set_fps_done:
	return rc;
}

static int32_t tcm9001md_write_exp_gain(uint16_t gain, uint32_t line)
{
	int32_t rc = 0;
#if 0
	uint16_t max_legal_gain = 0x0200;
	uint32_t ll_ratio; /* Q10 */
	uint32_t ll_pck, fl_lines;
	uint16_t offset = 4;
	uint32_t  gain_msb, gain_lsb;
	uint32_t  intg_t_msb, intg_t_lsb;
	uint32_t  ll_pck_msb, ll_pck_lsb;

	struct tcm9001md_i2c_reg_conf tbl[2];

	CDBG("Line:%d tcm9001md_write_exp_gain \n", __LINE__);

	if (tcm9001md_ctrl->sensormode == SENSOR_PREVIEW_MODE) {

		tcm9001md_ctrl->my_reg_gain = gain;
		tcm9001md_ctrl->my_reg_line_count = (uint16_t)line;

		fl_lines = tcm9001md_reg_pat[S_RES_PREVIEW].size_h +
			tcm9001md_reg_pat[S_RES_PREVIEW].blk_l;

		ll_pck = tcm9001md_reg_pat[S_RES_PREVIEW].size_w +
			tcm9001md_reg_pat[S_RES_PREVIEW].blk_p;

	} else {

		fl_lines = tcm9001md_reg_pat[S_RES_CAPTURE].size_h +
			tcm9001md_reg_pat[S_RES_CAPTURE].blk_l;

		ll_pck = tcm9001md_reg_pat[S_RES_CAPTURE].size_w +
			tcm9001md_reg_pat[S_RES_CAPTURE].blk_p;
	}

	if (gain > max_legal_gain)
		gain = max_legal_gain;

	/* in Q10 */
	line = (line * tcm9001md_ctrl->fps_divider);

	if (fl_lines < (line / 0x400))
		ll_ratio = (line / (fl_lines - offset));
	else
		ll_ratio = 0x400;

	/* update gain registers */
	gain_msb = (gain & 0xFF00) >> 8;
	gain_lsb = gain & 0x00FF;
	tbl[0].waddr = REG_ANALOGUE_GAIN_CODE_GLOBAL_MSB;
	tbl[0].bdata = gain_msb;
	tbl[1].waddr = REG_ANALOGUE_GAIN_CODE_GLOBAL_LSB;
	tbl[1].bdata = gain_lsb;
	rc = tcm9001md_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));
	if (rc < 0)
		goto write_gain_done;

	ll_pck = ll_pck * ll_ratio;
	ll_pck_msb = ((ll_pck / 0x400) & 0xFF00) >> 8;
	ll_pck_lsb = (ll_pck / 0x400) & 0x00FF;
	tbl[0].waddr = REG_LINE_LENGTH_PCK_MSB;
	tbl[0].bdata = ll_pck_msb;
	tbl[1].waddr = REG_LINE_LENGTH_PCK_LSB;
	tbl[1].bdata = ll_pck_lsb;
	rc = tcm9001md_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));
	if (rc < 0)
		goto write_gain_done;

	line = line / ll_ratio;
	intg_t_msb = (line & 0xFF00) >> 8;
	intg_t_lsb = (line & 0x00FF);
	tbl[0].waddr = REG_COARSE_INTEGRATION_TIME;
	tbl[0].bdata = intg_t_msb;
	tbl[1].waddr = REG_COARSE_INTEGRATION_TIME_LSB;
	tbl[1].bdata = intg_t_lsb;
	rc = tcm9001md_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));
#endif
write_gain_done:
	return rc;
}

static int32_t tcm9001md_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
	int32_t rc = 0;

	CDBG("Line:%d tcm9001md_set_pict_exp_gain \n", __LINE__);

	rc =
		tcm9001md_write_exp_gain(gain, line);

	return rc;
}
static int32_t tcm9001md_video_config(int mode, int res)
{
	int32_t rc = 0;

	switch (res) {
	case S_QTR_SIZE:
		rc = tcm9001md_setting(S_UPDATE_PERIODIC, S_RES_PREVIEW);
		if (rc < 0)
			return rc;

		CDBG("tcm9001md sensor configuration done!\n");
		break;

	case S_FULL_SIZE:
		rc = tcm9001md_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
		if (rc < 0)
			return rc;

		break;

	default:
		return 0;
	} /* switch */

	tcm9001md_ctrl->prev_res = res;
	tcm9001md_ctrl->curr_res = res;
	tcm9001md_ctrl->sensormode = mode;

	rc =
		tcm9001md_write_exp_gain(tcm9001md_ctrl->my_reg_gain,
			tcm9001md_ctrl->my_reg_line_count);

	return 0;
}

static int32_t tcm9001md_snapshot_config(int mode)
{
	int32_t rc = 0;

	rc = tcm9001md_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
	if (rc < 0)
		return rc;

	tcm9001md_ctrl->curr_res = tcm9001md_ctrl->pict_res;
	tcm9001md_ctrl->sensormode = mode;

	return rc;
}

static int32_t tcm9001md_raw_snapshot_config(int mode)
{
	int32_t rc = 0;

	rc = tcm9001md_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
	if (rc < 0)
		return rc;

	tcm9001md_ctrl->curr_res = tcm9001md_ctrl->pict_res;
	tcm9001md_ctrl->sensormode = mode;

	return rc;
}

static int32_t tcm9001md_set_sensor_mode(int mode, int res)
{
	int32_t rc = 0;

	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		rc = tcm9001md_video_config(mode, res);
		break;

	case SENSOR_SNAPSHOT_MODE:
		rc = tcm9001md_snapshot_config(mode);
		break;

	case SENSOR_RAW_SNAPSHOT_MODE:
		rc = tcm9001md_raw_snapshot_config(mode);
		break;

	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int32_t tcm9001md_set_default_focus(void)
{
	int32_t rc = 0;

  rc = tcm9001md_i2c_write_b(tcm9001md_client->addr,
		0x3131, 0);
	if (rc < 0)
		return rc;

  rc = tcm9001md_i2c_write_b(tcm9001md_client->addr,
		0x3132, 0);
	if (rc < 0)
		return rc;

	tcm9001md_ctrl->curr_lens_pos = 0;

	return rc;
}

static int32_t tcm9001md_move_focus(int direction, int32_t num_steps)
{
	int32_t rc = 0;
	int32_t i;
	int16_t step_direction;
	int16_t actual_step;
	int16_t next_pos, pos_offset;
	int16_t init_code = 50;
	uint8_t next_pos_msb, next_pos_lsb;
	int16_t s_move[5];
	uint32_t gain; /* Q10 format */

	if (direction == MOVE_NEAR)
		step_direction = 20;
	else if (direction == MOVE_FAR)
		step_direction = -20;
	else {
		CDBG("tcm9001md_move_focus failed at line %d ...\n", __LINE__);
		return -EINVAL;
	}

	actual_step = step_direction * (int16_t)num_steps;
	pos_offset = init_code + tcm9001md_ctrl->curr_lens_pos;
	gain = actual_step * 0x400 / 5;

	for (i = 0; i <= 4; i++) {
		if (actual_step >= 0)
			s_move[i] = (((i+1)*gain+0x200)-(i*gain+0x200))/0x400;
		else
			s_move[i] = (((i+1)*gain-0x200)-(i*gain-0x200))/0x400;
	}

	/* Ring Damping Code */
	for (i = 0; i <= 4; i++) {
		next_pos = (int16_t)(pos_offset + s_move[i]);

		if (next_pos > (738 + init_code))
			next_pos = 738 + init_code;
		else if (next_pos < 0)
			next_pos = 0;

		CDBG("next_position in damping mode = %d\n", next_pos);
		/* Writing the Values to the actuator */
		if (next_pos == init_code)
			next_pos = 0x00;

		next_pos_msb = next_pos >> 8;
		next_pos_lsb = next_pos & 0x00FF;

		rc = tcm9001md_i2c_write_b(tcm9001md_client->addr,
			0x3131, next_pos_msb);
		if (rc < 0)
			break;

		rc = tcm9001md_i2c_write_b(tcm9001md_client->addr,
			0x3132, next_pos_lsb);
		if (rc < 0)
			break;

		pos_offset = next_pos;
		tcm9001md_ctrl->curr_lens_pos = pos_offset - init_code;
		if (i < 4)
			mdelay(3);
	}

	return rc;
}

static int tcm9001md_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;

	if (copy_from_user(&cdata,
			(void *)argp,
			sizeof(struct sensor_cfg_data)))
		return -EFAULT;

	mutex_lock(&tcm9001md_mutex);

	CDBG("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
	printk("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
	switch (cdata.cfgtype) {
	case CFG_GET_PICT_FPS:
		tcm9001md_get_pict_fps(cdata.cfg.gfps.prevfps,
			&(cdata.cfg.gfps.pictfps));

		if (copy_to_user((void *)argp, &cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;

	case CFG_GET_PREV_L_PF:
		cdata.cfg.prevl_pf = tcm9001md_get_prev_lines_pf();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;

	case CFG_GET_PREV_P_PL:
		cdata.cfg.prevp_pl = tcm9001md_get_prev_pixels_pl();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;

	case CFG_GET_PICT_L_PF:
		cdata.cfg.pictl_pf = tcm9001md_get_pict_lines_pf();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;

	case CFG_GET_PICT_P_PL:
		cdata.cfg.pictp_pl = tcm9001md_get_pict_pixels_pl();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;

	case CFG_GET_PICT_MAX_EXP_LC:
		cdata.cfg.pict_max_exp_lc =
			tcm9001md_get_pict_max_exp_lc();

		if (copy_to_user((void *)argp,
				&cdata,
				sizeof(struct sensor_cfg_data)))
			rc = -EFAULT;
		break;

	case CFG_SET_FPS:
	case CFG_SET_PICT_FPS:
		rc = tcm9001md_set_fps(&(cdata.cfg.fps));
		break;

	case CFG_SET_EXP_GAIN:
		rc =
			tcm9001md_write_exp_gain(cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
		break;

	case CFG_SET_PICT_EXP_GAIN:
		CDBG("Line:%d CFG_SET_PICT_EXP_GAIN \n", __LINE__);
		rc =
			tcm9001md_set_pict_exp_gain(
				cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
		break;

	case CFG_SET_MODE:
		rc =
			tcm9001md_set_sensor_mode(
			cdata.mode, cdata.rs);
		break;

	case CFG_PWR_DOWN:
		rc = tcm9001md_power_down();
		break;

	case CFG_MOVE_FOCUS:
		rc =
			tcm9001md_move_focus(
			cdata.cfg.focus.dir,
			cdata.cfg.focus.steps);
		break;

	case CFG_SET_DEFAULT_FOCUS:
		rc =
			tcm9001md_set_default_focus();
		break;

	case CFG_SET_EFFECT:
		rc = tcm9001md_set_effect(
					cdata.cfg.effect);
		break;

	case CFG_SET_WB:
			rc = tcm9001md_set_wb(
						cdata.cfg.wb);
			break;
			
	case CFG_SET_BRIGHTNESS:
			rc = tcm9001md_set_brightness(
						cdata.cfg.brightness);
			break;

	case CFG_SET_ANTIBANDING:
			rc = tcm9001md_set_antibanding(
						cdata.cfg.antibanding);
			break;	

	case CFG_SET_CONTRAST:
			rc = tcm9001md_set_contrast(
						cdata.cfg.contrast);
			break;	
	case CFG_SET_ISO:
			rc = tcm9001md_set_iso(
						cdata.cfg.iso);
			break;	

	case CFG_SET_EXPOSURE_MODE:
			rc = tcm9001md_set_exposure_mode(
						cdata.cfg.exposure_mode);
			break;	

	case CFG_GET_AF_MAX_STEPS:
	case CFG_SET_LENS_SHADING:
	default:
		rc = -EINVAL;
		break;
	}

	mutex_unlock(&tcm9001md_mutex);
	return rc;
}

static int tcm9001md_sensor_probe(const struct msm_camera_sensor_info *info,
		struct msm_sensor_ctrl *s)
{
	int rc = 0;
	/*******yhwang-temp******/
	rc = gpio_request(1, "tcm9001md");
	printk("[camera yhwang] tcm9001md gpio_request reset=%d\n", rc);
	if (!rc || rc == -EBUSY)
	{
		gpio_direction_output(1, 0);
		gpio_free(1);            
	}
	else printk(KERN_ERR "tcm9001md reset error: request gpio %d failed: "
			"%d\n", 1 , rc);
        
	rc = gpio_request(20, "tcm9001md");
	printk("[camera yhwang] tcm9001md gpio_request pwd low=%d\n", rc);
	if(!rc || rc == -EBUSY)
	{
		gpio_direction_output(20, 0);
		gpio_free(20);
	}
	else printk(KERN_ERR "tcm9001md pwd error: request gpio %d failed: "
			"%d\n", 20, rc);
	printk("[camera yhwang] ......\n");
	/*******************************/  

	/* pull down power-down */
	rc = gpio_request(info->sensor_pwd, "tcm9001md");
	printk("[camera] gpio_request pwd low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(info->sensor_pwd, 0);
	else printk(KERN_ERR "tcm9001md error: request gpio %d failed: "
			"%d\n", info->sensor_pwd, rc);

	tcm9001md_power_enable();

	/* pull up power-down */
	rc = gpio_request(info->sensor_pwd, "tcm9001md");
	printk("[camera] gpio_request pwd high=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(info->sensor_pwd, 1);
	else printk(KERN_ERR "tcm9001md error: request gpio %d failed: "
			"%d\n", info->sensor_pwd, rc);
	mdelay(10);

	rc = i2c_add_driver(&tcm9001md_i2c_driver);
	if (rc < 0 || tcm9001md_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_fail;
	}

	msm_camio_clk_rate_set(TCM9001MD_DEFAULT_CLOCK_RATE);
	mdelay(10);

	/* pull down power-down */
	rc = gpio_request(info->sensor_pwd, "tcm9001md");
	printk("[camera] gpio_request pwd low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(info->sensor_pwd, 0);
	else printk(KERN_ERR "tcm9001md error: request gpio %d failed: "
			"%d\n", info->sensor_pwd, rc);
	mdelay(10);

	rc = tcm9001md_probe_init_sensor(info);
	printk("[camera] tcm9001md_probe_init_sensor=%d\n", rc);
	kernel_ini_tcm9001md_result = rc;
	if (rc < 0)
		goto probe_fail;
       
	s->s_init = tcm9001md_sensor_open_init;
	s->s_release = tcm9001md_sensor_release;
	s->s_config  = tcm9001md_sensor_config;
	tcm9001md_probe_init_done(info);

	tcm9001md_power_disable();

	return rc;

probe_fail:
	tcm9001md_power_disable();
	CDBG("SENSOR PROBE FAILS!\n");
	return rc;
}

static int __tcm9001md_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, tcm9001md_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __tcm9001md_probe,
	.driver = {
		.name = "msm_camera_tcm9001md",
		.owner = THIS_MODULE,
	},
};

static int __init tcm9001md_init(void)
{
	cci_smem_value_t *smem_cci_smem_value;
    smem_cci_smem_value = smem_alloc( SMEM_CCI_SMEM_VALUE, sizeof( cci_smem_value_t ));
	if(smem_cci_smem_value->cci_project_id == PROJECT_ID_CAP8)
		return platform_driver_register(&msm_camera_driver);
	else
		return -EIO;
}

module_init(tcm9001md_init);

