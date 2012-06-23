/*
#include "mach/../../cci_smem.h"
#include "mach/../../smd_private.h"
#include <mach/msm_battery.h>
cci_smem_value_t *smem_cci_smem_value;

#ifdef CONFIG_FB_MSM_LCDC
// LCDC interface
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/earlysuspend.h>
#include <mach/gpio.h>
#include <mach/pmic.h>
#include "msm_fb.h"
#include <mach/vreg.h>

#ifdef CONFIG_FB_MSM_TRY_MDDI_CATCH_LCDC_PRISM
#include "mddihosti.h"
#endif

#define LCDC_VSYNC 		127
#define LCDC_HSYNC		128
#define LCDC_DEN  		129
#define LCDC_PCLK  		

#define LCDC_RED_0  	// EBI2_ADR_12
#define LCDC_RED_1  	// EBI2_ADR_13
#define LCDC_RED_2  	// EBI2_ADR_14
#define LCDC_RED_3  	// EBI2_ADR_15
#define LCDC_RED_4  	// EBI2_ADR_16
#define LCDC_RED_5  98
#define LCDC_RED_6  99
#define LCDC_RED_7  100

#define LCDC_GREEN_0	112
#define LCDC_GREEN_1	111
#define LCDC_GREEN_2	121
#define LCDC_GREEN_3	120
#define LCDC_GREEN_4	119
#define LCDC_GREEN_5	130
#define LCDC_GREEN_6	// EBI2_ADR_11
#define LCDC_GREEN_7	// EBI2_ADR_10

#define LCDC_BLUE_0		126
#define LCDC_BLUE_1		125
#define LCDC_BLUE_2		118
#define LCDC_BLUE_3		117
#define LCDC_BLUE_4		116
#define LCDC_BLUE_5		115
#define LCDC_BLUE_6		114
#define LCDC_BLUE_7		113

#define SPI_SDO     102
#define SPI_CLK     132
#define SPI_CS      131

#define GPIO_LOW_VALUE 0
#define GPIO_HIGH_VALUE 1

static int spi_cs;
static int spi_sclk;
static int spi_miso;
static int lcd_reset;
static int lcd_bl_en;
static int ret_volume_up;
static int ret_volume_down;

static struct early_suspend lcdc_early;
static struct early_suspend mddi_early;

extern bool BKL_EN;

struct toshiba_state_type{
	boolean disp_initialized;
	boolean display_on;
	boolean disp_powered_up;
};

static uint32_t gpio_table[] = {
	GPIO_CFG(LCDC_RED_5, 3, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(LCDC_RED_6, 3, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(LCDC_RED_7, 3, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
};

static struct toshiba_state_type toshiba_state = { 0 };
static struct msm_panel_common_pdata *lcdc_toshiba_pdata;

static void spi_pin_assign(void)
{
	printk(KERN_INFO "[John]%s: \r\n", __func__);
	// Setting the Default GPIO's
	spi_sclk 	= 132;
	spi_cs   	= 131;
	spi_miso  = 102;
	lcd_reset = 89;
	if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
		lcd_bl_en = 23;
	else
		lcd_bl_en = 32;
}

static void toshiba_disp_powerup(void)
{
	int rc;
	struct vreg *vreg_rfrx2 = NULL;
	struct vreg *vreg_gp5 = NULL;

	printk(KERN_INFO "[John]%s: \r\n", __func__);
	if (!toshiba_state.disp_powered_up && !toshiba_state.display_on)
	{
		// Reset the hardware first
		// Include DAC power up implementation here
	  toshiba_state.disp_powered_up = TRUE;
	}

  if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
  {
		vreg_gp5 = vreg_get(0, "gp5");
		rc = vreg_set_level(vreg_gp5, 2600);
		if (rc) 
		{
			printk(KERN_INFO "%s: vreg set gp5 level failed (%d)\r\n",	__func__, rc);
		}
		rc = vreg_enable(vreg_gp5);
		if (rc) 
		{
			printk(KERN_INFO "%s: vreg enable gp5 failed (%d)\r\n", __func__, rc);
		}

		mdelay(10);

		vreg_rfrx2 = vreg_get(0, "rfrx2");
		rc = vreg_set_level(vreg_rfrx2, 2800);
		if (rc) 
		{
			printk(KERN_INFO "%s: vreg set rfrx2 level failed (%d)\r\n", __func__, rc);
		}
		rc = vreg_enable(vreg_rfrx2);
		if (rc) 
		{
			printk(KERN_INFO "%s: vreg enable gp2 failed (%d)\r\n", __func__, rc);
		}

		printk(KERN_INFO "[John]%s: K5\r\n", __func__);
	}
	else
	{
		vreg_rfrx2 = vreg_get(0, "rfrx2");
		rc = vreg_set_level(vreg_rfrx2, 2800);
		if (rc) 
		{
			printk(KERN_INFO "%s: vreg set rfrx2 level failed (%d)\r\n", __func__, rc);
		}
		rc = vreg_enable(vreg_rfrx2);
		if (rc) 
		{
			printk(KERN_INFO "%s: vreg enable gp2 failed (%d)\r\n", __func__, rc);
		}

		mdelay(10);

		vreg_gp5 = vreg_get(0, "gp5");
		rc = vreg_set_level(vreg_gp5, 2600);
		if (rc) 
		{
			printk(KERN_INFO "%s: vreg set gp5 level failed (%d)\r\n",	__func__, rc);
		}
		rc = vreg_enable(vreg_gp5);
		if (rc) 
		{
			printk(KERN_INFO "%s: vreg enable gp5 failed (%d)\r\n", __func__, rc);
		}

		printk(KERN_INFO "[John]%s: K4&CAP8\r\n", __func__);
	}
}

void spi_write(char data)
{
	int i = 7;

	int mask[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	for(; i >= 0; i--) //turn it the clock time
	{
		gpio_set_value(SPI_CLK, GPIO_LOW_VALUE);

		gpio_set_value(SPI_SDO, ((data & mask[i]) >> i));
		udelay(1);
		
		gpio_set_value(SPI_CLK, GPIO_HIGH_VALUE);
		udelay(1);
	}
}

void spi_write_k5(int dcx, char data)
{
	int i = 7;
	int mask[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	// D/CX
	gpio_set_value(SPI_CLK, GPIO_LOW_VALUE);

	gpio_set_value(SPI_SDO, dcx);
	udelay(1);

	gpio_set_value(SPI_CLK, GPIO_HIGH_VALUE);
	udelay(1);

	for(; i >= 0; i--) //turn it the clock time
	{
		gpio_set_value(SPI_CLK, GPIO_LOW_VALUE);

		gpio_set_value(SPI_SDO, ((data & mask[i]) >> i));
		udelay(1);

		gpio_set_value(SPI_CLK, GPIO_HIGH_VALUE);
		udelay(1);
	}
}

static void toshiba_disp_on(void)
{
	//int i;
	printk(KERN_INFO "[John]%s: \r\n", __func__);

	if(smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
	{
		printk(KERN_INFO "[John]%s: K5\r\n", __func__);
		
		gpio_set_value(lcd_reset, 1);
	  mdelay(10);
	  gpio_set_value(lcd_reset, 0);
	  mdelay(1);
	  gpio_set_value(lcd_reset, 1);
	  mdelay(100);
	}
	else
	{
		printk(KERN_INFO "[John]%s: K4&CAP8\r\n", __func__);
		
		gpio_set_value(lcd_reset, 1);
		mdelay(10);
		gpio_set_value(lcd_reset, 0);
		mdelay(1);
		gpio_set_value(lcd_reset, 1);
	  mdelay(50);
	}

	// In future, this will remove
	gpio_set_value(lcd_bl_en, 1);

	gpio_set_value(spi_cs, 1);		// high
	gpio_set_value(spi_sclk, 1);	// high
	gpio_set_value(spi_miso, 0);

	if (toshiba_state.disp_powered_up && !toshiba_state.display_on)
	{
			switch(smem_cci_smem_value->cci_project_id)
			{
				case PROJECT_ID_K4:
				case PROJECT_ID_CAP8:
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x01);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x00);//1
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x02);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x02);
		spi_write(0x00);//2
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x03);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x10);
		//spi_write(0x90);//3
		// For LCDC display tearing issue.
		// Aaron 2010.09.10
		spi_write(0x30);//3
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x08);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x02);
		spi_write(0x07);//4
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x10);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x00);//5
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x11);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x07);//6
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x12);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x00);//7
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x13);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x00);//8
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x07);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x01);//9
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
		mdelay(200);
		
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x0c);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x10);//10
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x0f);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x1b);//11
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x10);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x11);
		spi_write(0x90);//12
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x11);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x02);
		spi_write(0x27);//13
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		mdelay(50);

	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x12);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		// Solved the RF issue.
		//spi_write(0x8a);//14
		spi_write(0x08);//14
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		mdelay(50);

	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x13);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x17);
		spi_write(0x00);//15
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x29);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x19);//16
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0xb1);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x80);//17
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0xb3);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x2c);//18
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x30);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x00);//19
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x31);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x07);
		spi_write(0x07);//20
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x32);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x06);//21
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x35);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x04);
		spi_write(0x03);//22
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x36);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x04);//23
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x37);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x07);//24
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x38);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x00);//25
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x39);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x07);
		spi_write(0x07);//26
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x3c);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x03);
		spi_write(0x04);//27
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x3d);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x04);//28
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x50);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x00);//29
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x51);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		if (!ret_volume_down)
		{
			spi_write(0xdf);//30
		}
		else
		{
			spi_write(0xef);//30
		}
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x52);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x00);//31
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x53);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x3f);//32
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x60);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		//spi_write(0x27);
		// For LCDC display tearing issue.
		// Aaron 2010.09.10
		spi_write(0xA7);
		spi_write(0x00);//33
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x61);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x01);//34
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x07);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x01);//35
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x07);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x21);//36
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x07);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x23);//37
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
		mdelay(5);
		
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x07);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x33);//38
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	        
		mdelay(5);
		
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0xa2);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x00);
		spi_write(0x01);//39
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x22);//40
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
#if 0		
		for(i=0;i<240*3;i++)
		{
	        gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x72);
			spi_write(0xf8);
			spi_write(0x00);
	        gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		}
		
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x0c);// 000cV-ram   
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
	    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x72);
		spi_write(0x01);
		spi_write(0x10);//10; 0100 RGB
		gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		
		gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write(0x70);
		spi_write(0x00);
		spi_write(0x22);//37
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
#endif
						break;
				case PROJECT_ID_K5:
				default:
	  gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0x01);//01
	  gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		mdelay(120);

	  gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0xb9);//02
		spi_write_k5(1, 0xff);
		spi_write_k5(1, 0x83);
		spi_write_k5(1, 0x68);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0xb6);//03
		spi_write_k5(1, 0x80);
		spi_write_k5(1, 0x81);
		spi_write_k5(1, 0x76);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0xb1);//04
		spi_write_k5(1, 0x00);
		spi_write_k5(1, 0x01);
		spi_write_k5(1, 0x1e);
		spi_write_k5(1, 0x00);
		spi_write_k5(1, 0x22);
		spi_write_k5(1, 0x11);
		spi_write_k5(1, 0x8d);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0xe0);//05
		spi_write_k5(1, 0x00);
		spi_write_k5(1, 0x2b);
		spi_write_k5(1, 0x2a);
		spi_write_k5(1, 0x39);
		spi_write_k5(1, 0x33);
		spi_write_k5(1, 0x39);
		spi_write_k5(1, 0x38);
		spi_write_k5(1, 0x7f);
		spi_write_k5(1, 0x0c);
		spi_write_k5(1, 0x05);
		spi_write_k5(1, 0x09);
		spi_write_k5(1, 0x0f);
		spi_write_k5(1, 0x1a);
		spi_write_k5(1, 0x06);
		spi_write_k5(1, 0x0c);
		spi_write_k5(1, 0x06);
		spi_write_k5(1, 0x15);
		spi_write_k5(1, 0x14);
		spi_write_k5(1, 0x3f);
		spi_write_k5(1, 0x00);
		spi_write_k5(1, 0x47);
		spi_write_k5(1, 0x05);
		spi_write_k5(1, 0x10);
		spi_write_k5(1, 0x16);
		spi_write_k5(1, 0x1a);
		spi_write_k5(1, 0x13);
		spi_write_k5(1, 0x00);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0xcc);//06
		spi_write_k5(1, 0x0f);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0x51);//07
		spi_write_k5(1, 0x80);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0x53);//08
		spi_write_k5(1, 0x24);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0x55);//09
		spi_write_k5(1, 0x01);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0x11);//10
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		mdelay(120);

		gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0xea);//11
		spi_write_k5(1, 0x00);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0xb3);//12
		spi_write_k5(1, 0x0e);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		// Rotate screen
		gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0x36);//13
		spi_write_k5(1, 0xc0);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		// Set PWM frequence = 10k
		gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0xc9);//14
		spi_write_k5(1, 0x0f);
		spi_write_k5(1, 0x6e);
		spi_write_k5(1, 0x00);
		spi_write_k5(1, 0x00);
		spi_write_k5(1, 0x20);
		spi_write_k5(1, 0x00);
		spi_write_k5(1, 0x01);
		spi_write_k5(1, 0x20);
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

    gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
		spi_write_k5(0, 0x29);//15
    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		mdelay(5);
					break;
		}

		toshiba_state.display_on = TRUE;
	}
}

static int lcdc_toshiba_panel_on(struct platform_device *pdev)
{
	printk(KERN_INFO "[John]%s: \r\n", __func__);
	BKL_EN=1;
	if (!toshiba_state.disp_initialized) {
		// Configure reset GPIO that drives DAC
		toshiba_disp_powerup();
		toshiba_disp_on();
		toshiba_state.disp_initialized = TRUE;
	}
	return 0;
}

static int lcdc_toshiba_panel_off(struct platform_device *pdev)
{
	int rc;
	struct vreg *vreg_rfrx2 = NULL;
	struct vreg *vreg_gp5 = NULL;	
	BKL_EN=0;
	printk(KERN_INFO "[John]%s: \r\n", __func__);
	if (toshiba_state.disp_powered_up && toshiba_state.display_on)
	{
		// Main panel power off (Deep standby in)
		if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
		{
			gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write_k5(0, 0x28);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

			mdelay(40);

			gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write_k5(0, 0x10);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		}
		else
		{
			// Backlight control
		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x70);
			spi_write(0x00);
			spi_write(0xb3);
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x72);
			spi_write(0x00);
			spi_write(0x00);
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

			mdelay(10);

			// Power off sequence
		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x70);
			spi_write(0x00);
			spi_write(0x07);
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x72);
			spi_write(0x01);
			spi_write(0x31); // 1
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

			mdelay(40);

		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x70);
			spi_write(0x00);
			spi_write(0x07);
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x72);
			spi_write(0x01);
			spi_write(0x30); // 2
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

			mdelay(40);

		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x70);
			spi_write(0x00);
			spi_write(0x07);
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x72);
			spi_write(0x00);
			spi_write(0x00); // 3
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x70);
			spi_write(0x00);
			spi_write(0x10);
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		       gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x72);
			spi_write(0x00);
			spi_write(0x00); // 4
		       gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		}

		mdelay(5);

		gpio_set_value(spi_cs, 0);
		gpio_set_value(spi_sclk, 0);
		gpio_set_value(spi_miso, 0);

		// In the future, this will remove
		// Display Backlight off
		gpio_set_value(32, GPIO_LOW_VALUE);

		gpio_set_value(lcd_reset, 0);

		mdelay(5);

		if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
		{
			vreg_rfrx2 = vreg_get(0, "rfrx2");
			rc = vreg_disable(vreg_rfrx2);
			if (rc)
			{
				printk("%s: vreg disable rfrx2 failed (%d)\r\n", __func__, rc);
			}

			mdelay(10);

			vreg_gp5 = vreg_get(0, "gp5");
			rc = vreg_disable(vreg_gp5);
			if (rc) 
			{
				printk("%s: vreg disable gp5 failed (%d)\r\n", __func__, rc);
			}
		}
		else
		{
			vreg_gp5 = vreg_get(0, "gp5");
			rc = vreg_disable(vreg_gp5);
			if (rc) 
			{
				printk("%s: vreg disable gp5 failed (%d)\r\n", __func__, rc);
			}

			mdelay(10);

			vreg_rfrx2 = vreg_get(0, "rfrx2");
			rc = vreg_disable(vreg_rfrx2);
			if (rc)
			{
				printk("%s: vreg disable rfrx2 failed (%d)\r\n", __func__, rc);
			}
		}

		toshiba_state.display_on = FALSE;
		toshiba_state.disp_initialized = FALSE;
	}
	return 0;
}

static void lcdc_toshiba_set_backlight(struct msm_fb_data_type *mfd)
{
	static int enable_pwm = 1;
	int bl_level;
	int max = 255;
	int min = 31;

	bl_level = mfd->bl_level;
	if(bl_level > max)
		bl_level = max;
	else if (bl_level == 0)
		bl_level = 0;
	else if ((bl_level < min) && (bl_level != 0))
		bl_level = 8;
	// Dark 0~ light 255

	// Modify 2010/08/16 Battery device status update (rang 0~255)
        cci_batt_device_status_update(0xE0,0);
        if (bl_level >= max)
           cci_batt_device_status_update(CCI_BATT_DEVICE_ON_LCD_BACKLIGHT_150,1);
        else if (bl_level >= 160)
           cci_batt_device_status_update(CCI_BATT_DEVICE_ON_LCD_BACKLIGHT_100,1);
        else if (bl_level >= min)
           cci_batt_device_status_update(CCI_BATT_DEVICE_ON_LCD_BACKLIGHT_50,1);

	if (toshiba_state.disp_initialized)
	{
		printk(KERN_INFO "[John]%s: mfd->bl_level=%d bl_level=%d\r\n", __func__, mfd->bl_level, bl_level);

		if ((enable_pwm == 0) && (bl_level > 0))
		{
		  if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
		  {
		  	gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
				spi_write_k5(0, 0x53);
				spi_write_k5(1, 0x24);
    		gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		  }
		  else
		  {
			  gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
				spi_write(0x70);
				spi_write(0x00);
				spi_write(0xb3);
			  gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
			  gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
				spi_write(0x72);
				spi_write(0x00);
				spi_write(0x2c);
			  gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
			}
		  enable_pwm = 1;
		}
		else if ((enable_pwm == 1) && (bl_level == 0))
		{
		  if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
		  {
		  	gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
				spi_write_k5(0, 0x53);
				spi_write_k5(1, 0x00);
    		gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		  }
		  else
		  {
				gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
				spi_write(0x70);
				spi_write(0x00);
				spi_write(0xb3);
			  gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
			  gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
				spi_write(0x72);
				spi_write(0x00);
				spi_write(0x00);
			  gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
			}
		  enable_pwm = 0;
		}

		// Write the display brightness
		if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
		{
			gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write_k5(0, 0x51);
			spi_write_k5(1, bl_level);
	    gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		}
		else
		{
		  gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x70);
			spi_write(0x00);
			spi_write(0xb1);
		  gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End
		  gpio_set_value(SPI_CS,GPIO_LOW_VALUE);//Start
			spi_write(0x72);
			spi_write(0x00);
			spi_write(bl_level);
		  gpio_set_value(SPI_CS,GPIO_HIGH_VALUE);//End

			mdelay(1);
		}
	}
}

void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

static int __init toshiba_probe(struct platform_device *pdev)
{
	printk(KERN_INFO "[John]%s: \r\n", __func__);

	spi_pin_assign();
	if (pdev->id == 0) {
		lcdc_toshiba_pdata = pdev->dev.platform_data;
		spi_pin_assign();
		return 0;
	}

	msm_fb_add_device(pdev);
	return 0;
}

static int toshiba_suspend(struct platform_device *pdev)
{
	int rc;
	struct vreg *vreg_rfrx2 = NULL;
	struct vreg *vreg_gp5 = NULL;
	
	//printk("[Aaron][%s]\r\n", __func__);
	
	gpio_set_value(lcd_reset, 0);
	gpio_set_value(lcd_bl_en, 0);

	if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K5)
	{
		vreg_rfrx2 = vreg_get(0, "rfrx2");
		rc = vreg_disable(vreg_rfrx2);
		if (rc)
		{
			printk("%s: vreg disable rfrx2 failed (%d)\r\n", __func__, rc);
		}

		mdelay(10);

		vreg_gp5 = vreg_get(0, "gp5");
		rc = vreg_disable(vreg_gp5);
		if (rc) 
		{
			printk("%s: vreg disable gp5 failed (%d)\r\n", __func__, rc);
		}
	}
	else
	{
		vreg_gp5 = vreg_get(0, "gp5");
		rc = vreg_disable(vreg_gp5);
		if (rc) 
		{
			printk("%s: vreg disable gp5 failed (%d)\r\n", __func__, rc);
		}

		mdelay(10);

		vreg_rfrx2 = vreg_get(0, "rfrx2");
		rc = vreg_disable(vreg_rfrx2);
		if (rc)
		{
			printk("%s: vreg disable rfrx2 failed (%d)\r\n", __func__, rc);
		}
	}
	
	return 0;
}

static int toshiba_early_suspend(struct platform_device *pdev)
{	
	//printk("[Aaron][%s]\r\n", __func__);

	return 0;
}

static int toshiba_later_resume(struct platform_device *pdev)
{	
	//printk("[Aaron][%s]\r\n", __func__);

	return 0;
}

static int toshiba_resume(struct platform_device *pdev)
{	
	//printk("[Aaron][%s]\r\n", __func__);
	
	gpio_set_value(lcd_reset, 1);
	
	return 0;
}

static struct platform_driver this_lcdc_driver = {
	.probe  = toshiba_probe,
	.suspend = toshiba_suspend,
	.resume = toshiba_resume,
	.driver = {
		.name   = "lcdc_toshiba_wvga",
	},
};

static struct msm_fb_panel_data toshiba_panel_data = {
	.on = lcdc_toshiba_panel_on,
	.off = lcdc_toshiba_panel_off,
	.set_backlight = lcdc_toshiba_set_backlight,
};

static struct platform_device this_lcdc_device = {
	.name   = "lcdc_toshiba_wvga",
	.id	= 1,
	.dev	= {
		.platform_data = &toshiba_panel_data,
	}
};

static int __init lcdc_toshiba_panel_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;
#ifdef CONFIG_FB_MSM_TRY_MDDI_CATCH_LCDC_PRISM
	if (mddi_get_client_id() != 0)
		return 0;

	ret = msm_fb_detect_client("lcdc_toshiba_wvga_pt");
	if (ret)
		return 0;
#endif

	// Set the LCDC_MUX_CTL and MDP_LCDC_EN to 1. All gpio can be auto-configured,
	// except the gpio98~100. So configure them to LCDC_RED5~7 ...
	config_gpio_table(gpio_table, ARRAY_SIZE(gpio_table));

	printk(KERN_INFO "[John]%s: \r\n", __func__);
	
	ret = platform_driver_register(&this_lcdc_driver);
	if (ret)
		return ret;

	printk(KERN_INFO "[John]%s: platform_driver_register(&this_lcdc_driver) OK\r\n",__func__);

	pinfo = &toshiba_panel_data.panel_info;
	
	ret_volume_up = gpio_get_value(42);
	printk(KERN_INFO "[John][%s] GPIO_42(Volumn Up)=%d\r\n", __func__, ret_volume_up);
	ret_volume_down = gpio_get_value(41);
	printk(KERN_INFO "[John][%s] GPIO_41(Volumn Down)=%d\r\n", __func__, ret_volume_down);

	switch(smem_cci_smem_value->cci_project_id)
	{
		case PROJECT_ID_K4:
		case PROJECT_ID_CAP8:
				if (!ret_volume_down)
				{
					pinfo->xres = 224;
					pinfo->yres = 320;
				}
				else
				{
					pinfo->xres = 240;
					pinfo->yres = 320;
				}
				break;
		case PROJECT_ID_K5:
		default:
				pinfo->xres = 320;
				pinfo->yres = 240;
				break;
	}

	pinfo->type = LCDC_PANEL;
	pinfo->pdest = DISPLAY_1;
	pinfo->wait_cycle = 0;
	pinfo->bpp = 18;
	pinfo->fb_num = 2;
	// 30Mhz mdp_lcdc_pclk and mdp_lcdc_pad_pcl
	pinfo->clk_rate = 6144000;
	pinfo->bl_max = 255;
	pinfo->bl_min = 0;

	pinfo->lcdc.h_back_porch = 3;	// hsw = 8 + hbp=184
	pinfo->lcdc.h_front_porch = 3;
	pinfo->lcdc.h_pulse_width = 5;
	pinfo->lcdc.v_back_porch = 7;	// vsw=1 + vbp = 2
	pinfo->lcdc.v_front_porch = 2;
	pinfo->lcdc.v_pulse_width = 3;
	pinfo->lcdc.border_clr = 0;     // blk
	pinfo->lcdc.underflow_clr = 0xff;       // blue
	pinfo->lcdc.hsync_skew = 5;

	ret = platform_device_register(&this_lcdc_device);
	if (ret)
		platform_driver_unregister(&this_lcdc_driver);

	lcdc_early.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	lcdc_early.suspend = toshiba_early_suspend;
	lcdc_early.resume = toshiba_later_resume;
	register_early_suspend(&lcdc_early);

	printk(KERN_INFO "[John]%s: platform_device_register(&this_lcdc_device) OK\r\n",__func__);

	return ret;
}
#endif // CONFIG_FB_MSM_LCDC
*/

#include "mach/../../cci_smem.h"
#include "mach/../../smd_private.h"
#include <mach/msm_battery.h>
cci_smem_value_t *smem_cci_smem_value;

//#ifdef CONFIG_FB_MSM_MDDI
// MDDI interface
#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include <asm/gpio.h>
#include <mach/msm_rpcrouter.h>
#include <mach/vreg.h>
#include <mach/mpp.h>
#include <linux/leds.h>
#include <linux/mutex.h>
#include <linux/i2c.h>

#define CCI_AB60_HVGA_PRIM 	1
#define LCD_BLK_EN 		32
#define LCD_RST_N 		89
#define LCD_ID0  			20
#define LCD_RST_N_113 113

#define START_DISABLE_HIBERNATION 0
#define STOP_DISABLE_HIBERNATION 1
#define DISABLE_HIBERNATION_THRESHOLD 200

#define BLK_UI_MAX_VAL	255
#define BLK_UI_MIN_VAL	30
#define BLK_LEVEL_NUM	5

typedef enum 
{
   	LCD_TYPE_AUO = 0,
  	LCD_TYPE_TPO,
  	LCD_TYPE_INVALID = 0xFF
} E_LCD_TYPE;

#define ENABLE_DEBUG 		1
#define ENABLE_DEBUG_FUNCTION 	0
#define TPO_DISABLE_HIBERNATION_FUNCTION	1

#if ENABLE_DEBUG
	#define MSG_DBG(format,args...) do { \
				printk(KERN_INFO "[LCD] %s():%d: " format, __func__, __LINE__, ##args); \
			} while(0)
	#define MSG_ERR(format,args...) do { \
				printk(KERN_ERR "[LCD][ERROR] %s():%d: " format, __func__, __LINE__, ##args); \
			} while(0)			
#if ENABLE_DEBUG_FUNCTION
	#define MSG_FUN_ENTER	printk(KERN_INFO "[LCD] %s()+: %d\r\n", __func__, __LINE__)
	#define MSG_FUN_EXIT	printk(KERN_INFO "[LCD] %s()-: %d\r\n", __func__, __LINE__)
#else
	#define MSG_FUN_ENTER	do { } while(0)
	#define MSG_FUN_EXIT	do { } while(0)
#endif

#else
	#define MSG_DBG(format,args...) do { } while(0)
	#define MSG_ERR(format,args...) do { \
				printk(KERN_ERR "[LCD][ERROR] %s():%d: " format, __func__, __LINE__, ##args); \
			} while(0)	
	#define MSG_FUN_ENTER	do { } while(0)
	#define MSG_FUN_EXIT	do { } while(0)		
#endif

static int panel_type_number = -1;
module_param_named( panel_type, panel_type_number, int, S_IRUGO | S_IWUSR | S_IWGRP);

/*******************************************************************************
 * * Local Variable Declaration
 * *******************************************************************************/
static uint32 mddi_cci_ab60_rows_per_second = 13830;	// 5200000/376
static uint32 mddi_cci_ab60_rows_per_refresh = 338;
static uint32 mddi_cci_ab60_usecs_per_refresh = 24440;	// (376+338)/5200000
static boolean mddi_cci_ab60_debug_60hz_refresh = FALSE;
//static int backlight_val = 128;
E_LCD_TYPE g_lcd_type = LCD_TYPE_INVALID;
//static cci_hw_id_type hw_type = HW_ID_EMU;
#if TPO_DISABLE_HIBERNATION_FUNCTION
static boolean g_disable_hibernation_state = FALSE;
#endif
static boolean blk_on_timer_initialized = FALSE;
boolean LCD_powered = TRUE;

//static unsigned char blk_ui_range[BLK_LEVEL_NUM];
//static unsigned char other_blk_val_transform_table[BLK_LEVEL_NUM] =  { 30, 55, 90, 165, 255 };
//static unsigned char auo_blk_val_transform_table_dvt2[BLK_LEVEL_NUM] =  { 30, 55, 90, 165, 255 };
//static unsigned char auo_blk_val_transform_table_pvt[BLK_LEVEL_NUM]  =  {  5, 22, 50, 135, 255 };
//static unsigned char tpo_blk_val_transform_table_dvt2[BLK_LEVEL_NUM] =  { 30, 70, 105, 180, 255 };
//static unsigned char tpo_blk_val_transform_table_pvt[BLK_LEVEL_NUM]  =  { 5, 30, 65, 130, 255 };
//static bool b_blk_transform_init = FALSE;
//static unsigned char *p_blk_val_transform;

struct timer_list tpo_update_timer;
struct timer_list blk_on_timer;
static DECLARE_MUTEX(tpo_timer_mutex);
static DECLARE_MUTEX(blk_on_timer_mutex);

/*******************************************************************************
 * * Local Function Declaration
 * *******************************************************************************/
static void mddi_cci_ab60_prim_lcd_init(void);
static void mddi_cci_ab60_lcd_set_backlight(struct msm_fb_data_type *mfd);
static struct msm_panel_common_pdata *mddi_cci_ab60_pdata;
boolean write_client_reg_TPO(uint32 reg_addr,uint8 *reg_Value, mddih_host_data_packet_size_type reg_packet_size, boolean wait);
static void mddi_cci_ab60_set_backlight_level(int value);
//static void set_lcd_backlight(struct led_classdev *led_dev, enum led_brightness value);

/*******************************************************************************
 * * External Variable Declaration
 * *******************************************************************************/
extern uint32 mddi_host_core_version;
/*******************************************************************************
 * * External Function Declaration
 * *******************************************************************************/
/*
void blk_value_transform_init(void)
{
	int i, blk_ui_gap;

	blk_ui_range[0] = BLK_UI_MIN_VAL;
	blk_ui_range[BLK_LEVEL_NUM-1] = BLK_UI_MAX_VAL;

	blk_ui_gap = (BLK_UI_MAX_VAL - BLK_UI_MIN_VAL)/(BLK_LEVEL_NUM-1);		
	for(i=1; i<(BLK_LEVEL_NUM-1); i++ )
		blk_ui_range[i] = blk_ui_range[i-1] + blk_ui_gap;

	if( hw_type > HW_ID_DVT2 )
	{
		switch( g_lcd_type )
		{
			case LCD_TYPE_TPO:
				MSG_DBG("[BLK][PVT][TPO]\n");
				p_blk_val_transform = tpo_blk_val_transform_table_pvt;
				break;
			case LCD_TYPE_AUO:
				MSG_DBG("[BLK][PVT][AUO]\n");
				p_blk_val_transform = auo_blk_val_transform_table_pvt;
				break;	
			default:
				MSG_ERR("[BLK][PVT]LCD type: INVALID!\n");
				p_blk_val_transform = other_blk_val_transform_table;
				break;
		}		
	}
	else
	{
		switch( g_lcd_type )
		{
			case LCD_TYPE_TPO:
				MSG_DBG("[BLK][TPO]\n");
				p_blk_val_transform = tpo_blk_val_transform_table_dvt2;
				break;
			case LCD_TYPE_AUO:
				MSG_DBG("[BLK][AUO]\n");
				p_blk_val_transform = auo_blk_val_transform_table_dvt2;
				break;	
			default:
				MSG_ERR("[BLK]LCD type: INVALID!\n");
				p_blk_val_transform = other_blk_val_transform_table;
				break;
		}			
	}			

	b_blk_transform_init = TRUE;
}

unsigned char blk_value_transform(unsigned char ui_val)
{
	int i;
	unsigned char transform_val = 0;
	
	if(!b_blk_transform_init)
		blk_value_transform_init();

	if( ui_val <= blk_ui_range[0] )
		transform_val = *p_blk_val_transform;
	else if( ui_val >= blk_ui_range[BLK_LEVEL_NUM-1] )
		transform_val = *(p_blk_val_transform + BLK_LEVEL_NUM-1);
	else
	{
		for(i=0; i<(BLK_LEVEL_NUM-1); i++ )
			if( ui_val > blk_ui_range[i] && ui_val <= blk_ui_range[i+1] )
			{
				transform_val = ( *(p_blk_val_transform +i +1 ) - *(p_blk_val_transform +i) ) * (ui_val-blk_ui_range[i]) / (blk_ui_range[i+1] - blk_ui_range[i]) + *(p_blk_val_transform +i);
				break;
			}
	}
	return transform_val;
}
*/
void backlight_on( boolean bEnable )
{
	if(bEnable)
	{
		MSG_DBG("John Backlight on!\r\n");
		gpio_direction_output(LCD_BLK_EN,1);
	}
	else
	{
		MSG_DBG("John Backlight off!\r\n");
		gpio_direction_output(LCD_BLK_EN,0);
	}
}

void blk_on_timer_handler(unsigned long data)
{
	MSG_FUN_ENTER;

	blk_on_timer_initialized = FALSE;

	if( LCD_powered )
		backlight_on(TRUE);

	MSG_FUN_EXIT;	
}

void blk_on_proc(void)
{
	down(&blk_on_timer_mutex);

	if( !blk_on_timer_initialized )
	{
		init_timer(&blk_on_timer);
		blk_on_timer.function = blk_on_timer_handler;
		blk_on_timer.expires = jiffies + (HZ/100 * 30);		
		add_timer(&blk_on_timer);	
		blk_on_timer_initialized = TRUE;
	}	
	else
	{
		blk_on_timer.function = blk_on_timer_handler;
		mod_timer(&blk_on_timer, jiffies + (HZ/100 * 30));
	}

	up(&blk_on_timer_mutex);
}

#if TPO_DISABLE_HIBERNATION_FUNCTION
void disable_hibernation_timer_handler(unsigned long data)
{
	boolean bEnable = (boolean)data;
	
	MSG_FUN_ENTER;
	
	if( bEnable )
	{
		if (!g_disable_hibernation_state )
		{
			mddi_host_disable_hibernation(TRUE);
			g_disable_hibernation_state = TRUE;
		}
		tpo_update_timer.data = FALSE;
		mod_timer(&tpo_update_timer, jiffies + (HZ*2));				
	}
	else
	{
		if ( g_disable_hibernation_state )
		{
			mddi_host_disable_hibernation(FALSE);
			g_disable_hibernation_state = FALSE;
		}
	}
	
	MSG_FUN_EXIT;	
}
#endif
/*
void disable_hibernation_proc(boolean bEnable)
{
#if TPO_DISABLE_HIBERNATION_FUNCTION

	static bool timer_init = FALSE;

	if( hw_type >= HW_ID_PVT )
		return;
		
	if( bEnable && backlight_val < DISABLE_HIBERNATION_THRESHOLD  )
		return;
	
	if( !timer_init )
	{
		init_timer(&tpo_update_timer);
		tpo_update_timer.function = disable_hibernation_timer_handler;
		tpo_update_timer.data = (unsigned long)bEnable;
		tpo_update_timer.expires = jiffies + (HZ/1000);
		add_timer(&tpo_update_timer);	
		timer_init = TRUE;
	}
	else
	{
		tpo_update_timer.data = (unsigned long)bEnable;
		mod_timer(&tpo_update_timer, jiffies + (HZ/1000));
	}
#endif
}
*/
static void mddi_cci_ab60_set_backlight_level(int value)
{
	static int enable_pwm = 1;
	//int dvt_or_evt = 2;
	int err = 0;
	//int value_tmp = 0;
	uint8  RegPrm[5];
	//int max;
	//int min;

	MSG_FUN_ENTER;
	if ( !LCD_powered )
		return;
   
	if ( value > 255 )
		value = 255;
    
	if ((enable_pwm == 0) && (value > 0))
	{
		RegPrm[0]	 = 0x2c;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x003d, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
	    enable_pwm = 1;
	}
	else if ((enable_pwm == 1) && (value <= 0))
	{
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x003d, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
	    enable_pwm = 0;
	}

	RegPrm[0]	 = value;
	RegPrm[1]	 = 0x00;
	RegPrm[2]	 = 0x00;
	RegPrm[3]	 = 0x00;
	err = write_client_reg_TPO(0x003c, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
    
	mdelay(10);

	MSG_FUN_EXIT;
}
/*
static void set_lcd_backlight(struct led_classdev *led_dev, enum led_brightness value)
{
}
*/
boolean write_client_reg_TPO(uint32 reg_addr,uint8 *reg_Value, mddih_host_data_packet_size_type reg_packet_size, boolean wait)
{
	mddih_reg_write_type_uint8   reg_write;
	reg_write.addr  = reg_addr;
	reg_write.pValue = reg_Value;
	reg_write.packet_size = reg_packet_size;
	return(boolean) mddi_queue_multi_register_write_uint8(reg_write, wait, 0);
}

static void auo_lcd_power_on_sequence(void)
{
}

static void tpo_lcd_power_on_sequence(void)
{
	int err=0, retry=0;

	MSG_FUN_ENTER;
	gpio_direction_output(LCD_RST_N, 0);
/*
	if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K4H)
		if ((smem_cci_smem_value->cci_hw_id == HW_ID_DVT2) ||
			  (smem_cci_smem_value->cci_hw_id > HW_ID_DVT2))
			gpio_direction_output(LCD_RST_N_113, 0);
*/

	msleep(10);	// Delay 10ms
	gpio_direction_output(LCD_RST_N, 1);
/*
	if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K4H)
		if ((smem_cci_smem_value->cci_hw_id == HW_ID_DVT2) ||
			  	(smem_cci_smem_value->cci_hw_id > HW_ID_DVT2))
			gpio_direction_output(LCD_RST_N_113, 1);
*/

	msleep(60);	// Delay 60ms

restart:
	retry++;
	err = 0;
	{
		uint8 RegPrm[5];

		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 1
		err = write_client_reg_TPO(0x00ff, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		// Rotation register
		//if ((smem_cci_smem_value->cci_project_id == PROJECT_ID_CAP6) ||
		//		(smem_cci_smem_value->cci_project_id == PROJECT_ID_CAP2))
			RegPrm[0]	 = 0x08;
		//else
		//	RegPrm[0]	 = 0x0b;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 2
		err = write_client_reg_TPO(0x0016, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 3
		err = write_client_reg_TPO(0x00e2, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 4
		err = write_client_reg_TPO(0x00e3, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 5
		err = write_client_reg_TPO(0x00f2, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x1c;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 6
		err = write_client_reg_TPO(0x00e4, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x1c;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 7
		err = write_client_reg_TPO(0x00e5, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 8
		err = write_client_reg_TPO(0x00e6, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x1c;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 9
		err = write_client_reg_TPO(0x00e7, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x01;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 10
		err = write_client_reg_TPO(0x0019, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(10);
		
		RegPrm[0]	 = 0x01;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 11
		err = write_client_reg_TPO(0x0029, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x22;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 12
		err = write_client_reg_TPO(0x0018, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 13
		err = write_client_reg_TPO(0x002a, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x13;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 14
		err = write_client_reg_TPO(0x002b, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 15
		err = write_client_reg_TPO(0x0002, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 16
		err = write_client_reg_TPO(0x0003, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x01;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 17
		err = write_client_reg_TPO(0x0004, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x3f;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 18
		err = write_client_reg_TPO(0x0005, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 19
		err = write_client_reg_TPO(0x0006, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 20
		err = write_client_reg_TPO(0x0007, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x01;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 21
		err = write_client_reg_TPO(0x0008, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0xdf;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 22
		err = write_client_reg_TPO(0x0009, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x92;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 23
		err = write_client_reg_TPO(0x0024, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x73;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 24
		err = write_client_reg_TPO(0x0025, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x30;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 25
		err = write_client_reg_TPO(0x001b, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(10);
		
		RegPrm[0]	 = 0x22;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 26
		err = write_client_reg_TPO(0x001d, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(10);
		
		RegPrm[0]	 = 0x01;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 27
		err = write_client_reg_TPO(0x0040, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x3b;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0041, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x39;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0042, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x35;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0043, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x2e;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0044, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x3e;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0045, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x3a;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0046, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x7f;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0047, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x0b;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0048, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x05;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0049, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x06;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x004a, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x0f;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x004b, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x1f;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x004c, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x01;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0050, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x11;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0051, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x0a;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0052, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x06;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0053, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x04;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0054, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x3e;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0055, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0056, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x45;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0057, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0058, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x10;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x0059, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x19;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x005a, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x1a;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x005b, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x14;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x005c, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0xc0;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x005d, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x02;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 28
		err = write_client_reg_TPO(0x0001, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x05;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 29
		err = write_client_reg_TPO(0x001a, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x03;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 30
		err = write_client_reg_TPO(0x001c, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(10);
		
		RegPrm[0]	 = 0x88;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 31
		err = write_client_reg_TPO(0x001f, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(10);
		
		RegPrm[0]	 = 0x80;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 32
		err = write_client_reg_TPO(0x001f, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(10);
			
		RegPrm[0]	 = 0x90;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 33
		err = write_client_reg_TPO(0x001f, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(10);
		
		RegPrm[0]	 = 0xd2;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 34
		err = write_client_reg_TPO(0x001f, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(10);

		// Clear Gram to 0x0
		RegPrm[0]	 = 0x02;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 34 -01
		err = write_client_reg_TPO(0x00ff, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

		RegPrm[0]	 = 0x03;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 34 -02
		err = write_client_reg_TPO(0x0017, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

		msleep(5);

		RegPrm[0]	 = 0x01;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 34 -03
		err = write_client_reg_TPO(0x0017, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 34 -04
		err = write_client_reg_TPO(0x00ff, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

		RegPrm[0]	 = 0x38;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 35
		err = write_client_reg_TPO(0x0028, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(40);
	
		RegPrm[0]	 = 0x3c;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 36
		err = write_client_reg_TPO(0x0028, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		msleep(40);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 37
		err = write_client_reg_TPO(0x0080, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 38
		err = write_client_reg_TPO(0x0081, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 39
		err = write_client_reg_TPO(0x0082, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x00;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 40
		err = write_client_reg_TPO(0x0083, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x06;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 41
		err = write_client_reg_TPO(0x0017, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x1f;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 42
		err = write_client_reg_TPO(0x002d, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x08;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 43
		err = write_client_reg_TPO(0x0060, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
		
		RegPrm[0]	 = 0x90;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00; // 44
		err = write_client_reg_TPO(0x00e8, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
/*
		if ((smem_cci_smem_value->cci_project_id == PROJECT_ID_K4H))
		{
			if ((smem_cci_smem_value->cci_hw_id == HW_ID_DVT1) ||
			  	(smem_cci_smem_value->cci_hw_id > HW_ID_DVT1))
			{
				// Set the frequence of PWM = 12k
				RegPrm[0]	 = 0x01;
				RegPrm[1]	 = 0x00;
				RegPrm[2]	 = 0x00;
				RegPrm[3]	 = 0x00;
				err = write_client_reg_TPO(0x00ff, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

				RegPrm[0]	 = 0x01;
				RegPrm[1]	 = 0x00;
				RegPrm[2]	 = 0x00;
				RegPrm[3]	 = 0x00;
				err = write_client_reg_TPO(0x00c5, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

				RegPrm[0]	 = 0x00;
				RegPrm[1]	 = 0x00;
				RegPrm[2]	 = 0x00;
				RegPrm[3]	 = 0x00;
				err = write_client_reg_TPO(0x00ff, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

				printk(KERN_INFO "[John]%s: K4H DVT\r\n", __func__);
			}
		}
*/

        if(strstr(smem_cci_smem_value->cci_mid,"CA22")) 
        {
    		// Set the frequence of PWM = 24k
    		RegPrm[0]	 = 0x01;
    		RegPrm[1]	 = 0x00;
    		RegPrm[2]	 = 0x00;
    		RegPrm[3]	 = 0x00;
    		err = write_client_reg_TPO(0x00ff, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

    		RegPrm[0]	 = 0x00;
    		RegPrm[1]	 = 0x00;
    		RegPrm[2]	 = 0x00;
    		RegPrm[3]	 = 0x00;
    		err = write_client_reg_TPO(0x00c5, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

            RegPrm[0]	 = 0x0f;
    		RegPrm[1]	 = 0x00;
    		RegPrm[2]	 = 0x00;
    		RegPrm[3]	 = 0x00;
    		err = write_client_reg_TPO(0x00c3, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);


    		RegPrm[0]	 = 0x00;
    		RegPrm[1]	 = 0x00;
    		RegPrm[2]	 = 0x00;
    		RegPrm[3]	 = 0x00;
    		err = write_client_reg_TPO(0x00ff, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
        }
		printk(KERN_INFO "[John]%s: CA23 PVT\r\n", __func__);
        
		// Backlight control enable
		RegPrm[0]	 = 0x2c;
		RegPrm[1]	 = 0x00;
		RegPrm[2]	 = 0x00;
		RegPrm[3]	 = 0x00;
		err = write_client_reg_TPO(0x003d, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);
	}
	
	if( (retry <= 3) && ((err == 1)) ) {
		gpio_direction_output(LCD_RST_N, 0);
/*
		if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K4H)
			if ((smem_cci_smem_value->cci_hw_id == HW_ID_DVT2) ||
			  	(smem_cci_smem_value->cci_hw_id > HW_ID_DVT2))
				gpio_direction_output(LCD_RST_N_113, 0);
*/
		msleep(1);	// Delay 1ms
		MSG_ERR("goto restart\r\n");
		goto restart;
	}
	
	MSG_FUN_EXIT;
}

static void auo_lcd_power_off_sequence(void)
{
}

static void tpo_lcd_power_off_sequence(void)
{
	int rc;
	struct vreg *vreg_rfrx2 = NULL;
	struct vreg *vreg_gp5 = NULL;
	int  err=0;
	uint8 RegPrm[5];

	MSG_FUN_ENTER;

	RegPrm[0]	 = 0x38;
	RegPrm[1]	 = 0x00;
	RegPrm[2]	 = 0x00;
	RegPrm[3]	 = 0x00; // 1
	err = write_client_reg_TPO(0x0028, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

	msleep(40);

	RegPrm[0]	 = 0x24;
	RegPrm[1]	 = 0x00;
	RegPrm[2]	 = 0x00;
	RegPrm[3]	 = 0x00; // 2
	err = write_client_reg_TPO(0x0028, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

	msleep(40);

	RegPrm[0]	 = 0x04;
	RegPrm[1]	 = 0x00;
	RegPrm[2]	 = 0x00;
	RegPrm[3]	 = 0x00; // 3
	err = write_client_reg_TPO(0x0028, RegPrm, MDDI_DATA_PACKET_SIZE_4_BYTES, TRUE);

	gpio_direction_output(LCD_RST_N, 0);
/*
	if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K4H)
		if ((smem_cci_smem_value->cci_hw_id == HW_ID_DVT2) ||
			  	(smem_cci_smem_value->cci_hw_id > HW_ID_DVT2))
			gpio_direction_output(LCD_RST_N_113, 0);
*/

	vreg_gp5 = vreg_get(0, "gp5");
	rc = vreg_disable(vreg_gp5);
	if (rc)
	{
		printk("%s: vreg enable gp5 failed (%d)\r\n", __func__, rc);
	}

	vreg_rfrx2 = vreg_get(0, "rfrx2");	
	rc = vreg_disable(vreg_rfrx2);
	if (rc)
	{
		printk("%s: vreg enable rfrx2 failed (%d)\r\n", __func__, rc);
	}

	MSG_FUN_EXIT;
}

static void lcd_power_on_init(void)
{
	int rc;
	struct vreg *vreg_rfrx2 = NULL;
	struct vreg *vreg_gp5 = NULL;

	MSG_DBG("John lcd_power_on_init 1\r\n");

	vreg_rfrx2 = vreg_get(0, "rfrx2");

	rc = vreg_enable(vreg_rfrx2);
	rc = vreg_set_level(vreg_rfrx2, 2800);
	if (rc) 
	{
		printk(KERN_INFO "%s: vreg set rfrx2 level failed (%d)\r\n", __func__, rc);
	}
	if (rc) 
	{
		MSG_ERR("%s: vreg enable gp2 failed (%d)\r\n", __func__, rc);
	}

	vreg_gp5 = vreg_get(0, "gp5");
	//if ((smem_cci_smem_value->cci_project_id == PROJECT_ID_CAP6) ||
	//	  (smem_cci_smem_value->cci_project_id == PROJECT_ID_CAP2))
		rc = vreg_set_level(vreg_gp5, 2600);
	//else
	//	rc = vreg_set_level(vreg_gp5, 1800);
	if (rc) 
	{
		printk(KERN_INFO "%s: vreg set gp5 level failed (%d)\r\n",	__func__, rc);
	}
	rc = vreg_enable(vreg_gp5);
	if (rc) 
	{
		MSG_ERR("%s: vreg enable gp5 failed (%d)\r\n", __func__, rc);
	}
	MSG_DBG("John lcd_power_on_init 2\r\n");
}

static void mddi_cci_ab60_power_on_sequence(void)
{
	MSG_FUN_ENTER;

	switch( g_lcd_type )
	{
		case LCD_TYPE_TPO:
			MSG_DBG("John mddi_cci_ab60_power_on_sequence LCD_TYPE_TPO\r\n");
			tpo_lcd_power_on_sequence();
			backlight_on(TRUE);
			break;
		case LCD_TYPE_AUO:
			MSG_DBG("John mddi_cci_ab60_power_on_sequence LCD_TYPE_AUO\r\n");
			auo_lcd_power_on_sequence();
			backlight_on(TRUE);
			break;	
		default:
			MSG_ERR("LCD type: INVALID!\n");
			break;
	}

	MSG_FUN_EXIT;
}

#if 1
static void mddi_cci_ab60_lcd_set_backlight(struct msm_fb_data_type *mfd)
{
	//unsigned char transform_val;

	MSG_FUN_ENTER;

	//MSG_DBG("########  mfd->bl_level = %d ############## \n", mfd->bl_level);
/*
	transform_val = blk_value_transform((unsigned char)mfd->bl_level);
	backlight_val = 0xFF - transform_val;
	//MSG_DBG(" mfd->bl_level=%d, transform_val=%d, backlight_val=%d ############## \n", mfd->bl_level, transform_val, backlight_val);
	mddi_cci_ab60_set_backlight_level(backlight_val);
*/
	mddi_cci_ab60_set_backlight_level(mfd->bl_level);
//#endif
/*	
	backlight_val =255 - (mfd->bl_level & 0xFF);
	mddi_cci_ab60_set_backlight_level(backlight_val);
*/	

	MSG_FUN_EXIT;	
}
#else

static DEFINE_MUTEX(key_light_mutex);
static void mddi_cci_ab60_lcd_set_backlight(struct msm_fb_data_type *mfd)
{
	int32 level=0x0;
        //CB60_SCR_4644 SOF
        u8 buf[10];
        //unsigned i = 0;

	MSG_FUN_ENTER;

	memset(buf, 0X0, sizeof(buf));
        //CB60_SCR_4644 EOF
	MSG_DBG("######## Config LCM_BL_EN GPIO 91 level = %d ############## \n", mfd->bl_level);

	level = mfd->bl_level;

	//james@cci added for backlight control
	if(level == 7 ) {
		backlight_on(TRUE);
		MSG_DBG("%s LCD_BLK_EN = %d\n",__func__,gpio_get_value(LCD_BLK_EN));
		MSG_DBG("do backlighton\n");
	}
	//james@cci added for backlight control
	if(level == 0) {//change to 1
		backlight_on(FALSE);
		MSG_DBG("%s LCD_BLK_EN = %d\n",__func__,gpio_get_value(LCD_BLK_EN));
                //CB60_SCR_4644 SOF
mutex_lock(&key_light_mutex);
/*
                //del_timer(&keylight_timer);
		i = 0;
		hrtimer_cancel(&led_timer);
	        do{
                buf[i] = wled_off[i];
		buf[i] = LED_CURRENT_STATE[i];
                i++;
               }while(i<10);
               i2c_master_send(TCA6507_i2c_client, buf, 10);  
		keylight_flag = 0 ;
*/
mutex_unlock(&key_light_mutex);
                //CB60_SCR_4644 EOF
		MSG_DBG("do backlightoff\n");
	}

	MSG_FUN_EXIT;
}

#endif

static void mddi_cci_ab60_prim_lcd_init(void)
{
	MSG_FUN_ENTER;

	lcd_power_on_init();

	if( !LCD_powered )
	{
		MSG_DBG("John mddi_cci_ab60_prim_lcd_init 1\r\n");
		LCD_powered = TRUE;
		mddi_cci_ab60_power_on_sequence();
	}
	else
	{
		MSG_DBG("John mddi_cci_ab60_prim_lcd_init 2\r\n");
 	}

	/* Set the MDP pixel data attributes for Primary Display */
	mddi_host_write_pix_attr_reg(MDDI_DEFAULT_PRIM_PIX_ATTR);  	

	MSG_FUN_EXIT;
}

static int mddi_cci_ab60_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	MSG_FUN_ENTER;
	//BKL_EN=1;
	mfd = platform_get_drvdata(pdev);
	if (!mfd)
	{
		MSG_ERR("mfd is NULL.\r\n");
		MSG_FUN_EXIT;
		return -ENODEV;
	}

	if (mfd->key != MFD_KEY)
	{
		MSG_ERR("mfd->key != MFD_KEY.\r\n");
		MSG_FUN_EXIT;		
		return -EINVAL;
	}

	if (mfd->panel.id == CCI_AB60_HVGA_PRIM)
		mddi_cci_ab60_prim_lcd_init();
	else
	{
		MSG_ERR(KERN_ALERT "%s:%d --> Unknown LCD Panel id = %d, return -EINVAL\r\n", __func__, __LINE__, mfd->panel.id);
		return -EINVAL;
	}

	MSG_FUN_EXIT;	
	return 0;
}

static int mddi_cci_ab60_lcd_off(struct platform_device *pdev)
{
	MSG_FUN_ENTER;
	LCD_powered = FALSE;
	//BKL_EN=0;
	switch( g_lcd_type )
	{
		case LCD_TYPE_TPO:
			backlight_on(FALSE);
			tpo_lcd_power_off_sequence();
			break;
		case LCD_TYPE_AUO:
			backlight_on(FALSE);
			auo_lcd_power_off_sequence();
			break;	
		default:
			MSG_ERR("LCD type: INVALID!\r\n");
			break;
	}

	//disable_hibernation_proc(FALSE);

	MSG_FUN_EXIT;
	return 0;
}

static E_LCD_TYPE get_lcd_type(void)
{
/*
	int lcd_id;
	E_LCD_TYPE lcd_type;
	
	gpio_direction_input(LCD_ID0);
	lcd_id = gpio_get_value(LCD_ID0);
	
	switch(lcd_id)
	{
		case 0:
			lcd_type = LCD_TYPE_AUO;
			break;
		case 1:
			lcd_type = LCD_TYPE_TPO;
			break;				
		default:
			lcd_type = LCD_TYPE_INVALID;
			break;
	}
	return lcd_type;
*/
	return LCD_TYPE_TPO;
//#endif
}

static int __init mddi_cci_ab60_probe(struct platform_device *pdev)
{
	MSG_FUN_ENTER;

	if (pdev->id == 0) {
		MSG_DBG("%s id == 0\r\n", __func__);
		mddi_cci_ab60_pdata = pdev->dev.platform_data;
		return 0;
	}	

	g_lcd_type = get_lcd_type();

	MSG_DBG("%s g_lcd_type=%d\r\n", __func__, g_lcd_type);
	panel_type_number = (int)g_lcd_type;
	
	msm_fb_add_device(pdev);

	MSG_FUN_EXIT;
	return 0;
}
/*
static int mddi_cci_ab60_suspend(struct platform_device *pdev)
{
	int rc;
	struct vreg *vreg_rfrx2 = NULL;
	struct vreg *vreg_gp5 = NULL;
	
	//printk("[Aaron][%s]\r\n", __func__);
	
	gpio_set_value(LCD_RST_N, 0);
	gpio_set_value(LCD_BLK_EN, 0);

	vreg_gp5 = vreg_get(0, "gp5");
	rc = vreg_disable(vreg_gp5);
	if (rc)
	{
		printk("%s: vreg enable gp5 failed (%d)\r\n", __func__, rc);
	}

	vreg_rfrx2 = vreg_get(0, "rfrx2");	
	rc = vreg_disable(vreg_rfrx2);
	if (rc)
	{
		printk("%s: vreg enable rfrx2 failed (%d)\r\n", __func__, rc);
	}

	return 0;
}
*/
/*
static int mddi_cci_ab60_early_suspend(struct platform_device *pdev)
{
	//printk("[Aaron][%s]\r\n", __func__);

	return 0;
}
*/
/*
static int mddi_cci_ab60_later_resume(struct platform_device *pdev)
{
	//printk("[Aaron][%s]\r\n", __func__);

	return 0;
}
*/
static int mddi_cci_ab60_resume(struct platform_device *pdev)
{
	//printk("[Aaron][%s]\r\n", __func__);
	
	gpio_set_value(LCD_RST_N, 1);
	
	return 0;
}
/*
static struct led_classdev lcd_backlight = {
	.name = "bkl",
	.brightness_set = set_lcd_backlight,
	.default_trigger = "charger",
};
*/
static struct platform_driver this_mddi_driver = {
	.probe  = mddi_cci_ab60_probe,
	//.suspend = mddi_cci_ab60_suspend,
	.resume = mddi_cci_ab60_resume,
	.driver = {
		.name   = "mddi_cci_ab60_wvga",
	},
};

static struct msm_fb_panel_data mddi_cci_ab60_panel_data = {
	.on = mddi_cci_ab60_lcd_on,
	.off = mddi_cci_ab60_lcd_off,
	.set_backlight = mddi_cci_ab60_lcd_set_backlight,
	.set_vsync_notifier = NULL,
};

static struct platform_device this_mddi_device = {
	.name   = "mddi_cci_ab60_wvga",
	.id	= CCI_AB60_HVGA_PRIM,
	.dev	= {
		.platform_data = &mddi_cci_ab60_panel_data,
	}
};

static int __init mddi_cci_ab60_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

#ifdef CONFIG_FB_MSM_MDDI_AUTO_DETECT
	u32 id;
	id = mddi_get_client_id();

	if (((id >> 16) != 0x0) || ((id & 0xffff) != 0x8835))
		return 0;
#endif
	MSG_FUN_ENTER;

	if (gpio_request(LCD_RST_N, "lcd_rst_n"))
		MSG_ERR("failed to request gpio lcd_rst_n\n");
	//gpio_configure(LCD_RST_N,GPIOF_DRIVE_OUTPUT);
	MSG_DBG("%s LCD_RST_N = %d\r\n",__func__,gpio_get_value(LCD_RST_N));
/*
	if (smem_cci_smem_value->cci_project_id == PROJECT_ID_K4H)
	{
		if ((smem_cci_smem_value->cci_hw_id == HW_ID_DVT2) ||
			  (smem_cci_smem_value->cci_hw_id > HW_ID_DVT2))
		{
				gpio_configure(LCD_RST_N_113,GPIOF_DRIVE_OUTPUT);
				MSG_DBG("%s LCD_RST_N_113 = %d\r\n",__func__,gpio_get_value(LCD_RST_N_113));
		}
	}
*/
	if (gpio_request(LCD_BLK_EN, "lcd_drv_en"))
		MSG_ERR("failed to request gpio lcd_drv_en\n");
	//gpio_configure(LCD_BLK_EN,GPIOF_DRIVE_OUTPUT);
	MSG_DBG("%s LCD_BLK_EN = %d\r\n",__func__,gpio_get_value(LCD_BLK_EN));

	if (mddi_host_core_version > 8) {
		/* can use faster refresh with newer hw revisions */
		mddi_cci_ab60_debug_60hz_refresh = TRUE;

		/* Timing variables for tracking vsync */
		/* dot_clock = 6.00MHz
		 * horizontal count = 296
		 * vertical count = 338
		 * refresh rate = 6000000/(296+338) = 60Hz
		 */
		mddi_cci_ab60_rows_per_second = 20270;	/* 6000000/296 */
		mddi_cci_ab60_rows_per_refresh = 338;
		mddi_cci_ab60_usecs_per_refresh = 16674;	/* (296+338)/6000000 */
		MSG_DBG("##### %s mddi_host_core_version > 8 #####\n",__func__);
	} else {
		/* Timing variables for tracking vsync */
		/* dot_clock = 5.20MHz
		 * horizontal count = 376
		 * vertical count = 338
		 * refresh rate = 5200000/(376+338) = 41Hz
		 */
		mddi_cci_ab60_rows_per_second = 13830;	/* 5200000/376 */
		mddi_cci_ab60_rows_per_refresh = 338;
		mddi_cci_ab60_usecs_per_refresh = 24440;	/* (376+338)/5200000 */

	}
	ret = platform_driver_register(&this_mddi_driver);
	if (ret)
	{
		MSG_ERR("platform_driver_register return fail.\n");
		MSG_FUN_EXIT;
		return ret;
	}

	pinfo = &mddi_cci_ab60_panel_data.panel_info;
	pinfo->xres = 320;
	pinfo->yres = 480;
	pinfo->type = MDDI_PANEL;
	pinfo->pdest = DISPLAY_1;
	pinfo->mddi.vdopkt = MDDI_DEFAULT_PRIM_PIX_ATTR;
	pinfo->wait_cycle = 0;
	pinfo->bpp = 16;
	pinfo->fb_num = 2;
	pinfo->clk_rate =    81920000;
	pinfo->clk_min =     80000000;
	pinfo->clk_max =    100000000;
	pinfo->lcd.vsync_enable = FALSE;//TRUE;
	pinfo->lcd.refx100 =
		(mddi_cci_ab60_rows_per_second * 100) / mddi_cci_ab60_rows_per_refresh;
	pinfo->lcd.v_back_porch = 8;
	pinfo->lcd.v_front_porch = 8;
	pinfo->lcd.v_pulse_width = 0;
	pinfo->lcd.hw_vsync_mode = FALSE;//TRUE;
	pinfo->lcd.vsync_notifier_period = 0;//(1 * HZ);
	pinfo->bl_max = 255;
	pinfo->bl_min = 0;

	ret = platform_device_register(&this_mddi_device);
	if (ret)
		platform_driver_unregister(&this_mddi_driver);

	//led_classdev_register(NULL, &lcd_backlight);
	//mddi_early.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	//mddi_early.suspend = mddi_cci_ab60_early_suspend;
	//mddi_early.resume = mddi_cci_ab60_later_resume;
	//register_early_suspend(&mddi_early);

	MSG_FUN_EXIT;
	return ret;
}
//#endif // CONFIG_FB_MSM_MDDI

static int __init mddi_lcdc_cci_init(void)
{
	int ret;

	smem_cci_smem_value = smem_alloc( SMEM_CCI_SMEM_VALUE, sizeof( cci_smem_value_t ));

    if(smem_cci_smem_value==0)
    {
        printk("can not get the smem_cci_smem_value!!!!!!!!!!!\n");
       // return -1;
    }
/*
	if(smem_cci_smem_value != 0)
	{
		switch(smem_cci_smem_value->cci_project_id)
		{
			case PROJECT_ID_K4H:
			case PROJECT_ID_CAP6:
			case PROJECT_ID_CAP2:
#ifdef CONFIG_FB_MSM_MDDI
*/
					ret = mddi_cci_ab60_init();
					printk(KERN_INFO "[John] mddi_lcdc_cci_init: MDDI ret=%d\r\n", ret);
/*
#endif // CONFIG_FB_MSM_MDDI
					break;
			case PROJECT_ID_K4:
			case PROJECT_ID_CAP8:
			case PROJECT_ID_K5:
			default:
#ifdef CONFIG_FB_MSM_LCDC
					ret = lcdc_toshiba_panel_init();
					printk(KERN_INFO "[John] mddi_lcdc_cci_init: LCDC ret=%d\r\n", ret);
#endif // CONFIG_FB_MSM_LCDC
					break;
		}
	}
	else
	{
#ifdef CONFIG_FB_MSM_LCDC
			ret = lcdc_toshiba_panel_init();
			printk(KERN_INFO "[John] mddi_lcdc_cci_init: LCDC default.\r\n");
#endif // CONFIG_FB_MSM_LCDC
	}
*/
	return ret;
}

module_init(mddi_lcdc_cci_init);
