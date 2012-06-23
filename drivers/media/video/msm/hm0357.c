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
/*
#include "mach/../../cci_smem.h"
#include "mach/../../smd_private.h"
*/
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/camera.h>
#include "hm0357.h"

#include <linux/device.h> /* for vreg.h */
#include <mach/vreg.h>
//#include <mach/msm_battery.h>

/*temp swich for debug*/
#define MASK 1

#define hm0357_REG_MODEL_ID   0x0000
#define hm0357_MODEL_ID       0x05CA
#define INFO_CHIPID_H          0x0001
#define INFO_CHIPID_L          0x0002

#define hm0357_BRIGHTNESS_DEGREE		11

#if !MASK
//#define hm0357_SHARPNESS_DEGREE		11
#endif
/*system config*/
#define REG_CLKRC	0x0025
#define REG_PIDH        0x0001
#define REG_PIDL        0x0002
#define REG_SYS		0x3012
#define SYS_RESET	0x80  //can any value
#define REG_SFTRST  0x0022
#define hm0357_DEFAULT_CLOCK_RATE  12000000 //12M

#if !MASK
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

struct reg_struct hm0357_reg_pat[2] =  {
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
#endif

struct hm0357_work {
	struct work_struct work;
};
static struct hm0357_work *hm0357_sensorw;
static struct i2c_client *hm0357_client;

struct hm0357_ctrl {
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

struct hm0357_i2c_reg_conf {
	unsigned short waddr;
	unsigned char  bdata;
};

static uint16_t Brightness_Value[hm0357_BRIGHTNESS_DEGREE] = {0xd0, 0xc0, 0xb0, 0xa0, 0x90, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50};

#if !MASK
//static uint16_t Contrast_Value = 0x00; 
//static uint16_t Saturation_Value = 0x64; 
//static uint16_t Sharpness_Value[hm0357_SHARPNESS_DEGREE] = {0xFF81, 0xFF9A, 0xFFB3, 0xFFCC, 0xFFE5, 0x0000, 0x001B, 0x0034, 0x004D, 0x0066, 0x007F};
#endif
static uint16_t Preview_Frames = 0x00; 

/* color effect mode */
static CAM_REG_ADDR_VAL_TYPE hm0357ColorEffectMode_Off[] = 
{
	{ 0x0488, 0x10 },
	{ 0x0486, 0x00 },
	{ 0x0487, 0xFF },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};

static CAM_REG_ADDR_VAL_TYPE hm0357ColorEffectMode_Grayscale[] = 
{
	{ 0x0488, 0x11 },
	{ 0x0486, 0x80 },
	{ 0x0487, 0x80 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};

static CAM_REG_ADDR_VAL_TYPE hm0357ColorEffectMode_Negative[] = 
{
	{ 0x0488, 0x02 },
	{ 0x0486, 0x80 },
	{ 0x0487, 0x80 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};

static CAM_REG_ADDR_VAL_TYPE hm0357ColorEffectMode_SepiaYellow[] = 
{
	{ 0x0488, 0x11},
	{ 0x0486, 0x00},
	{ 0x0487, 0xA0},
	{ 0x0100, 0x01},
	{ 0x0101, 0x01},
	{ 0xffff, 0xff},
};

static CAM_REG_ADDR_VAL_TYPE hm0357ColorEffectMode_SepiaGreen[] =
{
	{ 0x0488, 0x11},
	{ 0x0486, 0x00},
	{ 0x0487, 0x60},
	{ 0x0100, 0x01},
	{ 0x0101, 0x01},
	{ 0xffff, 0xff},
};
/*
static CAM_REG_ADDR_VAL_TYPE hm0357ColorEffectMode_SepiaBlue[] = 
{
	{ 0x0488, 0x11},
	{ 0x0486, 0xF0},
	{ 0x0487, 0x60},
	{ 0x0100, 0x01},
	{ 0x0101, 0x01},
	{ 0xffff, 0xff},
};

static CAM_REG_ADDR_VAL_TYPE hm0357ColorEffectMode_SepiaRed[] = 
{
	{ 0x0488, 0x11 },
	{ 0x0486, 0x00 },
	{ 0x0487, 0xF0 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};
*/

/*WB*/
static CAM_REG_ADDR_VAL_TYPE hm0357WBMode_Auto[] = {
	{ 0x0380, 0xFF },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};

static CAM_REG_ADDR_VAL_TYPE Shm0357WBMode_Cloudy[] = {
	{ 0x0380, 0xFD },
	{ 0x032D, 0xF0 },
	{ 0x032E, 0x01 },
	{ 0x032F, 0x00 },
	{ 0x0330, 0x01 },
	{ 0x0331, 0x00 },
	{ 0x0332, 0x01 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};

static CAM_REG_ADDR_VAL_TYPE hm0357WBMode_Daylight[] = {
	{ 0x0380, 0xFD },
	{ 0x032D, 0xA0 },
	{ 0x032E, 0x01 },
	{ 0x032F, 0x00 },
	{ 0x0330, 0x01 },
	{ 0x0331, 0x20 },
	{ 0x0332, 0x01 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};

static CAM_REG_ADDR_VAL_TYPE hm0357WBMode_Coolwhite[] = {
	{ 0x0380, 0xFD },
	{ 0x032D, 0x96 },
	{ 0x032E, 0x01 },
	{ 0x032F, 0x00 },
	{ 0x0330, 0x01 },
	{ 0x0331, 0x46 },
	{ 0x0332, 0x01 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};

static CAM_REG_ADDR_VAL_TYPE hm0357WBMode_Tunstense[] = {
	{ 0x0380, 0xFD },
	{ 0x032D, 0x80 },
	{ 0x032E, 0x01 },
	{ 0x032F, 0x00 },
	{ 0x0330, 0x01 },
	{ 0x0331, 0x70 },
	{ 0x0332, 0x01 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};
/*
static CAM_REG_ADDR_VAL_TYPE hm0357WBMode_Alight[] = {
	{ 0x0380, 0xFD },
	{ 0x032D, 0x40 },
	{ 0x032E, 0x01 },
	{ 0x032F, 0x00 },
	{ 0x0330, 0x01 },
	{ 0x0331, 0xA0 },
	{ 0x0332, 0x01 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
	{ 0xffff, 0xff },
};
*/

#if !MASK
/*BdFlt*/
static CAM_REG_ADDR_VAL_TYPE hm0357BdFltMode_50Hz[] = {
	{HM0357_REG_ISPCTRL1_FLICKER, 0x00},
	{HM0357_REG_FSTEP50H, 0x00},
	{HM0357_REG_FSTEP50L, 0x54},
};

static CAM_REG_ADDR_VAL_TYPE hm0357BdFltMode_60Hz[] = {
	{HM0357_REG_ISPCTRL1_FLICKER, 0x01},
	{HM0357_REG_FSTEP60H, 0x00},
	{HM0357_REG_FSTEP60L, 0xA8},
};

// EV compensation
static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_N03[] = {
	{ 0x038E, 0x28 },
	{ 0x0381, 0x38 },
	{ 0x0382, 0x18 },
	{ 0x04C0, 0xb0 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
};

static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_N02[] = {
	{ 0x038E, 0x30 },
	{ 0x0381, 0x40 },
	{ 0x0382, 0x20 },
	{ 0x04C0, 0xa0 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
};

static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_N01[] = {
	{ 0x038E, 0x38 },
	{ 0x0381, 0x48 },
	{ 0x0382, 0x28 },
	{ 0x04C0, 0x90 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
};

static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_000[] = {
	{ 0x038E, 0x40 },
	{ 0x0381, 0x50 },
	{ 0x0382, 0x30 },
	{ 0x04C0, 0x00 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
};

static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_P01[] = {	
	{ 0x038E, 0x50 },
	{ 0x0481, 0x60 },
	{ 0x0482, 0x40 },
	{ 0x04C0, 0x10 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },

};

static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_P02[] = {
	{ 0x038E, 0x60 },
	{ 0x0481, 0x70 },
	{ 0x0482, 0x50 },
	{ 0x04C0, 0x20 },
	{ 0x0100, 0x01 },
	{ 0x0101, 0x01 },
};

static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_P03[] = {
	{ 0x038E, 0x70 },
	{ 0x0381, 0x80 },
	{ 0x0382, 0x60 },
	{ 0x04C0, 0x01 },
	{ 0x0100, 0x30 },
	{ 0x0101, 0x01 },
};

//COLOR BAR
static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_T01[] = {
	{0x0028, 0x83},
};
//red-->  blue
static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_T02[] = {
	{0x0028, 0x85},
};
//green  
static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_T03[] = {
	{0x0028, 0x88},
};
//blue-->red
static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_T04[] = {
	{0x0028, 0x8b},
};
//restore
static CAM_REG_ADDR_VAL_TYPE hm0357EvComp_T00[] = {
	{0x0028, 0x00},
};
#endif

static CAM_REG_ADDR_VAL_TYPE hm0357_init_setting[] = {
	{0x0022, 0x00},
	{0x0020, 0x08},//08
	{0x0023, 0x9f},//9a->9f
	{0x008f, 0x00},
	{0x000f, 0x00},
	{0x0012, 0x0c},
	{0x0013, 0x01},//	;01
	{0x0015, 0x02},
	{0x0016, 0x01},
	{0x0025, 0x00},// 0
	{0x0027, 0x10},//{0x0027, 0x10}UYVY {0x0027, 0x18}VYUY {0x0027, 0x30}YUYV {0x0027, 0x38}YVYU
	{0x0040, 0x0f},
	{0x0053, 0x00},
	{0x0075, 0x40},
	{0x0041, 0x02},
	{0x0045, 0xca},
	{0x0046, 0x4f},
	{0x004a, 0x0a},
	{0x004b, 0x72},
	{0x004d, 0xbf},
	{0x004e, 0x30},
	{0x0055, 0x10},
	{0x0070, 0x44},
	{0x0071, 0xb9},
	{0x0072, 0x55},
	{0x0073, 0x10},
	{0x0081, 0xd2},
	{0x0082, 0xa6},
	{0x0083, 0x70},
	{0x0085, 0x11},
	{0x0086, 0xa7},
	{0x0088, 0xe1},
	{0x008a, 0x2d},
	{0x008d, 0x20},
	{0x0090, 0x00},
	{0x0091, 0x10},
	{0x0092, 0x11},
	{0x0093, 0x12},
	{0x0094, 0x13},
	{0x0095, 0x17},
	{0x00a0, 0xc0},
	{0x011f, 0x44},
	{0x0120, 0x36},//	;37<=60Hz
	{0x0121, 0x80},
	{0x0122, 0x6b},
	{0x0123, 0xa5},
	{0x0124, 0x52},
	{0x0125, 0xdf},
	{0x0126, 0x71},
	{0x0140, 0x14},
	{0x0141, 0x0a},
	{0x0142, 0x14},
	{0x0143, 0x0a},
	{0x0144, 0x0c},
	{0x0145, 0x0c},
	{0x0146, 0x65},
	{0x0147, 0x0a},
	{0x0148, 0x80},
	{0x0149, 0x0c},
	{0x014a, 0x80},
	{0x014b, 0x80},
	{0x014c, 0x80},
	{0x014d, 0x2e},
	{0x014e, 0x05},
	{0x014f, 0x05},
	{0x0150, 0x0d},
	{0x0155, 0x00},
	{0x0156, 0x0a},
	{0x0157, 0x0a},
	{0x0158, 0x0a},
	{0x0159, 0x0a},
	{0x0160, 0x14},
	{0x0161, 0x14},
	{0x0162, 0x0a},
	{0x0163, 0x0a},
	{0x01b0, 0x33},
	{0x01b1, 0x10},
	{0x01b2, 0x10},
	{0x01b3, 0x30},
	{0x01b4, 0x18},
	{0x01d8, 0x20},
	{0x01de, 0x50},
	{0x01e4, 0x0a},
	{0x01e5, 0x10},
	{0x0220, 0x00},
	{0x0221, 0xa2},
	{0x0222, 0x00},
	{0x0223, 0x80},
	{0x0224, 0x80},
	{0x0225, 0x00},
	{0x0226, 0x90},
	{0x0227, 0x80},
	{0x0229, 0x90},
	{0x022a, 0x80},
	{0x022b, 0x00},
	{0x022c, 0x96},
	{0x022d, 0x10},
	{0x022e, 0x10},
	{0x022f, 0x10},
	{0x0230, 0x10},
	{0x0231, 0x10},
	{0x0233, 0x10},
	{0x0234, 0x10},
	{0x0235, 0x48},
	{0x0236, 0x01},
	{0x0237, 0x48},
	{0x0238, 0x01},
	{0x023b, 0x48},
	{0x023c, 0x01},
	{0x023d, 0xf8},
	{0x023e, 0x00},
	{0x023f, 0xf8},
	{0x0240, 0x00},
	{0x0243, 0xf8},
	{0x0244, 0x00},
	{0x0251, 0x0a},
	{0x0252, 0x06},
	{0x0280, 0x0b},
	{0x0282, 0x15},
	{0x0284, 0x2a},
	{0x0286, 0x4c},
	{0x0288, 0x5a},
	{0x028a, 0x67},
	{0x028c, 0x73},
	{0x028e, 0x7d},
	{0x0290, 0x86},
	{0x0292, 0x8e},
	{0x0294, 0x9e},
	{0x0296, 0xac},
	{0x0298, 0xc0},
	{0x029a, 0xd2},
	{0x029c, 0xe2},
	{0x029e, 0x27},
	{0x02a0, 0x06},
	{0x02c0, 0xcc},
	{0x02c1, 0x01},
	{0x02c2, 0xbb},
	{0x02c3, 0x04},
	{0x02c4, 0x11},
	{0x02c5, 0x04},
	{0x02c6, 0x2e},
	{0x02c7, 0x04},
	{0x02c8, 0x6c},
	{0x02c9, 0x01},
	{0x02ca, 0x3e},
	{0x02cb, 0x04},
	{0x02cc, 0x04},
	{0x02cd, 0x04},
	{0x02ce, 0xbe},
	{0x02cf, 0x04},
	{0x02e0, 0x03},
	{0x02d0, 0xc2},
	{0x02d1, 0x01},
	{0x02f0, 0x74},
	{0x02f1, 0x04},
	{0x02f2, 0xe3},
	{0x02f3, 0x00},
	{0x02f4, 0x6f},
	{0x02f5, 0x04},
	{0x02f6, 0x2f},
	{0x02f7, 0x04},
	{0x02f8, 0x42},
	{0x02f9, 0x04},
	{0x02fa, 0x71},
	{0x02fb, 0x00},
	{0x02fc, 0x38},
	{0x02fd, 0x04},
	{0x02fe, 0x78},
	{0x02ff, 0x04},
	{0x0300, 0xb0},
	{0x0301, 0x00},
	{0x0328, 0x00},
	{0x0329, 0x04},
	{0x032d, 0x66},
	{0x032e, 0x01},
	{0x032f, 0x00},
	{0x0330, 0x01},
	{0x0331, 0x66},
	{0x0332, 0x01},
	{0x0333, 0x00},
	{0x0334, 0x00},
	{0x0335, 0x00},
	{0x033e, 0x00},
	{0x033f, 0x00},
	{0x0340, 0x20},
	{0x0341, 0x44},
	{0x0342, 0x49},
	{0x0343, 0x30},
	{0x0344, 0x7d},
	{0x0345, 0x40},
	{0x0346, 0x78},
	{0x0347, 0x4d},
	{0x0348, 0x68},
	{0x0349, 0x6b},
	{0x034a, 0x5a},
	{0x034b, 0x70},
	{0x034c, 0x4b},
	{0x0350, 0x90},
	{0x0351, 0x90},
	{0x0352, 0x18},
	{0x0353, 0x18},
	{0x0354, 0x80},
	{0x0355, 0x50},
	{0x0356, 0xb0},
	{0x0357, 0xd0},
	{0x0358, 0x05},
	{0x035a, 0x05},
	{0x035b, 0xa0},
	{0x0381, 0x4c},
	{0x0382, 0x34},
	{0x0383, 0x20},
	{0x038a, 0x80},
	{0x038b, 0x10},
	{0x038c, 0xc1},
	{0x038e, 0x40},
	{0x038f, 0x03},
	{0x0390, 0xe0},
	{0x0393, 0x80},
	{0x0395, 0x12},
	{0x0398, 0x03},//0x02  AFR for 13Mhz start 20101210
	{0x0399, 0x00},//0xA0
	{0x039a, 0x04},//0x03
	{0x039b, 0x00},//0x00
	{0x039c, 0x06},//0x03
	{0x039d, 0x00},//0xA0
	{0x039e, 0x08},//0x04
	{0x039f, 0x00},//0x40
	{0x03a0, 0x0a},//0x05
	{0x03a1, 0x00},//0x60  AFR for 13Mhz end
	{0x03a6, 0x10},
	{0x03a7, 0x20},
	{0x03a8, 0x30},
	{0x03a9, 0x38},
	{0x03ae, 0x24},
	{0x03af, 0x20},
	{0x03b0, 0x12},
	{0x03b1, 0x0a},
	{0x03bb, 0x5f},
	{0x03bc, 0xff},
	{0x03bd, 0x00},
	{0x03be, 0x00},
	{0x03bf, 0x1d},
	{0x03c0, 0x1d},
	{0x03c0, 0xff},
	{0x03c3, 0x0f},
	{0x03d0, 0xe0},
	{0x0420, 0x83},
	{0x0421, 0x00},
	{0x0422, 0x00},
	{0x0423, 0x85},
	{0x0430, 0x00},
	{0x0431, 0x50},
	{0x0432, 0x30},
	{0x0433, 0x30},
	{0x0434, 0x00},
	{0x0435, 0x40},
	{0x0436, 0x00},
	{0x0450, 0xff},
	{0x0451, 0xff},
	{0x0452, 0xb0},
	{0x0453, 0x70},
	{0x0454, 0x00},
	{0x045a, 0x00},
	{0x045b, 0x50},
	{0x045c, 0x00},
	{0x045d, 0xb0},
	{0x0465, 0x02},
	{0x0466, 0x04},
	{0x047a, 0x00},
	{0x047b, 0x00},
	{0x0480, 0x40},
	{0x0481, 0x05},
	{0x04b0, 0x50},
	{0x04b1, 0x00},
	{0x04b3, 0x00},
	{0x04b4, 0x00},
	{0x04b6, 0x3D},
	{0x04b9, 0x0a},
	{0x0540, 0x00},
	{0x0541, 0x7c},//0x44 for 13MHz 20101210
	{0x0542, 0x00},
	{0x0543, 0x95},//0x52 for 13MHz 20101210
	{0x0580, 0x01},
	{0x0581, 0x04},
	{0x0590, 0x20},
	{0x0591, 0x30},
	{0x0594, 0x10},
	{0x0595, 0x20},
	{0x05a0, 0x0a},
	{0x05a1, 0x0c},
	{0x05a2, 0x50},
	{0x05a3, 0x60},
	{0x05b0, 0x28},
	{0x05b1, 0x06},
	{0x05d0, 0x08},
	{0x05e4, 0x04},
	{0x05e5, 0x00},
	{0x05e6, 0x83},
	{0x05e7, 0x02},
	{0x05e8, 0x04},
	{0x05e9, 0x00},
	{0x05ea, 0xe3},
	{0x05eb, 0x01},
/*	{0x0090, 0x00},	//Start 20101210
	{0x0091, 0x01},
	{0x0092, 0x02},
	{0x0093, 0x03},
	{0x0094, 0x07},
	{0x0095, 0x08},
	{0x00A0, 0xC4},
	{0x01E5, 0x0A},
	{0x03A6, 0x0D},
	{0x03A7, 0x0A},	
	{0x03A8, 0x23},
	{0x03A9, 0x33},
	{0x03AE, 0x20},
	{0x03AF, 0x17},	
	{0x03B0, 0x07},
	{0x03B1, 0x08}, //End */
	{0x0000, 0x01},
	{0x0100, 0x01},
	{0x0101, 0x01},
	{0x0005, 0x01},
	{0xffff, 0xff},
	//{0x0004, 0x10}, //PWD 
};

// Preview Configuration 
static CAM_REG_ADDR_VAL_TYPE hm0357_config_yuv422_vga[]={
	{0x0006, 0x02},
	{0x000D, 0x00},
	{0x000E, 0x00},
	{0x0125, 0xDF},
	{0x05E5, 0x00},
	{0x05E4, 0x04},
	{0x05E7, 0x02},
	{0x05E6, 0x83},
	{0x05E9, 0x00},
	{0x05E8, 0x04},
	{0x05EB, 0x01},
	{0x05EA, 0xE3},
	{0xffff, 0xff},
};

static CAM_REG_ADDR_VAL_TYPE hm0357_config_yuv422_qvga[]={
	{0x0006, 0x02},//RDCFG  //0x03: erital and mirror all flip
	{0x000D, 0x00},//VREAD
	{0x000E, 0x00},//HREAD
	{0x0125, 0xFF},//Scaler[5]=1
	{0x05E5, 0x00},//x-start H
	{0x05E4, 0x04},//x-strat L
	{0x05E7, 0x02},//x-end H
	{0x05E6, 0x83},//x-end L
	{0x05E9, 0x00},//y-start H
	{0x05E8, 0x04},//y-strat L
	{0x05EB, 0x01},//y-end H
	{0x05EA, 0xE3},//y-end L
    //new added 
	{0x05b0, 0x18},
	{0x0480, 0x40},
	{0xffff, 0xff},
};

#if !MASK
static CAM_REG_ADDR_VAL_TYPE hm0357_config_yuv422_qqvga[]={
	{0x0006, 0x02},
	{0x000D, 0x01},
	{0x000E, 0x01},
	{0x0125, 0xFF},
	{0x0124, 0x52},
	{0x05E5, 0x00},
	{0x05E4, 0x02},
	{0x05E7, 0x01},
	{0x05E6, 0x41},
	{0x05E9, 0x00},
	{0x05E8, 0x04},
	{0x05EB, 0x00},
	{0x05EA, 0xF3},
	{0xffff, 0xff},
};

static CAM_REG_ADDR_VAL_TYPE hm0357_config_yuv422_cif[]={
	{0x0006, 0x04},
	{0x000D, 0x00},
	{0x000E, 0x00},
	{0x0125, 0xDF},
	{0x05E5, 0x00},
	{0x05E4, 0x04},
	{0x05E7, 0x01},
	{0x05E6, 0x63},
	{0x05E9, 0x00},
	{0x05E8, 0x05},
	{0x05EB, 0x01},
	{0x05EA, 0x24},
	{0xffff, 0xff},
};

static CAM_REG_ADDR_VAL_TYPE hm0357_config_yuv422_qcif[]={
	{0x0006, 0x04},
	{0x000D, 0x01},
	{0x000E, 0x01},
	{0x0125, 0xDF},
	{0x05E5, 0x00},
	{0x05E4, 0x04},
	{0x05E7, 0x00},
	{0x05E6, 0xB3},
	{0x05E9, 0x00},
	{0x05E8, 0x05},
	{0x05EB, 0x00},
	{0x05EA, 0x94},
	{0xffff, 0xff},
};
#endif

static struct hm0357_ctrl *hm0357_ctrl;
static int hm0357_load_init_config(void);
static DECLARE_WAIT_QUEUE_HEAD(hm0357_wait_queue);
DEFINE_MUTEX(hm0357_mutex);

void hm0357_power_enable(void)
{
	int rc;
	struct vreg *vreg_cam;
	printk("#### %s ####\n", __FUNCTION__);

	vreg_cam = vreg_get(0, "wlan");  //I/O used
	rc = vreg_set_level(vreg_cam,2600);
	if (rc)
		printk("#### vreg set wlan level failed ####\n");
	rc = vreg_enable(vreg_cam);
	if (rc)
		printk("#### vreg enable wlan level failed ####\n");

	mdelay(3);
	vreg_cam = vreg_get(0, "gp3"); //ACORE used
	rc = vreg_set_level(vreg_cam,2800);
	if (rc)
		printk("#### vreg set gp3 level failed ####\n");
	rc = vreg_enable(vreg_cam);
	if (rc)
		printk("#### vreg enable gp3 level failed ####\n");
	
}

void hm0357_power_disable(void)
{
	int rc;
	struct vreg *vreg_cam;
	printk("#### %s ####\n", __FUNCTION__);

	vreg_cam = vreg_get(0, "gp3");	//ACORE used
	rc = vreg_set_level(vreg_cam,0);
	if (rc)
		printk("#### vreg set gp3 level failed ####\n");
	rc = vreg_disable(vreg_cam);
	if (rc)
		printk("#### vreg disable gp3 level failed ####\n");

	mdelay(3);
	vreg_cam = vreg_get(0, "wlan");	//I/O used
	rc = vreg_set_level(vreg_cam,0);
	if (rc)
		printk("#### vreg set wlan level failed ####\n");
	rc = vreg_disable(vreg_cam);
	if (rc)
		printk("#### vreg disable wlan level failed ####\n");
	
}

static int hm0357_i2c_rxdata(unsigned short saddr, unsigned char *rxdata,
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

	if (i2c_transfer(hm0357_client->adapter, msgs, 2) < 0) {
		CDBG("hm0357_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t hm0357_i2c_txdata(unsigned short saddr,
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

	if (i2c_transfer(hm0357_client->adapter, msg, 1) < 0) {
		CDBG("hm0357_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}

static int32_t hm0357_i2c_write_byte(unsigned short saddr, unsigned short waddr,
	unsigned char bdata)
{
	int32_t rc = -EIO;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = bdata;

	rc = hm0357_i2c_txdata(saddr, buf, 3);

	if (rc < 0)
		CDBG("i2c_write_b failed, addr = 0x%x, val = 0x%x!\n",
			waddr, bdata);

	return rc;
}

static int32_t camsensor_hm0357_i2c_write_byte(unsigned short reg_addr, unsigned char reg_data)
{
	int32_t rc = 0;
	rc = hm0357_i2c_write_byte(hm0357_client->addr,reg_addr,reg_data);
	return rc;
}

static int32_t hm0357_i2c_read_byte(unsigned short saddr, unsigned short raddr,
	unsigned char *rdata)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	buf[0] = (raddr & 0xFF00)>>8;
	buf[1] = (raddr & 0x00FF);

	rc = hm0357_i2c_rxdata(saddr, buf, 1);
	if (rc < 0)
		return rc;

	*rdata = buf[0];

	if (rc < 0)
		CDBG("hm0357_i2c_read failed!\n");

	return rc;
}

static int32_t camsensor_hm0357_i2c_read_byte(unsigned short reg_addr, unsigned char *reg_data)
{
	int32_t rc = 0;
	rc = hm0357_i2c_read_byte(hm0357_client->addr,reg_addr,reg_data);
	return rc;
}

static int32_t hm0357_write_array(CAM_REG_ADDR_VAL_TYPE *reg_list)
{
   int32_t  rc = 0;
 
   while (reg_list->addr != 0xffff || reg_list->val != 0xff)
   {
		rc = camsensor_hm0357_i2c_write_byte(reg_list->addr, reg_list->val);
		if(rc)
			printk("%s: rc=%d!!!!\n",__func__,rc);
		if(rc < 0)
		{
			return rc;
		}
		reg_list++;
		mdelay(1); //delay 1ms 
   }
   return 0;
}

static int hm0357_probe_init_done(const struct msm_camera_sensor_info *data)
{
	gpio_direction_output(data->sensor_pwd, 1);
	gpio_free(data->sensor_pwd);
	return 0;
}

/*
 static void hm0357_write_test(void)
{
    u8 value[3]={0};
	
    printk("%s yalin_sang\n",__func__);

	camsensor_hm0357_i2c_read_byte(0x04c0, &value[0]);
	printk("start: 0x04c0=0x%x \n",value[0]);
		
	camsensor_hm0357_i2c_write_byte(0x04c0, value[0]+5);
	mdelay(1);

	camsensor_hm0357_i2c_read_byte(0x04c0, &value[1]);
	printk("end: 0x04c0=0x%x  \n",value[1]);

	if(value[0] + 5 == value[1])
		printk("hm0357 write test succeed!\n");
}
*/

static int hm0357_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int32_t  rc;
	unsigned char chipid[2]={0};
	printk("####%s####\n",__func__);

	mdelay(30);//delay 100MCLK after enable mclk

	/*send soft reset*/
	rc = camsensor_hm0357_i2c_write_byte(REG_SFTRST,SYS_RESET);
	if(rc < 0)
	{
		goto init_probe_fail;
	}
	CDBG("hm0357_sensor_init(): reseting sensor.\n");
	mdelay(30);

    /*set pwd soft config*/
	rc = camsensor_hm0357_i2c_write_byte(0x0004,0x10);
	if(rc < 0)
	{
		goto init_probe_fail;
	}
	CDBG("hm0357_sensor_init(): pwd soft config.\n");
	mdelay(3);

    /*sensor id identify*/
	rc = camsensor_hm0357_i2c_read_byte(INFO_CHIPID_H, &chipid[0]);
	printk("[camera] camsensor_hm0357_i2c_read=%d\n", rc);
	if (rc < 0)
		goto init_probe_fail;
	
	rc = camsensor_hm0357_i2c_read_byte(INFO_CHIPID_L, &chipid[1]);
	printk("[camera] camsensor_hm0357_i2c_read=%d\n", rc);
	if (rc < 0)
		goto init_probe_fail;

	if ((chipid[0] == 0x03)&&(chipid[1] == 0x57)) {
		printk("Sensor Hm0357 detect successed!!!\n");
		//hm0357_write_test();
	}
	else{
		printk("Sensor Hm0357 detect failed!!!");
		printk("[camera] detected chipid = 0x%x%x !!\n", chipid[0],chipid[1]);
		goto init_probe_fail;
	}
	//printk("#### %s ####\n", __FUNCTION__);
	goto init_probe_done;

init_probe_fail:
	hm0357_probe_init_done(data);
init_probe_done:
	return rc;
}

static int hm0357_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&hm0357_wait_queue);
	return 0;
}

static const struct i2c_device_id hm0357_i2c_id[] = {
	{ "hm0357", 0},
	{ }
};

static int hm0357_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	CDBG("hm0357_probe called!\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CDBG("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	hm0357_sensorw = kzalloc(sizeof(struct hm0357_work), GFP_KERNEL);
	if (!hm0357_sensorw) {
		CDBG("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, hm0357_sensorw);
	hm0357_init_client(client);
	hm0357_client = client;

	//mdelay(3);

	CDBG("hm0357_probe successed! rc = %d\n", rc);
	return 0;

probe_failure:
	CDBG("hm0357_probe failed! rc = %d\n", rc);
	return rc;
}

static struct i2c_driver hm0357_i2c_driver = {
	.id_table = hm0357_i2c_id,
	.probe  = hm0357_i2c_probe,
	.remove = __exit_p(hm0357_i2c_remove),
	.driver = {
		.name = "hm0357",
	},
};

#if !MASK
static int32_t hm0357_write_FlickerMode_60hz_reg(void)
{
   int32_t rc = 0;

	return rc;
}

static int32_t hm0357_write_FlickerMode_50hz_reg(void)
{
   int32_t rc = 0;
	return rc;
}
#endif

/*here, capture is configed yuv422 VGA*/
static int32_t hm0357_config_capture(void)
{
	int32_t rc = 0;  
	printk("####%s####\n",__func__);
	
	rc = hm0357_write_array(hm0357_config_yuv422_vga);
	return rc;
}

/*here, preview is configed yuv422 QVGA*/
static int32_t hm0357_config_preview(void)
{
	int32_t rc = 0;  
	printk("####%s####\n",__func__);

	rc = hm0357_write_array(hm0357_config_yuv422_qvga);
	return rc;
}

static int32_t hm0357_setting(enum msm_s_reg_update rupdate,
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
				rc = hm0357_config_capture();
				if(rc < 0)
				{
					return rc;
				}
				//mdelay(100);
				break;
			   
	        case S_RES_PREVIEW:
				rc = hm0357_config_preview();
				if(rc < 0)
				{
					return rc;
				}
				//mdelay(100);
				break;
			
	        default:
	          return rc;
	      } /* rt */
		break; /* UPDATE_PERIODIC */

	case S_REG_INIT:
		rc = hm0357_write_array(hm0357_init_setting);
		printk("%s:init: rc=%d\n", __FUNCTION__,rc);
		if(rc < 0)
		{
		    return rc;
		}	   
		
		mdelay(3);
		Preview_Frames = 0;
		break; /* case REG_INIT: */

	default:
		rc = -EINVAL;
		break;
	} /* switch (rupdate) */

	//	printk("[Camera] test pattern\n");
	return rc;
}

static int32_t hm0357_load_init_config(void)
{
	int32_t rc = 0;
	
	rc = hm0357_setting(S_REG_INIT, S_RES_PREVIEW);
	return rc;
}

static int hm0357_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int32_t  rc;
	printk("#### %s ####\n", __FUNCTION__);

	hm0357_ctrl = kzalloc(sizeof(struct hm0357_ctrl), GFP_KERNEL);
	if (!hm0357_ctrl) {
		CDBG("hm0357_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	hm0357_ctrl->fps_divider = 1 * 0x00000400;
	hm0357_ctrl->pict_fps_divider = 1 * 0x00000400;
	hm0357_ctrl->set_test = S_TEST_OFF;
	hm0357_ctrl->prev_res = S_QTR_SIZE;
	hm0357_ctrl->pict_res = S_FULL_SIZE;

	if (data)
		hm0357_ctrl->sensordata = data;

	/* pull down power-down */
	rc = gpio_request(data->sensor_pwd, "hm0357");
	printk("[camera] gpio_request pwd low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(data->sensor_pwd, 0);
	else printk(KERN_ERR "hm0357 error: request gpio %d failed: "
			"%d\n", data->sensor_pwd, rc);
	
	mdelay(1);
	/*enable power*/
	hm0357_power_enable();

	mdelay(1);
	/* enable clk */
	msm_camio_clk_enable(CAMIO_VFE_CLK);
	printk("[camera] enable clk\n");
	/* enable mclk first */
	msm_camio_clk_rate_set(hm0357_DEFAULT_CLOCK_RATE);
	
	mdelay(1);
	msm_camio_camif_pad_reg_reset();

	rc = hm0357_probe_init_sensor(data);
	if (rc < 0)
		goto init_fail1;

	/*load init setting*/
	rc = hm0357_load_init_config();
	if(rc < 0)
	{
		goto init_fail1;
	}
	mdelay(1);
    //printk("#### %s ####hm0357_ctrl->prev_res=%d\n", __FUNCTION__,hm0357_ctrl->prev_res);

	if (hm0357_ctrl->prev_res == S_QTR_SIZE)
		rc = hm0357_setting(S_UPDATE_PERIODIC, S_RES_PREVIEW);
	else
		rc = hm0357_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);

	if (rc < 0) {
		CDBG("hm0357_setting failed. rc = %d\n", rc);
		goto init_fail1;
	}

	goto init_done;

init_fail1:
	hm0357_probe_init_done(data);
	kfree(hm0357_ctrl);
init_done:
	return rc;
}

static int32_t hm0357_power_down(void)
{
	int32_t rc = 0;
	return rc;
}

static int hm0357_sensor_release(void)
{
	int rc = -EBADF;
	printk("#### %s ####\n", __FUNCTION__);

	mutex_lock(&hm0357_mutex);
	hm0357_power_down();

	gpio_direction_output(hm0357_ctrl->sensordata->sensor_pwd, 1);
	gpio_free(hm0357_ctrl->sensordata->sensor_pwd);
	mdelay(1);

	/*disable clk*/
	msm_camio_clk_disable(CAMIO_VFE_CLK);
	printk("[camera] disable clk\n");
	mdelay(1);

	hm0357_power_disable();

	kfree(hm0357_ctrl);
	hm0357_ctrl = NULL;

	CDBG("hm0357_release completed\n");

	mutex_unlock(&hm0357_mutex);
	return rc;
}


static int32_t hm0357_set_brightness (int8_t brightness)
{
	uint16_t brightness_value = 0;
	int32_t rc = 0;
	uint32_t  brightness_offset = 2;
	unsigned char reg_value;

	brightness += brightness_offset; // App only has range of 0~8, and Brightness_Value[6] gives us best result
	if ((brightness >= 0) && (brightness < hm0357_BRIGHTNESS_DEGREE))
	{
		brightness_value = Brightness_Value[brightness];
	}
	else
	{
		brightness_value = Brightness_Value[5];
	}

	camsensor_hm0357_i2c_read_byte(HM0357_REG_ISPCTRL6_BRIGHT,&reg_value);
	reg_value = reg_value | 0x40;
	rc = camsensor_hm0357_i2c_write_byte(HM0357_REG_ISPCTRL6_BRIGHT, reg_value);
	if (rc < 0)
	{
		return rc;
	}	
	
	rc = camsensor_hm0357_i2c_write_byte(HM0357_REG_BRIGHT_STRENGTH, brightness_value);
	if (rc < 0)
	{
		return rc;
	}	

	return rc;  
}

static int32_t hm0357_set_contrast (int8_t contrast)
{
#if !MASK
	int32_t rc = 0;
    /*here, reg address and value not correct ,need modify later*/
	//rc = camsensor_hm0357_i2c_write_byte(HM0357_REG_ISPCTRL6_CONTRAST,contrast);
 	if (rc < 0)
	{
		return rc;
	}	

	return rc;  
#else
	return 0;
#endif
}

static int32_t hm0357_set_effect (int8_t effect)
{
	int32_t rc = 0;
	printk("%s: effect=%d\n",__func__,effect);
	 switch(effect)
	 {
	    case CAMERA_EFFECT_OFF:
			rc = hm0357_write_array(hm0357ColorEffectMode_Off);
	    	if (rc < 0)
		   	{
	      		return rc;
		   	}	
			break;
		
	    case CAMERA_EFFECT_MONO:
			rc = hm0357_write_array(hm0357ColorEffectMode_Grayscale);
			if (rc < 0)
			{
				return rc;
			}	
			break;
		   
	    case CAMERA_EFFECT_NEGATIVE:
			rc = hm0357_write_array(hm0357ColorEffectMode_Negative);
			if (rc < 0)
			{
				return rc;
			}	
			break;
	    case CAMERA_EFFECT_SEPIA:
			rc = hm0357_write_array(hm0357ColorEffectMode_SepiaYellow);
			if (rc < 0)
			{
				return rc;
			}	
			break;
	    case CAMERA_EFFECT_AQUA:
			rc = hm0357_write_array(hm0357ColorEffectMode_SepiaGreen);
			if (rc < 0)
			{
				return rc;
			}	
			break;	
/*
	    case CAMERA_EFFECT_SEPIA_YELLOW:
			rc = hm0357_write_array(hm0357ColorEffectMode_SepiaYellow);
			if (rc < 0)
			{
				return rc;
			}	
			break;

	    case CAMERA_EFFECT_SEPIA_GREEN:
			rc = hm0357_write_array(hm0357ColorEffectMode_SepiaGreen);
			if (rc < 0)
			{
				return rc;
			}	
			break;

	    case CAMERA_EFFECT_SEPIA_BLUE:
			rc = hm0357_write_array(hm0357ColorEffectMode_SepiaBlue)
			if (rc < 0)
			{
				return rc;
			}	
			break;
			
		case CAMERA_EFFECT_SEPIA_RED:
			rc = hm0357_write_array(hm0357ColorEffectMode_SepiaRed)
			if (rc < 0)
			{
				return rc;
			}	
			break;
*/
		default:
		    printk("%s: unknown color effect, effect=%d\n",__func__,effect);
			rc = hm0357_write_array(hm0357ColorEffectMode_Off);
	    	if (rc < 0)
		   	{
	      		return rc;
		   	}	
		 break;	
	 }

  return rc;  
}

static int32_t hm0357_set_antibanding (int8_t antibanding)
{
#if !MASK
   int32_t rc = 0; 

   switch(antibanding)
   {
      case CAMERA_ANTIBANDING_60HZ:
         rc = hm0357_write_array(hm0357BdFltMode_60Hz);
         if (rc < 0)
         {
            return rc;
         } 
	  break;	 
      case CAMERA_ANTIBANDING_50HZ:
         rc = hm0357_write_array(hm0357BdFltMode_50Hz);
         if (rc < 0)
         {
            return rc;
         } 
	  break;	 
	default:
	   break;
   }
   
  return rc;  
#else
	return 0;
#endif
}

static int32_t hm0357_set_wb (int8_t wb)
{
	int32_t rc = 0;

	switch(wb)
	{
		case CAMERA_WB_AUTO:
			rc = hm0357_write_array(hm0357WBMode_Auto);
			if (rc < 0)
			{
				return rc;
			}  	
     		break; 	  

		case CAMERA_WB_CLOUDY_DAYLIGHT:
			rc = hm0357_write_array(Shm0357WBMode_Cloudy);
			if (rc < 0)
			{
         			return rc;
			}  	
			break; 	 

		case CAMERA_WB_DAYLIGHT:
			rc = hm0357_write_array(hm0357WBMode_Daylight);
			if (rc < 0)
			{
         			return rc;
			}  	
			break; 	 
		case CAMERA_WB_FLUORESCENT:
			rc = hm0357_write_array(hm0357WBMode_Coolwhite);
			if (rc < 0)
			{
         			return rc;
			}  	
			break; 	 
		case CAMERA_WB_INCANDESCENT:
			rc = hm0357_write_array(hm0357WBMode_Tunstense);
			if (rc < 0)
			{
         			return rc;
			}  	
			break; 	
/*
		case CAMERA_WB_COOWHITE:
			rc = hm0357_write_array(hm0357WBMode_Coolwhite);	
			if (rc < 0)
			{
				return rc;
			}  
			break; 	 

		case CAMERA_WB_TUNSTENSE:
			rc = hm0357_write_array(hm0357WBMode_Tunstense);	
			if (rc < 0)
			{
				return rc;
			}  
			break; 	 

		case CAMERA_WB_A_LIGHT:
			rc = hm0357_write_array(hm0357WBMode_Alight);
			if (rc < 0)
			{
				return rc;
			}  		  	  	
			break; 	 
*/
		default:
			rc = hm0357_write_array(hm0357WBMode_Auto);
			if (rc < 0)
			{
				return rc;
			}  				
			break; 	 
	}

	return rc;  
}

static int32_t hm0357_set_iso (int8_t iso)
{
	return 0;
}

static int32_t hm0357_set_exposure_mode(int8_t exposure_mode)
{
	return 0;
}

static int32_t hm0357_get_ae_status(int8_t *sdata)
{
	return 0;
}

static int32_t hm0357_set_fps(struct fps_cfg *fps)
{
#if !MASK
	/* input is new fps in Q10 format */
	int32_t rc = 0;
	enum msm_s_setting setting;
	printk("#### %s ####\n", __FUNCTION__);

	hm0357_ctrl->fps_divider = fps->fps_div;

	if (hm0357_ctrl->sensormode == SENSOR_PREVIEW_MODE)
		setting = S_RES_PREVIEW;
	else
		setting = S_RES_CAPTURE;

  rc = hm0357_i2c_write_byte(hm0357_client->addr,
		REG_FRAME_LENGTH_LINES_MSB,
		(((hm0357_reg_pat[setting].size_h +
			hm0357_reg_pat[setting].blk_l) *
			hm0357_ctrl->fps_divider / 0x400) & 0xFF00) >> 8);
	if (rc < 0)
		goto set_fps_done;

  rc = hm0357_i2c_write_byte(hm0357_client->addr,
		REG_FRAME_LENGTH_LINES_LSB,
		(((hm0357_reg_pat[setting].size_h +
			hm0357_reg_pat[setting].blk_l) *
			hm0357_ctrl->fps_divider / 0x400) & 0x00FF));

set_fps_done:
	return rc;
#else
	return 0;
#endif
}

static int32_t hm0357_write_exp_gain(uint16_t gain, uint32_t line)
{
	int32_t rc = 0;

	return rc;
}

static int32_t hm0357_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
	int32_t rc = 0;

	CDBG("Line:%d hm0357_set_pict_exp_gain \n", __LINE__);

	rc =
		hm0357_write_exp_gain(gain, line);

	return rc;
}

static int32_t hm0357_video_config(int mode, int res)
{
	int32_t rc = 0;
	printk("#### %s ####\n", __FUNCTION__);
	
	switch (res) {
	case S_QTR_SIZE:
		rc = hm0357_setting(S_UPDATE_PERIODIC, S_RES_PREVIEW);
		printk("rc=%d\n",rc);
		if (rc < 0)
			return rc;

		CDBG("hm0357 sensor configuration done!\n");
		break;

	case S_FULL_SIZE:
		rc = hm0357_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
		if (rc < 0)
			return rc;

		break;

	default:
		return 0;
	} /* switch */

	hm0357_ctrl->prev_res = res;
	hm0357_ctrl->curr_res = res;
	hm0357_ctrl->sensormode = mode;

	rc = hm0357_write_exp_gain(hm0357_ctrl->my_reg_gain,
			hm0357_ctrl->my_reg_line_count);

	return rc;
}

static int32_t hm0357_snapshot_config(int mode)
{
	int32_t rc = 0;

	rc = hm0357_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
	if (rc < 0)
		return rc;

	hm0357_ctrl->curr_res = hm0357_ctrl->pict_res;
	hm0357_ctrl->sensormode = mode;

	return rc;
}

static int32_t hm0357_raw_snapshot_config(int mode)
{
	int32_t rc = 0;

	rc = hm0357_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
	if (rc < 0)
		return rc;

	hm0357_ctrl->curr_res = hm0357_ctrl->pict_res;
	hm0357_ctrl->sensormode = mode;

	return rc;
}

static int32_t hm0357_set_sensor_mode(int mode, int res)
{
	int32_t rc = 0;
	printk("#### %s(mode=%d) ####\n", __FUNCTION__,mode);
	
	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		rc = hm0357_video_config(mode, res);
		break;

	case SENSOR_SNAPSHOT_MODE:
		rc = hm0357_snapshot_config(mode);
		break;

	case SENSOR_RAW_SNAPSHOT_MODE:
		rc = hm0357_raw_snapshot_config(mode);
		break;

	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int hm0357_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;
	printk("#### %s ####\n", __FUNCTION__);

	if (copy_from_user(&cdata,
			(void *)argp,
			sizeof(struct sensor_cfg_data)))
		return -EFAULT;

	mutex_lock(&hm0357_mutex);

	CDBG("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
	printk("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
	switch (cdata.cfgtype) {
	case CFG_SET_FPS:
	case CFG_SET_PICT_FPS:
		rc = hm0357_set_fps(&(cdata.cfg.fps));
		break;

	case CFG_SET_EXP_GAIN:
		rc =
			hm0357_write_exp_gain(cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
		break;

	case CFG_SET_PICT_EXP_GAIN:
		CDBG("Line:%d CFG_SET_PICT_EXP_GAIN \n", __LINE__);
		rc =
			hm0357_set_pict_exp_gain(
				cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
		break;

	case CFG_SET_MODE:
		rc =
			hm0357_set_sensor_mode(
			cdata.mode, cdata.rs);
		break;

	case CFG_PWR_DOWN:
		rc = hm0357_power_down();
		break;

	case CFG_SET_EFFECT:
		rc = hm0357_set_effect(
					cdata.cfg.effect);
		break;

	case CFG_SET_WB:
		rc = hm0357_set_wb(
					cdata.cfg.wb);
		break;
			
	case CFG_SET_BRIGHTNESS:
 		rc = hm0357_set_brightness(
					cdata.cfg.brightness);
		break;

	case CFG_SET_ANTIBANDING:
		rc = hm0357_set_antibanding(
					cdata.cfg.antibanding);
		break;	

	case CFG_SET_CONTRAST:
		rc = hm0357_set_contrast(
					cdata.cfg.contrast);
		break;	

	case CFG_SET_ISO:
		rc = hm0357_set_iso(
					cdata.cfg.iso);
		break;	

	case CFG_SET_EXPOSURE_MODE:
		rc = hm0357_set_exposure_mode(
					cdata.cfg.exposure_mode);
		break;	

	case CFG_GET_AE_STATUS:
		{
			uint8_t	*sdata = &cdata.cfg.ae_fps;
			rc = hm0357_get_ae_status(sdata);
			if (!rc && copy_to_user(argp, &cdata, sizeof(struct sensor_cfg_data)))
				rc = -EFAULT;
		}
		break;	

	case CFG_GET_AF_MAX_STEPS:
	case CFG_SET_LENS_SHADING:
	default:
		rc = -EINVAL;
		break;
	}

	mutex_unlock(&hm0357_mutex);
	return rc;
}

static int hm0357_sensor_probe(const struct msm_camera_sensor_info *info,
		struct msm_sensor_ctrl *s)
{
	int rc = 0;
    printk("#### %s ####\n", __FUNCTION__);

	/* pull down power-down */
	rc = gpio_request(info->sensor_pwd, "hm0357");
	printk("[camera] gpio_request pwd low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(info->sensor_pwd, 0);
	else printk(KERN_ERR "hm0357 error: request gpio %d failed: "
			"%d\n", info->sensor_pwd, rc);

	mdelay(1);

	hm0357_power_enable();

	rc = i2c_add_driver(&hm0357_i2c_driver);
	if (rc < 0 || hm0357_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_fail;
	}

	mdelay(1);

	/* enable clk */
	msm_camio_clk_enable(CAMIO_VFE_CLK);
	printk("[camera] enable clk\n");
	msm_camio_clk_rate_set(hm0357_DEFAULT_CLOCK_RATE);

	rc = hm0357_probe_init_sensor(info);
	printk("[camera] hm0357_probe_init_sensor=%d\n", rc);
	if (rc < 0)
		goto probe_fail;

	s->s_init = hm0357_sensor_open_init;
	s->s_release = hm0357_sensor_release;
	s->s_config  = hm0357_sensor_config;
	hm0357_probe_init_done(info);

	/*disable clk*/
	msm_camio_clk_disable(CAMIO_VFE_CLK);
	printk("[camera] disable clk\n");

	mdelay(1);

	hm0357_power_disable();

	return rc;

probe_fail:
	CDBG("SENSOR PROBE FAILS!\n");

	/*disable clk*/
	msm_camio_clk_disable(CAMIO_VFE_CLK);
	printk("[camera] disable clk\n");

	mdelay(1);

	hm0357_power_disable();

	return rc;
}

static int __hm0357_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, hm0357_sensor_probe);
}

static struct platform_driver msm_camera_driver = {
	.probe = __hm0357_probe,
	.driver = {
		.name = "msm_camera_hm0357",
		.owner = THIS_MODULE,
	},
};

static int __init hm0357_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}

module_init(hm0357_init);
