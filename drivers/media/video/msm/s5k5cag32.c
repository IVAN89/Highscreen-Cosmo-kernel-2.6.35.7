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
#include "s5k5cag.h"

#include <linux/device.h> /* for vreg.h */
#include <mach/vreg.h>
#include <mach/msm_battery.h>

//#define T_CCI_CAMERA_INITIAL_SHORT_VERSION

#define S5K5CAG_REG_MODEL_ID   0x0000
#define S5K5CAG_MODEL_ID       0x05CA

#define INFO_CHIPID_H          0x0000
#define INFO_CHIPID_L          0x0040

/* PLL Registers */
#define REG_PRE_PLL_CLK_DIV           0x0305
#define REG_PLL_MULTIPLIER_MSB        0x0306
#define REG_PLL_MULTIPLIER_LSB        0x0307
#define REG_VT_PIX_CLK_DIV            0x0301
#define REG_VT_SYS_CLK_DIV            0x0303
#define REG_OP_PIX_CLK_DIV            0x0309
#define REG_OP_SYS_CLK_DIV            0x030B

/* Data Format Registers */
#define REG_CCP_DATA_FORMAT_MSB       0x0112
#define REG_CCP_DATA_FORMAT_LSB       0x0113

/* Output Size */
#define REG_X_OUTPUT_SIZE_MSB         0x034C
#define REG_X_OUTPUT_SIZE_LSB         0x034D
#define REG_Y_OUTPUT_SIZE_MSB         0x034E
#define REG_Y_OUTPUT_SIZE_LSB         0x034F

/* Binning */
#define REG_X_EVEN_INC                0x0381
#define REG_X_ODD_INC                 0x0383
#define REG_Y_EVEN_INC                0x0385
#define REG_Y_ODD_INC                 0x0387
/*Reserved register */
#define REG_BINNING_ENABLE            0x3014

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
#define S5K5CAG_REG_MODE_SELECT      0x0100
#define S5K5CAG_MODE_SELECT_STREAM     0x01   /* start streaming */
#define S5K5CAG_MODE_SELECT_SW_STANDBY 0x00   /* software standby */
#define S5K5CAG_REG_SOFTWARE_RESET   0x0103
#define S5K5CAG_SOFTWARE_RESET         0x01
#define REG_TEST_PATTERN_MODE         0xB054

#define S5K5CAG_BRIGHTNESS_DEGREE		11
#define S5K5CAG_SHARPNESS_DEGREE		11
#define S5K5CAG_FRAMES_BEFORE_SLOW_AWB		3

#define S5K5CAG_DEFAULT_CLOCK_RATE  24000000

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

struct reg_struct s5k5cag_reg_pat[2] =  {
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

struct s5k5cag_work {
	struct work_struct work;
};
static struct s5k5cag_work *s5k5cag_sensorw;
static struct i2c_client *s5k5cag_client;

struct s5k5cag_ctrl {
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

struct s5k5cag_i2c_reg_conf {
	unsigned short waddr;
	unsigned char  bdata;
};

CAM_REG_ADDR_VAL_TYPE	cam_sw_resets_setup[]=
{
{0x0010, 0x0001},    // Reset
{0x1030, 0x0000},    // Clear host interrupt so main will wait
{0x0014, 0x0001},    // ARM go
};

#ifdef T_CCI_CAMERA_INITIAL_SHORT_VERSION
static CAM_REG_ADDR_VAL_TYPE	cam_short_version_porting_setup[]=
{
// Init Parameters
{0x700001CC,		0x5DC0},	/*input clock=24MHz*/
{0x700001CE,		0x0000},
{0x700001EE,		0x0001},	/*2 PLL configurations*/
{0x700001F6,		0x2BF2},	/*1st system CLK 58MHz*/
{0x700001F8,		0x3A88},	/*PVI clock 1 40MHz*/
{0x700001FA,		0x3AA8},
{0x70000208,		0x0001},	/*2nd system CLK 58MHz*/
// SET PREVIEW CONFIGURATION_0
{0x7000026C,		0x0400}, //1024
{0x7000026E,		0x0300}, //768
{0x70000270,		0x0005}, //5 YUV
{0x70000272,		0x3AA6}, //PCLK max
{0x70000274,		0x3A98}, //PCLK min
{0x70000276,		0x0100},
{0x70000278,		0x0800},
{0x7000027A,		0x0042},
{0x7000027C,		0x0010},
{0x7000027E,		0x01E0},
{0x70000280,		0x0000},
{0x70000282,		0x0000},
{0x70000284,		0x0000},
{0x70000286,		0x0001}, //1b: FR (bin) 2b: Quality (no-bin)
{0x70000288,		0x0535}, //30fps 014D; 15fps 029a; 535 7.5FPS; a6a 3.75fps;
{0x7000028A,		0x014D},
// APPLY PREVIEW CONFIGURATION & RUN PREVIEW
{0x7000023C,		0x0000},
{0x70000240,		0x0001},
{0x70000230,		0x0001},
{0x7000023E,		0x0001},
{0x70000220,		0x0001},
{0x70000222,		0x0001},
// SET CAPTURE CONFIGURATION_0
{0x7000035C,		0x0000},
{0x7000035E,		0x0800}, //2048
{0x70000360,		0x0600}, //1536
{0x70000362,		0x0005}, //YUV
{0x70000364,		0x4273}, //PCLK max
{0x70000366,		0x4268}, //PCLK min, 4268 68MHz,
{0x70000368,		0x0100},
{0x7000036A,		0x0800},
{0x7000036C,		0x0042},
{0x7000036E,		0x0010},
{0x70000370,		0x03C0},
{0x70000372,		0x08fc},
{0x70000374,		0x0001},
{0x70000376,		0x0000},
{0x70000378,		0x0002},
{0x7000037A,		0x0A6A},
{0x7000037C,		0x029A},
};
#endif

static unsigned char cam_initial_sset_setup[]=
{
{0x0F},{0x12},
{0xB5},{0x10},
{0x48},{0x27},
{0x21},{0xC0},
{0x80},{0x41},
{0x48},{0x25},
{0x4A},{0x26},
{0x30},{0x20},
{0x83},{0x82},
{0x1D},{0x12},
{0x83},{0xC2},
{0x48},{0x22},
{0x30},{0x40},
{0x80},{0x41},
{0x48},{0x21},
{0x49},{0x22},
{0x30},{0x60},
{0x83},{0x81},
{0x1D},{0x09},
{0x83},{0xC1},
{0x48},{0x21},
{0x49},{0x1D},
{0x88},{0x02},
{0x39},{0x80},
{0x80},{0x4A},
{0x88},{0x42},
{0x80},{0x8A},
{0x88},{0x82},
{0x80},{0xCA},
{0x88},{0xC2},
{0x81},{0x0A},
{0x89},{0x02},
{0x49},{0x1C},
{0x80},{0xCA},
{0x89},{0x42},
{0x81},{0x4A},
{0x89},{0x82},
{0x83},{0x0A},
{0x89},{0xC2},
{0x83},{0x4A},
{0x8A},{0x00},
{0x49},{0x18},
{0x81},{0x88},
{0x49},{0x18},
{0x48},{0x19},
{0xF0},{0x00},
{0xFA},{0x0C},
{0x49},{0x18},
{0x48},{0x19},
{0x63},{0x41},
{0x49},{0x19},
{0x48},{0x19},
{0xF0},{0x00},
{0xFA},{0x05},
{0x48},{0x16},
{0x49},{0x18},
{0x38},{0x40},
{0x62},{0xC1},
{0x49},{0x18},
{0x38},{0x80},
{0x63},{0xC1},
{0x49},{0x17},
{0x63},{0x01},
{0x49},{0x17},
{0x30},{0x40},
{0x61},{0x81},
{0x49},{0x17},
{0x48},{0x17},
{0xF0},{0x00},
{0xF9},{0xF5},
{0x49},{0x17},
{0x48},{0x17},
{0xF0},{0x00},
{0xF9},{0xF1},
{0x49},{0x17},
{0x48},{0x17},
{0xF0},{0x00},
{0xF9},{0xED},
{0xBC},{0x10},
{0xBC},{0x08},
{0x47},{0x18},
{0x11},{0x00},
{0xD0},{0x00},
{0x26},{0x7C},
{0x00},{0x00},
{0x2C},{0xE8},
{0x00},{0x00},
{0x32},{0x74},
{0x70},{0x00},
{0xF4},{0x00},
{0xD0},{0x00},
{0xF5},{0x20},
{0xD0},{0x00},
{0x2D},{0xF1},
{0x70},{0x00},
{0x89},{0xA9},
{0x00},{0x00},
{0x2E},{0x43},
{0x70},{0x00},
{0x01},{0x40},
{0x70},{0x00},
{0x2E},{0x75},
{0x70},{0x00},
{0xB4},{0xF7},
{0x00},{0x00},
{0x2E},{0xFF},
{0x70},{0x00},
{0x2F},{0x23},
{0x70},{0x00},
{0x2F},{0xCD},
{0x70},{0x00},
{0x2F},{0xE1},
{0x70},{0x00},
{0x2F},{0xB5},
{0x70},{0x00},
{0x01},{0x3D},
{0x00},{0x01},
{0x30},{0x67},
{0x70},{0x00},
{0x58},{0x23},
{0x00},{0x00},
{0x30},{0xB5},
{0x70},{0x00},
{0xD7},{0x89},
{0x00},{0x00},
{0xB5},{0x70},
{0x68},{0x04},
{0x68},{0x45},
{0x68},{0x81},
{0x68},{0x40},
{0x29},{0x00},
{0x68},{0x80},
{0xD0},{0x07},
{0x49},{0xC2},
{0x89},{0x49},
{0x08},{0x4A},
{0x18},{0x80},
{0xF0},{0x00},
{0xF9},{0xB8},
{0x80},{0xA0},
{0xE0},{0x00},
{0x80},{0xA0},
{0x88},{0xA0},
{0x28},{0x00},
{0xD0},{0x10},
{0x68},{0xA9},
{0x68},{0x28},
{0x08},{0x4A},
{0x18},{0x80},
{0xF0},{0x00},
{0xF9},{0xAC},
{0x80},{0x20},
{0x1D},{0x2D},
{0xCD},{0x03},
{0x08},{0x4A},
{0x18},{0x80},
{0xF0},{0x00},
{0xF9},{0xA5},
{0x80},{0x60},
{0xBC},{0x70},
{0xBC},{0x08},
{0x47},{0x18},
{0x20},{0x00},
{0x80},{0x60},
{0x80},{0x20},
{0xE7},{0xF8},
{0xB5},{0x10},
{0xF0},{0x00},
{0xF9},{0xA0},
{0x48},{0xB1},
{0x49},{0xB2},
{0x88},{0x00},
{0x4A},{0xB2},
{0x28},{0x05},
{0xD0},{0x03},
{0x4B},{0xB1},
{0x79},{0x5B},
{0x2B},{0x00},
{0xD0},{0x05},
{0x20},{0x01},
{0x80},{0x08},
{0x80},{0x10},
{0xBC},{0x10},
{0xBC},{0x08},
{0x47},{0x18},
{0x28},{0x00},
{0xD1},{0xFA},
{0x20},{0x00},
{0x80},{0x08},
{0x80},{0x10},
{0xE7},{0xF6},
{0xB5},{0xF8},
{0x24},{0x07},
{0x2C},{0x06},
{0xD0},{0x35},
{0x2C},{0x07},
{0xD0},{0x33},
{0x48},{0xA3},
{0x8B},{0xC1},
{0x29},{0x00},
{0xD0},{0x2A},
{0x00},{0xA2},
{0x18},{0x15},
{0x4A},{0xA4},
{0x6D},{0xEE},
{0x8A},{0x92},
{0x42},{0x96},
{0xD9},{0x23},
{0x00},{0x28},
{0x30},{0x80},
{0x00},{0x07},
{0x69},{0xC0},
{0xF0},{0x00},
{0xF9},{0x6D},
{0x1C},{0x71},
{0x02},{0x80},
{0xF0},{0x00},
{0xF9},{0x69},
{0x00},{0x06},
{0x48},{0x98},
{0x00},{0x61},
{0x18},{0x08},
{0x8D},{0x80},
{0x0A},{0x01},
{0x06},{0x00},
{0x0E},{0x00},
{0x1A},{0x08},
{0xF0},{0x00},
{0xF9},{0x6C},
{0x00},{0x02},
{0x6D},{0xE9},
{0x6F},{0xE8},
{0x1A},{0x08},
{0x43},{0x51},
{0x03},{0x00},
{0x1C},{0x49},
{0xF0},{0x00},
{0xF9},{0x55},
{0x04},{0x01},
{0x04},{0x30},
{0x0C},{0x00},
{0x43},{0x01},
{0x61},{0xF9},
{0xE0},{0x04},
{0x00},{0xA2},
{0x49},{0x90},
{0x18},{0x10},
{0x30},{0x80},
{0x61},{0xC1},
{0x1E},{0x64},
{0xD2},{0xC5},
{0x20},{0x06},
{0xF0},{0x00},
{0xF9},{0x5B},
{0x20},{0x07},
{0xF0},{0x00},
{0xF9},{0x58},
{0xBC},{0xF8},
{0xBC},{0x08},
{0x47},{0x18},
{0xB5},{0x10},
{0xF0},{0x00},
{0xF9},{0x5A},
{0x28},{0x00},
{0xD0},{0x0A},
{0x48},{0x81},
{0x8B},{0x81},
{0x00},{0x89},
{0x18},{0x08},
{0x6D},{0xC1},
{0x48},{0x83},
{0x8A},{0x80},
{0x42},{0x81},
{0xD9},{0x01},
{0x20},{0x01},
{0xE7},{0xA1},
{0x20},{0x00},
{0xE7},{0x9F},
{0xB5},{0xF8},
{0x00},{0x04},
{0x4F},{0x80},
{0x22},{0x7D},
{0x89},{0x38},
{0x01},{0x52},
{0x43},{0x42},
{0x48},{0x7E},
{0x90},{0x00},
{0x8A},{0x01},
{0x08},{0x48},
{0x18},{0x10},
{0xF0},{0x00},
{0xF9},{0x1F},
{0x21},{0x0F},
{0xF0},{0x00},
{0xF9},{0x42},
{0x49},{0x7A},
{0x8C},{0x49},
{0x09},{0x0E},
{0x01},{0x36},
{0x43},{0x06},
{0x49},{0x79},
{0x2C},{0x00},
{0xD0},{0x03},
{0x20},{0x01},
{0x02},{0x40},
{0x43},{0x30},
{0x81},{0x08},
{0x48},{0x76},
{0x2C},{0x00},
{0x8D},{0x00},
{0xD0},{0x01},
{0x25},{0x01},
{0xE0},{0x00},
{0x25},{0x00},
{0x49},{0x72},
{0x43},{0x28},
{0x80},{0x08},
{0x20},{0x7D},
{0x00},{0xC0},
{0xF0},{0x00},
{0xF9},{0x30},
{0x2C},{0x00},
{0x49},{0x6E},
{0x03},{0x28},
{0x43},{0x30},
{0x81},{0x08},
{0x88},{0xF8},
{0x2C},{0x00},
{0x01},{0xAA},
{0x43},{0x10},
{0x80},{0x88},
{0x2C},{0x00},
{0xD0},{0x0B},
{0x98},{0x00},
{0x8A},{0x01},
{0x48},{0x69},
{0xF0},{0x00},
{0xF8},{0xF1},
{0x49},{0x69},
{0x88},{0x09},
{0x43},{0x48},
{0x04},{0x00},
{0x0C},{0x00},
{0xF0},{0x00},
{0xF9},{0x18},
{0x00},{0x20},
{0xF0},{0x00},
{0xF9},{0x1D},
{0x48},{0x65},
{0x70},{0x04},
{0xE7},{0xA1},
{0xB5},{0x10},
{0x00},{0x04},
{0xF0},{0x00},
{0xF9},{0x1E},
{0x60},{0x20},
{0x49},{0x62},
{0x8B},{0x49},
{0x07},{0x89},
{0xD0},{0x01},
{0x00},{0x40},
{0x60},{0x20},
{0xE7},{0x4A},
{0xB5},{0x10},
{0xF0},{0x00},
{0xF9},{0x1B},
{0x48},{0x5E},
{0x88},{0x80},
{0x06},{0x01},
{0x48},{0x53},
{0x16},{0x09},
{0x81},{0x41},
{0xE7},{0x40},
{0xB5},{0xF8},
{0x00},{0x0F},
{0x4C},{0x54},
{0x34},{0x20},
{0x25},{0x00},
{0x57},{0x65},
{0x00},{0x39},
{0xF0},{0x00},
{0xF9},{0x13},
{0x90},{0x00},
{0x26},{0x00},
{0x57},{0xA6},
{0x4C},{0x4B},
{0x42},{0xAE},
{0xD0},{0x1B},
{0x4D},{0x53},
{0x8A},{0xE8},
{0x28},{0x00},
{0xD0},{0x13},
{0x48},{0x4C},
{0x8A},{0x01},
{0x8B},{0x80},
{0x43},{0x78},
{0xF0},{0x00},
{0xF8},{0xB5},
{0x89},{0xA9},
{0x1A},{0x41},
{0x48},{0x4D},
{0x38},{0x20},
{0x8A},{0xC0},
{0x43},{0x48},
{0x17},{0xC1},
{0x0D},{0x89},
{0x18},{0x08},
{0x12},{0x80},
{0x89},{0x61},
{0x1A},{0x08},
{0x81},{0x60},
{0xE0},{0x03},
{0x88},{0xA8},
{0x06},{0x00},
{0x16},{0x00},
{0x81},{0x60},
{0x20},{0x0A},
{0x5E},{0x20},
{0x42},{0xB0},
{0xD0},{0x11},
{0xF0},{0x00},
{0xF8},{0xAB},
{0x1D},{0x40},
{0x00},{0xC3},
{0x1A},{0x18},
{0x21},{0x4B},
{0xF0},{0x00},
{0xF8},{0x97},
{0x21},{0x1F},
{0xF0},{0x00},
{0xF8},{0xBA},
{0x21},{0x0A},
{0x5E},{0x61},
{0x0F},{0xC9},
{0x01},{0x49},
{0x43},{0x01},
{0x48},{0x3C},
{0x81},{0xC1},
{0x98},{0x00},
{0xE7},{0x48},
{0xB5},{0xF1},
{0xB0},{0x82},
{0x25},{0x00},
{0x48},{0x39},
{0x90},{0x01},
{0x24},{0x00},
{0x20},{0x28},
{0x43},{0x68},
{0x4A},{0x39},
{0x49},{0x37},
{0x18},{0x87},
{0x18},{0x40},
{0x90},{0x00},
{0x98},{0x00},
{0x00},{0x66},
{0x9A},{0x01},
{0x19},{0x80},
{0x21},{0x8C},
{0x5A},{0x09},
{0x8A},{0x80},
{0x88},{0x12},
{0xF0},{0x00},
{0xF8},{0xCA},
{0x53},{0xB8},
{0x1C},{0x64},
{0x2C},{0x14},
{0xDB},{0xF1},
{0x1C},{0x6D},
{0x2D},{0x03},
{0xDB},{0xE6},
{0x98},{0x02},
{0x68},{0x00},
{0x06},{0x00},
{0x0E},{0x00},
{0xF0},{0x00},
{0xF8},{0xC5},
{0xBC},{0xFE},
{0xBC},{0x08},
{0x47},{0x18},
{0xB5},{0x70},
{0x68},{0x05},
{0x24},{0x04},
{0xF0},{0x00},
{0xF8},{0xC5},
{0x28},{0x00},
{0xD1},{0x03},
{0xF0},{0x00},
{0xF8},{0xC9},
{0x28},{0x00},
{0xD0},{0x00},
{0x24},{0x00},
{0x35},{0x40},
{0x88},{0xE8},
{0x05},{0x00},
{0xD4},{0x03},
{0x48},{0x22},
{0x89},{0xC0},
{0x28},{0x00},
{0xD0},{0x02},
{0x20},{0x08},
{0x43},{0x04},
{0xE0},{0x01},
{0x20},{0x10},
{0x43},{0x04},
{0x48},{0x1F},
{0x8B},{0x80},
{0x07},{0x00},
{0x0F},{0x81},
{0x20},{0x01},
{0x29},{0x00},
{0xD0},{0x00},
{0x43},{0x04},
{0x49},{0x1C},
{0x8B},{0x0A},
{0x42},{0xA2},
{0xD0},{0x04},
{0x07},{0x62},
{0xD5},{0x02},
{0x4A},{0x19},
{0x32},{0x20},
{0x81},{0x10},
{0x83},{0x0C},
{0xE6},{0x93},
{0x0C},{0x3C},
{0x70},{0x00},
{0x26},{0xE8},
{0x70},{0x00},
{0x61},{0x00},
{0xD0},{0x00},
{0x65},{0x00},
{0xD0},{0x00},
{0x1A},{0x7C},
{0x70},{0x00},
{0x11},{0x20},
{0x70},{0x00},
{0xFF},{0xFF},
{0x00},{0x00},
{0x33},{0x74},
{0x70},{0x00},
{0x1D},{0x6C},
{0x70},{0x00},
{0x16},{0x7C},
{0x70},{0x00},
{0xF4},{0x00},
{0xD0},{0x00},
{0x2C},{0x2C},
{0x70},{0x00},
{0x40},{0xA0},
{0x00},{0xDD},
{0xF5},{0x20},
{0xD0},{0x00},
{0x2C},{0x29},
{0x70},{0x00},
{0x1A},{0x54},
{0x70},{0x00},
{0x15},{0x64},
{0x70},{0x00},
{0xF2},{0xA0},
{0xD0},{0x00},
{0x24},{0x40},
{0x70},{0x00},
{0x32},{0x74},
{0x70},{0x00},
{0x05},{0xA0},
{0x70},{0x00},
{0x28},{0x94},
{0x70},{0x00},
{0x12},{0x24},
{0x70},{0x00},
{0xB0},{0x00},
{0xD0},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0x1A},{0x3F},
{0x00},{0x01},
{0x47},{0x78},
{0x46},{0xC0},
{0xF0},{0x04},
{0xE5},{0x1F},
{0x1F},{0x48},
{0x00},{0x01},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0x24},{0xBD},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0x36},{0xDD},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0xB4},{0xCF},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0xB5},{0xD7},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0x36},{0xED},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0xF5},{0x3F},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0xF5},{0xD9},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0x01},{0x3D},
{0x00},{0x01},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0xF5},{0xC9},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0xFA},{0xA9},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0x37},{0x23},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0x58},{0x23},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0xD7},{0x71},
{0x00},{0x00},
{0x47},{0x78},
{0x46},{0xC0},
{0xC0},{0x00},
{0xE5},{0x9F},
{0xFF},{0x1C},
{0xE1},{0x2F},
{0xD7},{0x5B},
{0x00},{0x00},
{0x7E},{0x77},
{0x00},{0x00},
//
// End T&P part
};

static uint32_t cam_long_version_sset_porting_setup[]=
{
//============================================================
// CIS/APS/Analog setting        - 400LSB  SYSCLK 32MHz
//============================================================
// This registers are for FACTORY ONLY. If you change it without prior notification,
// YOU are RESPONSIBLE for the FAILURE that will happen in the future.
//============================================================

{0x002A157A},
{0x0F120001},
{0x002A1578},
{0x0F120001},
{0x002A1576},
{0x0F120020},
{0x002A1574},
{0x0F120006},
{0x002A156E},
{0x0F120001},	// Slope calibration tolerance in units of 1/256
{0x002A1568},
{0x0F1200FC},

//ADC control

{0x002A155A},
{0x0F1201CC},	//ADC SAT of 450mV for 10bit default in EVT1
{0x002A157E},
{0x0F120C80},	// 3200 Max. Reset ramp DCLK counts (default 2048 0x800)
{0x0F120578},	// 1400 Max. Reset ramp DCLK counts for x3.5
{0x002A157C},
{0x0F120190},	// 400 Reset ramp for x1 in DCLK counts
{0x002A1570},
{0x0F1200A0},	// 160 LSB
{0x0F120010},	// reset threshold
{0x002A12C4},
{0x0F12006A},	// 106 additional timing columns.
{0x002A12C8},
{0x0F1208AC},	// 2220 ADC columns in normal mode including Hold & Latch
{0x0F120050},	// 80 addition of ADC columns in Y-ave mode (default 244 0x74)
//WRITE #senHal_ForceModeType           0001    // Long exposure mode

{0x002A1696},
{0x0F120000},	// based on APS guidelines
{0x0F120000},	// based on APS guidelines
{0x0F1200C6},	// default. 1492 used for ADC dark characteristics
{0x0F1200C6},	// default. 1492 used for ADC dark characteristics
{0x002A12B8},
{0x0F120B00},	//disable CINTR 0

{0x002A1690},
{0x0F120001},	// when set double sampling is activated - requires different set of pointers

{0x002A12B0},
{0x0F120055},	// comp and pixel bias control 0xF40E - default for EVT1
{0x0F12005A},	// comp and pixel bias control 0xF40E for binning mode

{0x002A337A},
{0x0F120006},	// [7] - is used for rest-only mode (EVT0 value is 0xD and HW 0x6)
{0x002A169E},
{0x0F120007},	// [3:0]- specifies the target (default 7)- DCLK = 64MHz instead of 116MHz.

{0x002A327C},
{0x0F121000},	//Enable DBLR Regulation
{0x0F126998},	//VPIX 2.8
{0x0F120078},	//[0] Static RC Filter
{0x0F1204FE},	//[7:4] Full RC Filter
{0x0F128800},	//Add Load to CDS block

{0x002A3274},
{0x0F1203FF},	// 0000   //     Set IO driving current 2mA for GS500
{0x0F1203FF},	// 0000 //       Set IO driving current
{0x0F123FFF},	// 0000 //       Set IO driving current
{0x0F120FFF},	// 0000 //       Set IO driving current

//Asserting CDS pointers - Long exposure MS Normal
// Conditions: 10bit, ADC_SAT = 450mV ; ramp_del = 40 ; ramp_start = 60
{0x002A12D2},
{0x0F120003},	// aig_ld_ptr0
{0x002A12DA},
{0x0F120884},	// aig_ld_ptr1
{0x002A12E2},
{0x0F120001},	// aig_sla_ptr0
{0x002A12EA},
{0x0F120885},	// aig_sla_ptr1
{0x002A12F2},
{0x0F120001},	// aig_slb_ptr0
{0x002A12FA},
{0x0F120885},	// aig_slb_ptr1
{0x002A1302},
{0x0F120006},	// aig_rxa_ptr0
{0x002A130A},
{0x0F120881},	// aig_rxa_ptr1
{0x002A1312},
{0x0F120006},	// aig_rxb_ptr0
{0x002A131A},
{0x0F120881},	// aig_rxb_ptr1
{0x002A1322},
{0x0F1203A2},	// aig_txa_ptr0
{0x002A132A},
{0x0F1203F2},	// aig_txa_ptr1
{0x002A1332},
{0x0F1203A2},	// aig_txb_ptr0
{0x002A133A},
{0x0F1203F2},	// aig_txb_ptr1
{0x002A1342},
{0x0F120002},	// aig_s1_ptr0
{0x002A134A},
{0x0F12003C},	// aig_s1_ptr1
{0x002A1352},
{0x0F1201D3},	// aig_s1_ptr2
{0x002A135A},
{0x0F12020B},	// aig_s1_ptr3
{0x002A1362},
{0x0F120002},	// aig_s1_ptr4
{0x002A136A},
{0x0F120419},	// aig_s1_ptr5
{0x002A1372},
{0x0F120630},	// aig_s1_ptr6
{0x002A137A},
{0x0F120668},	// aig_s1_ptr7
{0x002A1382},
{0x0F120001},	// aig_rmp_mode_ptr0
{0x002A138A},
{0x0F1203A2},	// aig_rmp_mode_ptr1
{0x002A1392},
{0x0F120000},	// aig_rmp_mode_ptr2
{0x002A139A},
{0x0F120000},	// aig_rmp_mode_ptr3
{0x002A13A2},
{0x0F12003D},	// aig_rmp_rst_ptr0
{0x002A13AA},
{0x0F1201D0},	// aig_rmp_rst_ptr1
{0x002A13B2},
{0x0F12020C},	// aig_rmp_rst_ptr2
{0x002A13BA},
{0x0F12039F},	// aig_rmp_rst_ptr3
{0x002A13C2},
{0x0F12041A},	// aig_rmp_rst_ptr4
{0x002A13CA},
{0x0F12062D},	// aig_rmp_rst_ptr5
{0x002A13D2},
{0x0F120669},	// aig_rmp_rst_ptr6
{0x002A13DA},
{0x0F12087C},	// aig_rmp_rst_ptr7
{0x002A13E2},
{0x0F120040},	// aig_cnt_en_ptr0
{0x002A13EA},
{0x0F1201D0},	// aig_cnt_en_ptr1
{0x002A13F2},
{0x0F12020F},	// aig_cnt_en_ptr2
{0x002A13FA},
{0x0F12039F},	// aig_cnt_en_ptr3
{0x002A1402},
{0x0F12041D},	// aig_cnt_en_ptr4
{0x002A140A},
{0x0F12062D},	// aig_cnt_en_ptr5
{0x002A1412},
{0x0F12066C},	// aig_cnt_en_ptr6
{0x002A141A},
{0x0F12087C},	// aig_cnt_en_ptr7
{0x002A1422},
{0x0F120040},	// aig_cnt_en2_ptr0
{0x002A142A},
{0x0F1201D0},	// aig_cnt_en2_ptr1
{0x002A1432},
{0x0F12020F},	// aig_cnt_en2_ptr2
{0x002A143A},
{0x0F12039F},	// aig_cnt_en2_ptr3
{0x002A1442},
{0x0F12041D},	// aig_cnt_en2_ptr4
{0x002A144A},
{0x0F12062D},	// aig_cnt_en2_ptr5
{0x002A1452},
{0x0F12066C},	// aig_cnt_en2_ptr6
{0x002A145A},
{0x0F12087C},	// aig_cnt_en2_ptr7
{0x002A1462},
{0x0F12003D},	// aig_cmp_rst_ptr0
{0x002A146A},
{0x0F1201D2},	// aig_cmp_rst_ptr1
{0x002A1472},
{0x0F12020C},	// aig_cmp_rst_ptr2
{0x002A147A},
{0x0F1203A1},	// aig_cmp_rst_ptr3
{0x002A1482},
{0x0F12041A},	// aig_cmp_rst_ptr4
{0x002A148A},
{0x0F12062F},	// aig_cmp_rst_ptr5
{0x002A1492},
{0x0F120669},	// aig_cmp_rst_ptr6
{0x002A149A},
{0x0F12087E},	// aig_cmp_rst_ptr7
{0x002A14A2},
{0x0F1203A2},	// aig_conv1_ptr0
{0x002A14AA},
{0x0F1203AF},	// aig_conv1_ptr1
{0x002A14B2},
{0x0F120000},	// aig_conv1_ptr2
{0x002A14BA},
{0x0F120000},	// aig_conv1_ptr3
{0x002A14C2},
{0x0F120000},	// aig_conv1_ptr4
{0x002A14CA},
{0x0F120000},	// aig_conv1_ptr5
{0x002A14D2},
{0x0F120000},	// aig_conv1_ptr6
{0x002A14DA},
{0x0F120000},	// aig_conv1_ptr7
{0x002A14E2},
{0x0F1203AA},	// aig_conv2_ptr0
{0x002A14EA},
{0x0F1203B7},	// aig_conv2_ptr1
{0x002A14F2},
{0x0F120000},	// aig_conv2_ptr2
{0x002A14FA},
{0x0F120000},	// aig_conv2_ptr3
{0x002A1502},
{0x0F120000},	// aig_conv2_ptr4
{0x002A150A},
{0x0F120000},	// aig_conv2_ptr5
{0x002A1512},
{0x0F120000},	// aig_conv2_ptr6
{0x002A151A},
{0x0F120000},	// aig_conv2_ptr7
{0x002A1522},
{0x0F120001},	// aig_cnt_rst_ptr0
{0x002A152A},
{0x0F12000F},	// aig_cnt_rst_ptr1
{0x002A1532},
{0x0F1205AD},	// aig_ramp_slope_sel_ptr0
{0x002A153A},
{0x0F12062F},	// aig_ramp_slope_sel_ptr1
{0x002A1542},
{0x0F1207FC},	// aig_ramp_slope_sel_ptr2
{0x002A154A},
{0x0F120000},	// aig_ramp_slope_sel_ptr3

//Asserting CDS pointers - Long exposure Binning mode
// Conditions: 10bit, ADC_SAT = 450mV ; ramp_del = 40 ; ramp_start = 60
{0x002A12D4},
{0x0F120003},	// aig_ld_ptr0
{0x002A12DC},
{0x0F1208CF},	// aig_ld_ptr1
{0x002A12E4},
{0x0F120001},	// aig_sla_ptr0
{0x002A12EC},
{0x0F120467},	// aig_sla_ptr1
{0x002A12F4},
{0x0F12046A},	// aig_slb_ptr0
{0x002A12FC},
{0x0F1208D0},	// aig_slb_ptr1
{0x002A1304},
{0x0F120020},	// aig_rxa_ptr0
{0x002A130C},
{0x0F120463},	// aig_rxa_ptr1
{0x002A1314},
{0x0F120489},	// aig_rxb_ptr0
{0x002A131C},
{0x0F1208CC},	// aig_rxb_ptr1
{0x002A1324},
{0x0F1201D3},	// aig_txa_ptr0
{0x002A132C},
{0x0F120223},	// aig_txa_ptr1
{0x002A1334},
{0x0F12063C},	// aig_txb_ptr0
{0x002A133C},
{0x0F12068C},	// aig_txb_ptr1
{0x002A1344},
{0x0F120002},	// aig_s1_ptr0
{0x002A134C},
{0x0F12003C},	// aig_s1_ptr1
{0x002A1354},
{0x0F1201D3},	// aig_s1_ptr2
{0x002A135C},
{0x0F12024A},	// aig_s1_ptr3
{0x002A1364},
{0x0F12046B},	// aig_s1_ptr4
{0x002A136C},
{0x0F1204A5},	// aig_s1_ptr5
{0x002A1374},
{0x0F12063C},	// aig_s1_ptr6
{0x002A137C},
{0x0F1206B3},	// aig_s1_ptr7
{0x002A1384},
{0x0F120001},	// aig_rmp_mode_ptr0
{0x002A138C},
{0x0F1201D3},	// aig_rmp_mode_ptr1
{0x002A1394},
{0x0F120461},	// aig_rmp_mode_ptr2
{0x002A139C},
{0x0F12063C},	// aig_rmp_mode_ptr3
{0x002A13A4},
{0x0F12003D},	// aig_rmp_rst_ptr0
{0x002A13AC},
{0x0F1201D0},	// aig_rmp_rst_ptr1
{0x002A13B4},
{0x0F12024B},	// aig_rmp_rst_ptr2
{0x002A13BC},
{0x0F12045E},	// aig_rmp_rst_ptr3
{0x002A13C4},
{0x0F1204A6},	// aig_rmp_rst_ptr4
{0x002A13CC},
{0x0F120639},	// aig_rmp_rst_ptr5
{0x002A13D4},
{0x0F1206B4},	// aig_rmp_rst_ptr6
{0x002A13DC},
{0x0F1208C7},	// aig_rmp_rst_ptr7
{0x002A13E4},
{0x0F120040},	// aig_cnt_en_ptr0
{0x002A13EC},
{0x0F1201D0},	// aig_cnt_en_ptr1
{0x002A13F4},
{0x0F12024E},	// aig_cnt_en_ptr2
{0x002A13FC},
{0x0F12045E},	// aig_cnt_en_ptr3
{0x002A1404},
{0x0F1204A9},	// aig_cnt_en_ptr4
{0x002A140C},
{0x0F120639},	// aig_cnt_en_ptr5
{0x002A1414},
{0x0F1206B7},	// aig_cnt_en_ptr6
{0x002A141C},
{0x0F1208C7},	// aig_cnt_en_ptr7
{0x002A1424},
{0x0F120040},	// aig_cnt_en2_ptr0
{0x002A142C},
{0x0F1201D0},	// aig_cnt_en2_ptr1
{0x002A1434},
{0x0F12024E},	// aig_cnt_en2_ptr2
{0x002A143C},
{0x0F12045E},	// aig_cnt_en2_ptr3
{0x002A1444},
{0x0F1204A9},	// aig_cnt_en2_ptr4
{0x002A144C},
{0x0F120639},	// aig_cnt_en2_ptr5
{0x002A1454},
{0x0F1206B7},	// aig_cnt_en2_ptr6
{0x002A145C},
{0x0F1208C7},	// aig_cnt_en2_ptr7
{0x002A1464},
{0x0F12003D},	// aig_cmp_rst_ptr0
{0x002A146C},
{0x0F1201D2},	// aig_cmp_rst_ptr1
{0x002A1474},
{0x0F12024B},	// aig_cmp_rst_ptr2
{0x002A147C},
{0x0F120460},	// aig_cmp_rst_ptr3
{0x002A1484},
{0x0F1204A6},	// aig_cmp_rst_ptr4
{0x002A148C},
{0x0F12063B},	// aig_cmp_rst_ptr5
{0x002A1494},
{0x0F1206B4},	// aig_cmp_rst_ptr6
{0x002A149C},
{0x0F1208C9},	// aig_cmp_rst_ptr7
{0x002A14A4},
{0x0F1201D3},	// aig_conv1_ptr0
{0x002A14AC},
{0x0F1201E0},	// aig_conv1_ptr1
{0x002A14B4},
{0x0F120461},	// aig_conv1_ptr2
{0x002A14BC},
{0x0F12046E},	// aig_conv1_ptr3
{0x002A14C4},
{0x0F12063C},	// aig_conv1_ptr4
{0x002A14CC},
{0x0F120649},	// aig_conv1_ptr5
{0x002A14D4},
{0x0F120000},	// aig_conv1_ptr6
{0x002A14DC},
{0x0F120000},	// aig_conv1_ptr7
{0x002A14E4},
{0x0F1201DB},	// aig_conv2_ptr0
{0x002A14EC},
{0x0F1201E8},	// aig_conv2_ptr1
{0x002A14F4},
{0x0F120469},	// aig_conv2_ptr2
{0x002A14FC},
{0x0F120476},	// aig_conv2_ptr3
{0x002A1504},
{0x0F120644},	// aig_conv2_ptr4
{0x002A150C},
{0x0F120651},	// aig_conv2_ptr5
{0x002A1514},
{0x0F120000},	// aig_conv2_ptr6
{0x002A151C},
{0x0F120000},	// aig_conv2_ptr7
{0x002A1524},
{0x0F120001},	// aig_cnt_rst_ptr0
{0x002A152C},
{0x0F12000F},	// aig_cnt_rst_ptr1
{0x002A1534},
{0x0F1203DE},	// aig_ramp_slope_sel_ptr0
{0x002A153C},
{0x0F120460},	// aig_ramp_slope_sel_ptr1
{0x002A1544},
{0x0F120847},	// aig_ramp_slope_sel_ptr2
{0x002A154C},
{0x0F120000},	// aig_ramp_slope_sel_ptr3

//Asserting CDS pointers - Short exposure MS Normal x3.5
// Conditions: 10bit, ADC_SAT = 450mV ; ramp_del = 40 ; ramp_start = 60

{0x002A12D6},
{0x0F120003},	// aig_ld_ptr0
{0x002A12DE},
{0x0F120500},	// aig_ld_ptr1
{0x002A12E6},
{0x0F120001},	// aig_sla_ptr0
{0x002A12EE},
{0x0F120501},	// aig_sla_ptr1
{0x002A12F6},
{0x0F120001},	// aig_slb_ptr0
{0x002A12FE},
{0x0F120501},	// aig_slb_ptr1
{0x002A1306},
{0x0F120006},	// aig_rxa_ptr0
{0x002A130E},
{0x0F1204FD},	// aig_rxa_ptr1
{0x002A1316},
{0x0F120006},	// aig_rxb_ptr0
{0x002A131E},
{0x0F1204FD},	// aig_rxb_ptr1
{0x002A1326},
{0x0F1201E0},	// aig_txa_ptr0
{0x002A132E},
{0x0F120230},	// aig_txa_ptr1
{0x002A1336},
{0x0F1201E0},	// aig_txb_ptr0
{0x002A133E},
{0x0F120230},	// aig_txb_ptr1
{0x002A1346},
{0x0F120002},	// aig_s1_ptr0
{0x002A134E},
{0x0F12003C},	// aig_s1_ptr1
{0x002A1356},
{0x0F1200F2},	// aig_s1_ptr2
{0x002A135E},
{0x0F12012A},	// aig_s1_ptr3
{0x002A1366},
{0x0F120002},	// aig_s1_ptr4
{0x002A136E},
{0x0F120257},	// aig_s1_ptr5
{0x002A1376},
{0x0F12038D},	// aig_s1_ptr6
{0x002A137E},
{0x0F1203C5},	// aig_s1_ptr7
{0x002A1386},
{0x0F120001},	// aig_rmp_mode_ptr0
{0x002A138E},
{0x0F1201E0},	// aig_rmp_mode_ptr1
{0x002A1396},
{0x0F120000},	// aig_rmp_mode_ptr2
{0x002A139E},
{0x0F120000},	// aig_rmp_mode_ptr3
{0x002A13A6},
{0x0F12003D},	// aig_rmp_rst_ptr0
{0x002A13AE},
{0x0F1200EF},	// aig_rmp_rst_ptr1
{0x002A13B6},
{0x0F12012B},	// aig_rmp_rst_ptr2
{0x002A13BE},
{0x0F1201DD},	// aig_rmp_rst_ptr3
{0x002A13C6},
{0x0F120258},	// aig_rmp_rst_ptr4
{0x002A13CE},
{0x0F12038A},	// aig_rmp_rst_ptr5
{0x002A13D6},
{0x0F1203C6},	// aig_rmp_rst_ptr6
{0x002A13DE},
{0x0F1204F8},	// aig_rmp_rst_ptr7
{0x002A13E6},
{0x0F120040},	// aig_cnt_en_ptr0
{0x002A13EE},
{0x0F1200EF},	// aig_cnt_en_ptr1
{0x002A13F6},
{0x0F12012E},	// aig_cnt_en_ptr2
{0x002A13FE},
{0x0F1201DD},	// aig_cnt_en_ptr3
{0x002A1406},
{0x0F12025B},	// aig_cnt_en_ptr4
{0x002A140E},
{0x0F12038A},	// aig_cnt_en_ptr5
{0x002A1416},
{0x0F1203C9},	// aig_cnt_en_ptr6
{0x002A141E},
{0x0F1204F8},	// aig_cnt_en_ptr7
{0x002A1426},
{0x0F120040},	// aig_cnt_en2_ptr0
{0x002A142E},
{0x0F1200EF},	// aig_cnt_en2_ptr1
{0x002A1436},
{0x0F12012E},	// aig_cnt_en2_ptr2
{0x002A143E},
{0x0F1201DD},	// aig_cnt_en2_ptr3
{0x002A1446},
{0x0F12025B},	// aig_cnt_en2_ptr4
{0x002A144E},
{0x0F12038A},	// aig_cnt_en2_ptr5
{0x002A1456},
{0x0F1203C9},	// aig_cnt_en2_ptr6
{0x002A145E},
{0x0F1204F8},	// aig_cnt_en2_ptr7
{0x002A1466},
{0x0F12003D},	// aig_cmp_rst_ptr0
{0x002A146E},
{0x0F1200F1},	// aig_cmp_rst_ptr1
{0x002A1476},
{0x0F12012B},	// aig_cmp_rst_ptr2
{0x002A147E},
{0x0F1201DF},	// aig_cmp_rst_ptr3
{0x002A1486},
{0x0F120258},	// aig_cmp_rst_ptr4
{0x002A148E},
{0x0F12038C},	// aig_cmp_rst_ptr5
{0x002A1496},
{0x0F1203C6},	// aig_cmp_rst_ptr6
{0x002A149E},
{0x0F1204FA},	// aig_cmp_rst_ptr7
{0x002A14A6},
{0x0F1201E0},	// aig_conv1_ptr0
{0x002A14AE},
{0x0F1201ED},	// aig_conv1_ptr1
{0x002A14B6},
{0x0F120000},	// aig_conv1_ptr2
{0x002A14BE},
{0x0F120000},	// aig_conv1_ptr3
{0x002A14C6},
{0x0F120000},	// aig_conv1_ptr4
{0x002A14CE},
{0x0F120000},	// aig_conv1_ptr5
{0x002A14D6},
{0x0F120000},	// aig_conv1_ptr6
{0x002A14DE},
{0x0F120000},	// aig_conv1_ptr7
{0x002A14E6},
{0x0F1201E8},	// aig_conv2_ptr0
{0x002A14EE},
{0x0F1201F5},	// aig_conv2_ptr1
{0x002A14F6},
{0x0F120000},	// aig_conv2_ptr2
{0x002A14FE},
{0x0F120000},	// aig_conv2_ptr3
{0x002A1506},
{0x0F120000},	// aig_conv2_ptr4
{0x002A150E},
{0x0F120000},	// aig_conv2_ptr5
{0x002A1516},
{0x0F120000},	// aig_conv2_ptr6
{0x002A151E},
{0x0F120000},	// aig_conv2_ptr7
{0x002A1526},
{0x0F120001},	// aig_cnt_rst_ptr0
{0x002A152E},
{0x0F12000F},	// aig_cnt_rst_ptr1
{0x002A1536},
{0x0F12030A},	// aig_ramp_slope_sel_ptr0
{0x002A153E},
{0x0F12038C},	// aig_ramp_slope_sel_ptr1
{0x002A1546},
{0x0F120478},	// aig_ramp_slope_sel_ptr2
{0x002A154E},
{0x0F120000},	// aig_ramp_slope_sel_ptr3

//Asserting CDS pointers - Short exposure Binning mode x3.5
// Conditions: 10bit, ADC_SAT = 450mV ; ramp_del = 40 ; ramp_start = 60

{0x002A12D8},
{0x0F120003},	// aig_ld_ptr0
{0x002A12E0},
{0x0F12054B},	// aig_ld_ptr1
{0x002A12E8},
{0x0F120001},	// aig_sla_ptr0
{0x002A12F0},
{0x0F1202A5},	// aig_sla_ptr1
{0x002A12F8},
{0x0F1202A8},	// aig_slb_ptr0
{0x002A1300},
{0x0F12054C},	// aig_slb_ptr1
{0x002A1308},
{0x0F120020},	// aig_rxa_ptr0
{0x002A1310},
{0x0F1202A1},	// aig_rxa_ptr1
{0x002A1318},
{0x0F1202C7},	// aig_rxb_ptr0
{0x002A1320},
{0x0F120548},	// aig_rxb_ptr1
{0x002A1328},
{0x0F1200F2},	// aig_txa_ptr0
{0x002A1330},
{0x0F120142},	// aig_txa_ptr1
{0x002A1338},
{0x0F120399},	// aig_txb_ptr0
{0x002A1340},
{0x0F1203E9},	// aig_txb_ptr1
{0x002A1348},
{0x0F120002},	// aig_s1_ptr0
{0x002A1350},
{0x0F12003C},	// aig_s1_ptr1
{0x002A1358},
{0x0F1200F2},	// aig_s1_ptr2
{0x002A1360},
{0x0F120169},	// aig_s1_ptr3
{0x002A1368},
{0x0F1202A9},	// aig_s1_ptr4
{0x002A1370},
{0x0F1202E3},	// aig_s1_ptr5
{0x002A1378},
{0x0F120399},	// aig_s1_ptr6
{0x002A1380},
{0x0F120410},	// aig_s1_ptr7
{0x002A1388},
{0x0F120001},	// aig_rmp_mode_ptr0
{0x002A1390},
{0x0F1200F2},	// aig_rmp_mode_ptr1
{0x002A1398},
{0x0F12029F},	// aig_rmp_mode_ptr2
{0x002A13A0},
{0x0F120399},	// aig_rmp_mode_ptr3
{0x002A13A8},
{0x0F12003D},	// aig_rmp_rst_ptr0
{0x002A13B0},
{0x0F1200EF},	// aig_rmp_rst_ptr1
{0x002A13B8},
{0x0F12016A},	// aig_rmp_rst_ptr2
{0x002A13C0},
{0x0F12029C},	// aig_rmp_rst_ptr3
{0x002A13C8},
{0x0F1202E4},	// aig_rmp_rst_ptr4
{0x002A13D0},
{0x0F120396},	// aig_rmp_rst_ptr5
{0x002A13D8},
{0x0F120411},	// aig_rmp_rst_ptr6
{0x002A13E0},
{0x0F120543},	// aig_rmp_rst_ptr7
{0x002A13E8},
{0x0F120040},	// aig_cnt_en_ptr0
{0x002A13F0},
{0x0F1200EF},	// aig_cnt_en_ptr1
{0x002A13F8},
{0x0F12016D},	// aig_cnt_en_ptr2
{0x002A1400},
{0x0F12029C},	// aig_cnt_en_ptr3
{0x002A1408},
{0x0F1202E7},	// aig_cnt_en_ptr4
{0x002A1410},
{0x0F120396},	// aig_cnt_en_ptr5
{0x002A1418},
{0x0F120414},	// aig_cnt_en_ptr6
{0x002A1420},
{0x0F120543},	// aig_cnt_en_ptr7
{0x002A1428},
{0x0F120040},	// aig_cnt_en2_ptr0
{0x002A1430},
{0x0F1200EF},	// aig_cnt_en2_ptr1
{0x002A1438},
{0x0F12016D},	// aig_cnt_en2_ptr2
{0x002A1440},
{0x0F12029C},	// aig_cnt_en2_ptr3
{0x002A1448},
{0x0F1202E7},	// aig_cnt_en2_ptr4
{0x002A1450},
{0x0F120396},	// aig_cnt_en2_ptr5
{0x002A1458},
{0x0F120414},	// aig_cnt_en2_ptr6
{0x002A1460},
{0x0F120543},	// aig_cnt_en2_ptr7
{0x002A1468},
{0x0F12003D},	// aig_cmp_rst_ptr0
{0x002A1470},
{0x0F1200F1},	// aig_cmp_rst_ptr1
{0x002A1478},
{0x0F12016A},	// aig_cmp_rst_ptr2
{0x002A1480},
{0x0F12029E},	// aig_cmp_rst_ptr3
{0x002A1488},
{0x0F1202E4},	// aig_cmp_rst_ptr4
{0x002A1490},
{0x0F120398},	// aig_cmp_rst_ptr5
{0x002A1498},
{0x0F120411},	// aig_cmp_rst_ptr6
{0x002A14A0},
{0x0F120545},	// aig_cmp_rst_ptr7
{0x002A14A8},
{0x0F1200F2},	// aig_conv1_ptr0
{0x002A14B0},
{0x0F1200FF},	// aig_conv1_ptr1
{0x002A14B8},
{0x0F12029F},	// aig_conv1_ptr2
{0x002A14C0},
{0x0F1202AC},	// aig_conv1_ptr3
{0x002A14C8},
{0x0F120399},	// aig_conv1_ptr4
{0x002A14D0},
{0x0F1203A6},	// aig_conv1_ptr5
{0x002A14D8},
{0x0F120000},	// aig_conv1_ptr6
{0x002A14E0},
{0x0F120000},	// aig_conv1_ptr7
{0x002A14E8},
{0x0F1200FA},	// aig_conv2_ptr0
{0x002A14F0},
{0x0F120107},	// aig_conv2_ptr1
{0x002A14F8},
{0x0F1202A7},	// aig_conv2_ptr2
{0x002A1500},
{0x0F1202B4},	// aig_conv2_ptr3
{0x002A1508},
{0x0F1203A1},	// aig_conv2_ptr4
{0x002A1510},
{0x0F1203AE},	// aig_conv2_ptr5
{0x002A1518},
{0x0F120000},	// aig_conv2_ptr6
{0x002A1520},
{0x0F120000},	// aig_conv2_ptr7
{0x002A1528},
{0x0F120001},	// aig_cnt_rst_ptr0
{0x002A1530},
{0x0F12000F},	// aig_cnt_rst_ptr1
{0x002A1538},
{0x0F12021C},	// aig_ramp_slope_sel_ptr0
{0x002A1540},
{0x0F12029E},	// aig_ramp_slope_sel_ptr1
{0x002A1548},
{0x0F1204C3},	// aig_ramp_slope_sel_ptr2
{0x002A1550},
{0x0F120000},	// aig_ramp_slope_sel_ptr3
//============================================================
//
// Analog Setting END
//
//============================================================




//============================================================
// ISP-FE Setting
//============================================================
{0x002A158A},
{0x0F12EAF0},
{0x002A15C6},
{0x0F120020},
{0x0F120060},
{0x002A15BC},
{0x0F120200},

{0x002A1608},
{0x0F120100},
{0x0F120100},
{0x0F120100},
{0x0F120100},

//============================================================
// ISP-FE Setting END
//============================================================
//============================================================
// Frame rate setting
//============================================================
//  How to set
//  1. Exposure value
//          dec2hex((1 / (frame rate you want(ms))) * 100d * 5d)
//  2. Analog Digital gain
//          dec2hex((Analog gain you want) * 256d)
//============================================================
//------- 20100609-tw org exposure
//WRITE #lt_uMaxExp1          5DC00000      //#lt_uMaxExp1  32 30ms   9~10ea    // 15fps
//WRITE #lt_uMaxExp2          6D600000      //#lt_uMaxExp2  67 65ms 18~20ea // 7.5fps
//WRITE #evt1_lt_uMaxExp3     9C400000      //MaxExp3  83 80ms   24~25ea
//WRITE #evt1_lt_uMaxExp4     BB800000      //MaxExp4   125ms   38ea
//
//WRITE #lt_uCapMaxExp1       5DC00000      // 15fps
//WRITE #lt_uCapMaxExp2       6D600000      // 7.5fps
//WRITE #evt1_lt_uCapMaxExp3  9C400000      //CapMaxExp3
//WRITE #evt1_lt_uCapMaxExp4  BB800000      //CapMaxExp4
//
//WRITE #lt_uMaxAnGain1       0150          //
//WRITE #lt_uMaxAnGain2       0280          //lt_uMaxAnGain2
//WRITE #evt1_lt_uMaxAnGain3  02A0          //MaxAnGain3
//WRITE #evt1_lt_uMaxAnGain4  0800          //MaxAnGain4
//
//WRITE #lt_uMaxDigGain       0100
//WRITE #lt_uMaxTotGain       8000          //Max Gain 8

//------- 20100609-tw more exposure range
{0x002A0530},
{0x0F123415},
{0x0F120000},
{0x0F126720},
{0x0F120000},
{0x002A167C},
{0x0F129AB0},
{0x0F120000},
{0x0F1286A0},
{0x0F120003},
//WRITE 70001682           0003

{0x002A0538},
{0x0F123415},
{0x0F120000},
{0x0F126720},
{0x0F120000},
{0x002A1684},
{0x0F12FFFF},
{0x0F120000},
{0x0F1286A0},
{0x0F120003},
//WRITE 7000168A           0003

{0x002A0540},
{0x0F120150},
{0x0F120280},
{0x002A168C},
{0x0F1202A0},
{0x0F120700},

{0x002A0544},
{0x0F120100},
{0x0F128000},	//Max Gain 8

{0x002A051A},
{0x0F120111},
{0x0F1200F0},

//
//WRITE #setot_bUseBayer32Bin 0001 //Enable Bayer Downscaler
{0x002A1694},
{0x0F120001},	//expand forbidde zone
};

static unsigned char cam_long_version_sset_tuning_setup1[]= 
{
//============================================================
//Tuning part
//============================================================
//Calibrations go here
// Start of AS Grid Colibration
{0x0F},{0x12},
{0x01},{0x30},
{0x01},{0x0E},
{0x01},{0x19},
{0x00},{0xF0},
{0x01},{0x00},
{0x00},{0xE2},
{0x00},{0xED},
{0x00},{0xD3},
{0x01},{0x1D},
{0x01},{0x01},
{0x01},{0x05},
{0x00},{0xF2},
{0x00},{0xD5},
{0x00},{0xF0},
{0x00},{0xEF},
{0x00},{0xDA},
{0x00},{0xF8},
{0x00},{0xEE},
{0x00},{0xE8},
{0x00},{0xF0},
{0x00},{0xDC},
{0x00},{0xEC},
{0x00},{0xE3},
{0x00},{0xF4},
{0x00},{0xD0},
{0x00},{0xEC},
{0x00},{0xE3},
{0x00},{0xF4},
{0x00},{0xE8},
{0x00},{0xEF},
{0x00},{0xE5},
{0x00},{0xFC},
{0x00},{0x24},
{0x00},{0x24},
{0x00},{0x24},
{0x00},{0x00},
{0x00},{0x54},
{0x00},{0x21},
{0x00},{0x24},
{0x00},{0x00},
{0x00},{0x4B},
{0x00},{0x21},
{0x00},{0x21},
{0x00},{0x00},
{0x00},{0x20},
{0x00},{0x20},
{0x00},{0x1F},
{0x00},{0x00},
{0x00},{0x30},
{0x00},{0x17},
{0x00},{0x15},
{0x00},{0x00},
{0x00},{0x2A},
{0x00},{0x13},
{0x00},{0x11},
{0x00},{0x00},
{0x00},{0x2A},
{0x00},{0x13},
{0x00},{0x11},
{0x00},{0x00},
{0x00},{0x2B},
{0x00},{0x11},
{0x00},{0x12},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup2[]= 
{
{0x0F},{0x12},
{0x00},{0x01},
{0x03},{0xEE},
{0x03},{0x02},
{0x00},{0x0D},
{0x00},{0x10},
};

static unsigned char cam_long_version_sset_tuning_setup3[]= 
{
{0x0F},{0x12},
{0x01},{0x23},
{0x00},{0xF9},
{0x00},{0xD0},
{0x00},{0xC0},
{0x00},{0xB2},
{0x00},{0xA7},
{0x00},{0xA6},
{0x00},{0xAA},
{0x00},{0xB1},
{0x00},{0xC0},
{0x00},{0xCF},
{0x00},{0xFA},
{0x01},{0x23},
{0x00},{0xDF},
{0x00},{0xC3},
{0x00},{0xAB},
{0x00},{0x97},
{0x00},{0x81},
{0x00},{0x73},
{0x00},{0x6D},
{0x00},{0x76},
{0x00},{0x81},
{0x00},{0x97},
{0x00},{0xAA},
{0x00},{0xC3},
{0x00},{0xDD},
{0x00},{0xB2},
{0x00},{0x9D},
{0x00},{0x83},
{0x00},{0x67},
{0x00},{0x4B},
{0x00},{0x3B},
{0x00},{0x36},
{0x00},{0x40},
{0x00},{0x4A},
{0x00},{0x67},
{0x00},{0x83},
{0x00},{0x9D},
{0x00},{0xB2},
{0x00},{0x9B},
{0x00},{0x85},
{0x00},{0x67},
{0x00},{0x42},
{0x00},{0x28},
{0x00},{0x18},
{0x00},{0x16},
{0x00},{0x1E},
{0x00},{0x28},
{0x00},{0x42},
{0x00},{0x66},
{0x00},{0x84},
{0x00},{0x9B},
{0x00},{0x8F},
{0x00},{0x77},
{0x00},{0x51},
{0x00},{0x2B},
{0x00},{0x12},
{0x00},{0x07},
{0x00},{0x05},
{0x00},{0x0B},
{0x00},{0x1D},
{0x00},{0x42},
{0x00},{0x6E},
{0x00},{0x9C},
{0x00},{0x9F},
{0x00},{0x8E},
{0x00},{0x6F},
{0x00},{0x46},
{0x00},{0x21},
{0x00},{0x0A},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x04},
{0x00},{0x16},
{0x00},{0x39},
{0x00},{0x67},
{0x00},{0x94},
{0x00},{0x9E},
{0x00},{0x90},
{0x00},{0x73},
{0x00},{0x4D},
{0x00},{0x28},
{0x00},{0x10},
{0x00},{0x06},
{0x00},{0x03},
{0x00},{0x09},
{0x00},{0x1A},
{0x00},{0x3F},
{0x00},{0x6C},
{0x00},{0x98},
{0x00},{0xA0},
{0x00},{0x9E},
{0x00},{0x80},
{0x00},{0x5C},
{0x00},{0x3A},
{0x00},{0x20},
{0x00},{0x14},
{0x00},{0x11},
{0x00},{0x19},
{0x00},{0x20},
{0x00},{0x3A},
{0x00},{0x66},
{0x00},{0x8E},
{0x00},{0xAF},
{0x00},{0xBD},
{0x00},{0x98},
{0x00},{0x78},
{0x00},{0x59},
{0x00},{0x41},
{0x00},{0x32},
{0x00},{0x31},
{0x00},{0x38},
{0x00},{0x40},
{0x00},{0x58},
{0x00},{0x84},
{0x00},{0xA9},
{0x00},{0xD1},
{0x00},{0xEE},
{0x00},{0xB7},
{0x00},{0x9C},
{0x00},{0x85},
{0x00},{0x6F},
{0x00},{0x63},
{0x00},{0x61},
{0x00},{0x6C},
{0x00},{0x6E},
{0x00},{0x84},
{0x00},{0xAD},
{0x00},{0xCA},
{0x01},{0x07},
{0x01},{0x31},
{0x00},{0xE8},
{0x00},{0xBC},
{0x00},{0xAC},
{0x00},{0x9A},
{0x00},{0x95},
{0x00},{0x94},
{0x00},{0x9D},
{0x00},{0x99},
{0x00},{0xAB},
{0x00},{0xD0},
{0x01},{0x01},
{0x01},{0x53},
{0x00},{0xE7},
{0x00},{0xBA},
{0x00},{0x9C},
{0x00},{0x8F},
{0x00},{0x88},
{0x00},{0x81},
{0x00},{0x7D},
{0x00},{0x80},
{0x00},{0x81},
{0x00},{0x87},
{0x00},{0x90},
{0x00},{0xAF},
{0x00},{0xD7},
{0x00},{0xAF},
{0x00},{0x8F},
{0x00},{0x80},
{0x00},{0x71},
{0x00},{0x63},
{0x00},{0x5A},
{0x00},{0x56},
{0x00},{0x5B},
{0x00},{0x60},
{0x00},{0x6A},
{0x00},{0x73},
{0x00},{0x84},
{0x00},{0x9D},
{0x00},{0x8A},
{0x00},{0x72},
{0x00},{0x63},
{0x00},{0x4E},
{0x00},{0x3B},
{0x00},{0x30},
{0x00},{0x2D},
{0x00},{0x31},
{0x00},{0x38},
{0x00},{0x49},
{0x00},{0x59},
{0x00},{0x6A},
{0x00},{0x7F},
{0x00},{0x73},
{0x00},{0x61},
{0x00},{0x4D},
{0x00},{0x32},
{0x00},{0x1E},
{0x00},{0x14},
{0x00},{0x12},
{0x00},{0x17},
{0x00},{0x1E},
{0x00},{0x2F},
{0x00},{0x47},
{0x00},{0x58},
{0x00},{0x6C},
{0x00},{0x68},
{0x00},{0x55},
{0x00},{0x3C},
{0x00},{0x20},
{0x00},{0x0D},
{0x00},{0x05},
{0x00},{0x03},
{0x00},{0x08},
{0x00},{0x16},
{0x00},{0x2F},
{0x00},{0x4D},
{0x00},{0x69},
{0x00},{0x6F},
{0x00},{0x66},
{0x00},{0x4F},
{0x00},{0x34},
{0x00},{0x19},
{0x00},{0x07},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x03},
{0x00},{0x11},
{0x00},{0x2A},
{0x00},{0x49},
{0x00},{0x66},
{0x00},{0x6F},
{0x00},{0x66},
{0x00},{0x51},
{0x00},{0x38},
{0x00},{0x1D},
{0x00},{0x0B},
{0x00},{0x04},
{0x00},{0x03},
{0x00},{0x09},
{0x00},{0x17},
{0x00},{0x30},
{0x00},{0x4F},
{0x00},{0x69},
{0x00},{0x72},
{0x00},{0x70},
{0x00},{0x59},
{0x00},{0x44},
{0x00},{0x2B},
{0x00},{0x19},
{0x00},{0x11},
{0x00},{0x10},
{0x00},{0x16},
{0x00},{0x1C},
{0x00},{0x2E},
{0x00},{0x4C},
{0x00},{0x64},
{0x00},{0x80},
{0x00},{0x88},
{0x00},{0x6A},
{0x00},{0x57},
{0x00},{0x43},
{0x00},{0x32},
{0x00},{0x29},
{0x00},{0x29},
{0x00},{0x30},
{0x00},{0x35},
{0x00},{0x46},
{0x00},{0x63},
{0x00},{0x78},
{0x00},{0x9D},
{0x00},{0xB3},
{0x00},{0x82},
{0x00},{0x71},
{0x00},{0x64},
{0x00},{0x56},
{0x00},{0x4F},
{0x00},{0x50},
{0x00},{0x59},
{0x00},{0x5A},
{0x00},{0x68},
{0x00},{0x82},
{0x00},{0x93},
{0x00},{0xCC},
{0x00},{0xEF},
{0x00},{0xAD},
{0x00},{0x8B},
{0x00},{0x80},
{0x00},{0x77},
{0x00},{0x75},
{0x00},{0x77},
{0x00},{0x80},
{0x00},{0x7B},
{0x00},{0x86},
{0x00},{0x9E},
{0x00},{0xBF},
{0x01},{0x13},
{0x00},{0xF6},
{0x00},{0xC6},
{0x00},{0xA5},
{0x00},{0x98},
{0x00},{0x92},
{0x00},{0x8D},
{0x00},{0x8F},
{0x00},{0x98},
{0x00},{0xA1},
{0x00},{0xAB},
{0x00},{0xB7},
{0x00},{0xDC},
{0x01},{0x04},
{0x00},{0xC0},
{0x00},{0x9D},
{0x00},{0x8C},
{0x00},{0x7C},
{0x00},{0x6F},
{0x00},{0x67},
{0x00},{0x66},
{0x00},{0x71},
{0x00},{0x7C},
{0x00},{0x8B},
{0x00},{0x98},
{0x00},{0xAB},
{0x00},{0xC4},
{0x00},{0x9B},
{0x00},{0x82},
{0x00},{0x71},
{0x00},{0x5B},
{0x00},{0x46},
{0x00},{0x3D},
{0x00},{0x3B},
{0x00},{0x45},
{0x00},{0x4F},
{0x00},{0x64},
{0x00},{0x7A},
{0x00},{0x8C},
{0x00},{0xA1},
{0x00},{0x88},
{0x00},{0x73},
{0x00},{0x5D},
{0x00},{0x40},
{0x00},{0x2A},
{0x00},{0x1F},
{0x00},{0x1E},
{0x00},{0x28},
{0x00},{0x2F},
{0x00},{0x43},
{0x00},{0x5F},
{0x00},{0x74},
{0x00},{0x89},
{0x00},{0x81},
{0x00},{0x6A},
{0x00},{0x4E},
{0x00},{0x2F},
{0x00},{0x19},
{0x00},{0x0F},
{0x00},{0x0E},
{0x00},{0x15},
{0x00},{0x27},
{0x00},{0x44},
{0x00},{0x65},
{0x00},{0x85},
{0x00},{0x88},
{0x00},{0x83},
{0x00},{0x68},
{0x00},{0x47},
{0x00},{0x28},
{0x00},{0x13},
{0x00},{0x0A},
{0x00},{0x08},
{0x00},{0x0D},
{0x00},{0x1E},
{0x00},{0x39},
{0x00},{0x5A},
{0x00},{0x79},
{0x00},{0x83},
{0x00},{0x87},
{0x00},{0x6C},
{0x00},{0x4E},
{0x00},{0x2E},
{0x00},{0x18},
{0x00},{0x0E},
{0x00},{0x0A},
{0x00},{0x10},
{0x00},{0x1F},
{0x00},{0x3A},
{0x00},{0x5A},
{0x00},{0x75},
{0x00},{0x81},
{0x00},{0x96},
{0x00},{0x79},
{0x00},{0x5C},
{0x00},{0x3E},
{0x00},{0x26},
{0x00},{0x1A},
{0x00},{0x16},
{0x00},{0x1B},
{0x00},{0x1E},
{0x00},{0x30},
{0x00},{0x4F},
{0x00},{0x69},
{0x00},{0x8A},
{0x00},{0xB4},
{0x00},{0x8E},
{0x00},{0x72},
{0x00},{0x59},
{0x00},{0x42},
{0x00},{0x33},
{0x00},{0x30},
{0x00},{0x33},
{0x00},{0x36},
{0x00},{0x45},
{0x00},{0x62},
{0x00},{0x79},
{0x00},{0xA2},
{0x00},{0xE5},
{0x00},{0xA9},
{0x00},{0x91},
{0x00},{0x7E},
{0x00},{0x68},
{0x00},{0x5B},
{0x00},{0x57},
{0x00},{0x5B},
{0x00},{0x59},
{0x00},{0x63},
{0x00},{0x7E},
{0x00},{0x91},
{0x00},{0xCF},
{0x01},{0x25},
{0x00},{0xDA},
{0x00},{0xB0},
{0x00},{0x9F},
{0x00},{0x8F},
{0x00},{0x86},
{0x00},{0x80},
{0x00},{0x82},
{0x00},{0x7A},
{0x00},{0x82},
{0x00},{0x99},
{0x00},{0xBC},
{0x01},{0x17},
{0x00},{0x9E},
{0x00},{0x84},
{0x00},{0x6A},
{0x00},{0x66},
{0x00},{0x67},
{0x00},{0x68},
{0x00},{0x6B},
{0x00},{0x70},
{0x00},{0x74},
{0x00},{0x79},
{0x00},{0x80},
{0x00},{0x9D},
{0x00},{0xB7},
{0x00},{0x70},
{0x00},{0x5D},
{0x00},{0x55},
{0x00},{0x50},
{0x00},{0x4C},
{0x00},{0x4B},
{0x00},{0x4C},
{0x00},{0x52},
{0x00},{0x59},
{0x00},{0x61},
{0x00},{0x67},
{0x00},{0x76},
{0x00},{0x80},
{0x00},{0x4C},
{0x00},{0x43},
{0x00},{0x3E},
{0x00},{0x35},
{0x00},{0x2C},
{0x00},{0x29},
{0x00},{0x2A},
{0x00},{0x30},
{0x00},{0x35},
{0x00},{0x43},
{0x00},{0x4F},
{0x00},{0x5B},
{0x00},{0x62},
{0x00},{0x3A},
{0x00},{0x35},
{0x00},{0x2C},
{0x00},{0x1E},
{0x00},{0x15},
{0x00},{0x11},
{0x00},{0x13},
{0x00},{0x17},
{0x00},{0x1C},
{0x00},{0x29},
{0x00},{0x3B},
{0x00},{0x46},
{0x00},{0x4D},
{0x00},{0x31},
{0x00},{0x2B},
{0x00},{0x1E},
{0x00},{0x0F},
{0x00},{0x05},
{0x00},{0x05},
{0x00},{0x05},
{0x00},{0x08},
{0x00},{0x11},
{0x00},{0x24},
{0x00},{0x39},
{0x00},{0x4C},
{0x00},{0x47},
{0x00},{0x2E},
{0x00},{0x26},
{0x00},{0x17},
{0x00},{0x08},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x01},
{0x00},{0x09},
{0x00},{0x19},
{0x00},{0x2D},
{0x00},{0x3F},
{0x00},{0x3F},
{0x00},{0x30},
{0x00},{0x28},
{0x00},{0x1A},
{0x00},{0x0B},
{0x00},{0x02},
{0x00},{0x01},
{0x00},{0x01},
{0x00},{0x02},
{0x00},{0x09},
{0x00},{0x18},
{0x00},{0x2A},
{0x00},{0x3A},
{0x00},{0x3C},
{0x00},{0x3C},
{0x00},{0x30},
{0x00},{0x24},
{0x00},{0x16},
{0x00},{0x0D},
{0x00},{0x0A},
{0x00},{0x09},
{0x00},{0x0A},
{0x00},{0x0C},
{0x00},{0x16},
{0x00},{0x25},
{0x00},{0x32},
{0x00},{0x41},
{0x00},{0x56},
{0x00},{0x40},
{0x00},{0x35},
{0x00},{0x2A},
{0x00},{0x22},
{0x00},{0x1D},
{0x00},{0x1D},
{0x00},{0x1D},
{0x00},{0x1D},
{0x00},{0x24},
{0x00},{0x32},
{0x00},{0x3E},
{0x00},{0x54},
{0x00},{0x80},
{0x00},{0x5A},
{0x00},{0x4C},
{0x00},{0x46},
{0x00},{0x40},
{0x00},{0x3D},
{0x00},{0x3D},
{0x00},{0x3D},
{0x00},{0x38},
{0x00},{0x3B},
{0x00},{0x47},
{0x00},{0x53},
{0x00},{0x7A},
{0x00},{0xBC},
{0x00},{0x86},
{0x00},{0x6B},
{0x00},{0x64},
{0x00},{0x60},
{0x00},{0x5E},
{0x00},{0x5E},
{0x00},{0x5D},
{0x00},{0x53},
{0x00},{0x53},
{0x00},{0x60},
{0x00},{0x7A},
{0x00},{0xBD},
};

static unsigned char cam_long_version_sset_tuning_setup4[]= 
{
{0x0F},{0x12},
{0x00},{0x01},
};

static unsigned char cam_long_version_sset_tuning_setup5[]= 
{
{0x0F},{0x12},
{0x02},{0x9E},
{0x03},{0x4C},
};

static unsigned char cam_long_version_sset_tuning_setup6[]= 
{
{0x0F},{0x12},
{0x00},{0xC4},
{0x00},{0xCE},
{0x00},{0xED},
{0x01},{0x2C},
{0x01},{0x68},
{0x01},{0x92},
{0x01},{0xA2},

{0x00},{0xF8},
{0x00},{0xEE},
{0x00},{0xE8},
{0x00},{0xF0},
};
// END of AS Grid Colibration

// Strat of AWB Gray zone and Grid Colibration
static unsigned char cam_long_version_sset_tuning_setup7[]= 
{
{0x0F},{0x12},
{0x00},{0x01},
};

static unsigned char cam_long_version_sset_tuning_setup8[]= 
{
{0x0F},{0x12},
{0x00},{0xB7},
{0x00},{0xB3},
};

static unsigned char cam_long_version_sset_tuning_setup9[]= 
{
{0x0F},{0x12},
{0x0F},{0x73},
{0x0F},{0xC5},
{0x10},{0x27},
{0x10},{0xB1},
{0x10},{0xF7},
{0x11},{0x32},
{0x00},{0xB0},
{0x00},{0xB9},
};

static unsigned char cam_long_version_sset_tuning_setup10[]= 
{
{0x0F},{0x12},
{0x03},{0x10},
{0x03},{0x37},
{0x03},{0x61},
};

static unsigned char cam_long_version_sset_tuning_setup11[]= 
{
{0x0F},{0x12},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0xFF},{0xE1},
{0x00},{0x0F},
{0x00},{0x00},
{0xFF},{0xDD},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x0F},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0xFF},{0xF7},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0xFF},{0xDD},
{0x00},{0x07},
{0xFF},{0xED},
{0x00},{0x00},
{0x00},{0x11},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0xFF},{0xED},
};

static unsigned char cam_long_version_sset_tuning_setup12[]= 
{
{0x0F},{0x12},
{0x03},{0x88},
{0x03},{0x9F},
{0x03},{0x3C},
{0x03},{0x9F},
{0x02},{0xFE},
{0x03},{0x7B},
{0x02},{0xC6},
{0x03},{0x4E},
{0x02},{0x96},
{0x03},{0x23},
{0x02},{0x71},
{0x02},{0xF8},
{0x02},{0x55},
{0x02},{0xCE},
{0x02},{0x3E},
{0x02},{0xA9},
{0x02},{0x2C},
{0x02},{0x8B},
{0x02},{0x1B},
{0x02},{0x6D},
{0x02},{0x0B},
{0x02},{0x56},
{0x01},{0xFA},
{0x02},{0x41},
{0x01},{0xF1},
{0x02},{0x2D},
{0x01},{0xF1},
{0x02},{0x1B},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x05},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup13[]= 
{
{0x0F},{0x12},
{0x01},{0x24},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup14[]= 
{
{0x0F},{0x12},
{0x03},{0x78},
{0x03},{0xB7},
{0x02},{0xEA},
{0x03},{0xB6},
{0x02},{0x8A},
{0x03},{0x6E},
{0x02},{0x4E},
{0x03},{0x1A},
{0x02},{0x18},
{0x02},{0xCF},
{0x01},{0xF4},
{0x02},{0x89},
{0x01},{0xD8},
{0x02},{0x5B},
{0x01},{0xD8},
{0x02},{0x32},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x06},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup15[]= 
{
{0x0F},{0x12},
{0x01},{0x12},
{0x00},{0x00},
};

//------- fm "20100318_.tset", TW
static unsigned char cam_long_version_sset_tuning_setup16[]= 
{
{0x0F},{0x12},
{0x02},{0x60},
{0x02},{0x7F},
{0x02},{0x49},
{0x02},{0x7E},
{0x02},{0x32},
{0x02},{0x67},
{0x02},{0x32},
{0x02},{0x4F},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x05},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup17[]= 
{
{0x0F},{0x12},
{0x02},{0x2A},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup18[]= 
{
{0x0F},{0x12},
{0x03},{0x3C},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup19[]= 
{
{0x0F},{0x12},
{0x01},{0x5C},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup20[]= 
{
{0x0F},{0x12},
{0x46},{0x10},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup21[]= 
{
{0x0F},{0x12},
{0x01},{0x36},
{0x01},{0x17},
};

static unsigned char cam_long_version_sset_tuning_setup22[]= 
{
{0x0F},{0x12},
{0x04},{0x50},
};

static unsigned char cam_long_version_sset_tuning_setup23[]= 
{
{0x0F},{0x12},
{0x05},{0x18},
};

static unsigned char cam_long_version_sset_tuning_setup24[]= 
{
{0x0F},{0x12},
{0x00},{0x00},
};
// End of AWB Gray zone and Grid Colibration

//Start_0f Gamma param
static unsigned char cam_long_version_sset_tuning_setup25[]= 
{
{0x0F},{0x12},
{0x00},{0x30},
};

static unsigned char cam_long_version_sset_tuning_setup26[]= 
{
{0x0F},{0x12},
{0x00},{0x0F},
};

static unsigned char cam_long_version_sset_tuning_setup27[]= 
{
{0x0F},{0x12},
{0x00},{0x00},
{0x00},{0x04},
{0x00},{0x10},
{0x00},{0x2A},
{0x00},{0x62},
{0x00},{0xD5},
{0x01},{0x38},
{0x01},{0x61},
{0x01},{0x86},
{0x01},{0xBC},
{0x01},{0xE8},
{0x02},{0x0F},
{0x02},{0x32},
{0x02},{0x73},
{0x02},{0xAF},
{0x03},{0x09},
{0x03},{0x55},
{0x03},{0x94},
{0x03},{0xCE},
{0x03},{0xFF},
{0x00},{0x00},
{0x00},{0x04},
{0x00},{0x10},
{0x00},{0x2A},
{0x00},{0x62},
{0x00},{0xD5},
{0x01},{0x38},
{0x01},{0x61},
{0x01},{0x86},
{0x01},{0xBC},
{0x01},{0xE8},
{0x02},{0x0F},
{0x02},{0x32},
{0x02},{0x73},
{0x02},{0xAF},
{0x03},{0x09},
{0x03},{0x55},
{0x03},{0x94},
{0x03},{0xCE},
{0x03},{0xFF},
{0x00},{0x00},
{0x00},{0x04},
{0x00},{0x10},
{0x00},{0x2A},
{0x00},{0x62},
{0x00},{0xD5},
{0x01},{0x38},
{0x01},{0x61},
{0x01},{0x86},
{0x01},{0xBC},
{0x01},{0xE8},
{0x02},{0x0F},
{0x02},{0x32},
{0x02},{0x73},
{0x02},{0xAF},
{0x03},{0x09},
{0x03},{0x55},
{0x03},{0x94},
{0x03},{0xCE},
{0x03},{0xFF},

{0x00},{0x00},
{0x00},{0x04},
{0x00},{0x10},
{0x00},{0x2A},
{0x00},{0x62},
{0x00},{0xD5},
{0x01},{0x38},
{0x01},{0x61},
{0x01},{0x86},
{0x01},{0xBC},
{0x01},{0xE8},
{0x02},{0x0F},
{0x02},{0x32},
{0x02},{0x73},
{0x02},{0xAF},
{0x03},{0x09},
{0x03},{0x55},
{0x03},{0x94},
{0x03},{0xCE},
{0x03},{0xFF},
{0x00},{0x00},
{0x00},{0x04},
{0x00},{0x10},
{0x00},{0x2A},
{0x00},{0x62},
{0x00},{0xD5},
{0x01},{0x38},
{0x01},{0x61},
{0x01},{0x86},
{0x01},{0xBC},
{0x01},{0xE8},
{0x02},{0x0F},
{0x02},{0x32},
{0x02},{0x73},
{0x02},{0xAF},
{0x03},{0x09},
{0x03},{0x55},
{0x03},{0x94},
{0x03},{0xCE},
{0x03},{0xFF},
{0x00},{0x00},
{0x00},{0x04},
{0x00},{0x10},
{0x00},{0x2A},
{0x00},{0x62},
{0x00},{0xD5},
{0x01},{0x38},
{0x01},{0x61},
{0x01},{0x86},
{0x01},{0xBC},
{0x01},{0xE8},
{0x02},{0x0F},
{0x02},{0x32},
{0x02},{0x73},
{0x02},{0xAF},
{0x03},{0x09},
{0x03},{0x55},
{0x03},{0x94},
{0x03},{0xCE},
{0x03},{0xFF},
//end of Gamma param

};

static unsigned char cam_long_version_sset_tuning_setup28[]= 
{
{0x0F},{0x12},
{0x00},{0xB5},
{0x00},{0xE9},
{0x01},{0x15},
{0x01},{0x35},
{0x01},{0x68},
{0x01},{0x90},
{0x01},{0x9C},

{0x01},{0x68},
{0x01},{0x0E},
{0x01},{0x0C},
{0x01},{0x0C},
{0x01},{0x00},
{0x00},{0xF9},
{0x00},{0xEB},
};

static unsigned char cam_long_version_sset_tuning_setup29[]= 
{
{0x0F},{0x12},
{0x00},{0xD6},
{0x00},{0xE9},
{0x01},{0x15},
{0x01},{0x35},
{0x01},{0x68},
{0x01},{0x90},
};

//start of CCM parameters
static unsigned char cam_long_version_sset_tuning_setup30[]= 
{
{0x0F},{0x12},
{0x02},{0x27},
{0xFF},{0x79},
{0xFF},{0xAF},
{0x00},{0x2E},
{0x01},{0xC3},
{0xFE},{0x2A},
{0xFE},{0xC3},
{0xFF},{0xD2},
{0x03},{0x43},
{0x02},{0x75},
{0x01},{0x62},
{0xFD},{0x8C},
{0x01},{0x9C},
{0xFF},{0x3A},
{0x02},{0xA2},
{0xFD},{0xF0},
{0x02},{0xD8},
{0x01},{0x5F},
{0x01},{0xFB},	//A
{0xFF},{0xAA},
{0xFF},{0xAA},
{0xFF},{0x44},
{0x03},{0x17},
{0xFD},{0xC0},
{0xFF},{0x3B},
{0xFF},{0xB7},
{0x02},{0xE7},
{0x01},{0xA8},
{0x01},{0x96},
{0xFE},{0x25},
{0x01},{0xF8},
{0xFE},{0xD3},
{0x02},{0xAE},
{0xFE},{0x85},
{0x03},{0x34},
{0x00},{0x6F},
{0x01},{0x5B},	//
{0xFF},{0x99},
{0xFF},{0xAC},
{0xFF},{0x79},
{0x01},{0x35},
{0xFE},{0xFD},
{0x00},{0x4C},
{0x00},{0x08},
{0x03},{0x38},
{0x00},{0x9E},
{0x00},{0x61},
{0xFE},{0xCA},
{0x02},{0x61},
{0xFF},{0x36},
{0x02},{0xA5},
{0xFF},{0x13},
{0x02},{0x01},
{0x01},{0x3F},
{0x02},{0x2E},	//CWF
{0xFF},{0x4C},
{0xFF},{0xD5},
{0xFE},{0x6F},
{0x02},{0x7D},
{0xFF},{0x2F},
{0xFF},{0x8C},
{0xFF},{0xAC},
{0x02},{0xA2},
{0x01},{0xAA},
{0x01},{0x04},
{0xFE},{0xB5},
{0x02},{0x76},
{0xFF},{0x03},
{0x02},{0x00},
{0xFF},{0x0D},
{0x01},{0xC1},
{0x01},{0x59},
{0x01},{0x8E},
{0xFF},{0xB8},
{0xFF},{0xEF},
{0xFF},{0x57},
{0x01},{0x8D},
{0xFF},{0x85},
{0xFF},{0xFE},
{0x00},{0x11},
{0x02},{0x2B},
{0x00},{0xE4},
{0x00},{0xBA},
{0xFF},{0x25},
{0x01},{0x69},
{0xFF},{0xC1},
{0x01},{0x5D},
{0xFE},{0x13},
{0x01},{0x4B},
{0x01},{0x75},
{0x01},{0xA8},	//D50
{0xFF},{0xBC},
{0xFF},{0xEB},
{0xFE},{0xA9},
{0x02},{0x4E},
{0xFF},{0x24},
{0x00},{0x08},
{0xFF},{0xB7},
{0x02},{0x19},
{0x01},{0x10},
{0x01},{0x10},
{0xFF},{0x43},
{0x02},{0x41},
{0xFF},{0x84},
{0x01},{0xB4},
{0xFE},{0xDD},
{0x02},{0x4B},
{0x00},{0xFF},
};

static unsigned char cam_long_version_sset_tuning_setup31[]= 
{
{0x0F},{0x12},
{0x01},{0xD5},
{0xFF},{0xBB},
{0xFF},{0xF1},
{0xFF},{0x40},
{0x01},{0x8D},
{0xFF},{0x3C},
{0x00},{0x1E},
{0xFF},{0xB8},
{0x03},{0x32},
{0x01},{0x27},
{0x01},{0x4D},
{0xFF},{0x15},
{0x01},{0xF3},
{0xFD},{0xB4},
{0x01},{0xC8},
{0xFE},{0x73},
{0x01},{0x94},
{0x01},{0x2F},
};
//end of CCM parameters

//Start of Out/Indoors detector parameters
static unsigned char cam_long_version_sset_tuning_setup32[]= 
{
{0x0F},{0x12},
{0x00},{0x05},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup33[]= 
{
{0x0F},{0x12},
{0xFF},{0xE5},
{0x00},{0x3C},
{0xFF},{0x5B},
{0x00},{0x87},
{0xFF},{0x43},
{0x00},{0x9C},
{0xFF},{0x35},
{0x00},{0xA1},
{0xFF},{0x26},
{0x00},{0xA1},
{0x12},{0xA5},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup34[]= 
{
{0x0F},{0x12},
{0x4D},{0x98},
{0x00},{0x00},
};

static unsigned char cam_long_version_sset_tuning_setup35[]= 
{
{0x0F},{0x12},
{0x03},{0xE8},
{0x00},{0x00},
};
//end of Out/Indoors detector parameters

//Start of AFIT params
static unsigned char cam_long_version_sset_tuning_setup36[]= 
{
{0x0F},{0x12},
{0x00},{0x49},
{0x00},{0x5F},
{0x00},{0xCB},
{0x01},{0xE0},
{0x02},{0x20},
};

static unsigned char cam_long_version_sset_tuning_setup37[]= 
{
{0x0F},{0x12},
{0x00},{0x14},
{0x00},{0x00},
{0x00},{0x14},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xC1},
{0x03},{0xFF},
{0x00},{0x9C},
{0x01},{0x7C},
{0x03},{0xFF},
{0x00},{0x0C},
{0x00},{0x10},
{0x01},{0x2C},
{0x03},{0xE8},
{0x00},{0x46},
{0x00},{0x5A},
{0x00},{0x70},
{0x00},{0x28},
{0x00},{0x28},
{0x01},{0xAA},
{0x00},{0x3C},
{0x00},{0x3C},
{0x00},{0x05},
{0x00},{0x05},
{0x00},{0x3C},
{0x00},{0x14},
{0x00},{0x3C},
{0x00},{0x1E},
{0x00},{0x3C},
{0x00},{0x1E},
{0x0A},{0x24},
{0x17},{0x01},
{0x02},{0x29},
{0x14},{0x03},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xFF},
{0x0A},{0x3B},
{0x14},{0x14},
{0x03},{0x01},
{0xFF},{0x07},
{0x08},{0x1E},
{0x0A},{0x1E},
{0x0F},{0x0F},
{0x0A},{0x00},
{0x00},{0x28},
{0x00},{0x1E},
{0x00},{0x02},
{0x00},{0xFF},
{0x11},{0x02},
{0x00},{0x1B},
{0x09},{0x00},
{0x06},{0x00},
{0x05},{0x04},
{0x03},{0x06},
{0x74},{0x03},
{0x00},{0x78},
{0x09},{0x3C},
{0x00},{0x80},
{0x14},{0x14},
{0x01},{0x01},
{0x46},{0x01},
{0x6E},{0x44},
{0x46},{0x64},
{0x0A},{0x0A},
{0x00},{0x03},
{0x1E},{0x00},
{0x07},{0x14},
{0x14},{0xFF},
{0x14},{0x04},
{0x0F},{0x14},
{0x40},{0x0F},
{0x02},{0x04},
{0x14},{0x03},
{0x01},{0x14},
{0x01},{0x01},
{0x44},{0x46},
{0x64},{0x6E},
{0x00},{0x28},
{0x03},{0x0A},
{0x00},{0x00},
{0x14},{0x1E},
{0xFF},{0x07},
{0x04},{0x32},
{0x00},{0x00},
{0x0F},{0x0F},
{0x04},{0x40},
{0x03},{0x02},
{0x14},{0x14},
{0x01},{0x01},
{0x46},{0x01},
{0x6E},{0x44},
{0x28},{0x64},
{0x0A},{0x00},
{0x00},{0x03},
{0x1E},{0x00},
{0x07},{0x14},
{0x32},{0xFF},
{0x00},{0x04},
{0x0F},{0x00},
{0x40},{0x0F},
{0x02},{0x04},
{0x00},{0x03},
{0x00},{0x0C},
{0x00},{0x00},
{0x00},{0x14},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xC1},
{0x03},{0xFF},
{0x00},{0x9C},
{0x01},{0x7C},
{0x03},{0xFF},
{0x00},{0x0C},
{0x00},{0x10},
{0x01},{0x2C},
{0x03},{0xE8},
{0x00},{0x46},
{0x00},{0x5A},
{0x00},{0x70},
{0x00},{0x00},
{0x00},{0x00},
{0x01},{0xAA},
{0x00},{0x32},
{0x00},{0x32},
{0x00},{0x05},
{0x00},{0x05},
{0x00},{0x64},
{0x00},{0x14},
{0x00},{0x3C},
{0x00},{0x1E},
{0x00},{0x3C},
{0x00},{0x1E},
{0x0A},{0x24},
{0x17},{0x01},
{0x02},{0x29},
{0x14},{0x03},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xFF},
{0x02},{0x3B},
{0x14},{0x14},
{0x03},{0x01},
{0xFF},{0x07},
{0x08},{0x1E},
{0x0A},{0x1E},
{0x0F},{0x0F},
{0x0A},{0x00},
{0x00},{0x28},
{0x00},{0x05},
{0x00},{0x02},
{0x00},{0xFF},
{0x11},{0x02},
{0x00},{0x1B},
{0x09},{0x00},
{0x06},{0x00},
{0x05},{0x04},
{0x03},{0x06},
{0x94},{0x03},
{0x00},{0x6E},
{0x00},{0x46},
{0x00},{0x80},
{0x23},{0x23},
{0x01},{0x01},
{0x3C},{0x01},
{0x55},{0x3A},
{0x46},{0x49},
{0x0A},{0x0A},
{0x00},{0x03},
{0x1E},{0x02},
{0x07},{0x64},
{0x14},{0xFF},
{0x14},{0x04},
{0x0F},{0x14},
{0x40},{0x0F},
{0x02},{0x04},
{0x1E},{0x03},
{0x01},{0x1E},
{0x01},{0x01},
{0x3A},{0x3C},
{0x58},{0x5A},
{0x00},{0x28},
{0x03},{0x0A},
{0x00},{0x00},
{0x14},{0x1E},
{0xFF},{0x07},
{0x04},{0x32},
{0x00},{0x00},
{0x0F},{0x0F},
{0x04},{0x40},
{0x03},{0x02},
{0x1E},{0x1E},
{0x01},{0x01},
{0x3C},{0x01},
{0x5A},{0x3A},
{0x28},{0x58},
{0x0A},{0x00},
{0x00},{0x03},
{0x1E},{0x00},
{0x07},{0x14},
{0x32},{0xFF},
{0x00},{0x04},
{0x0F},{0x00},
{0x40},{0x0F},
{0x02},{0x04},
{0x00},{0x03},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xC1},
{0x03},{0xFF},
{0x00},{0x9C},
{0x01},{0x7C},
{0x03},{0xFF},
{0x00},{0x0C},
{0x00},{0x10},
{0x01},{0x2C},
{0x03},{0xE8},
{0x00},{0x46},
{0x00},{0x5A},
{0x00},{0x70},
{0x00},{0x00},
{0x00},{0x00},
{0x01},{0xAA},
{0x00},{0x32},
{0x00},{0x32},
{0x00},{0x05},
{0x00},{0x05},
{0x00},{0xC8},
{0x00},{0x14},
{0x00},{0x3C},
{0x00},{0x1E},
{0x00},{0x3C},
{0x00},{0x1E},
{0x0A},{0x24},
{0x17},{0x01},
{0x02},{0x29},
{0x14},{0x03},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xFF},
{0x02},{0x3B},
{0x14},{0x14},
{0x03},{0x01},
{0xFF},{0x07},
{0x08},{0x1E},
{0x0A},{0x1E},
{0x0F},{0x0F},
{0x0A},{0x05},
{0x00},{0x28},
{0x00},{0x05},
{0x00},{0x01},
{0x00},{0xFF},
{0x11},{0x02},
{0x00},{0x1B},
{0x09},{0x00},
{0x06},{0x00},
{0x05},{0x04},
{0x03},{0x06},
{0x94},{0x02},
{0x00},{0x80},
{0x00},{0x80},
{0x00},{0x80},
{0x3C},{0x3C},
{0x01},{0x01},
{0x1E},{0x01},
{0x28},{0x2D},
{0x46},{0x32},
{0x0A},{0x0A},
{0x23},{0x03},
{0x14},{0x02},
{0x08},{0xC8},
{0x14},{0x64},
{0xB4},{0x02},
{0x28},{0x14},
{0x40},{0x0A},
{0x02},{0x04},
{0x28},{0x03},
{0x01},{0x28},
{0x01},{0x01},
{0x22},{0x24},
{0x32},{0x36},
{0x00},{0x28},
{0x03},{0x0A},
{0x04},{0x10},
{0x14},{0x1E},
{0xFF},{0x07},
{0x04},{0x32},
{0x40},{0x50},
{0x0F},{0x0F},
{0x04},{0x40},
{0x03},{0x02},
{0x28},{0x28},
{0x01},{0x01},
{0x24},{0x01},
{0x36},{0x22},
{0x28},{0x32},
{0x0A},{0x00},
{0x10},{0x03},
{0x1E},{0x04},
{0x07},{0x14},
{0x32},{0xFF},
{0x50},{0x04},
{0x0F},{0x40},
{0x40},{0x0F},
{0x02},{0x04},
{0x00},{0x03},
{0xFF},{0xFA},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xC1},
{0x03},{0xFF},
{0x00},{0x9C},
{0x01},{0x7C},
{0x03},{0xFF},
{0x00},{0x0C},
{0x00},{0x10},
{0x00},{0xC8},
{0x03},{0xE8},
{0x00},{0x46},
{0x00},{0x50},
{0x00},{0x70},
{0x00},{0x00},
{0x00},{0x00},
{0x01},{0xAA},
{0x00},{0x32},
{0x00},{0x32},
{0x00},{0x05},
{0x00},{0x05},
{0x00},{0xC8},
{0x00},{0x14},
{0x00},{0x2D},
{0x00},{0x19},
{0x00},{0x2D},
{0x00},{0x19},
{0x0A},{0x24},
{0x17},{0x01},
{0x02},{0x29},
{0x14},{0x03},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xFF},
{0x02},{0x3B},
{0x14},{0x14},
{0x03},{0x01},
{0xFF},{0x07},
{0x08},{0x1E},
{0x0A},{0x1E},
{0x0F},{0x0F},
{0x0A},{0x05},
{0x00},{0x28},
{0x00},{0x05},
{0x00},{0x01},
{0x00},{0xFF},
{0x10},{0x02},
{0x00},{0x1E},
{0x09},{0x00},
{0x06},{0x00},
{0x05},{0x04},
{0x03},{0x07},
{0x94},{0x02},
{0x00},{0x80},
{0x00},{0x80},
{0x00},{0x80},
{0x46},{0x46},
{0x01},{0x01},
{0x1E},{0x01},
{0x28},{0x28},
{0x32},{0x28},
{0x0A},{0x0A},
{0x23},{0x03},
{0x14},{0x02},
{0x08},{0xB4},
{0x14},{0x64},
{0xB4},{0x02},
{0x28},{0x14},
{0x40},{0x0A},
{0x02},{0x04},
{0x3C},{0x03},
{0x01},{0x3C},
{0x01},{0x01},
{0x1C},{0x1E},
{0x1E},{0x22},
{0x00},{0x28},
{0x03},{0x0A},
{0x02},{0x14},
{0x0E},{0x14},
{0xFF},{0x06},
{0x04},{0x32},
{0x40},{0x52},
{0x15},{0x0C},
{0x04},{0x40},
{0x03},{0x02},
{0x3C},{0x3C},
{0x01},{0x01},
{0x1E},{0x01},
{0x22},{0x1C},
{0x28},{0x1E},
{0x0A},{0x00},
{0x14},{0x03},
{0x14},{0x02},
{0x06},{0x0E},
{0x32},{0xFF},
{0x52},{0x04},
{0x0C},{0x40},
{0x40},{0x15},
{0x02},{0x04},
{0x00},{0x03},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xC1},
{0x03},{0xFF},
{0x00},{0x9C},
{0x01},{0x7C},
{0x03},{0xFF},
{0x00},{0x0C},
{0x00},{0x10},
{0x00},{0x32},
{0x02},{0x8A},
{0x00},{0x32},
{0x01},{0xF4},
{0x00},{0x70},
{0x00},{0x00},
{0x00},{0x00},
{0x01},{0xAA},
{0x00},{0x3C},
{0x00},{0x50},
{0x00},{0x00},
{0x00},{0x00},
{0x00},{0xB4},
{0x00},{0x14},
{0x00},{0x46},
{0x00},{0x19},
{0x00},{0x46},
{0x00},{0x19},
{0x0A},{0x24},
{0x17},{0x01},
{0x02},{0x29},
{0x05},{0x03},
{0x08},{0x0F},
{0x08},{0x08},
{0x00},{0x00},
{0x00},{0xFF},
{0x02},{0x2D},
{0x14},{0x14},
{0x03},{0x01},
{0xFF},{0x07},
{0x06},{0x1E},
{0x0A},{0x1E},
{0x06},{0x06},
{0x0A},{0x05},
{0x00},{0x28},
{0x00},{0x02},
{0x00},{0x01},
{0x00},{0xFF},
{0x10},{0x02},
{0x00},{0x1E},
{0x09},{0x00},
{0x06},{0x00},
{0x05},{0x04},
{0x03},{0x07},
{0x94},{0x01},
{0x00},{0x80},
{0x00},{0x80},
{0x00},{0x80},
{0x50},{0x50},
{0x01},{0x01},
{0x1E},{0x01},
{0x14},{0x19},
{0x32},{0x0F},
{0x0A},{0x0A},
{0x25},{0x04},
{0x0A},{0x08},
{0x08},{0x32},
{0x14},{0x32},
{0xA0},{0x01},
{0x2A},{0x0A},
{0x40},{0x06},
{0x06},{0x04},
{0x46},{0x06},
{0x01},{0x46},
{0x01},{0x01},
{0x1C},{0x18},
{0x18},{0x19},
{0x00},{0x28},
{0x03},{0x0A},
{0x05},{0x14},
{0x0C},{0x14},
{0xFF},{0x05},
{0x04},{0x32},
{0x40},{0x52},
{0x15},{0x14},
{0x04},{0x40},
{0x03},{0x02},
{0x46},{0x46},
{0x01},{0x01},
{0x18},{0x01},
{0x19},{0x1C},
{0x28},{0x18},
{0x0A},{0x00},
{0x14},{0x03},
{0x14},{0x05},
{0x05},{0x0C},
{0x32},{0xFF},
{0x52},{0x04},
{0x14},{0x40},
{0x40},{0x15},
{0x02},{0x04},
{0x00},{0x03},

{0x7F},{0xFA},
{0xFF},{0xBD},
{0x26},{0xFE},
{0xF7},{0xBC},
{0x7E},{0x06},
{0x00},{0xD3},
//End of AFIT params


};

// Fill RAM with alternative op-codes
static unsigned char cam_long_version_sset_tuning_setup38[]= 
{
{0x0F},{0x12},
{0x00},{0x07},	// Modify LSB to control AWBB_YThreshLow
{0x00},{0xE2},	//
{0x00},{0x05},	// Modify LSB to control AWBB_YThreshLowBrLow
{0x00},{0xE2},	//
};

static unsigned char cam_long_version_sset_tuning_setup39[]= 
{
{0x0F},{0x12},
{0x00},{0x10},
{0xFF},{0xFE},
};

static unsigned char cam_long_version_sset_tuning_setup40[]= 
{
{0x0F},{0x12},
{0x02},{0x80},
{0x00},{0x01},	//AWB speed
};
//============================================================
//END Tuning part
//============================================================
//
//

static uint32_t cam_long_version_sset_pll_setup[]=
{
{0x002A01CC},
{0x0F125DC0},	//input clock=24MHz
{0x0F120000},
{0x002A01EE},
{0x0F120002},	//2 PLL configurations

{0x002A01F6},
{0x0F1238A4},	//1st system CLK 58MHz
{0x0F123A98},	//PVI clock 1 44MHz,  4268 68MHz, 3A98 60MHz
{0x0F123AA6},	//

{0x0F1238A4},	//2nd system CLK 58MHz
{0x0F124268},	//PVI clock 2, 5DB0 96MHz,    4268 68MHz NG,
{0x0F124273},	//             5DD0 96.064Mhz

{0x002A0208},
{0x0F120001},
};

static uint32_t cam_long_version_sset1_porting_setup[]=
{
///////////////////////////////////////////
//For 58MHz Sys Clk, 30 FPS VGA
//PREVIEW CONFIGURATION 0 (VGA, YUV, 15-30fps)
{0x002A026C},
{0x0F120400},	//1024
{0x0F120300},	//768
{0x0F120005},	//5 YUV
{0x0F123AA6},	//PCLK max
{0x0F123A98},	//PCLK min

{0x002A027A},
{0x0F120042},	//
{0x0F120010},	//Qualified clock

{0x002A0282},
{0x0F120000},	//PLL config
{0x0F120000},
{0x0F120001},	//1b: FR (bin) 2b: Quality (no-bin)
{0x0F120535},	//30fps 014D; 15fps 029a; 535 7.5FPS; a6a 3.75fps;
{0x0F12014D},


//CAPTURE CONFIGURATION 0 (QXGA, YUV, 14fps)
{0x002A035E},
{0x0F120800},	//2048
{0x0F120600},	//1536
{0x0F120005},	//YUV
{0x0F124273},	//PCLK max
{0x0F124268},	//PCLK min, 4268 68MHz,

{0x002A036C},
{0x0F120042},
{0x0F120010},	//Qualified clock

{0x002A0374},
{0x0F120001},	//PLL config
{0x0F120000},
{0x0F120002},	//1b: FR (bin) 2b: Quality (no-bin)
{0x0F120A6A},	//29a 15fps; 2ca 13.9fps; a6a 3.75fps;
{0x0F12029A},	//0535

////////////////////////////////////////////////////////
//New Configuration FW Sync
{0x002A023C},
{0x0F120000},	//58MHz Sys Clock
{0x002A0244},
{0x0F120000},
{0x002A0240},
{0x0F120001},
{0x002A0230},
{0x0F120001},
{0x002A023E},
{0x0F120001},
{0x002A0246},
{0x0F120001},
{0x002A0220},
{0x0F120001},
{0x0F120001},
};

static unsigned char cam_long_version_sset_slow_awb_speed[]= 
{
{0x0F},{0x12},
{0x00},{0x58},
{0x00},{0x11},	//AWB speed
};

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

static uint16_t Brightness_Value[S5K5CAG_BRIGHTNESS_DEGREE] = {0xFF81, 0xFF9A, 0xFFB3, 0xFFCC, 0xFFE5, 0x0000, 0x001B, 0x0034, 0x004D, 0x0066, 0x007F};
static uint16_t Contrast_Value = 0x00; 
static uint16_t Saturation_Value = 0x64; 
static uint16_t Sharpness_Value[S5K5CAG_SHARPNESS_DEGREE] = {0xFF81, 0xFF9A, 0xFFB3, 0xFFCC, 0xFFE5, 0x0000, 0x001B, 0x0034, 0x004D, 0x0066, 0x007F};
static uint16_t Preview_Frames = 0x00; 

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

static struct s5k5cag_ctrl *s5k5cag_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(s5k5cag_wait_queue);
DEFINE_MUTEX(s5k5cag_mutex);

void s5k5cag_power_enable(void)
{
       int rc;
       struct vreg *vreg_cam;
    	cci_smem_value_t *smem_cci_smem_value;
       printk("#### %s ####\n", __FUNCTION__);

       vreg_cam = vreg_get(0, "gp3"); //ACORE used
       rc = vreg_set_level(vreg_cam,2800);
       if (rc)
               printk("#### vreg set gp3 level failed ####\n");
       rc = vreg_enable(vreg_cam);
       if (rc)
               printk("#### vreg enable gp3 level failed ####\n");

       mdelay(20);

    	smem_cci_smem_value = smem_alloc( SMEM_CCI_SMEM_VALUE, sizeof( cci_smem_value_t ));
    	printk("#### cci_project_id=%d ####\n", smem_cci_smem_value->cci_project_id);
    	switch(smem_cci_smem_value->cci_project_id)
    	{
    		case PROJECT_ID_K5:
		       vreg_cam = vreg_get(0, "msme2"); //DCORE used
		       rc = vreg_set_level(vreg_cam,1500);
		       if (rc)
		               printk("#### vreg set msme2 level failed ####\n");
		       rc = vreg_enable(vreg_cam);
		       if (rc)
		               printk("#### vreg enable msme2 level failed ####\n");
		       break;

    		default:
		       vreg_cam = vreg_get(0, "rftx"); //DCORE used
		       rc = vreg_set_level(vreg_cam,1500);
		       if (rc)
		               printk("#### vreg set rftx level failed ####\n");
		       rc = vreg_enable(vreg_cam);
		       if (rc)
		               printk("#### vreg enable rftx level failed ####\n");
		       break;
    	}

       mdelay(20);

       vreg_cam = vreg_get(0, "gp2");  //I/O used
       rc = vreg_set_level(vreg_cam,2600);
       if (rc)
               printk("#### vreg set gp2 level failed ####\n");
       rc = vreg_enable(vreg_cam);
       if (rc)
               printk("#### vreg enable gp2 level failed ####\n");

       //Modify 2010/08/17 Battery device status update(on)
       cci_batt_device_status_update(CCI_BATT_DEVICE_ON_CAMERA_150,1);

}

void s5k5cag_power_disable(void)
{
    	int rc;
    	struct vreg *vreg_cam;
    	cci_smem_value_t *smem_cci_smem_value;
    	printk("#### %s ####\n", __FUNCTION__);

    	vreg_cam = vreg_get(0, "gp2");	//I/O used
    	rc = vreg_set_level(vreg_cam,0);
    	if (rc)
    		printk("#### vreg set gp2 level failed ####\n");
    	rc = vreg_disable(vreg_cam);
    	if (rc)
    		printk("#### vreg disable gp2 level failed ####\n");

       mdelay(20);

    	smem_cci_smem_value = smem_alloc( SMEM_CCI_SMEM_VALUE, sizeof( cci_smem_value_t ));
    	printk("#### cci_project_id=%d ####\n", smem_cci_smem_value->cci_project_id);
    	switch(smem_cci_smem_value->cci_project_id)
    	{
    		case PROJECT_ID_K5:
		       vreg_cam = vreg_get(0, "msme2"); //DCORE used
		       rc = vreg_set_level(vreg_cam,0);
		       if (rc)
		               printk("#### vreg set msme2 level failed ####\n");
		       rc = vreg_disable(vreg_cam);
		       if (rc)
		               printk("#### vreg disable msme2 level failed ####\n");
		       break;

    		default:
		       vreg_cam = vreg_get(0, "rftx"); //DCORE used
		       rc = vreg_set_level(vreg_cam,0);
		       if (rc)
		               printk("#### vreg set rftx level failed ####\n");
		       rc = vreg_disable(vreg_cam);
		       if (rc)
		               printk("#### vreg disable rftx level failed ####\n");
		       break;
    	}

       mdelay(20);

    	vreg_cam = vreg_get(0, "gp3");	//ACORE used
    	rc = vreg_set_level(vreg_cam,0);
    	if (rc)
    		printk("#### vreg set gp3 level failed ####\n");
    	rc = vreg_disable(vreg_cam);
    	if (rc)
    		printk("#### vreg disable gp3 level failed ####\n");

       //Modify 2010/08/17 Battery device status update(off)
       cci_batt_device_status_update(CCI_BATT_DEVICE_ON_CAMERA_150,0);

}

static int s5k5cag_i2c_rxdata(unsigned short saddr, unsigned char *rxdata,
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

	if (i2c_transfer(s5k5cag_client->adapter, msgs, 2) < 0) {
		CDBG("s5k5cag_i2c_rxdata failed!\n");
		return -EIO;
	}

	return 0;
}

static int32_t s5k5cag_i2c_txdata(unsigned short saddr,
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

	if (i2c_transfer(s5k5cag_client->adapter, msg, 1) < 0) {
		CDBG("s5k5cag_i2c_txdata failed\n");
		return -EIO;
	}

	return 0;
}

static int32_t s5k5cag_i2c_write_b(unsigned short saddr, unsigned short waddr,
	unsigned char bdata)
{
	int32_t rc = -EIO;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = bdata;

	rc = s5k5cag_i2c_txdata(saddr, buf, 3);

	if (rc < 0)
		CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
			waddr, bdata);

	return rc;
}

static int32_t camsensor_i2c_write_byte(unsigned short saddr, unsigned char bdata)
{
	int32_t rc = 0;
	rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
		saddr,
		bdata);
	return rc;
}

static int32_t s5k5cag_i2c_write_w(unsigned short saddr, unsigned short waddr,
	unsigned short wdata)
{
	int32_t rc = -EIO;
	unsigned char buf[4];

	memset(buf, 0, sizeof(buf));
	buf[0] = (waddr & 0xFF00)>>8;
	buf[1] = (waddr & 0x00FF);
	buf[2] = (wdata & 0xFF00)>>8;
	buf[3] = (wdata & 0x00FF);

	rc = s5k5cag_i2c_txdata(saddr, buf, 4);

	if (rc < 0)
		CDBG("i2c_write_w failed, addr = 0x%x, val = 0x%x!\n",
			waddr, wdata);

	return rc;
}

static int32_t camsensor_i2c_write_word(unsigned short saddr, unsigned short sdata)
{
	int32_t rc = 0;
	rc = s5k5cag_i2c_write_w(s5k5cag_client->addr,
		saddr,
		sdata);
	return rc;
}

static int32_t s5k5cag_i2c_write_table(
	struct s5k5cag_i2c_reg_conf *reg_cfg_tbl, int num)
{
	int i;
	int32_t rc = -EIO;
	for (i = 0; i < num; i++) {
		rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
			reg_cfg_tbl->waddr, reg_cfg_tbl->bdata);
		if (rc < 0)
			break;
		reg_cfg_tbl++;
	}

	return rc;
}

static int32_t s5k5cag_i2c_read_w(unsigned short saddr, unsigned short raddr,
	unsigned short *rdata)
{
	int32_t rc = 0;
	unsigned char buf[4];

	if (!rdata)
		return -EIO;

	memset(buf, 0, sizeof(buf));

	buf[0] = (raddr & 0xFF00)>>8;
	buf[1] = (raddr & 0x00FF);

	rc = s5k5cag_i2c_rxdata(saddr, buf, 2);
	if (rc < 0)
		return rc;

	*rdata = buf[0] << 8 | buf[1];

	if (rc < 0)
		CDBG("s5k5cag_i2c_read failed!\n");

	return rc;
}

static int32_t camsensor_i2c_read_word(unsigned short saddr, unsigned short *sdata)
{
	int32_t rc = 0;
	rc = s5k5cag_i2c_read_w(s5k5cag_client->addr,
		saddr,
		sdata);
	return rc;
}

static int32_t camsensor_s5k5cag_i2c_write(uint16_t msb_reg, uint16_t lsb_reg, uint16_t data)
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

static int32_t camsensor_s5k5cag_i2c_read(uint16_t msb_reg, uint16_t lsb_reg, uint16_t *data)
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

static int32_t camsensor_s5k5cag_sw_reset_reg_setup(void)
{
   int32_t  rc = 0;
   int32_t		ary_size=0;
   int			i;
   
   ary_size = sizeof(cam_sw_resets_setup)/sizeof(CAM_REG_ADDR_VAL_TYPE);
   for(i=0;i<ary_size;i++)
   {
	rc = camsensor_i2c_write_word(cam_sw_resets_setup[i].addr, cam_sw_resets_setup[i].val);
	if(rc < 0)
	{
         return rc;
 	}
   }

   return rc;
}

static int s5k5cag_probe_init_done(const struct msm_camera_sensor_info *data)
{
	gpio_direction_output(data->sensor_pwd, 0);
	gpio_free(data->sensor_pwd);

	gpio_direction_output(data->sensor_reset, 0);
	gpio_free(data->sensor_reset);
	return 0;
}

static int s5k5cag_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int32_t  rc;
	uint16_t chipid = 0;

	rc = gpio_request(data->sensor_reset, "s5k5cag");
	printk("[camera] gpio_request reset=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(data->sensor_reset, 1);
	else
		goto init_probe_fail;

	mdelay(30);

	/*7. send soft reset*/
	rc = camsensor_s5k5cag_sw_reset_reg_setup();
	if(rc < 0)
	{
		goto init_probe_fail;
	}
	/*8. delay 5ms*/
	mdelay(100);
  
	CDBG("s5k5cag_sensor_init(): reseting sensor.\n");

	rc = camsensor_s5k5cag_i2c_read(INFO_CHIPID_H, INFO_CHIPID_L, &chipid);
	printk("[camera] camsensor_s5k5cag_i2c_read=%d\n", rc);
	if (rc < 0)
		goto init_probe_fail;

	printk("[camera] chipid0x%X=0x%X\n", chipid, S5K5CAG_MODEL_ID);
	if (chipid != S5K5CAG_MODEL_ID) {
		CDBG("S5K5CAG wrong model_id = 0x%x\n", chipid);
		rc = -ENODEV;
		goto init_probe_fail;
	}

	goto init_probe_done;

init_probe_fail:
	s5k5cag_probe_init_done(data);
init_probe_done:
	return rc;
}

static int s5k5cag_init_client(struct i2c_client *client)
{
	/* Initialize the MSM_CAMI2C Chip */
	init_waitqueue_head(&s5k5cag_wait_queue);
	return 0;
}

static const struct i2c_device_id s5k5cag_i2c_id[] = {
	{ "s5k5cag", 0},
	{ }
};

static int s5k5cag_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	CDBG("s5k5cag_probe called!\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CDBG("i2c_check_functionality failed\n");
		goto probe_failure;
	}

	s5k5cag_sensorw = kzalloc(sizeof(struct s5k5cag_work), GFP_KERNEL);
	if (!s5k5cag_sensorw) {
		CDBG("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, s5k5cag_sensorw);
	s5k5cag_init_client(client);
	s5k5cag_client = client;

	mdelay(50);

	CDBG("s5k5cag_probe successed! rc = %d\n", rc);
	return 0;

probe_failure:
	CDBG("s5k5cag_probe failed! rc = %d\n", rc);
	return rc;
}

static struct i2c_driver s5k5cag_i2c_driver = {
	.id_table = s5k5cag_i2c_id,
	.probe  = s5k5cag_i2c_probe,
	.remove = __exit_p(s5k5cag_i2c_remove),
	.driver = {
		.name = "s5k5cag",
	},
};

/*
static int32_t s5k5cag_test(enum msm_s_test_mode mo)
{
	int32_t rc = 0;

	if (mo == S_TEST_OFF)
		rc = 0;
	else
		rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
			REG_TEST_PATTERN_MODE, (uint16_t)mo);

	return rc;
}
*/


static int32_t s5k5cag_write_FlickerMode_60hz_reg(void)
{
   int32_t rc = 0;
#if 0
//#ifndef USE_CAMSENSOR_SAMSUNG_S5K5CAG  
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

static int32_t s5k5cag_write_FlickerMode_50hz_reg(void)
{
   int32_t rc = 0;
#if 0
//#ifndef USE_CAMSENSOR_SAMSUNG_S5K5CAG  
   boolean	status;
   uint32_t ary_size=0;
   int			i;
   
   MSG_HIGH("camsensor_s5k5cag_write_FlickerMode_50hz_reg",0,0,0);
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


static int32_t s5k5cag_write_AWB_reg(void)
{
   int32_t rc = 0;
   uint32_t		ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_awb_auto)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_s5k5cag_i2c_write((uint16_t)(cam_awb_auto[i].addr >> 16),
	  	(uint16_t)(cam_awb_auto[i].addr & 0xFFFF),
		cam_awb_auto[i].val);
      if(rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t s5k5cag_write_WB_cloudy_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_wb_cloudy)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_s5k5cag_i2c_write((uint16_t)((cam_wb_cloudy[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_wb_cloudy[i].addr & 0xFFFF),
		cam_wb_cloudy[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t s5k5cag_write_WB_horizon_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_wb_horizon)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_s5k5cag_i2c_write((uint16_t)(cam_wb_horizon[i].addr >>16),
	  	(uint16_t)(cam_wb_horizon[i].addr & 0xFFFF),
		cam_wb_horizon[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}


static int32_t s5k5cag_write_WB_fluorescent_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_wb_fluorescent)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_s5k5cag_i2c_write((uint16_t)(cam_wb_fluorescent[i].addr >>16),
	  	(uint16_t)(cam_wb_fluorescent[i].addr & 0xFFFF),
		cam_wb_fluorescent[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t s5k5cag_write_WB_incandescent_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_wb_incandescent)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_s5k5cag_i2c_write((uint16_t)(cam_wb_incandescent[i].addr >>16),
	  	(uint16_t)(cam_wb_incandescent[i].addr & 0xFFFF),
		cam_wb_incandescent[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t s5k5cag_write_WB_daylight_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_wb_daylight)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_s5k5cag_i2c_write((uint16_t)(cam_wb_daylight[i].addr >>16),
	  	(uint16_t)(cam_wb_daylight[i].addr & 0xFFFF),
		cam_wb_daylight[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t s5k5cag_write_bestshot_mode_beach_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_extra_mode_beach)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_s5k5cag_i2c_write((uint16_t)((cam_extra_mode_beach[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_extra_mode_beach[i].addr & 0xFFFF),
		cam_extra_mode_beach[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t s5k5cag_write_bestshot_mode_night_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_extra_mode_night)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_s5k5cag_i2c_write((uint16_t)((cam_extra_mode_night[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_extra_mode_night[i].addr & 0xFFFF),
		cam_extra_mode_night[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t s5k5cag_write_bestshot_mode_night_break_reg(void)
{
   int32_t rc = 0;
   uint32_t ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_extra_mode_night_break)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc = camsensor_s5k5cag_i2c_write((uint16_t)((cam_extra_mode_night_break[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_extra_mode_night_break[i].addr & 0xFFFF),
		cam_extra_mode_night_break[i].val);
      if (rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

#ifdef T_CCI_CAMERA_INITIAL_SHORT_VERSION
static int32_t camsensor_s5k5cag_short_version_porting_reg_setup(void)
{
   int32_t rc = 0;
   uint32_t		ary_size=0;
   int			i;

   ary_size=sizeof(cam_short_version_porting_setup)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_s5k5cag_i2c_write((uint16_t)((cam_short_version_porting_setup[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_short_version_porting_setup[i].addr & 0xFFFF),
		cam_short_version_porting_setup[i].val);
      if(rc < 0)
      {
         return rc;
      }
//	MSG_HIGH("[Write]0x%x = 0x%x",cam_short_version_porting_setup[i].addr, cam_short_version_porting_setup[i].val, 0);	  
   }

   return rc;
}
#else
static int32_t camsensor_s5k5cag_long_version_sset_porting_reg_setup(void)
{

   int32_t rc = 0;
   uint32_t		ary_size=0;
   int			i;
   cci_smem_value_t *smem_cci_smem_value;
   smem_cci_smem_value = smem_alloc( SMEM_CCI_SMEM_VALUE, sizeof( cci_smem_value_t ));

   rc = camsensor_i2c_write_word(0x0028, 0x7000);
   if(rc < 0)
   {
	return rc;
   }
   rc = camsensor_i2c_write_word(0x002A, 0x2CF8);
   if(rc < 0)
   {
	return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_initial_sset_setup, sizeof(cam_initial_sset_setup));
   if(rc < 0)
   {
      printk("[Camera Write] cam_initial_sset_setup error\n");
      return rc;
   }

   ary_size=sizeof(cam_long_version_sset_porting_setup)/sizeof(uint32_t);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_i2c_write_word((uint16_t)(cam_long_version_sset_porting_setup[i] >> 16),
		(uint16_t)(cam_long_version_sset_porting_setup[i] & 0xFFFF));
      if(rc < 0)
      {
         return rc;
      }
//	printk("[Camera Write]0x%x = 0x%x\n",(uint16_t)(cam_long_version_sset_porting_setup[i] >> 16), (uint16_t)(cam_long_version_sset_porting_setup[i] & 0xFFFF));	  
   }

   rc = camsensor_i2c_write_word(0x002A, 0x06CE);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup1, sizeof(cam_long_version_sset_tuning_setup1));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup1 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x075A);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup2, sizeof(cam_long_version_sset_tuning_setup2));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup2 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x347C);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup3, sizeof(cam_long_version_sset_tuning_setup3));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup3 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x074E);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup4, sizeof(cam_long_version_sset_tuning_setup4));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup4 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D30);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup5, sizeof(cam_long_version_sset_tuning_setup5));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup5 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x06B8);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup6, sizeof(cam_long_version_sset_tuning_setup6));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup6 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0E4A);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup7, sizeof(cam_long_version_sset_tuning_setup7));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup7 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0E32);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup8, sizeof(cam_long_version_sset_tuning_setup8));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup8 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0E22);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup9, sizeof(cam_long_version_sset_tuning_setup9));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup9 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0E1C);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup10, sizeof(cam_long_version_sset_tuning_setup10));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup10 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0DD4);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup11, sizeof(cam_long_version_sset_tuning_setup11));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup11 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0C48);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup12, sizeof(cam_long_version_sset_tuning_setup12));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup12 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0CA0);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup13, sizeof(cam_long_version_sset_tuning_setup13));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup13 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0CE0);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup14, sizeof(cam_long_version_sset_tuning_setup14));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup14 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D18);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup15, sizeof(cam_long_version_sset_tuning_setup15));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup15 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0CA4);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup16, sizeof(cam_long_version_sset_tuning_setup16));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup16 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0CDC);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup17, sizeof(cam_long_version_sset_tuning_setup17));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup17 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D1C);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup18, sizeof(cam_long_version_sset_tuning_setup18));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup18 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D20);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup19, sizeof(cam_long_version_sset_tuning_setup19));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup19 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D24);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup20, sizeof(cam_long_version_sset_tuning_setup20));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup20 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D2C);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup21, sizeof(cam_long_version_sset_tuning_setup21));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup21 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D46);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup22, sizeof(cam_long_version_sset_tuning_setup22));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup22 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D5C);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup23, sizeof(cam_long_version_sset_tuning_setup23));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup23 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0E3E);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup24, sizeof(cam_long_version_sset_tuning_setup24));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup24 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0F70);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup25, sizeof(cam_long_version_sset_tuning_setup25));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup25 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0F76);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup26, sizeof(cam_long_version_sset_tuning_setup26));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup26 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x3288);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup27, sizeof(cam_long_version_sset_tuning_setup27));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup27 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x1034);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup28, sizeof(cam_long_version_sset_tuning_setup28));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup28 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x06A6);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup29, sizeof(cam_long_version_sset_tuning_setup29));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup29 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x33A4);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup30, sizeof(cam_long_version_sset_tuning_setup30));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup30 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x3380);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup31, sizeof(cam_long_version_sset_tuning_setup31));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup31 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0DA0);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup32, sizeof(cam_long_version_sset_tuning_setup32));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup32 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0D88);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup33, sizeof(cam_long_version_sset_tuning_setup33));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup33 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0DA8);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup34, sizeof(cam_long_version_sset_tuning_setup34));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup34 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0DA4);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup35, sizeof(cam_long_version_sset_tuning_setup35));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup35 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0764);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup36, sizeof(cam_long_version_sset_tuning_setup36));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup36 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x07C4);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup37, sizeof(cam_long_version_sset_tuning_setup37));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup37 error\n");
      return rc;
   }
  
   rc = camsensor_i2c_write_word(0x002A, 0x2CE8);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup38, sizeof(cam_long_version_sset_tuning_setup38));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup38 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0210);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup39, sizeof(cam_long_version_sset_tuning_setup39));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup39 error\n");
      return rc;
   }

   rc = camsensor_i2c_write_word(0x002A, 0x0DCA);
   if(rc < 0)
   {
      return rc;
   }

   rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_tuning_setup40, sizeof(cam_long_version_sset_tuning_setup40));
   if(rc < 0)
   {
      printk("[Camera Write] cam_long_version_sset_tuning_setup40 error\n");
      return rc;
   }

   ary_size=sizeof(cam_long_version_sset_pll_setup)/sizeof(uint32_t);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_i2c_write_word((uint16_t)(cam_long_version_sset_pll_setup[i] >> 16),
		(uint16_t)(cam_long_version_sset_pll_setup[i] & 0xFFFF));
      if(rc < 0)
      {
         return rc;
      }
//	printk("[Camera Write]0x%x = 0x%x\n",(uint16_t)(cam_long_version_sset_pll_setup[i] >> 16), (uint16_t)(cam_long_version_sset_pll_setup[i] & 0xFFFF));	  
   }
   mdelay(100);

   // Need to rotate 180' for K5
   if(smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
   {
	   rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_0TC_PCFG_uCaptureMirror & 0xFFFF0000)>>16), (uint16_t)(REG_0TC_PCFG_uCaptureMirror & 0xFFFF), 0x0003);
	   if(rc < 0)
	   {
	      return rc;
	   }

	   rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_0TC_PCFG_uPrevMirror & 0xFFFF0000)>>16), (uint16_t)(REG_0TC_PCFG_uPrevMirror & 0xFFFF), 0x0003);
	   if(rc < 0)
	   {
	      return rc;
	   }
   }

   ary_size=sizeof(cam_long_version_sset1_porting_setup)/sizeof(uint32_t);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_i2c_write_word((uint16_t)(cam_long_version_sset1_porting_setup[i] >> 16),
		(uint16_t)(cam_long_version_sset1_porting_setup[i] & 0xFFFF));
      if(rc < 0)
      {
         return rc;
      }
//	printk("[Camera Write]0x%x = 0x%x\n",(uint16_t)(cam_long_version_sset1_porting_setup[i] >> 16), (uint16_t)(cam_long_version_sset1_porting_setup[i] & 0xFFFF));	  
   }
   
   return rc;
}
#endif

static int32_t camsensor_s5k5cag_new_config_capture_sync_reg_setup(void)
{

   int32_t rc = 0;
   uint32_t		ary_size=0;
   int			i;
   
   ary_size=sizeof(cam_new_configuration_capture_sync)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_s5k5cag_i2c_write((uint16_t)((cam_new_configuration_capture_sync[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_new_configuration_capture_sync[i].addr & 0xFFFF),
		cam_new_configuration_capture_sync[i].val);
      if(rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t camsensor_s5k5cag_new_config_preview_sync_reg_setup(void)
{

   int32_t rc = 0;
   uint32_t		ary_size=0;
   int			i;

   ary_size=sizeof(cam_new_configuration_preview_sync)/sizeof(CAM_REG_ADDR_VAL_TYPE);

   for(i=0;i<ary_size;i++)
   {
      rc=camsensor_s5k5cag_i2c_write((uint16_t)((cam_new_configuration_preview_sync[i].addr & 0xFFFF0000)>>16),
	  	(uint16_t)(cam_new_configuration_preview_sync[i].addr & 0xFFFF),
		cam_new_configuration_preview_sync[i].val);
      if(rc < 0)
      {
         return rc;
      }
   }
   
   return rc;
}

static int32_t camsensor_s5k5cag_camera_run(void)
{
	int32_t rc = 0;
	rc = camsensor_i2c_write_word(0x1000, 0x0001);
	return rc;
}

static int32_t s5k5cag_setting(enum msm_s_reg_update rupdate,
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
	           rc = camsensor_s5k5cag_new_config_capture_sync_reg_setup();
	           if(rc < 0)
	           {
	              return rc;
	           }
	           mdelay(100);
	           break;
			   
	        case S_RES_PREVIEW:
	           rc = camsensor_s5k5cag_new_config_preview_sync_reg_setup();
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
#ifdef T_CCI_CAMERA_INITIAL_SHORT_VERSION		
		rc = camsensor_s5k5cag_short_version_porting_reg_setup();
		if(rc < 0)
		{
		    return rc;
		} 		
		rc = s5k5cag_write_AWB_reg();
		if(rc < 0)
		{
		    return rc;
		} 	
#else /*T_CCI_CAMERA_INITIAL_SHORT_VERSION*/	
		rc = camsensor_s5k5cag_long_version_sset_porting_reg_setup();
		if(rc < 0)
		{
		    return rc;
		}	   
#endif /*T_CCI_CAMERA_INITIAL_SHORT_VERSION*/	   
		/* Set host interrupt so main start run*/
		rc = camsensor_s5k5cag_camera_run();
		if(rc < 0)
		{
		    return rc;
		}
		mdelay(10);
		Preview_Frames = 0;
		break; /* case REG_INIT: */

	default:
		rc = -EINVAL;
		break;
	} /* switch (rupdate) */

//	printk("[Camera] test pattern\n");
//	rc = camsensor_i2c_write_word(0xB054, 0x0001);
	return rc;
}

static int s5k5cag_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int32_t  rc;

       printk("#### %s ####\n", __FUNCTION__);

	s5k5cag_ctrl = kzalloc(sizeof(struct s5k5cag_ctrl), GFP_KERNEL);
	if (!s5k5cag_ctrl) {
		CDBG("s5k5cag_init failed!\n");
		rc = -ENOMEM;
		goto init_done;
	}

	s5k5cag_ctrl->fps_divider = 1 * 0x00000400;
	s5k5cag_ctrl->pict_fps_divider = 1 * 0x00000400;
	s5k5cag_ctrl->set_test = S_TEST_OFF;
	s5k5cag_ctrl->prev_res = S_QTR_SIZE;
	s5k5cag_ctrl->pict_res = S_FULL_SIZE;

	if (data)
		s5k5cag_ctrl->sensordata = data;

	/* pull down power-down */
	rc = gpio_request(data->sensor_pwd, "s5k5cag");
	printk("[camera] gpio_request pwd low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(data->sensor_pwd, 0);
	else printk(KERN_ERR "s5k5cag error: request gpio %d failed: "
			"%d\n", data->sensor_pwd, rc);

	/* pull down reset */
	rc = gpio_request(data->sensor_reset, "s5k5cag");
	printk("[camera] gpio_request reset low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(data->sensor_reset, 0);
	else printk(KERN_ERR "s5k5cag error: request gpio %d failed: "
			"%d\n", data->sensor_reset, rc);

	mdelay(20);

	s5k5cag_power_enable();

	mdelay(20);

	/* enable clk */
	msm_camio_clk_enable(CAMIO_VFE_CLK);
	printk("[camera] enable clk\n");

	/* enable mclk first */
	msm_camio_clk_rate_set(S5K5CAG_DEFAULT_CLOCK_RATE);
	mdelay(100);

	/* pull up power-down */
	rc = gpio_request(data->sensor_pwd, "s5k5cag");
	printk("[camera] gpio_request pwd high=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(data->sensor_pwd, 1);
	else printk(KERN_ERR "s5k5cag error: request gpio %d failed: "
			"%d\n", data->sensor_pwd, rc);
	mdelay(20);

	msm_camio_camif_pad_reg_reset();
	mdelay(20);

	rc = s5k5cag_probe_init_sensor(data);
	if (rc < 0)
		goto init_fail1;

	if (s5k5cag_ctrl->prev_res == S_QTR_SIZE)
		rc = s5k5cag_setting(S_REG_INIT, S_RES_PREVIEW);
	else
		rc = s5k5cag_setting(S_REG_INIT, S_RES_CAPTURE);

	if (rc < 0) {
		CDBG("s5k5cag_setting failed. rc = %d\n", rc);
		goto init_fail1;
	}

	goto init_done;

init_fail1:
	s5k5cag_probe_init_done(data);
	kfree(s5k5cag_ctrl);
init_done:
	return rc;
}

static int32_t s5k5cag_power_down(void)
{
	int32_t rc = 0;
	return rc;
}

static int s5k5cag_sensor_release(void)
{
	int rc = -EBADF;

	mutex_lock(&s5k5cag_mutex);

	s5k5cag_power_down();

	gpio_direction_output(s5k5cag_ctrl->sensordata->sensor_reset,
		0);
	gpio_free(s5k5cag_ctrl->sensordata->sensor_reset);

	gpio_direction_output(s5k5cag_ctrl->sensordata->sensor_pwd,
		0);
	gpio_free(s5k5cag_ctrl->sensordata->sensor_pwd);
	mdelay(1);

	/*disable clk*/
	msm_camio_clk_disable(CAMIO_VFE_CLK);
	printk("[camera] disable clk\n");

	mdelay(20);

	s5k5cag_power_disable();

	kfree(s5k5cag_ctrl);
	s5k5cag_ctrl = NULL;

	CDBG("s5k5cag_release completed\n");

	mutex_unlock(&s5k5cag_mutex);
	return rc;
}

static uint32_t s5k5cag_style_tuning(void)
{
	int32_t rc = 0;

	rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_UserContrast & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserContrast & 0xFFFF), Contrast_Value);
 	if (rc < 0)
	{
		return rc;
	}	
 	rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_UserSaturation & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserSaturation & 0xFFFF), Saturation_Value);
	if (rc < 0)
	{
		return rc;
	}	
	rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_UserSharpBlur & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserSharpBlur & 0xFFFF), Sharpness_Value[7]);
	if (rc < 0)
	{
		return rc;
	}	

	return rc;  
}

static int32_t s5k5cag_set_brightness (int8_t brightness)
{
	uint16_t brightness_value = 0;
	int32_t rc = 0;
	uint32_t  brightness_offset = 2;

	brightness += brightness_offset; // App only has range of 0~8, and Brightness_Value[6] gives us best result
	if ((brightness >= 0) && (brightness < S5K5CAG_BRIGHTNESS_DEGREE))
	{
		brightness_value = Brightness_Value[brightness];
	}
	else
	{
		brightness_value = Brightness_Value[5];
	}

	rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_UserBrightness & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserBrightness & 0xFFFF), brightness_value);
	if (rc < 0)
	{
		return rc;
	}	

	return rc;  
}

static int32_t s5k5cag_set_contrast (int8_t contrast)
{
	int32_t rc = 0;

	rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_UserContrast & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserContrast & 0xFFFF), contrast);
 	if (rc < 0)
	{
		return rc;
	}	

	return rc;  
}

static int32_t s5k5cag_set_sharpness(int8_t sharpness)
{
	uint16_t sharpness_value = 0;
	int32_t rc = 0;
	uint32_t  sharpness_offset = 0;

	sharpness += sharpness_offset;
	if ((sharpness >= 0) && (sharpness < S5K5CAG_SHARPNESS_DEGREE))
	{
		sharpness_value = Sharpness_Value[sharpness];
	}
	else
	{
		sharpness_value = Sharpness_Value[5];
	}

	rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_UserSharpBlur & 0xFFFF0000)>>16), (uint16_t)(REG_TC_UserSharpBlur & 0xFFFF), sharpness_value);
	if (rc < 0)
	{
		return rc;
	}	

	return rc;  
}

static int32_t s5k5cag_set_effect (int8_t effect)
{
 int32_t rc = 0;

	 switch(effect)
	 {
	    case CAMERA_EFFECT_OFF:
		rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0000);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
		break;
		
	    case CAMERA_EFFECT_NEGATIVE:
		rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0003);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
	       break;
		   
	    case CAMERA_EFFECT_SOLARIZE:
	       break;
		   
	    case CAMERA_EFFECT_SEPIA:
		rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0004);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
	       break;

	    case CAMERA_EFFECT_MONO:
		rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0001);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
	       break;

	    case CAMERA_EFFECT_AQUA:
		rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_TC_GP_SpecialEffects & 0xFFFF0000)>>16), 
			(uint16_t)(REG_TC_GP_SpecialEffects & 0xFFFF), 0x0005);
	    	if (rc < 0)
	   	{
	      		return rc;
	   	}	
	       break;

	     default:
		 break;	
	 }

  return rc;  
}

static int32_t s5k5cag_set_antibanding (int8_t antibanding)
{
#ifndef USE_CAMSENSOR_SAMSUNG_S5K5CAG  
   int32_t rc = 0; 

//   MSG_HIGH("into s5k5cag_set_antibanding antiband=%d",antibanding,0,0);
   switch(antibanding)
   {
      case CAMERA_ANTIBANDING_60HZ:
         rc = s5k5cag_write_FlickerMode_60hz_reg();
         if (rc < 0)
         {
            return rc;
         } 
	  break;	 
      case CAMERA_ANTIBANDING_50HZ:
         rc = s5k5cag_write_FlickerMode_50hz_reg();
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

static int32_t s5k5cag_set_wb (int8_t wb)
{
	int32_t rc = 0;

	switch(wb)
	{
		case CAMERA_WB_AUTO:
			rc = s5k5cag_write_AWB_reg();
			if (rc < 0)
			{
				return rc;
			}  	
         		break; 	  

		case CAMERA_WB_DAYLIGHT:
			rc = s5k5cag_write_WB_daylight_reg();
			if (rc < 0)
			{
         			return rc;
			}  	
			break; 	 

		case CAMERA_WB_CLOUDY_DAYLIGHT:
			rc = s5k5cag_write_WB_cloudy_reg();
			if (rc < 0)
			{
         			return rc;
			}  	
			break; 	 

		case CAMERA_WB_FLUORESCENT:
			rc = s5k5cag_write_WB_fluorescent_reg();	
			if (rc < 0)
			{
				return rc;
			}  
			break; 	 

		case CAMERA_WB_INCANDESCENT:
			rc = s5k5cag_write_WB_incandescent_reg();	
			if (rc < 0)
			{
				return rc;
			}  
			break; 	 
/*
		case CAMERA_WB_HORIZON:
			rc = s5k5cag_write_WB_horizon_reg();
			if (rc < 0)
			{
				return rc;
			}  		  	  	
			break; 	 
*/
		default:
			rc = s5k5cag_write_AWB_reg();
			if (rc < 0)
			{
				return rc;
			}  				
			break; 	 
	}

	return rc;  
}

static int32_t s5k5cag_set_iso (int8_t iso)
{
	int32_t rc = 0;

	switch(iso)
	{
		case CAMERA_ISO_100:
			//set manual ISO, 0 for auto
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0001);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x0064);
		 	if (rc < 0)
			{
				return rc;
			}
			break;

		case CAMERA_ISO_200:
			//set manual ISO, 0 for auto
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0001);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x00C8);
		 	if (rc < 0)
			{
				return rc;
			}
			break;

		case CAMERA_ISO_400:
			//set manual ISO, 0 for auto
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0001);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x0190);
		 	if (rc < 0)
			{
				return rc;
			}
			break;

		case CAMERA_ISO_800:
			//set manual ISO, 0 for auto
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0001);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x0320);
		 	if (rc < 0)
			{
				return rc;
			}
			break;

		case CAMERA_ISO_AUTO:
		default:
			//set manual ISO, 0 for auto
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoType & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoType & 0xFFFF), 0x0000);
		 	if (rc < 0)
			{
				return rc;
			}
			
			//now ISO == 100, 0064=ISO100 //00C8=ISO200, 0190=ISO400, 0320=ISO800
			rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoVal & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoVal & 0xFFFF), 0x0064);
		 	if (rc < 0)
			{
				return rc;
			}
			break;
	}

	//activate new ISO value
	rc = camsensor_s5k5cag_i2c_write((uint16_t)((REG_SF_USER_IsoChanged & 0xFFFF0000)>>16), (uint16_t)(REG_SF_USER_IsoChanged & 0xFFFF), 0x0001);

	return rc;  
}

static int32_t s5k5cag_set_exposure_mode(int8_t exposure_mode)
{
	int32_t rc = 0;
	uint32_t		ary_size=0;
	int			i;

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

	return rc;  
}

static int32_t s5k5cag_get_ae_status(int8_t *sdata)
{
	int rc = 0;
	uint16_t ae_status = 0;

	//printk("#### %s ####\n", __FUNCTION__);

	if (Preview_Frames < S5K5CAG_FRAMES_BEFORE_SLOW_AWB)
	{
	           Preview_Frames++;
	}
	else if (Preview_Frames == S5K5CAG_FRAMES_BEFORE_SLOW_AWB)
	{
	           Preview_Frames++;
	           rc = camsensor_i2c_write_word(0x002A, 0x0DCA);
	           if(rc < 0)
	           {
	              return rc;
	           }

	           rc = s5k5cag_i2c_txdata(s5k5cag_client->addr, cam_long_version_sset_slow_awb_speed, sizeof(cam_long_version_sset_slow_awb_speed));
	           if(rc < 0)
	           {
	              printk("[Camera Write] cam_long_version_sset_slow_awb_speed error\n");
	              return rc;
	           }
	}

	rc = camsensor_s5k5cag_i2c_read(((Mon_AE_CrntAvBr & 0xFFFF0000)>>16), (Mon_AE_CrntAvBr & 0xFFFF), &ae_status);
//	printk("[camera] Mon_AE_CrntAvBr=%d\n", ae_status);
	if (rc < 0)
		printk("#### s5k5cag_get_ae_status fail%d####\n", rc);

	*sdata = (int8_t) ae_status;

	return rc;
}

static int32_t s5k5cag_set_bestshot_mode (int8_t mode)
{
	int32_t rc = 0;
	uint32_t ary_size=0;
	int		i;

	 switch(mode)
	 {
		case CAMERA_BESTSHOT_NIGHT:
			rc = s5k5cag_write_bestshot_mode_night_reg();			
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
			rc = s5k5cag_write_bestshot_mode_beach_reg();			
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
			s5k5cag_style_tuning();
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
		rc = s5k5cag_write_bestshot_mode_night_break_reg();
		if (rc < 0)
 		{
 			return rc;
 		}	
	}
	
	return rc;  
}

static int32_t s5k5cag_set_fps(struct fps_cfg *fps)
{
	/* input is new fps in Q10 format */
	int32_t rc = 0;
	enum msm_s_setting setting;

	s5k5cag_ctrl->fps_divider = fps->fps_div;

	if (s5k5cag_ctrl->sensormode == SENSOR_PREVIEW_MODE)
		setting = S_RES_PREVIEW;
	else
		setting = S_RES_CAPTURE;

  rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
		REG_FRAME_LENGTH_LINES_MSB,
		(((s5k5cag_reg_pat[setting].size_h +
			s5k5cag_reg_pat[setting].blk_l) *
			s5k5cag_ctrl->fps_divider / 0x400) & 0xFF00) >> 8);
	if (rc < 0)
		goto set_fps_done;

  rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
		REG_FRAME_LENGTH_LINES_LSB,
		(((s5k5cag_reg_pat[setting].size_h +
			s5k5cag_reg_pat[setting].blk_l) *
			s5k5cag_ctrl->fps_divider / 0x400) & 0x00FF));

set_fps_done:
	return rc;
}

static int32_t s5k5cag_write_exp_gain(uint16_t gain, uint32_t line)
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

	struct s5k5cag_i2c_reg_conf tbl[2];

	CDBG("Line:%d s5k5cag_write_exp_gain \n", __LINE__);

	if (s5k5cag_ctrl->sensormode == SENSOR_PREVIEW_MODE) {

		s5k5cag_ctrl->my_reg_gain = gain;
		s5k5cag_ctrl->my_reg_line_count = (uint16_t)line;

		fl_lines = s5k5cag_reg_pat[S_RES_PREVIEW].size_h +
			s5k5cag_reg_pat[S_RES_PREVIEW].blk_l;

		ll_pck = s5k5cag_reg_pat[S_RES_PREVIEW].size_w +
			s5k5cag_reg_pat[S_RES_PREVIEW].blk_p;

	} else {

		fl_lines = s5k5cag_reg_pat[S_RES_CAPTURE].size_h +
			s5k5cag_reg_pat[S_RES_CAPTURE].blk_l;

		ll_pck = s5k5cag_reg_pat[S_RES_CAPTURE].size_w +
			s5k5cag_reg_pat[S_RES_CAPTURE].blk_p;
	}

	if (gain > max_legal_gain)
		gain = max_legal_gain;

	/* in Q10 */
	line = (line * s5k5cag_ctrl->fps_divider);

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
	rc = s5k5cag_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));
	if (rc < 0)
		goto write_gain_done;

	ll_pck = ll_pck * ll_ratio;
	ll_pck_msb = ((ll_pck / 0x400) & 0xFF00) >> 8;
	ll_pck_lsb = (ll_pck / 0x400) & 0x00FF;
	tbl[0].waddr = REG_LINE_LENGTH_PCK_MSB;
	tbl[0].bdata = ll_pck_msb;
	tbl[1].waddr = REG_LINE_LENGTH_PCK_LSB;
	tbl[1].bdata = ll_pck_lsb;
	rc = s5k5cag_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));
	if (rc < 0)
		goto write_gain_done;

	line = line / ll_ratio;
	intg_t_msb = (line & 0xFF00) >> 8;
	intg_t_lsb = (line & 0x00FF);
	tbl[0].waddr = REG_COARSE_INTEGRATION_TIME;
	tbl[0].bdata = intg_t_msb;
	tbl[1].waddr = REG_COARSE_INTEGRATION_TIME_LSB;
	tbl[1].bdata = intg_t_lsb;
	rc = s5k5cag_i2c_write_table(&tbl[0], ARRAY_SIZE(tbl));
#endif
write_gain_done:
	return rc;
}

static int32_t s5k5cag_set_pict_exp_gain(uint16_t gain, uint32_t line)
{
	int32_t rc = 0;

	CDBG("Line:%d s5k5cag_set_pict_exp_gain \n", __LINE__);

	rc =
		s5k5cag_write_exp_gain(gain, line);

	return rc;
}

static int32_t s5k5cag_video_config(int mode, int res)
{
	int32_t rc = 0;

	switch (res) {
	case S_QTR_SIZE:
		rc = s5k5cag_setting(S_UPDATE_PERIODIC, S_RES_PREVIEW);
		if (rc < 0)
			return rc;

		CDBG("s5k5cag sensor configuration done!\n");
		break;

	case S_FULL_SIZE:
		rc = s5k5cag_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
		if (rc < 0)
			return rc;

		break;

	default:
		return 0;
	} /* switch */

	s5k5cag_ctrl->prev_res = res;
	s5k5cag_ctrl->curr_res = res;
	s5k5cag_ctrl->sensormode = mode;

	rc =
		s5k5cag_write_exp_gain(s5k5cag_ctrl->my_reg_gain,
			s5k5cag_ctrl->my_reg_line_count);

	return rc;
}

static int32_t s5k5cag_snapshot_config(int mode)
{
	int32_t rc = 0;

	rc = s5k5cag_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
	if (rc < 0)
		return rc;

	s5k5cag_ctrl->curr_res = s5k5cag_ctrl->pict_res;
	s5k5cag_ctrl->sensormode = mode;

	return rc;
}

static int32_t s5k5cag_raw_snapshot_config(int mode)
{
	int32_t rc = 0;

	rc = s5k5cag_setting(S_UPDATE_PERIODIC, S_RES_CAPTURE);
	if (rc < 0)
		return rc;

	s5k5cag_ctrl->curr_res = s5k5cag_ctrl->pict_res;
	s5k5cag_ctrl->sensormode = mode;

	return rc;
}

static int32_t s5k5cag_set_sensor_mode(int mode, int res)
{
	int32_t rc = 0;

	switch (mode) {
	case SENSOR_PREVIEW_MODE:
		rc = s5k5cag_video_config(mode, res);
		break;

	case SENSOR_SNAPSHOT_MODE:
		rc = s5k5cag_snapshot_config(mode);
		break;

	case SENSOR_RAW_SNAPSHOT_MODE:
		rc = s5k5cag_raw_snapshot_config(mode);
		break;

	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int32_t s5k5cag_set_default_focus(void)
{
	int32_t rc = 0;

  rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
		0x3131, 0);
	if (rc < 0)
		return rc;

  rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
		0x3132, 0);
	if (rc < 0)
		return rc;

	s5k5cag_ctrl->curr_lens_pos = 0;

	return rc;
}

static int32_t s5k5cag_move_focus(int direction, int32_t num_steps)
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
		CDBG("s5k5cag_move_focus failed at line %d ...\n", __LINE__);
		return -EINVAL;
	}

	actual_step = step_direction * (int16_t)num_steps;
	pos_offset = init_code + s5k5cag_ctrl->curr_lens_pos;
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

		rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
			0x3131, next_pos_msb);
		if (rc < 0)
			break;

		rc = s5k5cag_i2c_write_b(s5k5cag_client->addr,
			0x3132, next_pos_lsb);
		if (rc < 0)
			break;

		pos_offset = next_pos;
		s5k5cag_ctrl->curr_lens_pos = pos_offset - init_code;
		if (i < 4)
			mdelay(3);
	}

	return rc;
}

static int s5k5cag_sensor_config(void __user *argp)
{
	struct sensor_cfg_data cdata;
	long   rc = 0;

	if (copy_from_user(&cdata,
			(void *)argp,
			sizeof(struct sensor_cfg_data)))
		return -EFAULT;

	mutex_lock(&s5k5cag_mutex);

	CDBG("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
	printk("%s: cfgtype = %d\n", __func__, cdata.cfgtype);
	switch (cdata.cfgtype) {
	case CFG_SET_FPS:
	case CFG_SET_PICT_FPS:
		rc = s5k5cag_set_fps(&(cdata.cfg.fps));
		break;

	case CFG_SET_EXP_GAIN:
		rc =
			s5k5cag_write_exp_gain(cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
		break;

	case CFG_SET_PICT_EXP_GAIN:
		CDBG("Line:%d CFG_SET_PICT_EXP_GAIN \n", __LINE__);
		rc =
			s5k5cag_set_pict_exp_gain(
				cdata.cfg.exp_gain.gain,
				cdata.cfg.exp_gain.line);
		break;

	case CFG_SET_MODE:
		rc =
			s5k5cag_set_sensor_mode(
			cdata.mode, cdata.rs);
		break;

	case CFG_PWR_DOWN:
		rc = s5k5cag_power_down();
		break;

	case CFG_SET_EFFECT:
		rc = s5k5cag_set_effect(
					cdata.cfg.effect);
		break;

	case CFG_SET_WB:
		rc = s5k5cag_set_wb(
					cdata.cfg.wb);
		break;
			
	case CFG_SET_BRIGHTNESS:
 		rc = s5k5cag_set_brightness(
					cdata.cfg.brightness);
		break;

	case CFG_SET_ANTIBANDING:
		rc = s5k5cag_set_antibanding(
					cdata.cfg.antibanding);
		break;	

	case CFG_SET_CONTRAST:
		rc = s5k5cag_set_contrast(
					cdata.cfg.contrast);
		break;	

	case CFG_SET_ISO:
		rc = s5k5cag_set_iso(
					cdata.cfg.iso);
		break;	

	case CFG_SET_EXPOSURE_MODE:
		rc = s5k5cag_set_exposure_mode(
					cdata.cfg.exposure_mode);
		break;	

	case CFG_GET_AE_STATUS:
		{
			uint8_t	*sdata = &cdata.cfg.ae_fps;
			rc = s5k5cag_get_ae_status(sdata);
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

	mutex_unlock(&s5k5cag_mutex);
	return rc;
}

static int s5k5cag_sensor_probe(const struct msm_camera_sensor_info *info,
		struct msm_sensor_ctrl *s)
{
	int rc = 0;

	/* pull down power-down */
	rc = gpio_request(info->sensor_pwd, "s5k5cag");
	printk("[camera] gpio_request pwd low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(info->sensor_pwd, 0);
	else printk(KERN_ERR "s5k5cag error: request gpio %d failed: "
			"%d\n", info->sensor_pwd, rc);

	/* pull down reset */
	rc = gpio_request(info->sensor_reset, "s5k5cag");
	printk("[camera] gpio_request reset low=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(info->sensor_reset, 0);
	else printk(KERN_ERR "s5k5cag error: request gpio %d failed: "
			"%d\n", info->sensor_reset, rc);

	mdelay(20);

	s5k5cag_power_enable();

	rc = i2c_add_driver(&s5k5cag_i2c_driver);
	if (rc < 0 || s5k5cag_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_fail;
	}

	mdelay(20);

	/* enable clk */
	msm_camio_clk_enable(CAMIO_VFE_CLK);
	printk("[camera] enable clk\n");

	msm_camio_clk_rate_set(S5K5CAG_DEFAULT_CLOCK_RATE);

	/* pull up power-down */
	rc = gpio_request(info->sensor_pwd, "s5k5cag");
	printk("[camera] gpio_request pwd high=%d\n", rc);
	if (!rc || rc == -EBUSY)
		gpio_direction_output(info->sensor_pwd, 1);
	else printk(KERN_ERR "s5k5cag error: request gpio %d failed: "
			"%d\n", info->sensor_pwd, rc);
	mdelay(40);

	rc = s5k5cag_probe_init_sensor(info);
	printk("[camera] s5k5cag_probe_init_sensor=%d\n", rc);
	if (rc < 0)
		goto probe_fail;

	s->s_init = s5k5cag_sensor_open_init;
	s->s_release = s5k5cag_sensor_release;
	s->s_config  = s5k5cag_sensor_config;
	s5k5cag_probe_init_done(info);

	/*disable clk*/
	msm_camio_clk_disable(CAMIO_VFE_CLK);
	printk("[camera] disable clk\n");

	mdelay(20);

	s5k5cag_power_disable();

	return rc;

probe_fail:
	CDBG("SENSOR PROBE FAILS!\n");

	/*disable clk*/
	msm_camio_clk_disable(CAMIO_VFE_CLK);
	printk("[camera] disable clk\n");

	mdelay(20);

	s5k5cag_power_disable();

	return rc;
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
	cci_smem_value_t *smem_cci_smem_value;
	smem_cci_smem_value = smem_alloc( SMEM_CCI_SMEM_VALUE, sizeof( cci_smem_value_t ));
    	printk("#### cci_project_id=%d ####\n", smem_cci_smem_value->cci_project_id);
	if(smem_cci_smem_value->cci_project_id == PROJECT_ID_K4H)
    	return 0;	
    	
	return platform_driver_register(&msm_camera_driver);
}

module_init(s5k5cag_init);

