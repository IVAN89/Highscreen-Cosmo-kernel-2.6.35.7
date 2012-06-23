//S5K5CAGX.ini
//www.dothinkey.com
//Preview Type:0:Raw 10 bit; 1:Raw 8 bit; 2:YUV422
//I2C Mode    :0:Normal 8Addr,8Data;  1:Samsung 8 Addr,8Data; 2:Micron 8 Addr,16Data
//I2C Mode    :3:Stmicro 16Addr,8Data;4:Micron2 16 Addr,16Data
//Out Format  :0:YCbYCr/RG_GB; 1:YCrYCb/GR_BG; 2:CbYCrY/GB_RG; 3:CrYCbY/BG_GR
//MCLK Speed  :0:12M; 1:24M; 2:48M
/*
[SENSOR]
Preview Width   = 2048
Preview Height  = 1536
Preview Type    = 2
Sensor Name     = S5K5CAGX
Reset Pin       = 1
Pwdn Pin        = 1
I2C Mode        = 4
I2C Slave ID    = 0x78
I2C Flag Reg    = 0x1006
I2C Flag Value  = 0x5ca
I2C Flag Mask   = 0xffff
I2C Flag Reg1   = 0x2
I2C Flag Value1 = 0x0
I2C Flag Mask1  = 0x0
Out Format      = 0x2

MCLK Speed      = 0x0
*/
\
#ifndef CAMSENSOR_S5K5CAG
#define CAMSENSOR_S5K5CAG

#include <mach/board.h>

struct cam_preview{
   int cnt;
   u8 data[2048];
};

/*
static u32 preview_s5k5cag_reg[][2]=
{
//#if 0
{0xFCFC,0xD000},
{0x0010,0x0001},	// Reset
{0x1030,0x0000},	// Clear host interrupt so main will wait
{0x0014,0x0001},	// ARM go
{0x0028,0xD000},	
{0x002A,0x1082},	
{0x0F12,0x03ff},//0155	// [9:8] D4, [7:6] D3, [5:4] D2, [3:2] D1, [1:0] D0
{0x0F12,0x03ff},//0155	// [9:8] D9, [7:6] D8, [5:4] D7, [3:2] D6, [1:0] D5
{0x0F12,0x1555},	// [5:4] GPIO3, [3:2] GPIO2, [1:0] GPIO1
{0x0F12,0x03ff},//05d5	// [11:10] SDA, [9:8] SCA, [7:6] PCLK, [3:2] VSYNC, [1:0] HSYNC    		
{0x0028,0x7000},	// Start T&P part 
{0x002A,0x2CF8},
{0x0F12,0xB510},
{0x0F12,0x490F},
{0x0F12,0x2000},
{0x0F12,0x8048},
{0x0F12,0x8088},
{0x0F12,0x490E},
{0x0F12,0x480E},
{0x0F12,0xF000},
{0x0F12,0xF949},
{0x0F12,0x490E},
{0x0F12,0x480E},
{0x0F12,0x6341},
{0x0F12,0x490E},
{0x0F12,0x38C0},
{0x0F12,0x63C1},
{0x0F12,0x490E},
{0x0F12,0x6301},
{0x0F12,0x490E},
{0x0F12,0x3040},
{0x0F12,0x6181},
{0x0F12,0x490D},
{0x0F12,0x480E},
{0x0F12,0xF000},
{0x0F12,0xF93A},
{0x0F12,0x490D},
{0x0F12,0x480E},
{0x0F12,0xF000},
{0x0F12,0xF936},
{0x0F12,0xBC10},
{0x0F12,0xBC08},
{0x0F12,0x4718},
{0x0F12,0x0000},
{0x0F12,0x1080},
{0x0F12,0xD000},
{0x0F12,0x2D69},
{0x0F12,0x7000},
{0x0F12,0x89A9},
{0x0F12,0x0000},
{0x0F12,0x2DBB},
{0x0F12,0x7000},
{0x0F12,0x0140},
{0x0F12,0x7000},
{0x0F12,0x2DED},
{0x0F12,0x7000},
{0x0F12,0x2E65},
{0x0F12,0x7000},
{0x0F12,0x2E79},
{0x0F12,0x7000},
{0x0F12,0x2E4D},
{0x0F12,0x7000},
{0x0F12,0x013D},
{0x0F12,0x0001},
{0x0F12,0x2F03},
{0x0F12,0x7000},
{0x0F12,0x5823},
{0x0F12,0x0000},
{0x0F12,0xB570},
{0x0F12,0x6804},
{0x0F12,0x6845},
{0x0F12,0x6881},
{0x0F12,0x6840},
{0x0F12,0x2900},
{0x0F12,0x6880},
{0x0F12,0xD007},
{0x0F12,0x4976},
{0x0F12,0x8949},
{0x0F12,0x084A},
{0x0F12,0x1880},
{0x0F12,0xF000},
{0x0F12,0xF914},
{0x0F12,0x80A0},
{0x0F12,0xE000},
{0x0F12,0x80A0},
{0x0F12,0x88A0},
{0x0F12,0x2800},
{0x0F12,0xD010},
{0x0F12,0x68A9},
{0x0F12,0x6828},
{0x0F12,0x084A},
{0x0F12,0x1880},
{0x0F12,0xF000},
{0x0F12,0xF908},
{0x0F12,0x8020},
{0x0F12,0x1D2D},
{0x0F12,0xCD03},
{0x0F12,0x084A},
{0x0F12,0x1880},
{0x0F12,0xF000},
{0x0F12,0xF901},
{0x0F12,0x8060},
{0x0F12,0xBC70},
{0x0F12,0xBC08},
{0x0F12,0x4718},
{0x0F12,0x2000},
{0x0F12,0x8060},
{0x0F12,0x8020},
{0x0F12,0xE7F8},
{0x0F12,0xB510},
{0x0F12,0xF000},
{0x0F12,0xF8FC},
{0x0F12,0x4865},
{0x0F12,0x4966},
{0x0F12,0x8800},
{0x0F12,0x4A66},
{0x0F12,0x2805},
{0x0F12,0xD003},
{0x0F12,0x4B65},
{0x0F12,0x795B},
{0x0F12,0x2B00},
{0x0F12,0xD005},
{0x0F12,0x2001},
{0x0F12,0x8008},
{0x0F12,0x8010},
{0x0F12,0xBC10},
{0x0F12,0xBC08},
{0x0F12,0x4718},
{0x0F12,0x2800},
{0x0F12,0xD1FA},
{0x0F12,0x2000},
{0x0F12,0x8008},
{0x0F12,0x8010},
{0x0F12,0xE7F6},
{0x0F12,0xB570},
{0x0F12,0x0004},
{0x0F12,0x485D},
{0x0F12,0x2C00},
{0x0F12,0x8D00},
{0x0F12,0xD001},
{0x0F12,0x2501},
{0x0F12,0xE000},
{0x0F12,0x2500},
{0x0F12,0x4E5B},
{0x0F12,0x4328},
{0x0F12,0x8030},
{0x0F12,0x207D},
{0x0F12,0x00C0},
{0x0F12,0xF000},
{0x0F12,0xF8DE},
{0x0F12,0x4858},
{0x0F12,0x2C00},
{0x0F12,0x8C40},
{0x0F12,0x0329},
{0x0F12,0x4308},
{0x0F12,0x8130},
{0x0F12,0x4856},
{0x0F12,0x2C00},
{0x0F12,0x8A40},
{0x0F12,0x01A9},
{0x0F12,0x4308},
{0x0F12,0x80B0},
{0x0F12,0x2C00},
{0x0F12,0xD00B},
{0x0F12,0x4853},
{0x0F12,0x8A01},
{0x0F12,0x4853},
{0x0F12,0xF000},
{0x0F12,0xF8BD},
{0x0F12,0x4953},
{0x0F12,0x8809},
{0x0F12,0x4348},
{0x0F12,0x0400},
{0x0F12,0x0C00},
{0x0F12,0xF000},
{0x0F12,0xF8C4},
{0x0F12,0x0020},
{0x0F12,0xF000},
{0x0F12,0xF8C9},
{0x0F12,0x484F},
{0x0F12,0x7004},
{0x0F12,0xE7AF},
{0x0F12,0xB510},
{0x0F12,0x0004},
{0x0F12,0xF000},
{0x0F12,0xF8CA},
{0x0F12,0x6020},
{0x0F12,0x494C},
{0x0F12,0x8B49},
{0x0F12,0x0789},
{0x0F12,0xD0BD},
{0x0F12,0x0040},
{0x0F12,0x6020},
{0x0F12,0xE7BA},
{0x0F12,0xB510},
{0x0F12,0xF000},
{0x0F12,0xF8C7},
{0x0F12,0x4848},
{0x0F12,0x8880},
{0x0F12,0x0601},
{0x0F12,0x4840},
{0x0F12,0x1609},
{0x0F12,0x8281},
{0x0F12,0xE7B0},
{0x0F12,0xB5F8},
{0x0F12,0x000F},
{0x0F12,0x4C3A},
{0x0F12,0x3420},
{0x0F12,0x2500},
{0x0F12,0x5765},
{0x0F12,0x0039},
{0x0F12,0xF000},
{0x0F12,0xF8BF},
{0x0F12,0x9000},
{0x0F12,0x2600},
{0x0F12,0x57A6},
{0x0F12,0x4C38},
{0x0F12,0x42AE},
{0x0F12,0xD01B},
{0x0F12,0x4D3D},
{0x0F12,0x8AE8},
{0x0F12,0x2800},
{0x0F12,0xD013},
{0x0F12,0x4832},
{0x0F12,0x8A01},
{0x0F12,0x8B80},
{0x0F12,0x4378},
{0x0F12,0xF000},
{0x0F12,0xF881},
{0x0F12,0x89A9},
{0x0F12,0x1A41},
{0x0F12,0x4837},
{0x0F12,0x3820},
{0x0F12,0x8AC0},
{0x0F12,0x4348},
{0x0F12,0x17C1},
{0x0F12,0x0D89},
{0x0F12,0x1808},
{0x0F12,0x1280},
{0x0F12,0x8AA1},
{0x0F12,0x1A08},
{0x0F12,0x82A0},
{0x0F12,0xE003},
{0x0F12,0x88A8},
{0x0F12,0x0600},
{0x0F12,0x1600},
{0x0F12,0x82A0},
{0x0F12,0x2014},
{0x0F12,0x5E20},
{0x0F12,0x42B0},
{0x0F12,0xD011},
{0x0F12,0xF000},
{0x0F12,0xF89F},
{0x0F12,0x1D40},
{0x0F12,0x00C3},
{0x0F12,0x1A18},
{0x0F12,0x214B},
{0x0F12,0xF000},
{0x0F12,0xF863},
{0x0F12,0x211F},
{0x0F12,0xF000},
{0x0F12,0xF89E},
{0x0F12,0x2114},
{0x0F12,0x5E61},
{0x0F12,0x0FC9},
{0x0F12,0x0149},
{0x0F12,0x4301},
{0x0F12,0x4826},
{0x0F12,0x81C1},
{0x0F12,0x9800},
{0x0F12,0xBCF8},
{0x0F12,0xBC08},
{0x0F12,0x4718},
{0x0F12,0xB5F1},
{0x0F12,0xB082},
{0x0F12,0x2500},
{0x0F12,0x4822},
{0x0F12,0x9001},
{0x0F12,0x2400},
{0x0F12,0x2028},
{0x0F12,0x4368},
{0x0F12,0x4A21},
{0x0F12,0x4917},
{0x0F12,0x1882},
{0x0F12,0x39E0},
{0x0F12,0x1847},
{0x0F12,0x9200},
{0x0F12,0x0066},
{0x0F12,0x19B8},
{0x0F12,0x9A01},
{0x0F12,0x3060},
{0x0F12,0x8B01},
{0x0F12,0x5BB8},
{0x0F12,0x8812},
{0x0F12,0xF000},
{0x0F12,0xF884},
{0x0F12,0x9900},
{0x0F12,0x5388},
{0x0F12,0x1C64},
{0x0F12,0x2C14},
{0x0F12,0xDBF1},
{0x0F12,0x1C6D},
{0x0F12,0x2D03},
{0x0F12,0xDBE5},
{0x0F12,0x9802},
{0x0F12,0x6800},
{0x0F12,0x0600},
{0x0F12,0x0E00},
{0x0F12,0xF000},
{0x0F12,0xF87E},
{0x0F12,0xBCFE},
{0x0F12,0xBC08},
{0x0F12,0x4718},
{0x0F12,0x0000},
{0x0F12,0x0C3C},
{0x0F12,0x7000},
{0x0F12,0x26E8},
{0x0F12,0x7000},
{0x0F12,0x6100},
{0x0F12,0xD000},
{0x0F12,0x6500},
{0x0F12,0xD000},
{0x0F12,0x1A7C},
{0x0F12,0x7000},
{0x0F12,0x2C2C},
{0x0F12,0x7000},
{0x0F12,0xF400},
{0x0F12,0xD000},
{0x0F12,0x167C},
{0x0F12,0x7000},
{0x0F12,0x3368},
{0x0F12,0x7000},
{0x0F12,0x1D6C},
{0x0F12,0x7000},
{0x0F12,0x40A0},
{0x0F12,0x00DD},
{0x0F12,0xF520},
{0x0F12,0xD000},
{0x0F12,0x2C29},
{0x0F12,0x7000},
{0x0F12,0x1A54},
{0x0F12,0x7000},
{0x0F12,0x1564},
{0x0F12,0x7000},
{0x0F12,0xF2A0},
{0x0F12,0xD000},
{0x0F12,0x2440},
{0x0F12,0x7000},
{0x0F12,0x05A0},
{0x0F12,0x7000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0x1A3F},
{0x0F12,0x0001},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xF004},
{0x0F12,0xE51F},
{0x0F12,0x1F48},
{0x0F12,0x0001},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0x24BD},
{0x0F12,0x0000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0xF53F},
{0x0F12,0x0000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0xF5D9},
{0x0F12,0x0000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0x013D},
{0x0F12,0x0001},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0xF5C9},
{0x0F12,0x0000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0xFAA9},
{0x0F12,0x0000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0x36DD},
{0x0F12,0x0000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0x36ED},
{0x0F12,0x0000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0x3723},
{0x0F12,0x0000},
{0x0F12,0x4778},
{0x0F12,0x46C0},
{0x0F12,0xC000},
{0x0F12,0xE59F},
{0x0F12,0xFF1C},
{0x0F12,0xE12F},
{0x0F12,0x5823},
{0x0F12,0x0000},
{0x0F12,0x7D3E},
{0x0F12,0x0000},
// Ed T&P part
        
// CS/APS/Analog setting- 400LSBSYSCLK 45MHz 
{0x0028,0x7000},
{0x002A,0x157A},	
{0x0F12,0x0001},	
{0x002A,0x1578},	
{0x0F12,0x0001},	
{0x002A,0x1576},	
{0x0F12,0x0020},	
{0x002A,0x1574},	
{0x0F12,0x0006},
{0x002A,0x156E},	
{0x0F12,0x0001},	
{0x002A,0x1568},	
{0x0F12,0x00FC},    	  	
{0x002A,0x155A},     	
{0x0F12,0x01CC},	 //ADC SAT of 450mV for 10bit default in EVT1
{0x002A,0x157E},	
{0x0F12,0x0C80},	// 3200 Max. Reset ramp DCLK counts (default 2048 0x800)
{0x0F12,0x0578},	// 1400 Max. Reset ramp DCLK counts for x3.5
{0x002A,0x157C},	
{0x0F12,0x0190},	// 400 Reset ramp for x1 in DCLK counts
{0x002A,0x1570},	
{0x0F12,0x00A0},	// 224 LSB
{0x0F12,0x0010},	// reset threshold
{0x002A,0x12C4},	
{0x0F12,0x006A},	// 106 additional timing columns.
{0x002A,0x12C8},	
{0x0F12,0x08AC},	//0834// 2100 ADC columns in normal mode including Hold & Latch    ***
{0x0F12,0x0050},	//0028// 40 addition of ADC columns in Y-ave mode (default 244 0x74) ***    
{0x002A,0x1696},	
{0x0F12,0x0000},	//0001// based on APS guidelines ****
{0x0F12,0x0000},	//0001// based on APS guidelines ****
{0x0F12,0x00C6},	// default. 1492 used for ADC dark characteristics
{0x0F12,0x00C6},	// default. 1492 used for ADC dark characteristics
{0x002A,0x1690},
{0x0F12,0x0001},	// when set double sampling is activated - requires different set of pointers
{0x002A,0x12B0},	
{0x0F12,0x0055},	// comp and pixel bias control 0xF40E - default for EVT1
{0x0F12,0x005A},	// comp and pixel bias control 0xF40E for binning mode
{0x002A,0x337A},	
{0x0F12,0x0006},	// [7] - is used for rest-only mode (EVT0 value is 0xD and HW 0x6)
{0x002A,0x169E},	
{0x0F12,0x000A},	//000D// [3:0]- specifies the target (default 7)- DCLK = 64MHz instead of 116MHz. **** 
{0x0028,0xD000},	
{0x002A,0xF406},	
{0x0F12,0x1000},	// [11]: Enable DBLR Regulation
{0x002A,0xF40A},	
{0x0F12,0x6998},	//7996// [3:0]: VPIX ~2.8V ****
{0x002A,0xF418},	
{0x0F12,0x0078},	// [0]: Static RC-filter
{0x0F12,0x04FE},	// [7:4]: Full RC-filter
{0x002A,0xF52C},	
{0x0F12,0x8800},	// [11]: Add load to CDS block
{0x0028,0x7000},
{0x002A,0x12D2},
{0x0F12,0x0003},	//0006 // #senHal_pContSenModesRegsArray[0][0]2 700012D2
{0x0F12,0x0003},	//0006 // #senHal_pContSenModesRegsArray[0][1]2 700012D4
{0x0F12,0x0003},	//0003 // #senHal_pContSenModesRegsArray[0][2]2 700012D6
{0x0F12,0x0003},	//0003 // #senHal_pContSenModesRegsArray[0][3]2 700012D8
{0x0F12,0x0884},	//0801 // #senHal_pContSenModesRegsArray[1][0]2 700012DA
{0x0F12,0x08CF},	//0829 // #senHal_pContSenModesRegsArray[1][1]2 700012DC
{0x0F12,0x0500},	//047D // #senHal_pContSenModesRegsArray[1][2]2 700012DE
{0x0F12,0x054B},	//04A5 // #senHal_pContSenModesRegsArray[1][3]2 700012E0
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[2][0]2 700012E2
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[2][1]2 700012E4
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[2][2]2 700012E6
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[2][3]2 700012E8
{0x0F12,0x0885},	//0802 // #senHal_pContSenModesRegsArray[3][0]2 700012EA
{0x0F12,0x0467},	//0415 // #senHal_pContSenModesRegsArray[3][1]2 700012EC
{0x0F12,0x0501},	//047E // #senHal_pContSenModesRegsArray[3][2]2 700012EE
{0x0F12,0x02A5},	//0253 // #senHal_pContSenModesRegsArray[3][3]2 700012F0
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[4][0]2 700012F2
{0x0F12,0x046A},	//0416 // #senHal_pContSenModesRegsArray[4][1]2 700012F4
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[4][2]2 700012F6
{0x0F12,0x02A8},	//0254 // #senHal_pContSenModesRegsArray[4][3]2 700012F8
{0x0F12,0x0885},	//0802 // #senHal_pContSenModesRegsArray[5][0]2 700012FA
{0x0F12,0x08D0},	//082A // #senHal_pContSenModesRegsArray[5][1]2 700012FC
{0x0F12,0x0501},	//047E // #senHal_pContSenModesRegsArray[5][2]2 700012FE
{0x0F12,0x054C},	//04A6 // #senHal_pContSenModesRegsArray[5][3]2 70001300
{0x0F12,0x0006},	//0010 // #senHal_pContSenModesRegsArray[6][0]2 70001302
{0x0F12,0x0020},	//0012 // #senHal_pContSenModesRegsArray[6][1]2 70001304
{0x0F12,0x0006},	//0006 // #senHal_pContSenModesRegsArray[6][2]2 70001306
{0x0F12,0x0020},	//000C // #senHal_pContSenModesRegsArray[6][3]2 70001308
{0x0F12,0x0881},	//07FE // #senHal_pContSenModesRegsArray[7][0]2 7000130A
{0x0F12,0x0463},	//0411 // #senHal_pContSenModesRegsArray[7][1]2 7000130C
{0x0F12,0x04FD},	//047A // #senHal_pContSenModesRegsArray[7][2]2 7000130E
{0x0F12,0x02A1},	//024F // #senHal_pContSenModesRegsArray[7][3]2 70001310
{0x0F12,0x0006},	//0010 // #senHal_pContSenModesRegsArray[8][0]2 70001312
{0x0F12,0x0489},	//0424 // #senHal_pContSenModesRegsArray[8][1]2 70001314
{0x0F12,0x0006},	//0006 // #senHal_pContSenModesRegsArray[8][2]2 70001316
{0x0F12,0x02C7},	//0262 // #senHal_pContSenModesRegsArray[8][3]2 70001318
{0x0F12,0x0881},	//07FE // #senHal_pContSenModesRegsArray[9][0]2 7000131A
{0x0F12,0x08CC},	//0826 // #senHal_pContSenModesRegsArray[9][1]2 7000131C
{0x0F12,0x04FD},	//047A // #senHal_pContSenModesRegsArray[9][2]2 7000131E
{0x0F12,0x0548},	//04A2 // #senHal_pContSenModesRegsArray[9][3]2 70001320
{0x0F12,0x03A2},	//036F // #senHal_pContSenModesRegsArray[10][0] 2 70001322
{0x0F12,0x01D3},	//01B7 // #senHal_pContSenModesRegsArray[10][1] 2 70001324
{0x0F12,0x01E0},	//01AD // #senHal_pContSenModesRegsArray[10][2] 2 70001326
{0x0F12,0x00F2},	//00D6 // #senHal_pContSenModesRegsArray[10][3] 2 70001328
{0x0F12,0x03F2},	//039B // #senHal_pContSenModesRegsArray[11][0] 2 7000132A
{0x0F12,0x0223},	//01E3 // #senHal_pContSenModesRegsArray[11][1] 2 7000132C
{0x0F12,0x0230},	//01D9 // #senHal_pContSenModesRegsArray[11][2] 2 7000132E
{0x0F12,0x0142},	//0102 // #senHal_pContSenModesRegsArray[11][3] 2 70001330
{0x0F12,0x03A2},	//036F // #senHal_pContSenModesRegsArray[12][0] 2 70001332
{0x0F12,0x063C},	//05CC // #senHal_pContSenModesRegsArray[12][1] 2 70001334
{0x0F12,0x01E0},	//01AD // #senHal_pContSenModesRegsArray[12][2] 2 70001336
{0x0F12,0x0399},	//0329 // #senHal_pContSenModesRegsArray[12][3] 2 70001338
{0x0F12,0x03F2},	//039B // #senHal_pContSenModesRegsArray[13][0] 2 7000133A
{0x0F12,0x068C},	//05F8 // #senHal_pContSenModesRegsArray[13][1] 2 7000133C
{0x0F12,0x0230},	//01D9 // #senHal_pContSenModesRegsArray[13][2] 2 7000133E
{0x0F12,0x03E9},	//0355 // #senHal_pContSenModesRegsArray[13][3] 2 70001340
{0x0F12,0x0002},	//0002 // #senHal_pContSenModesRegsArray[14][0] 2 70001342
{0x0F12,0x0002},	//0002 // #senHal_pContSenModesRegsArray[14][1] 2 70001344
{0x0F12,0x0002},	//0002 // #senHal_pContSenModesRegsArray[14][2] 2 70001346
{0x0F12,0x0002},	//0002 // #senHal_pContSenModesRegsArray[14][3] 2 70001348
{0x0F12,0x003C},	//0022 // #senHal_pContSenModesRegsArray[15][0] 2 7000134A
{0x0F12,0x003C},	//0020 // #senHal_pContSenModesRegsArray[15][1] 2 7000134C
{0x0F12,0x003C},	//0022 // #senHal_pContSenModesRegsArray[15][2] 2 7000134E
{0x0F12,0x003C},	//0020 // #senHal_pContSenModesRegsArray[15][3] 2 70001350
{0x0F12,0x01D3},	//01B9 // #senHal_pContSenModesRegsArray[16][0] 2 70001352
{0x0F12,0x01D3},	//01B7 // #senHal_pContSenModesRegsArray[16][1] 2 70001354
{0x0F12,0x00F2},	//00D8 // #senHal_pContSenModesRegsArray[16][2] 2 70001356
{0x0F12,0x00F2},	//00D6 // #senHal_pContSenModesRegsArray[16][3] 2 70001358
{0x0F12,0x020B},	//01D7 // #senHal_pContSenModesRegsArray[17][0] 2 7000135A
{0x0F12,0x024A},	//01F8 // #senHal_pContSenModesRegsArray[17][1] 2 7000135C
{0x0F12,0x012A},	//00F6 // #senHal_pContSenModesRegsArray[17][2] 2 7000135E
{0x0F12,0x0169},	//0117 // #senHal_pContSenModesRegsArray[17][3] 2 70001360
{0x0F12,0x0002},	//0002 // #senHal_pContSenModesRegsArray[18][0] 2 70001362
{0x0F12,0x046B},	//0417 // #senHal_pContSenModesRegsArray[18][1] 2 70001364
{0x0F12,0x0002},	//0002 // #senHal_pContSenModesRegsArray[18][2] 2 70001366
{0x0F12,0x02A9},	//0255 // #senHal_pContSenModesRegsArray[18][3] 2 70001368
{0x0F12,0x0419},	//03B0 // #senHal_pContSenModesRegsArray[19][0] 2 7000136A
{0x0F12,0x04A5},	//0435 // #senHal_pContSenModesRegsArray[19][1] 2 7000136C
{0x0F12,0x0257},	//01EE // #senHal_pContSenModesRegsArray[19][2] 2 7000136E
{0x0F12,0x02E3},	//0273 // #senHal_pContSenModesRegsArray[19][3] 2 70001370
{0x0F12,0x0630},	//05C7 // #senHal_pContSenModesRegsArray[20][0] 2 70001372
{0x0F12,0x063C},	//05CC // #senHal_pContSenModesRegsArray[20][1] 2 70001374
{0x0F12,0x038D},	//0324 // #senHal_pContSenModesRegsArray[20][2] 2 70001376
{0x0F12,0x0399},	//0329 // #senHal_pContSenModesRegsArray[20][3] 2 70001378
{0x0F12,0x0668},	//05E5 // #senHal_pContSenModesRegsArray[21][0] 2 7000137A
{0x0F12,0x06B3},	//060D // #senHal_pContSenModesRegsArray[21][1] 2 7000137C
{0x0F12,0x03C5},	//0342 // #senHal_pContSenModesRegsArray[21][2] 2 7000137E
{0x0F12,0x0410},	//036A // #senHal_pContSenModesRegsArray[21][3] 2 70001380
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[22][0] 2 70001382
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[22][1] 2 70001384
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[22][2] 2 70001386
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[22][3] 2 70001388
{0x0F12,0x03A2},	//036E // #senHal_pContSenModesRegsArray[23][0] 2 7000138A
{0x0F12,0x01D3},	//01B7 // #senHal_pContSenModesRegsArray[23][1] 2 7000138C
{0x0F12,0x01E0},	//01AC // #senHal_pContSenModesRegsArray[23][2] 2 7000138E
{0x0F12,0x00F2},	//00D6 // #senHal_pContSenModesRegsArray[23][3] 2 70001390
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[24][0] 2 70001392
{0x0F12,0x0461},	//040F // #senHal_pContSenModesRegsArray[24][1] 2 70001394
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[24][2] 2 70001396
{0x0F12,0x029F},	//024D // #senHal_pContSenModesRegsArray[24][3] 2 70001398
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[25][0] 2 7000139A
{0x0F12,0x063C},	//05CC // #senHal_pContSenModesRegsArray[25][1] 2 7000139C
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[25][2] 2 7000139E
{0x0F12,0x0399},	//0329 // #senHal_pContSenModesRegsArray[25][3] 2 700013A0
{0x0F12,0x003D},	//0023 // #senHal_pContSenModesRegsArray[26][0] 2 700013A2
{0x0F12,0x003D},	//0021 // #senHal_pContSenModesRegsArray[26][1] 2 700013A4
{0x0F12,0x003D},	//0023 // #senHal_pContSenModesRegsArray[26][2] 2 700013A6
{0x0F12,0x003D},	//0021 // #senHal_pContSenModesRegsArray[26][3] 2 700013A8
{0x0F12,0x01D0},	//01B6 // #senHal_pContSenModesRegsArray[27][0] 2 700013AA
{0x0F12,0x01D0},	//01B4 // #senHal_pContSenModesRegsArray[27][1] 2 700013AC
{0x0F12,0x00EF},	//00D5 // #senHal_pContSenModesRegsArray[27][2] 2 700013AE
{0x0F12,0x00EF},	//00D3 // #senHal_pContSenModesRegsArray[27][3] 2 700013B0
{0x0F12,0x020C},	//01D8 // #senHal_pContSenModesRegsArray[28][0] 2 700013B2
{0x0F12,0x024B},	//01F9 // #senHal_pContSenModesRegsArray[28][1] 2 700013B4
{0x0F12,0x012B},	//00F7 // #senHal_pContSenModesRegsArray[28][2] 2 700013B6
{0x0F12,0x016A},	//0118 // #senHal_pContSenModesRegsArray[28][3] 2 700013B8
{0x0F12,0x039F},	//036B // #senHal_pContSenModesRegsArray[29][0] 2 700013BA
{0x0F12,0x045E},	//040C // #senHal_pContSenModesRegsArray[29][1] 2 700013BC
{0x0F12,0x01DD},	//01A9 // #senHal_pContSenModesRegsArray[29][2] 2 700013BE
{0x0F12,0x029C},	//024A // #senHal_pContSenModesRegsArray[29][3] 2 700013C0
{0x0F12,0x041A},	//03B1 // #senHal_pContSenModesRegsArray[30][0] 2 700013C2
{0x0F12,0x04A6},	//0436 // #senHal_pContSenModesRegsArray[30][1] 2 700013C4
{0x0F12,0x0258},	//01EF // #senHal_pContSenModesRegsArray[30][2] 2 700013C6
{0x0F12,0x02E4},	//0274 // #senHal_pContSenModesRegsArray[30][3] 2 700013C8
{0x0F12,0x062D},	//05C4 // #senHal_pContSenModesRegsArray[31][0] 2 700013CA
{0x0F12,0x0639},	//05C9 // #senHal_pContSenModesRegsArray[31][1] 2 700013CC
{0x0F12,0x038A},	//0321 // #senHal_pContSenModesRegsArray[31][2] 2 700013CE
{0x0F12,0x0396},	//0326 // #senHal_pContSenModesRegsArray[31][3] 2 700013D0
{0x0F12,0x0669},	//05E6 // #senHal_pContSenModesRegsArray[32][0] 2 700013D2
{0x0F12,0x06B4},	//060E // #senHal_pContSenModesRegsArray[32][1] 2 700013D4
{0x0F12,0x03C6},	//0343 // #senHal_pContSenModesRegsArray[32][2] 2 700013D6
{0x0F12,0x0411},	//036B // #senHal_pContSenModesRegsArray[32][3] 2 700013D8
{0x0F12,0x087C},	//07F9 // #senHal_pContSenModesRegsArray[33][0] 2 700013DA
{0x0F12,0x08C7},	//0821 // #senHal_pContSenModesRegsArray[33][1] 2 700013DC
{0x0F12,0x04F8},	//0475 // #senHal_pContSenModesRegsArray[33][2] 2 700013DE
{0x0F12,0x0543},	//049D // #senHal_pContSenModesRegsArray[33][3] 2 700013E0
{0x0F12,0x0040},	//0026 // #senHal_pContSenModesRegsArray[34][0] 2 700013E2
{0x0F12,0x0040},	//0024 // #senHal_pContSenModesRegsArray[34][1] 2 700013E4
{0x0F12,0x0040},	//0026 // #senHal_pContSenModesRegsArray[34][2] 2 700013E6
{0x0F12,0x0040},	//0024 // #senHal_pContSenModesRegsArray[34][3] 2 700013E8
{0x0F12,0x01D0},	//01B6 // #senHal_pContSenModesRegsArray[35][0] 2 700013EA
{0x0F12,0x01D0},	//01B4 // #senHal_pContSenModesRegsArray[35][1] 2 700013EC
{0x0F12,0x00EF},	//00D5 // #senHal_pContSenModesRegsArray[35][2] 2 700013EE
{0x0F12,0x00EF},	//00D3 // #senHal_pContSenModesRegsArray[35][3] 2 700013F0
{0x0F12,0x020F},	//01DB // #senHal_pContSenModesRegsArray[36][0] 2 700013F2
{0x0F12,0x024E},	//01FC // #senHal_pContSenModesRegsArray[36][1] 2 700013F4
{0x0F12,0x012E},	//00FA // #senHal_pContSenModesRegsArray[36][2] 2 700013F6
{0x0F12,0x016D},	//011B // #senHal_pContSenModesRegsArray[36][3] 2 700013F8
{0x0F12,0x039F},	//036B // #senHal_pContSenModesRegsArray[37][0] 2 700013FA
{0x0F12,0x045E},	//040C // #senHal_pContSenModesRegsArray[37][1] 2 700013FC
{0x0F12,0x01DD},	//01A9 // #senHal_pContSenModesRegsArray[37][2] 2 700013FE
{0x0F12,0x029C},	//024A // #senHal_pContSenModesRegsArray[37][3] 2 70001400
{0x0F12,0x041D},	//03B4 // #senHal_pContSenModesRegsArray[38][0] 2 70001402
{0x0F12,0x04A9},	//0439 // #senHal_pContSenModesRegsArray[38][1] 2 70001404
{0x0F12,0x025B},	//01F2 // #senHal_pContSenModesRegsArray[38][2] 2 70001406
{0x0F12,0x02E7},	//0277 // #senHal_pContSenModesRegsArray[38][3] 2 70001408
{0x0F12,0x062D},	//05C4 // #senHal_pContSenModesRegsArray[39][0] 2 7000140A
{0x0F12,0x0639},	//05C9 // #senHal_pContSenModesRegsArray[39][1] 2 7000140C
{0x0F12,0x038A},	//0321 // #senHal_pContSenModesRegsArray[39][2] 2 7000140E
{0x0F12,0x0396},	//0326 // #senHal_pContSenModesRegsArray[39][3] 2 70001410
{0x0F12,0x066C},	//05E9 // #senHal_pContSenModesRegsArray[40][0] 2 70001412
{0x0F12,0x06B7},	//0611 // #senHal_pContSenModesRegsArray[40][1] 2 70001414
{0x0F12,0x03C9},	//0346 // #senHal_pContSenModesRegsArray[40][2] 2 70001416
{0x0F12,0x0414},	//036E // #senHal_pContSenModesRegsArray[40][3] 2 70001418
{0x0F12,0x087C},	//07F9 // #senHal_pContSenModesRegsArray[41][0] 2 7000141A
{0x0F12,0x08C7},	//0821 // #senHal_pContSenModesRegsArray[41][1] 2 7000141C
{0x0F12,0x04F8},	//0475 // #senHal_pContSenModesRegsArray[41][2] 2 7000141E
{0x0F12,0x0543},	//049D // #senHal_pContSenModesRegsArray[41][3] 2 70001420
{0x0F12,0x0040},	//0026 // #senHal_pContSenModesRegsArray[42][0] 2 70001422
{0x0F12,0x0040},	//0024 // #senHal_pContSenModesRegsArray[42][1] 2 70001424
{0x0F12,0x0040},	//0026 // #senHal_pContSenModesRegsArray[42][2] 2 70001426
{0x0F12,0x0040},	//0024 // #senHal_pContSenModesRegsArray[42][3] 2 70001428
{0x0F12,0x01D0},	//01B6 // #senHal_pContSenModesRegsArray[43][0] 2 7000142A
{0x0F12,0x01D0},	//01B4 // #senHal_pContSenModesRegsArray[43][1] 2 7000142C
{0x0F12,0x00EF},	//00D5 // #senHal_pContSenModesRegsArray[43][2] 2 7000142E
{0x0F12,0x00EF},	//00D3 // #senHal_pContSenModesRegsArray[43][3] 2 70001430
{0x0F12,0x020F},	//01DB // #senHal_pContSenModesRegsArray[44][0] 2 70001432
{0x0F12,0x024E},	//01FC // #senHal_pContSenModesRegsArray[44][1] 2 70001434
{0x0F12,0x012E},	//00FA // #senHal_pContSenModesRegsArray[44][2] 2 70001436
{0x0F12,0x016D},	//011B // #senHal_pContSenModesRegsArray[44][3] 2 70001438
{0x0F12,0x039F},	//036B // #senHal_pContSenModesRegsArray[45][0] 2 7000143A
{0x0F12,0x045E},	//040C // #senHal_pContSenModesRegsArray[45][1] 2 7000143C
{0x0F12,0x01DD},	//01A9 // #senHal_pContSenModesRegsArray[45][2] 2 7000143E
{0x0F12,0x029C},	//024A // #senHal_pContSenModesRegsArray[45][3] 2 70001440
{0x0F12,0x041D},	//03B4 // #senHal_pContSenModesRegsArray[46][0] 2 70001442
{0x0F12,0x04A9},	//0439 // #senHal_pContSenModesRegsArray[46][1] 2 70001444
{0x0F12,0x025B},	//01F2 // #senHal_pContSenModesRegsArray[46][2] 2 70001446
{0x0F12,0x02E7},	//0277 // #senHal_pContSenModesRegsArray[46][3] 2 70001448
{0x0F12,0x062D},	//05C4 // #senHal_pContSenModesRegsArray[47][0] 2 7000144A
{0x0F12,0x0639},	//05C9 // #senHal_pContSenModesRegsArray[47][1] 2 7000144C
{0x0F12,0x038A},	//0321 // #senHal_pContSenModesRegsArray[47][2] 2 7000144E
{0x0F12,0x0396},	//0326 // #senHal_pContSenModesRegsArray[47][3] 2 70001450
{0x0F12,0x066C},	//05E9 // #senHal_pContSenModesRegsArray[48][0] 2 70001452
{0x0F12,0x06B7},	//0611 // #senHal_pContSenModesRegsArray[48][1] 2 70001454
{0x0F12,0x03C9},	//0346 // #senHal_pContSenModesRegsArray[48][2] 2 70001456
{0x0F12,0x0414},	//036E // #senHal_pContSenModesRegsArray[48][3] 2 70001458
{0x0F12,0x087C},	//07F9 // #senHal_pContSenModesRegsArray[49][0] 2 7000145A
{0x0F12,0x08C7},	//0821 // #senHal_pContSenModesRegsArray[49][1] 2 7000145C
{0x0F12,0x04F8},	//0475 // #senHal_pContSenModesRegsArray[49][2] 2 7000145E
{0x0F12,0x0543},	//049D // #senHal_pContSenModesRegsArray[49][3] 2 70001460
{0x0F12,0x003D},	//0023 // #senHal_pContSenModesRegsArray[50][0] 2 70001462
{0x0F12,0x003D},	//0021 // #senHal_pContSenModesRegsArray[50][1] 2 70001464
{0x0F12,0x003D},	//0023 // #senHal_pContSenModesRegsArray[50][2] 2 70001466
{0x0F12,0x003D},	//0021 // #senHal_pContSenModesRegsArray[50][3] 2 70001468
{0x0F12,0x01D2},	//01B8 // #senHal_pContSenModesRegsArray[51][0] 2 7000146A
{0x0F12,0x01D2},	//01B6 // #senHal_pContSenModesRegsArray[51][1] 2 7000146C
{0x0F12,0x00F1},	//00D7 // #senHal_pContSenModesRegsArray[51][2] 2 7000146E
{0x0F12,0x00F1},	//00D5 // #senHal_pContSenModesRegsArray[51][3] 2 70001470
{0x0F12,0x020C},	//01D8 // #senHal_pContSenModesRegsArray[52][0] 2 70001472
{0x0F12,0x024B},	//01F9 // #senHal_pContSenModesRegsArray[52][1] 2 70001474
{0x0F12,0x012B},	//00F7 // #senHal_pContSenModesRegsArray[52][2] 2 70001476
{0x0F12,0x016A},	//0118 // #senHal_pContSenModesRegsArray[52][3] 2 70001478
{0x0F12,0x03A1},	//036D // #senHal_pContSenModesRegsArray[53][0] 2 7000147A
{0x0F12,0x0460},	//040E // #senHal_pContSenModesRegsArray[53][1] 2 7000147C
{0x0F12,0x01DF},	//01AB // #senHal_pContSenModesRegsArray[53][2] 2 7000147E
{0x0F12,0x029E},	//024C // #senHal_pContSenModesRegsArray[53][3] 2 70001480
{0x0F12,0x041A},	//03B1 // #senHal_pContSenModesRegsArray[54][0] 2 70001482
{0x0F12,0x04A6},	//0436 // #senHal_pContSenModesRegsArray[54][1] 2 70001484
{0x0F12,0x0258},	//01EF // #senHal_pContSenModesRegsArray[54][2] 2 70001486
{0x0F12,0x02E4},	//0274 // #senHal_pContSenModesRegsArray[54][3] 2 70001488
{0x0F12,0x062F},	//05C6 // #senHal_pContSenModesRegsArray[55][0] 2 7000148A
{0x0F12,0x063B},	//05CB // #senHal_pContSenModesRegsArray[55][1] 2 7000148C
{0x0F12,0x038C},	//0323 // #senHal_pContSenModesRegsArray[55][2] 2 7000148E
{0x0F12,0x0398},	//0328 // #senHal_pContSenModesRegsArray[55][3] 2 70001490
{0x0F12,0x0669},	//05E6 // #senHal_pContSenModesRegsArray[56][0] 2 70001492
{0x0F12,0x06B4},	//060E // #senHal_pContSenModesRegsArray[56][1] 2 70001494
{0x0F12,0x03C6},	//0343 // #senHal_pContSenModesRegsArray[56][2] 2 70001496
{0x0F12,0x0411},	//036B // #senHal_pContSenModesRegsArray[56][3] 2 70001498
{0x0F12,0x087E},	//07FB // #senHal_pContSenModesRegsArray[57][0] 2 7000149A
{0x0F12,0x08C9},	//0823 // #senHal_pContSenModesRegsArray[57][1] 2 7000149C
{0x0F12,0x04FA},	//0477 // #senHal_pContSenModesRegsArray[57][2] 2 7000149E
{0x0F12,0x0545},	//049F // #senHal_pContSenModesRegsArray[57][3] 2 700014A0
{0x0F12,0x03A2},	//036E // #senHal_pContSenModesRegsArray[58][0] 2 700014A2
{0x0F12,0x01D3},	//01B7 // #senHal_pContSenModesRegsArray[58][1] 2 700014A4
{0x0F12,0x01E0},	//01AC // #senHal_pContSenModesRegsArray[58][2] 2 700014A6
{0x0F12,0x00F2},	//00D6 // #senHal_pContSenModesRegsArray[58][3] 2 700014A8
{0x0F12,0x03AF},	//037B // #senHal_pContSenModesRegsArray[59][0] 2 700014AA
{0x0F12,0x01E0},	//01C4 // #senHal_pContSenModesRegsArray[59][1] 2 700014AC
{0x0F12,0x01ED},	//01B9 // #senHal_pContSenModesRegsArray[59][2] 2 700014AE
{0x0F12,0x00FF},	//00E3 // #senHal_pContSenModesRegsArray[59][3] 2 700014B0
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[60][0] 2 700014B2
{0x0F12,0x0461},	//040F // #senHal_pContSenModesRegsArray[60][1] 2 700014B4
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[60][2] 2 700014B6
{0x0F12,0x029F},	//024D // #senHal_pContSenModesRegsArray[60][3] 2 700014B8
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[61][0] 2 700014BA
{0x0F12,0x046E},	//041C // #senHal_pContSenModesRegsArray[61][1] 2 700014BC
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[61][2] 2 700014BE
{0x0F12,0x02AC},	//025A // #senHal_pContSenModesRegsArray[61][3] 2 700014C0
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[62][0] 2 700014C2
{0x0F12,0x063C},	//05CC // #senHal_pContSenModesRegsArray[62][1] 2 700014C4
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[62][2] 2 700014C6
{0x0F12,0x0399},	//0329 // #senHal_pContSenModesRegsArray[62][3] 2 700014C8
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[63][0] 2 700014CA
{0x0F12,0x0649},	//05D9 // #senHal_pContSenModesRegsArray[63][1] 2 700014CC
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[63][2] 2 700014CE
{0x0F12,0x03A6},	//0336 // #senHal_pContSenModesRegsArray[63][3] 2 700014D0
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[64][0] 2 700014D2
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[64][1] 2 700014D4
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[64][2] 2 700014D6
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[64][3] 2 700014D8
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[65][0] 2 700014DA
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[65][1] 2 700014DC
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[65][2] 2 700014DE
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[65][3] 2 700014E0
{0x0F12,0x03AA},	//0376 // #senHal_pContSenModesRegsArray[66][0] 2 700014E2
{0x0F12,0x01DB},	//01BF // #senHal_pContSenModesRegsArray[66][1] 2 700014E4
{0x0F12,0x01E8},	//01B4 // #senHal_pContSenModesRegsArray[66][2] 2 700014E6
{0x0F12,0x00FA},	//00DE // #senHal_pContSenModesRegsArray[66][3] 2 700014E8
{0x0F12,0x03B7},	//0383 // #senHal_pContSenModesRegsArray[67][0] 2 700014EA
{0x0F12,0x01E8},	//01CC // #senHal_pContSenModesRegsArray[67][1] 2 700014EC
{0x0F12,0x01F5},	//01C1 // #senHal_pContSenModesRegsArray[67][2] 2 700014EE
{0x0F12,0x0107},	//00EB // #senHal_pContSenModesRegsArray[67][3] 2 700014F0
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[68][0] 2 700014F2
{0x0F12,0x0469},	//0417 // #senHal_pContSenModesRegsArray[68][1] 2 700014F4
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[68][2] 2 700014F6
{0x0F12,0x02A7},	//0255 // #senHal_pContSenModesRegsArray[68][3] 2 700014F8
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[69][0] 2 700014FA
{0x0F12,0x0476},	//0424 // #senHal_pContSenModesRegsArray[69][1] 2 700014FC
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[69][2] 2 700014FE
{0x0F12,0x02B4},	//0262 // #senHal_pContSenModesRegsArray[69][3] 2 70001500
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[70][0] 2 70001502
{0x0F12,0x0644},	//05D4 // #senHal_pContSenModesRegsArray[70][1] 2 70001504
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[70][2] 2 70001506
{0x0F12,0x03A1},	//0331 // #senHal_pContSenModesRegsArray[70][3] 2 70001508
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[71][0] 2 7000150A
{0x0F12,0x0651},	//05E1 // #senHal_pContSenModesRegsArray[71][1] 2 7000150C
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[71][2] 2 7000150E
{0x0F12,0x03AE},	//033E // #senHal_pContSenModesRegsArray[71][3] 2 70001510
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[72][0] 2 70001512
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[72][1] 2 70001514
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[72][2] 2 70001516
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[72][3] 2 70001518
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[73][0] 2 7000151A
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[73][1] 2 7000151C
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[73][2] 2 7000151E
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[73][3] 2 70001520
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[74][0] 2 70001522
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[74][1] 2 70001524
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[74][2] 2 70001526
{0x0F12,0x0001},	//0001 // #senHal_pContSenModesRegsArray[74][3] 2 70001528
{0x0F12,0x000F},	//000F // #senHal_pContSenModesRegsArray[75][0] 2 7000152A
{0x0F12,0x000F},	//000F // #senHal_pContSenModesRegsArray[75][1] 2 7000152C
{0x0F12,0x000F},	//000F // #senHal_pContSenModesRegsArray[75][2] 2 7000152E
{0x0F12,0x000F},	//000F // #senHal_pContSenModesRegsArray[75][3] 2 70001530
{0x0F12,0x05AD},	//0544 // #senHal_pContSenModesRegsArray[76][0] 2 70001532
{0x0F12,0x03DE},	//038C // #senHal_pContSenModesRegsArray[76][1] 2 70001534
{0x0F12,0x030A},	//02A1 // #senHal_pContSenModesRegsArray[76][2] 2 70001536
{0x0F12,0x021C},	//01CA // #senHal_pContSenModesRegsArray[76][3] 2 70001538
{0x0F12,0x062F},	//05C6 // #senHal_pContSenModesRegsArray[77][0] 2 7000153A
{0x0F12,0x0460},	//040E // #senHal_pContSenModesRegsArray[77][1] 2 7000153C
{0x0F12,0x038C},	//0323 // #senHal_pContSenModesRegsArray[77][2] 2 7000153E
{0x0F12,0x029E},	//024C // #senHal_pContSenModesRegsArray[77][3] 2 70001540
{0x0F12,0x07FC},	//0779 // #senHal_pContSenModesRegsArray[78][0] 2 70001542
{0x0F12,0x0847},	//07A1 // #senHal_pContSenModesRegsArray[78][1] 2 70001544
{0x0F12,0x0478},	//03F5 // #senHal_pContSenModesRegsArray[78][2] 2 70001546
{0x0F12,0x04C3},	//041D // #senHal_pContSenModesRegsArray[78][3] 2 70001548
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[79][0] 2 7000154A
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[79][1] 2 7000154C
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[79][2] 2 7000154E
{0x0F12,0x0000},	//0000 // #senHal_pContSenModesRegsArray[79][3] 2 70001550    
//============================================================
// ISP-FE Setting
//============================================================                    
{0x002A,0x158A},   
{0x0F12,0xEAF0},   
{0x002A,0x15C6},   
{0x0F12,0x0020},   
{0x0F12,0x0060},   
{0x002A,0x15BC},   
{0x0F12,0x0200},   
          
{0x002A,0x1608},   
{0x0F12,0x0100},   
{0x0F12,0x0100},   
{0x0F12,0x0100},   
{0x0F12,0x0100},   
          
{0x002A,0x0F70},   
{0x0F12,0x0040},	 //36 //TVAR_ae_BrAve  //ae Target//
{0x002A,0x0530},   	                                                    
{0x0F12,0x3415},   ///3A98 //3A98////lt_uMaxExp1	32 30ms  9~10ea// 15fps  // 
{0x002A,0x0534},                                                               
{0x0F12,0x682A},   //68b0 //7EF4////lt_uMaxExp2	67 65ms	18~20ea // 7.5fps //
{0x002A,0x167C},                                                               
{0x0F12,0x8235},   //8340 //9C40//MaxExp3  83 80ms  24~25ea //                 
{0x002A,0x1680},                                                               
{0x0F12,0xC350},	//F424//MaxExp4   125ms  38ea //

{0x002A,0x0538},                                                               
{0x0F12,0x3415},   // 15fps //                                                 
{0x002A,0x053C},                                                               
{0x0F12,0x682A},   // 7.5fps //                                                
{0x002A,0x1684},                                                               
{0x0F12,0x8235},   //CapMaxExp3 //                                             
{0x002A,0x1688},                                                               
{0x0F12,0xC350},   //CapMaxExp4 //                                             

//Shutter tunpoint//        //gain * 256 = value//
{0x002A,0x0540},                                                               
{0x0F12,0x01B3},    //0170//0150//lt_uMaxAnGain1_700lux//                                              
{0x0F12,0x01B3},   //0200//0400//lt_uMaxAnGain2_400lux//                              
{0x002A,0x168C},                                                               
{0x0F12,0x02A0},   //0300//MaxAnGain3_200lux//                                       
{0x0F12,0x0710},   //MaxAnGain4 //                                             
//Shutter tunend//

{0x002A,0x0544},                                                               
{0x0F12,0x0100},   	                                                    
{0x0F12,0x8000},   //Max Gain 8 //                                             


{0x002A,0x1694},                                                               
{0x0F12,0x0001},   //expand forbidde zone //                                   

{0x002A,0x021A},                                                               
{0x0F12,0x0000},  //MBR off// 	                                                    


//==============================================//
//AFC                                          //
//==============================================//
{0x002A,0x04D2},
{0x0F12,0x065F},	//065f : Manual AFC on   067f : Manual AFC off //
{0x002A,0x04BA},
{0x0F12,0x0001},     // 0002: 60hz  0001 : 50hz //
{0x0F12,0x0001},     // afc update command //
               
          
          
{0x002A,0x06CE},   
{0x0F12,0x0120},	//TVAR_ash_GAsalpha[0] // 	   
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[1] // 
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[2] // 
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[3] // 
{0x0F12,0x0130},   //TVAR_ash_GAsalpha[4] // 
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[5] // 
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[6] // 
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[7] // 
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[8] // 
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[9] // 
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[10] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[11] //
{0x0F12,0x00F8},   //TVAR_ash_GAsalpha[12] //
{0x0F12,0x010A},   //TVAR_ash_GAsalpha[13] //
{0x0F12,0x010A},   //TVAR_ash_GAsalpha[14] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[15] //
{0x0F12,0x0101},   //TVAR_ash_GAsalpha[16] //
{0x0F12,0x0103},   //TVAR_ash_GAsalpha[17] //
{0x0F12,0x0103},   //TVAR_ash_GAsalpha[18] //
{0x0F12,0x0101},   //TVAR_ash_GAsalpha[19] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[20] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[21] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[22] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[23] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[24] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[25] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[26] //
{0x0F12,0x0100},   //TVAR_ash_GAsalpha[27] //
         
{0x0F12,0x00f8},	//TVAR_ash_GAS OutdoorAlpha[0] //   
{0x0F12,0x0100},   //TVAR_ash_GAS OutdoorAlpha[1] //
{0x0F12,0x0100},   //TVAR_ash_GAS OutdoorAlpha[2] //
{0x0F12,0x0100},   //TVAR_ash_GAS OutdoorAlpha[3] //
          
          
{0x0F12,0x0000},	//ash_GASBeta[0] // 	   
{0x0F12,0x0000},	//ash_GASBeta[1] //    
{0x0F12,0x0000},   //ash_GASBeta[2] // 
{0x0F12,0x0000},   //ash_GASBeta[3] // 
{0x0F12,0x0000},   //ash_GASBeta[4] // 
{0x0F12,0x0000},   //ash_GASBeta[5] // 
{0x0F12,0x0000},   //ash_GASBeta[6] // 
{0x0F12,0x0000},   //ash_GASBeta[7] // 
{0x0F12,0x0000},   //ash_GASBeta[8] // 
{0x0F12,0x0000},   //ash_GASBeta[9] // 
{0x0F12,0x0000},   //ash_GASBeta[10] //
{0x0F12,0x0000},   //ash_GASBeta[11] //
{0x0F12,0x0000},   //ash_GASBeta[12] //
{0x0F12,0x0000},   //ash_GASBeta[13] //
{0x0F12,0x0000},   //ash_GASBeta[14] //
{0x0F12,0x0000},   //ash_GASBeta[15] //
{0x0F12,0x0000},   //ash_GASBeta[16] //
{0x0F12,0x0000},   //ash_GASBeta[17] //
{0x0F12,0x0000},   //ash_GASBeta[18] //
{0x0F12,0x0000},   //ash_GASBeta[19] //
{0x0F12,0x0000},   //ash_GASBeta[20] //
{0x0F12,0x0000},   //ash_GASBeta[21] //
{0x0F12,0x0000},   //ash_GASBeta[22] //
{0x0F12,0x0000},   //ash_GASBeta[23] //
{0x0F12,0x0000},   //ash_GASBeta[24] //
{0x0F12,0x0000},   //ash_GASBeta[25] //
{0x0F12,0x0000},   //ash_GASBeta[26] //
{0x0F12,0x0000},   //ash_GASBeta[27] //
          
{0x0F12,0x0036},	//ash_GAS OutdoorBeta[0] //   
{0x0F12,0x001F},   //ash_GAS OutdoorBeta[1] //
{0x0F12,0x0020},   //ash_GAS OutdoorBeta[2] //
{0x0F12,0x0000},   //ash_GAS OutdoorBeta[3] //
          
{0x002A,0x075A},	//ash_bParabolicEstimation//   
{0x0F12,0x0000},   //ash_uParabolicCenterX   //
{0x0F12,0x0400},   //ash_uParabolicCenterY   //
{0x0F12,0x0300},   //ash_uParabolicscalingA  //
{0x0F12,0x0010},   //ash_uParabolicscalingB  //
{0x0F12,0x0011},   
         
{0x002A,0x347C},   
{0x0F12,0x01AE},	//TVAR_ash_pGAS[0] //     
{0x0F12,0x0170},   //TVAR_ash_pGAS[1] //  
{0x0F12,0x0145},   //TVAR_ash_pGAS[2] //  
{0x0F12,0x011E},   //TVAR_ash_pGAS[3] //  
{0x0F12,0x00FE},   //TVAR_ash_pGAS[4] //  
{0x0F12,0x00E6},   //TVAR_ash_pGAS[5] //  
{0x0F12,0x00DC},   //TVAR_ash_pGAS[6] //  
{0x0F12,0x00E0},   //TVAR_ash_pGAS[7] //  
{0x0F12,0x00F0},   //TVAR_ash_pGAS[8] //  
{0x0F12,0x0110},   //TVAR_ash_pGAS[9] //  
{0x0F12,0x0137},   //TVAR_ash_pGAS[10] // 
{0x0F12,0x0169},   //TVAR_ash_pGAS[11] // 
{0x0F12,0x01B4},   //TVAR_ash_pGAS[12] // 
{0x0F12,0x0186},   //TVAR_ash_pGAS[13] // 
{0x0F12,0x0154},   //TVAR_ash_pGAS[14] // 
{0x0F12,0x0120},   //TVAR_ash_pGAS[15] // 
{0x0F12,0x00ED},   //TVAR_ash_pGAS[16] // 
{0x0F12,0x00C2},   //TVAR_ash_pGAS[17] // 
{0x0F12,0x00A8},   //TVAR_ash_pGAS[18] // 
{0x0F12,0x009C},   //TVAR_ash_pGAS[19] // 
{0x0F12,0x00A3},   //TVAR_ash_pGAS[20] // 
{0x0F12,0x00B9},   //TVAR_ash_pGAS[21] // 
{0x0F12,0x00E3},   //TVAR_ash_pGAS[22] // 
{0x0F12,0x0119},   //TVAR_ash_pGAS[23] // 
{0x0F12,0x0153},   //TVAR_ash_pGAS[24] // 
{0x0F12,0x018C},   //TVAR_ash_pGAS[25] // 
{0x0F12,0x0156},   //TVAR_ash_pGAS[26] // 
{0x0F12,0x0122},   //TVAR_ash_pGAS[27] // 
{0x0F12,0x00E4},   //TVAR_ash_pGAS[28] // 
{0x0F12,0x00A9},   //TVAR_ash_pGAS[29] // 
{0x0F12,0x007B},   //TVAR_ash_pGAS[30] // 
{0x0F12,0x0060},   //TVAR_ash_pGAS[31] // 
{0x0F12,0x0055},   //TVAR_ash_pGAS[32] // 
{0x0F12,0x005C},   //TVAR_ash_pGAS[33] // 
{0x0F12,0x0076},   //TVAR_ash_pGAS[34] // 
{0x0F12,0x00A4},   //TVAR_ash_pGAS[35] // 
{0x0F12,0x00E1},   //TVAR_ash_pGAS[36] // 
{0x0F12,0x0125},   //TVAR_ash_pGAS[37] // 
{0x0F12,0x015E},   //TVAR_ash_pGAS[38] // 
{0x0F12,0x012E},   //TVAR_ash_pGAS[39] // 
{0x0F12,0x00F7},   //TVAR_ash_pGAS[40] // 
{0x0F12,0x00B2},   //TVAR_ash_pGAS[41] // 
{0x0F12,0x0074},   //TVAR_ash_pGAS[42] // 
{0x0F12,0x0046},   //TVAR_ash_pGAS[43] // 
{0x0F12,0x002A},   //TVAR_ash_pGAS[44] // 
{0x0F12,0x0021},   //TVAR_ash_pGAS[45] // 
{0x0F12,0x0029},   //TVAR_ash_pGAS[46] // 
{0x0F12,0x0044},   //TVAR_ash_pGAS[47] // 
{0x0F12,0x0074},   //TVAR_ash_pGAS[48] // 
{0x0F12,0x00B4},   //TVAR_ash_pGAS[49] // 
{0x0F12,0x00FF},   //TVAR_ash_pGAS[50] // 
{0x0F12,0x013D},   //TVAR_ash_pGAS[51] // 
{0x0F12,0x011D},   //TVAR_ash_pGAS[52] // 
{0x0F12,0x00E0},   //TVAR_ash_pGAS[53] // 
{0x0F12,0x0095},   //TVAR_ash_pGAS[54] // 
{0x0F12,0x0056},   //TVAR_ash_pGAS[55] // 
{0x0F12,0x0028},   //TVAR_ash_pGAS[56] // 
{0x0F12,0x000F},   //TVAR_ash_pGAS[57] // 
{0x0F12,0x0008},   //TVAR_ash_pGAS[58] // 
{0x0F12,0x0010},   //TVAR_ash_pGAS[59] // 
{0x0F12,0x0029},   //TVAR_ash_pGAS[60] // 
{0x0F12,0x005B},   //TVAR_ash_pGAS[61] // 
{0x0F12,0x009E},   //TVAR_ash_pGAS[62] // 
{0x0F12,0x00EC},   //TVAR_ash_pGAS[63] // 
{0x0F12,0x0130},   //TVAR_ash_pGAS[64] // 
{0x0F12,0x0118},   //TVAR_ash_pGAS[65] // 
{0x0F12,0x00D8},   //TVAR_ash_pGAS[66] // 
{0x0F12,0x008C},   //TVAR_ash_pGAS[67] // 
{0x0F12,0x004C},   //TVAR_ash_pGAS[68] // 
{0x0F12,0x001E},   //TVAR_ash_pGAS[69] // 
{0x0F12,0x0007},   //TVAR_ash_pGAS[70] // 
{0x0F12,0x0000},   //TVAR_ash_pGAS[71] // 
{0x0F12,0x0008},   //TVAR_ash_pGAS[72] // 
{0x0F12,0x0022},   //TVAR_ash_pGAS[73] // 
{0x0F12,0x0054},   //TVAR_ash_pGAS[74] // 
{0x0F12,0x0099},   //TVAR_ash_pGAS[75] // 
{0x0F12,0x00E9},   //TVAR_ash_pGAS[76] // 
{0x0F12,0x0131},   //TVAR_ash_pGAS[77] // 
{0x0F12,0x0116},   //TVAR_ash_pGAS[78] // 
{0x0F12,0x00DA},   //TVAR_ash_pGAS[79] // 
{0x0F12,0x0090},   //TVAR_ash_pGAS[80] // 
{0x0F12,0x0051},   //TVAR_ash_pGAS[81] // 
{0x0F12,0x0023},   //TVAR_ash_pGAS[82] // 
{0x0F12,0x000C},   //TVAR_ash_pGAS[83] // 
{0x0F12,0x0005},   //TVAR_ash_pGAS[84] // 
{0x0F12,0x000E},   //TVAR_ash_pGAS[85] // 
{0x0F12,0x0029},   //TVAR_ash_pGAS[86] // 
{0x0F12,0x005B},   //TVAR_ash_pGAS[87] // 
{0x0F12,0x00A0},   //TVAR_ash_pGAS[88] // 
{0x0F12,0x00EF},   //TVAR_ash_pGAS[89] // 
{0x0F12,0x0132},   //TVAR_ash_pGAS[90] // 
{0x0F12,0x0124},   //TVAR_ash_pGAS[91] // 
{0x0F12,0x00ED},   //TVAR_ash_pGAS[92] // 
{0x0F12,0x00A7},   //TVAR_ash_pGAS[93] // 
{0x0F12,0x006A},   //TVAR_ash_pGAS[94] // 
{0x0F12,0x003D},   //TVAR_ash_pGAS[95] // 
{0x0F12,0x0024},   //TVAR_ash_pGAS[96] // 
{0x0F12,0x001D},   //TVAR_ash_pGAS[97] // 
{0x0F12,0x0027},   //TVAR_ash_pGAS[98] // 
{0x0F12,0x0043},   //TVAR_ash_pGAS[99] // 
{0x0F12,0x0076},   //TVAR_ash_pGAS[100] //
{0x0F12,0x00B9},   //TVAR_ash_pGAS[101] //
{0x0F12,0x0104},   //TVAR_ash_pGAS[102] //
{0x0F12,0x0143},   //TVAR_ash_pGAS[103] //
{0x0F12,0x0141},   //TVAR_ash_pGAS[104] //
{0x0F12,0x0111},   //TVAR_ash_pGAS[105] //
{0x0F12,0x00D2},   //TVAR_ash_pGAS[106] //
{0x0F12,0x0097},   //TVAR_ash_pGAS[107] //
{0x0F12,0x006C},   //TVAR_ash_pGAS[108] //
{0x0F12,0x0053},   //TVAR_ash_pGAS[109] //
{0x0F12,0x004B},   //TVAR_ash_pGAS[110] //
{0x0F12,0x0056},   //TVAR_ash_pGAS[111] //
{0x0F12,0x0073},   //TVAR_ash_pGAS[112] //
{0x0F12,0x00A4},   //TVAR_ash_pGAS[113] //
{0x0F12,0x00E4},   //TVAR_ash_pGAS[114] //
{0x0F12,0x0127},   //TVAR_ash_pGAS[115] //
{0x0F12,0x0161},   //TVAR_ash_pGAS[116] //
{0x0F12,0x016D},   //TVAR_ash_pGAS[117] //
{0x0F12,0x0139},   //TVAR_ash_pGAS[118] //
{0x0F12,0x0105},   //TVAR_ash_pGAS[119] //
{0x0F12,0x00D2},   //TVAR_ash_pGAS[120] //
{0x0F12,0x00AA},   //TVAR_ash_pGAS[121] //
{0x0F12,0x0093},   //TVAR_ash_pGAS[122] //
{0x0F12,0x008C},   //TVAR_ash_pGAS[123] //
{0x0F12,0x0096},   //TVAR_ash_pGAS[124] //
{0x0F12,0x00B1},   //TVAR_ash_pGAS[125] //
{0x0F12,0x00DE},   //TVAR_ash_pGAS[126] //
{0x0F12,0x0115},   //TVAR_ash_pGAS[127] //
{0x0F12,0x014D},   //TVAR_ash_pGAS[128] //
{0x0F12,0x0189},   //TVAR_ash_pGAS[129] //
{0x0F12,0x019E},   //TVAR_ash_pGAS[130] //
{0x0F12,0x0160},   //TVAR_ash_pGAS[131] //
{0x0F12,0x0131},   //TVAR_ash_pGAS[132] //
{0x0F12,0x010C},   //TVAR_ash_pGAS[133] //
{0x0F12,0x00ED},   //TVAR_ash_pGAS[134] //
{0x0F12,0x00D9},   //TVAR_ash_pGAS[135] //
{0x0F12,0x00D4},   //TVAR_ash_pGAS[136] //
{0x0F12,0x00DC},   //TVAR_ash_pGAS[137] //
{0x0F12,0x00F3},   //TVAR_ash_pGAS[138] //
{0x0F12,0x0116},   //TVAR_ash_pGAS[139] //
{0x0F12,0x0140},   //TVAR_ash_pGAS[140] //
{0x0F12,0x0171},   //TVAR_ash_pGAS[141] //
{0x0F12,0x01B6},   //TVAR_ash_pGAS[142] //
{0x0F12,0x0167},   //TVAR_ash_pGAS[143] //
{0x0F12,0x0126},   //TVAR_ash_pGAS[144] //
{0x0F12,0x0103},   //TVAR_ash_pGAS[145] //
{0x0F12,0x00E2},   //TVAR_ash_pGAS[146] //
{0x0F12,0x00C5},   //TVAR_ash_pGAS[147] //
{0x0F12,0x00B0},   //TVAR_ash_pGAS[148] //
{0x0F12,0x00A6},   //TVAR_ash_pGAS[149] //
{0x0F12,0x00A9},   //TVAR_ash_pGAS[150] //
{0x0F12,0x00B8},   //TVAR_ash_pGAS[151] //
{0x0F12,0x00D2},   //TVAR_ash_pGAS[152] //
{0x0F12,0x00F0},   //TVAR_ash_pGAS[153] //
{0x0F12,0x0119},   //TVAR_ash_pGAS[154] //
{0x0F12,0x015E},   //TVAR_ash_pGAS[155] //
{0x0F12,0x0142},   //TVAR_ash_pGAS[156] //
{0x0F12,0x010C},   //TVAR_ash_pGAS[157] //
{0x0F12,0x00E4},   //TVAR_ash_pGAS[158] //
{0x0F12,0x00BA},   //TVAR_ash_pGAS[159] //
{0x0F12,0x0096},   //TVAR_ash_pGAS[160] //
{0x0F12,0x007F},   //TVAR_ash_pGAS[161] //
{0x0F12,0x0075},   //TVAR_ash_pGAS[162] //
{0x0F12,0x007A},   //TVAR_ash_pGAS[163] //
{0x0F12,0x008D},   //TVAR_ash_pGAS[164] //
{0x0F12,0x00AE},   //TVAR_ash_pGAS[165] //
{0x0F12,0x00D6},   //TVAR_ash_pGAS[166] //
{0x0F12,0x0103},   //TVAR_ash_pGAS[167] //
{0x0F12,0x013E},   //TVAR_ash_pGAS[168] //
{0x0F12,0x011B},   //TVAR_ash_pGAS[169] //
{0x0F12,0x00E7},   //TVAR_ash_pGAS[170] //
{0x0F12,0x00B8},   //TVAR_ash_pGAS[171] //
{0x0F12,0x0088},   //TVAR_ash_pGAS[172] //
{0x0F12,0x0062},   //TVAR_ash_pGAS[173] //
{0x0F12,0x0049},   //TVAR_ash_pGAS[174] //
{0x0F12,0x0040},   //TVAR_ash_pGAS[175] //
{0x0F12,0x0046},   //TVAR_ash_pGAS[176] //
{0x0F12,0x005C},   //TVAR_ash_pGAS[177] //
{0x0F12,0x007F},   //TVAR_ash_pGAS[178] //
{0x0F12,0x00AE},   //TVAR_ash_pGAS[179] //
{0x0F12,0x00E1},   //TVAR_ash_pGAS[180] //
{0x0F12,0x0117},   //TVAR_ash_pGAS[181] //
{0x0F12,0x00FA},   //TVAR_ash_pGAS[182] //
{0x0F12,0x00C5},   //TVAR_ash_pGAS[183] //
{0x0F12,0x0090},   //TVAR_ash_pGAS[184] //
{0x0F12,0x005E},   //TVAR_ash_pGAS[185] //
{0x0F12,0x0038},   //TVAR_ash_pGAS[186] //
{0x0F12,0x0020},   //TVAR_ash_pGAS[187] //
{0x0F12,0x0018},   //TVAR_ash_pGAS[188] //
{0x0F12,0x001F},   //TVAR_ash_pGAS[189] //
{0x0F12,0x0035},   //TVAR_ash_pGAS[190] //
{0x0F12,0x005B},   //TVAR_ash_pGAS[191] //
{0x0F12,0x008C},   //TVAR_ash_pGAS[192] //
{0x0F12,0x00C5},   //TVAR_ash_pGAS[193] //
{0x0F12,0x00FE},   //TVAR_ash_pGAS[194] //
{0x0F12,0x00EB},   //TVAR_ash_pGAS[195] //
{0x0F12,0x00B3},   //TVAR_ash_pGAS[196] //
{0x0F12,0x007A},   //TVAR_ash_pGAS[197] //
{0x0F12,0x0047},   //TVAR_ash_pGAS[198] //
{0x0F12,0x0021},   //TVAR_ash_pGAS[199] //
{0x0F12,0x000B},   //TVAR_ash_pGAS[200] //
{0x0F12,0x0005},   //TVAR_ash_pGAS[201] //
{0x0F12,0x000C},   //TVAR_ash_pGAS[202] //
{0x0F12,0x0022},   //TVAR_ash_pGAS[203] //
{0x0F12,0x0049},   //TVAR_ash_pGAS[204] //
{0x0F12,0x007C},   //TVAR_ash_pGAS[205] //
{0x0F12,0x00B8},   //TVAR_ash_pGAS[206] //
{0x0F12,0x00F4},   //TVAR_ash_pGAS[207] //
{0x0F12,0x00E6},   //TVAR_ash_pGAS[208] //
{0x0F12,0x00AD},   //TVAR_ash_pGAS[209] //
{0x0F12,0x0073},   //TVAR_ash_pGAS[210] //
{0x0F12,0x0040},   //TVAR_ash_pGAS[211] //
{0x0F12,0x001A},   //TVAR_ash_pGAS[212] //
{0x0F12,0x0005},   //TVAR_ash_pGAS[213] //
{0x0F12,0x0000},   //TVAR_ash_pGAS[214] //
{0x0F12,0x0008},   //TVAR_ash_pGAS[215] //
{0x0F12,0x001E},   //TVAR_ash_pGAS[216] //
{0x0F12,0x0045},   //TVAR_ash_pGAS[217] //
{0x0F12,0x0079},   //TVAR_ash_pGAS[218] //
{0x0F12,0x00B7},   //TVAR_ash_pGAS[219] //
{0x0F12,0x00F5},   //TVAR_ash_pGAS[220] //
{0x0F12,0x00E3},   //TVAR_ash_pGAS[221] //
{0x0F12,0x00AE},   //TVAR_ash_pGAS[222] //
{0x0F12,0x0076},   //TVAR_ash_pGAS[223] //
{0x0F12,0x0044},   //TVAR_ash_pGAS[224] //
{0x0F12,0x001E},   //TVAR_ash_pGAS[225] //
{0x0F12,0x000A},   //TVAR_ash_pGAS[226] //
{0x0F12,0x0005},   //TVAR_ash_pGAS[227] //
{0x0F12,0x000D},   //TVAR_ash_pGAS[228] //
{0x0F12,0x0025},   //TVAR_ash_pGAS[229] //
{0x0F12,0x004C},   //TVAR_ash_pGAS[230] //
{0x0F12,0x0080},   //TVAR_ash_pGAS[231] //
{0x0F12,0x00BC},   //TVAR_ash_pGAS[232] //
{0x0F12,0x00F8},   //TVAR_ash_pGAS[233] //
{0x0F12,0x00F0},   //TVAR_ash_pGAS[234] //
{0x0F12,0x00BD},   //TVAR_ash_pGAS[235] //
{0x0F12,0x0089},   //TVAR_ash_pGAS[236] //
{0x0F12,0x0059},   //TVAR_ash_pGAS[237] //
{0x0F12,0x0034},   //TVAR_ash_pGAS[238] //
{0x0F12,0x001F},   //TVAR_ash_pGAS[239] //
{0x0F12,0x0019},   //TVAR_ash_pGAS[240] //
{0x0F12,0x0023},   //TVAR_ash_pGAS[241] //
{0x0F12,0x003B},   //TVAR_ash_pGAS[242] //
{0x0F12,0x0063},   //TVAR_ash_pGAS[243] //
{0x0F12,0x0096},   //TVAR_ash_pGAS[244] //
{0x0F12,0x00CE},   //TVAR_ash_pGAS[245] //
{0x0F12,0x0105},   //TVAR_ash_pGAS[246] //
{0x0F12,0x010A},   //TVAR_ash_pGAS[247] //
{0x0F12,0x00DA},   //TVAR_ash_pGAS[248] //
{0x0F12,0x00AB},   //TVAR_ash_pGAS[249] //
{0x0F12,0x007E},   //TVAR_ash_pGAS[250] //
{0x0F12,0x005B},   //TVAR_ash_pGAS[251] //
{0x0F12,0x0046},   //TVAR_ash_pGAS[252] //
{0x0F12,0x0040},   //TVAR_ash_pGAS[253] //
{0x0F12,0x004A},   //TVAR_ash_pGAS[254] //
{0x0F12,0x0062},   //TVAR_ash_pGAS[255] //
{0x0F12,0x0088},   //TVAR_ash_pGAS[256] //
{0x0F12,0x00B9},   //TVAR_ash_pGAS[257] //
{0x0F12,0x00EB},   //TVAR_ash_pGAS[258] //
{0x0F12,0x011E},   //TVAR_ash_pGAS[259] //
{0x0F12,0x012E},   //TVAR_ash_pGAS[260] //
{0x0F12,0x00F8},   //TVAR_ash_pGAS[261] //
{0x0F12,0x00D1},   //TVAR_ash_pGAS[262] //
{0x0F12,0x00A9},   //TVAR_ash_pGAS[263] //
{0x0F12,0x0089},   //TVAR_ash_pGAS[264] //
{0x0F12,0x0077},   //TVAR_ash_pGAS[265] //
{0x0F12,0x0072},   //TVAR_ash_pGAS[266] //
{0x0F12,0x007C},   //TVAR_ash_pGAS[267] //
{0x0F12,0x0093},   //TVAR_ash_pGAS[268] //
{0x0F12,0x00B6},   //TVAR_ash_pGAS[269] //
{0x0F12,0x00DE},   //TVAR_ash_pGAS[270] //
{0x0F12,0x0108},   //TVAR_ash_pGAS[271] //
{0x0F12,0x013F},   //TVAR_ash_pGAS[272] //
{0x0F12,0x015D},   //TVAR_ash_pGAS[273] //
{0x0F12,0x011D},   //TVAR_ash_pGAS[274] //
{0x0F12,0x00F6},   //TVAR_ash_pGAS[275] //
{0x0F12,0x00D7},   //TVAR_ash_pGAS[276] //
{0x0F12,0x00BE},   //TVAR_ash_pGAS[277] //
{0x0F12,0x00AF},   //TVAR_ash_pGAS[278] //
{0x0F12,0x00AC},   //TVAR_ash_pGAS[279] //
{0x0F12,0x00B5},   //TVAR_ash_pGAS[280] //
{0x0F12,0x00CA},   //TVAR_ash_pGAS[281] //
{0x0F12,0x00E5},   //TVAR_ash_pGAS[282] //
{0x0F12,0x0104},   //TVAR_ash_pGAS[283] //
{0x0F12,0x0129},   //TVAR_ash_pGAS[284] //
{0x0F12,0x0168},   //TVAR_ash_pGAS[285] //
{0x0F12,0x015B},   //TVAR_ash_pGAS[286] //
{0x0F12,0x011A},   //TVAR_ash_pGAS[287] //
{0x0F12,0x00F7},   //TVAR_ash_pGAS[288] //
{0x0F12,0x00D9},   //TVAR_ash_pGAS[289] //
{0x0F12,0x00C1},   //TVAR_ash_pGAS[290] //
{0x0F12,0x00B2},   //TVAR_ash_pGAS[291] //
{0x0F12,0x00AF},   //TVAR_ash_pGAS[292] //
{0x0F12,0x00B9},   //TVAR_ash_pGAS[293] //
{0x0F12,0x00CE},   //TVAR_ash_pGAS[294] //
{0x0F12,0x00ED},   //TVAR_ash_pGAS[295] //
{0x0F12,0x010E},   //TVAR_ash_pGAS[296] //
{0x0F12,0x0139},   //TVAR_ash_pGAS[297] //
{0x0F12,0x0182},   //TVAR_ash_pGAS[298] //
{0x0F12,0x0136},   //TVAR_ash_pGAS[299] //
{0x0F12,0x00FF},   //TVAR_ash_pGAS[300] //
{0x0F12,0x00D9},   //TVAR_ash_pGAS[301] //
{0x0F12,0x00B2},   //TVAR_ash_pGAS[302] //
{0x0F12,0x0092},   //TVAR_ash_pGAS[303] //
{0x0F12,0x0080},   //TVAR_ash_pGAS[304] //
{0x0F12,0x007B},   //TVAR_ash_pGAS[305] //
{0x0F12,0x0086},   //TVAR_ash_pGAS[306] //
{0x0F12,0x009F},   //TVAR_ash_pGAS[307] //
{0x0F12,0x00C4},   //TVAR_ash_pGAS[308] //
{0x0F12,0x00F0},   //TVAR_ash_pGAS[309] //
{0x0F12,0x011F},   //TVAR_ash_pGAS[310] //
{0x0F12,0x015A},   //TVAR_ash_pGAS[311] //
{0x0F12,0x0114},   //TVAR_ash_pGAS[312] //
{0x0F12,0x00E0},   //TVAR_ash_pGAS[313] //
{0x0F12,0x00B2},   //TVAR_ash_pGAS[314] //
{0x0F12,0x0084},   //TVAR_ash_pGAS[315] //
{0x0F12,0x0062},   //TVAR_ash_pGAS[316] //
{0x0F12,0x004D},   //TVAR_ash_pGAS[317] //
{0x0F12,0x0047},   //TVAR_ash_pGAS[318] //
{0x0F12,0x0052},   //TVAR_ash_pGAS[319] //
{0x0F12,0x006B},   //TVAR_ash_pGAS[320] //
{0x0F12,0x0092},   //TVAR_ash_pGAS[321] //
{0x0F12,0x00C4},   //TVAR_ash_pGAS[322] //
{0x0F12,0x00F9},   //TVAR_ash_pGAS[323] //
{0x0F12,0x0130},   //TVAR_ash_pGAS[324] //
{0x0F12,0x00F7},   //TVAR_ash_pGAS[325] //
{0x0F12,0x00C2},   //TVAR_ash_pGAS[326] //
{0x0F12,0x008E},   //TVAR_ash_pGAS[327] //
{0x0F12,0x005E},   //TVAR_ash_pGAS[328] //
{0x0F12,0x003A},   //TVAR_ash_pGAS[329] //
{0x0F12,0x0024},   //TVAR_ash_pGAS[330] //
{0x0F12,0x001F},   //TVAR_ash_pGAS[331] //
{0x0F12,0x0028},   //TVAR_ash_pGAS[332] //
{0x0F12,0x0042},   //TVAR_ash_pGAS[333] //
{0x0F12,0x006A},   //TVAR_ash_pGAS[334] //
{0x0F12,0x009D},   //TVAR_ash_pGAS[335] //
{0x0F12,0x00D7},   //TVAR_ash_pGAS[336] //
{0x0F12,0x0110},   //TVAR_ash_pGAS[337] //
{0x0F12,0x00E8},   //TVAR_ash_pGAS[338] //
{0x0F12,0x00B0},   //TVAR_ash_pGAS[339] //
{0x0F12,0x0078},   //TVAR_ash_pGAS[340] //
{0x0F12,0x0047},   //TVAR_ash_pGAS[341] //
{0x0F12,0x0022},   //TVAR_ash_pGAS[342] //
{0x0F12,0x000D},   //TVAR_ash_pGAS[343] //
{0x0F12,0x0008},   //TVAR_ash_pGAS[344] //
{0x0F12,0x0010},   //TVAR_ash_pGAS[345] //
{0x0F12,0x0028},   //TVAR_ash_pGAS[346] //
{0x0F12,0x0050},   //TVAR_ash_pGAS[347] //
{0x0F12,0x0083},   //TVAR_ash_pGAS[348] //
{0x0F12,0x00BF},   //TVAR_ash_pGAS[349] //
{0x0F12,0x00FB},   //TVAR_ash_pGAS[350] //
{0x0F12,0x00E9},   //TVAR_ash_pGAS[351] //
{0x0F12,0x00AE},   //TVAR_ash_pGAS[352] //
{0x0F12,0x0075},   //TVAR_ash_pGAS[353] //
{0x0F12,0x0042},   //TVAR_ash_pGAS[354] //
{0x0F12,0x001C},   //TVAR_ash_pGAS[355] //
{0x0F12,0x0008},   //TVAR_ash_pGAS[356] //
{0x0F12,0x0002},   //TVAR_ash_pGAS[357] //
{0x0F12,0x000A},   //TVAR_ash_pGAS[358] //
{0x0F12,0x0021},   //TVAR_ash_pGAS[359] //
{0x0F12,0x0048},   //TVAR_ash_pGAS[360] //
{0x0F12,0x007C},   //TVAR_ash_pGAS[361] //
{0x0F12,0x00B8},   //TVAR_ash_pGAS[362] //
{0x0F12,0x00F6},   //TVAR_ash_pGAS[363] //
{0x0F12,0x00EF},   //TVAR_ash_pGAS[364] //
{0x0F12,0x00B7},   //TVAR_ash_pGAS[365] //
{0x0F12,0x007F},   //TVAR_ash_pGAS[366] //
{0x0F12,0x004C},   //TVAR_ash_pGAS[367] //
{0x0F12,0x0025},   //TVAR_ash_pGAS[368] //
{0x0F12,0x000F},   //TVAR_ash_pGAS[369] //
{0x0F12,0x0008},   //TVAR_ash_pGAS[370] //
{0x0F12,0x0010},   //TVAR_ash_pGAS[371] //
{0x0F12,0x0027},   //TVAR_ash_pGAS[372] //
{0x0F12,0x004E},   //TVAR_ash_pGAS[373] //
{0x0F12,0x0080},   //TVAR_ash_pGAS[374] //
{0x0F12,0x00BA},   //TVAR_ash_pGAS[375] //
{0x0F12,0x00F5},   //TVAR_ash_pGAS[376] //
{0x0F12,0x0106},   //TVAR_ash_pGAS[377] //
{0x0F12,0x00D1},   //TVAR_ash_pGAS[378] //
{0x0F12,0x009B},   //TVAR_ash_pGAS[379] //
{0x0F12,0x0069},   //TVAR_ash_pGAS[380] //
{0x0F12,0x0041},   //TVAR_ash_pGAS[381] //
{0x0F12,0x0029},   //TVAR_ash_pGAS[382] //
{0x0F12,0x0020},   //TVAR_ash_pGAS[383] //
{0x0F12,0x0027},   //TVAR_ash_pGAS[384] //
{0x0F12,0x003F},   //TVAR_ash_pGAS[385] //
{0x0F12,0x0065},   //TVAR_ash_pGAS[386] //
{0x0F12,0x0096},   //TVAR_ash_pGAS[387] //
{0x0F12,0x00CC},   //TVAR_ash_pGAS[388] //
{0x0F12,0x0104},   //TVAR_ash_pGAS[389] //
{0x0F12,0x0125},   //TVAR_ash_pGAS[390] //
{0x0F12,0x00F2},   //TVAR_ash_pGAS[391] //
{0x0F12,0x00C2},   //TVAR_ash_pGAS[392] //
{0x0F12,0x0092},   //TVAR_ash_pGAS[393] //
{0x0F12,0x006C},   //TVAR_ash_pGAS[394] //
{0x0F12,0x0052},   //TVAR_ash_pGAS[395] //
{0x0F12,0x0048},   //TVAR_ash_pGAS[396] //
{0x0F12,0x004E},   //TVAR_ash_pGAS[397] //
{0x0F12,0x0064},   //TVAR_ash_pGAS[398] //
{0x0F12,0x0087},   //TVAR_ash_pGAS[399] //
{0x0F12,0x00B4},   //TVAR_ash_pGAS[400] //
{0x0F12,0x00E3},   //TVAR_ash_pGAS[401] //
{0x0F12,0x0118},   //TVAR_ash_pGAS[402] //
{0x0F12,0x0152},   //TVAR_ash_pGAS[403] //
{0x0F12,0x0119},   //TVAR_ash_pGAS[404] //
{0x0F12,0x00F0},   //TVAR_ash_pGAS[405] //
{0x0F12,0x00C6},   //TVAR_ash_pGAS[406] //
{0x0F12,0x00A3},   //TVAR_ash_pGAS[407] //
{0x0F12,0x008A},   //TVAR_ash_pGAS[408] //
{0x0F12,0x007F},   //TVAR_ash_pGAS[409] //
{0x0F12,0x0084},   //TVAR_ash_pGAS[410] //
{0x0F12,0x0096},   //TVAR_ash_pGAS[411] //
{0x0F12,0x00B5},   //TVAR_ash_pGAS[412] //
{0x0F12,0x00DA},   //TVAR_ash_pGAS[413] //
{0x0F12,0x0102},   //TVAR_ash_pGAS[414] //
{0x0F12,0x013D},   //TVAR_ash_pGAS[415] //
{0x0F12,0x018F},   //TVAR_ash_pGAS[416] //
{0x0F12,0x014A},   //TVAR_ash_pGAS[417] //
{0x0F12,0x0121},   //TVAR_ash_pGAS[418] //
{0x0F12,0x0101},   //TVAR_ash_pGAS[419] //
{0x0F12,0x00E4},   //TVAR_ash_pGAS[420] //
{0x0F12,0x00CD},   //TVAR_ash_pGAS[421] //
{0x0F12,0x00C2},   //TVAR_ash_pGAS[422] //
{0x0F12,0x00C4},   //TVAR_ash_pGAS[423] //
{0x0F12,0x00D3},   //TVAR_ash_pGAS[424] //
{0x0F12,0x00EA},   //TVAR_ash_pGAS[425] //
{0x0F12,0x0104},   //TVAR_ash_pGAS[426] //
{0x0F12,0x0129},   //TVAR_ash_pGAS[427] //
{0x0F12,0x016C},   //TVAR_ash_pGAS[428] //
{0x0F12,0x0126},   //TVAR_ash_pGAS[429] //
{0x0F12,0x00F5},   //TVAR_ash_pGAS[430] //
{0x0F12,0x00D5},   //TVAR_ash_pGAS[431] //
{0x0F12,0x00BD},   //TVAR_ash_pGAS[432] //
{0x0F12,0x00A9},   //TVAR_ash_pGAS[433] //
{0x0F12,0x009D},   //TVAR_ash_pGAS[434] //
{0x0F12,0x009A},   //TVAR_ash_pGAS[435] //
{0x0F12,0x00A2},   //TVAR_ash_pGAS[436] //
{0x0F12,0x00B3},   //TVAR_ash_pGAS[437] //
{0x0F12,0x00CE},   //TVAR_ash_pGAS[438] //
{0x0F12,0x00EC},   //TVAR_ash_pGAS[439] //
{0x0F12,0x0116},   //TVAR_ash_pGAS[440] //
{0x0F12,0x0150},   //TVAR_ash_pGAS[441] //
{0x0F12,0x0104},   //TVAR_ash_pGAS[442] //
{0x0F12,0x00DE},   //TVAR_ash_pGAS[443] //
{0x0F12,0x00BD},   //TVAR_ash_pGAS[444] //
{0x0F12,0x009D},   //TVAR_ash_pGAS[445] //
{0x0F12,0x0083},   //TVAR_ash_pGAS[446] //
{0x0F12,0x0074},   //TVAR_ash_pGAS[447] //
{0x0F12,0x0071},   //TVAR_ash_pGAS[448] //
{0x0F12,0x0078},   //TVAR_ash_pGAS[449] //
{0x0F12,0x008C},   //TVAR_ash_pGAS[450] //
{0x0F12,0x00AC},   //TVAR_ash_pGAS[451] //
{0x0F12,0x00D3},   //TVAR_ash_pGAS[452] //
{0x0F12,0x00FF},   //TVAR_ash_pGAS[453] //
{0x0F12,0x012E},   //TVAR_ash_pGAS[454] //
{0x0F12,0x00E0},   //TVAR_ash_pGAS[455] //
{0x0F12,0x00BD},   //TVAR_ash_pGAS[456] //
{0x0F12,0x0095},   //TVAR_ash_pGAS[457] //
{0x0F12,0x0071},   //TVAR_ash_pGAS[458] //
{0x0F12,0x0055},   //TVAR_ash_pGAS[459] //
{0x0F12,0x0045},   //TVAR_ash_pGAS[460] //
{0x0F12,0x0041},   //TVAR_ash_pGAS[461] //
{0x0F12,0x0048},   //TVAR_ash_pGAS[462] //
{0x0F12,0x005D},   //TVAR_ash_pGAS[463] //
{0x0F12,0x007E},   //TVAR_ash_pGAS[464] //
{0x0F12,0x00A9},   //TVAR_ash_pGAS[465] //
{0x0F12,0x00DA},   //TVAR_ash_pGAS[466] //
{0x0F12,0x0105},   //TVAR_ash_pGAS[467] //
{0x0F12,0x00C3},   //TVAR_ash_pGAS[468] //
{0x0F12,0x009F},   //TVAR_ash_pGAS[469] //
{0x0F12,0x0073},   //TVAR_ash_pGAS[470] //
{0x0F12,0x004D},   //TVAR_ash_pGAS[471] //
{0x0F12,0x0030},   //TVAR_ash_pGAS[472] //
{0x0F12,0x001F},   //TVAR_ash_pGAS[473] //
{0x0F12,0x001B},   //TVAR_ash_pGAS[474] //
{0x0F12,0x0022},   //TVAR_ash_pGAS[475] //
{0x0F12,0x0036},   //TVAR_ash_pGAS[476] //
{0x0F12,0x0057},   //TVAR_ash_pGAS[477] //
{0x0F12,0x0083},   //TVAR_ash_pGAS[478] //
{0x0F12,0x00B8},   //TVAR_ash_pGAS[479] //
{0x0F12,0x00E4},   //TVAR_ash_pGAS[480] //
{0x0F12,0x00B2},   //TVAR_ash_pGAS[481] //
{0x0F12,0x008C},   //TVAR_ash_pGAS[482] //
{0x0F12,0x005D},	//TVAR_ash_pGAS[483] //                                                                        
{0x0F12,0x0036},	//TVAR_ash_pGAS[484] //                                                                        
{0x0F12,0x0018},	//TVAR_ash_pGAS[485] //                                                                        
{0x0F12,0x000A},	//TVAR_ash_pGAS[486] //                                                                        
{0x0F12,0x0005},	//TVAR_ash_pGAS[487] //                                                                        
{0x0F12,0x000B},	//TVAR_ash_pGAS[488] //                                                                        
{0x0F12,0x001D},	//TVAR_ash_pGAS[489] //                                                                        
{0x0F12,0x003E},	//TVAR_ash_pGAS[490] //                                                                        
{0x0F12,0x006A},	//TVAR_ash_pGAS[491] //                                                                        
{0x0F12,0x009F},	//TVAR_ash_pGAS[492] //                                                                        
{0x0F12,0x00CE},	//TVAR_ash_pGAS[493] //                                                                        
{0x0F12,0x00B0},	//TVAR_ash_pGAS[494] //                                                                        
{0x0F12,0x0089},	//TVAR_ash_pGAS[495] //                                                                        
{0x0F12,0x0059},	//TVAR_ash_pGAS[496] //                                                                        
{0x0F12,0x0031},	//TVAR_ash_pGAS[497] //                                                                        
{0x0F12,0x0012},	//TVAR_ash_pGAS[498] //                                                                        
{0x0F12,0x0004},	//TVAR_ash_pGAS[499] //                                                                        
{0x0F12,0x0000},	//TVAR_ash_pGAS[500] //                                                                        
{0x0F12,0x0005},	//TVAR_ash_pGAS[501] //                                                                        
{0x0F12,0x0016},	//TVAR_ash_pGAS[502] //                                                                        
{0x0F12,0x0036},	//TVAR_ash_pGAS[503] //                                                                        
{0x0F12,0x0062},	//TVAR_ash_pGAS[504] //                                                                        
{0x0F12,0x0097},	//TVAR_ash_pGAS[505] //                                                                        
{0x0F12,0x00C7},	//TVAR_ash_pGAS[506] //                                                                        
{0x0F12,0x00B4},	//TVAR_ash_pGAS[507] //                                                                        
{0x0F12,0x008F},	//TVAR_ash_pGAS[508] //                                                                        
{0x0F12,0x0061},	//TVAR_ash_pGAS[509] //                                                                        
{0x0F12,0x0038},	//TVAR_ash_pGAS[510] //                                                                        
{0x0F12,0x0019},	//TVAR_ash_pGAS[511] //                                                                        
{0x0F12,0x0009},	//TVAR_ash_pGAS[512] //                                                                        
{0x0F12,0x0004},	//TVAR_ash_pGAS[513] //                                                                        
{0x0F12,0x0009},	//TVAR_ash_pGAS[514] //                                                                        
{0x0F12,0x001B},	//TVAR_ash_pGAS[515] //                                                                        
{0x0F12,0x003A},	//TVAR_ash_pGAS[516] //                                                                        
{0x0F12,0x0065},	//TVAR_ash_pGAS[517] //                                                                        
{0x0F12,0x0098},	//TVAR_ash_pGAS[518] //                                                                        
{0x0F12,0x00C7},	//TVAR_ash_pGAS[519] //                                                                        
{0x0F12,0x00C9},	//TVAR_ash_pGAS[520] //                                                                        
{0x0F12,0x00A6},	//TVAR_ash_pGAS[521] //                                                                        
{0x0F12,0x0079},	//TVAR_ash_pGAS[522] //                                                                        
{0x0F12,0x0052},	//TVAR_ash_pGAS[523] //                                                                        
{0x0F12,0x0033},	//TVAR_ash_pGAS[524] //                                                                        
{0x0F12,0x0021},	//TVAR_ash_pGAS[525] //                                                                        
{0x0F12,0x001A},	//TVAR_ash_pGAS[526] //                                                                        
{0x0F12,0x001F},	//TVAR_ash_pGAS[527] //                                                                        
{0x0F12,0x0030},	//TVAR_ash_pGAS[528] //                                                                        
{0x0F12,0x004F},	//TVAR_ash_pGAS[529] //                                                                        
{0x0F12,0x0079},	//TVAR_ash_pGAS[530] //                                                                        
{0x0F12,0x00A9},	//TVAR_ash_pGAS[531] //                                                                        
{0x0F12,0x00D3},	//TVAR_ash_pGAS[532] //                                                                        
{0x0F12,0x00E6},	//TVAR_ash_pGAS[533] //                                                                        
{0x0F12,0x00C5},	//TVAR_ash_pGAS[534] //                                                                        
{0x0F12,0x009D},	//TVAR_ash_pGAS[535] //                                                                        
{0x0F12,0x0077},	//TVAR_ash_pGAS[536] //                                                                        
{0x0F12,0x0059},	//TVAR_ash_pGAS[537] //                                                                        
{0x0F12,0x0046},	//TVAR_ash_pGAS[538] //                                                                        
{0x0F12,0x003E},	//TVAR_ash_pGAS[539] //                                                                        
{0x0F12,0x0042},	//TVAR_ash_pGAS[540] //                                                                        
{0x0F12,0x0052},	//TVAR_ash_pGAS[541] //                                                                        
{0x0F12,0x006E},	//TVAR_ash_pGAS[542] //                                                                        
{0x0F12,0x0094},	//TVAR_ash_pGAS[543] //                                                                        
{0x0F12,0x00BF},	//TVAR_ash_pGAS[544] //                                                                        
{0x0F12,0x00E8},	//TVAR_ash_pGAS[545] //                                                                        
{0x0F12,0x0114},	//TVAR_ash_pGAS[546] //                                                                        
{0x0F12,0x00ED},	//TVAR_ash_pGAS[547] //                                                                        
{0x0F12,0x00C9},	//TVAR_ash_pGAS[548] //                                                                        
{0x0F12,0x00A8},	//TVAR_ash_pGAS[549] //                                                                        
{0x0F12,0x008B},	//TVAR_ash_pGAS[550] //                                                                        
{0x0F12,0x007A},	//TVAR_ash_pGAS[551] //                                                                        
{0x0F12,0x0071},	//TVAR_ash_pGAS[552] //                                                                        
{0x0F12,0x0073},	//TVAR_ash_pGAS[553] //                                                                        
{0x0F12,0x0080},	//TVAR_ash_pGAS[554] //                                                                        
{0x0F12,0x0099},	//TVAR_ash_pGAS[555] //                                                                        
{0x0F12,0x00B8},	//TVAR_ash_pGAS[556] //                                                                        
{0x0F12,0x00DC},	//TVAR_ash_pGAS[557] //                                                                        
{0x0F12,0x010B},	//TVAR_ash_pGAS[558] //                                                                        
{0x0F12,0x014D},	//TVAR_ash_pGAS[559] //                                                                        
{0x0F12,0x0119},	//TVAR_ash_pGAS[560] //                                                                        
{0x0F12,0x00F6},	//TVAR_ash_pGAS[561] //                                                                        
{0x0F12,0x00DB},	//TVAR_ash_pGAS[562] //                                                                        
{0x0F12,0x00C4},	//TVAR_ash_pGAS[563] //                                                                        
{0x0F12,0x00B4},	//TVAR_ash_pGAS[564] //                                                                        
{0x0F12,0x00AB},	//TVAR_ash_pGAS[565] //                                                                        
{0x0F12,0x00AB},	//TVAR_ash_pGAS[566] //                                                                        
{0x0F12,0x00B5},	//TVAR_ash_pGAS[567] //                                                                        
{0x0F12,0x00C6},	//TVAR_ash_pGAS[568] //                                                                        
{0x0F12,0x00DD},	//TVAR_ash_pGAS[569] //                                                                        
{0x0F12,0x00FF},	//TVAR_ash_pGAS[570] //                                                                        
{0x0F12,0x0132},	//TVAR_ash_pGAS[571] //                                                                        
            
{0x002A,0x074E},	    
{0x0F12,0x0001},	//ash_bLumaMode //         	                                                                        
{0x002A,0x0D30},                               
{0x0F12,0x02A8},	//awbb_GLocu //        
{0x0F12,0x0347},	//awbb_GLocuSB //      
                                       
{0x002A,0x06B8},                               
{0x0F12,0x00DF},	//TVAR_ash_AwbashCord[0] //                                                                        
{0x0F12,0x0101},	//TVAR_ash_AwbashCord[1] //                                                                        
{0x0F12,0x012A},	//TVAR_ash_AwbashCord[2] //                                                                        
{0x0F12,0x013E},	//TVAR_ash_AwbashCord[3] //                                                                        
{0x0F12,0x0176},	//TVAR_ash_AwbashCord[4] //                                                                        
{0x0F12,0x01A6},	//TVAR_ash_AwbashCord[5] //                                                                        
{0x0F12,0x01B6},	//TVAR_ash_AwbashCord[6] //                                                                        
                                                                                       
{0x002A,0x0664},                               
{0x0F12,0x013E},	//seti_uContrastCenter //                                                                          
                                     
                                     
{0x002A,0x06C6},                               
{0x0F12,0x010B},	//ash_CGrasalphaS[0] //
{0x0F12,0x0103},	//ash_CGrasalphaS[1] //
{0x0F12,0x00FC},	//ash_CGrasalphaS[2] //
{0x0F12,0x010C},	//ash_CGrasalphaS[3] //
           
{0x002A,0x0C48},    
{0x0F12,0x0390},//03C8 //03C9	//awbb_IndoorGrZones_m_BGrid[0] // 		                                                                        
{0x0F12,0x03A6},//03DE //03DE	//awbb_IndoorGrZones_m_BGrid[1] //                                                                         
{0x0F12,0x033A},//0372 //0372	//awbb_IndoorGrZones_m_BGrid[2] //                                                                         
{0x0F12,0x03B2},//03EA //03EA	//awbb_IndoorGrZones_m_BGrid[3] //                                                                         
{0x0F12,0x02FC},//0336 //0336	//awbb_IndoorGrZones_m_BGrid[4] //                                                                         
{0x0F12,0x03A4},//03DE //03DE	//awbb_IndoorGrZones_m_BGrid[5] //                                                                         
{0x0F12,0x02CA},//0302 //0302	//awbb_IndoorGrZones_m_BGrid[6] //                                                                         
{0x0F12,0x0368},//03A2 //03A2	//awbb_IndoorGrZones_m_BGrid[7] //                                                                         
{0x0F12,0x0290},//02C8 //02c8	//awbb_IndoorGrZones_m_BGrid[8] //                                                                         
{0x0F12,0x0330},//0368 //0368	//awbb_IndoorGrZones_m_BGrid[9] //                                                                         
{0x0F12,0x025A},//0292 //0292	//awbb_IndoorGrZones_m_BGrid[10] //                                                                        
{0x0F12,0x0302},//033A //033A	//awbb_IndoorGrZones_m_BGrid[11] //                                                                        
{0x0F12,0x023E},//0276 //0262	//awbb_IndoorGrZones_m_BGrid[12] //                                                                        
{0x0F12,0x02CE},//0306 //0306	//awbb_IndoorGrZones_m_BGrid[13] //                                                                        
{0x0F12,0x0222},//025A //0250	//awbb_IndoorGrZones_m_BGrid[14] //                                                                        
{0x0F12,0x0288},//02C2 //02C2	//awbb_IndoorGrZones_m_BGrid[15] //                                                                        
{0x0F12,0x020E},//0246 //023A	//awbb_IndoorGrZones_m_BGrid[16] //                                                                        
{0x0F12,0x026A},//02A2 //02A2	//awbb_IndoorGrZones_m_BGrid[17] //                                                                        
{0x0F12,0x01F8},//0232 //0228	//awbb_IndoorGrZones_m_BGrid[18] //                                                                        
{0x0F12,0x0260},//0298 //0298	//awbb_IndoorGrZones_m_BGrid[19] //                                                                        
{0x0F12,0x01E6},//021E //0210	//awbb_IndoorGrZones_m_BGrid[20] //                                                                        
{0x0F12,0x0246},//027E //029C	//awbb_IndoorGrZones_m_BGrid[21] //                                                                        
{0x0F12,0x01D0},//0208 //01FE	//awbb_IndoorGrZones_m_BGrid[22] //                                                                        
{0x0F12,0x0234},//026C //0292	//awbb_IndoorGrZones_m_BGrid[23] //                                                                        
{0x0F12,0x01B6},//01EE //01EE	//awbb_IndoorGrZones_m_BGrid[24] //                                                                        
{0x0F12,0x0224},//025C //0278	//awbb_IndoorGrZones_m_BGrid[25] //                                                                        
{0x0F12,0x0000},//01F0 //01F2	//awbb_IndoorGrZones_m_BGrid[26] //                                                                        
{0x0F12,0x0000},//0248 //0268	//awbb_IndoorGrZones_m_BGrid[27] //                                                                        
{0x0F12,0x0000},//0000 //0200	//awbb_IndoorGrZones_m_BGrid[28] //                                                                        
{0x0F12,0x0000},//0000 //0246	//awbb_IndoorGrZones_m_BGrid[29] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[30] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[31] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[32] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[33] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[34] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[35] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[36] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[37] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[38] //                                                                        
{0x0F12,0x0000},//0000	//awbb_IndoorGrZones_m_BGrid[39] //                                                                        
                                              
{0x0F12,0x0005},   //awbb_IndoorGrZones_m_Gridstep //  

{0x002A,0x0C9C},
{0x0F12,0x000C},
{0x002A,0x0CA0},                                       
{0x0F12,0x0138},	//awbb_IndoorGrZones_m_Boffs //
                                               
{0x002A,0x0CE0},                                       
{0x0F12,0x0382},	//awbb_LowBrGrZones_m_BGrid[0] //                                                                          
{0x0F12,0x03EC},	//awbb_LowBrGrZones_m_BGrid[1] //                                                                          
{0x0F12,0x030A},	//awbb_LowBrGrZones_m_BGrid[2] //                                                                          
{0x0F12,0x03E6},	//awbb_LowBrGrZones_m_BGrid[3] //                                                                          
{0x0F12,0x02A0},	//awbb_LowBrGrZones_m_BGrid[4] //                                                                          
{0x0F12,0x03DA},	//awbb_LowBrGrZones_m_BGrid[5] //                                                                          
{0x0F12,0x024A},	//awbb_LowBrGrZones_m_BGrid[6] //                                                                          
{0x0F12,0x039E},	//awbb_LowBrGrZones_m_BGrid[7] //                                                                          
{0x0F12,0x020C},	//awbb_LowBrGrZones_m_BGrid[8] //                                                                          
{0x0F12,0x0344},	//awbb_LowBrGrZones_m_BGrid[9] //                                                                          
{0x0F12,0x01DC},	//awbb_LowBrGrZones_m_BGrid[10] //                                                                         
{0x0F12,0x02F4},	//awbb_LowBrGrZones_m_BGrid[11] //                                                                         
{0x0F12,0x01AE},	//awbb_LowBrGrZones_m_BGrid[12] //                                                                         
{0x0F12,0x02A4},	//awbb_LowBrGrZones_m_BGrid[13] //                                                                         
{0x0F12,0x017C},	//awbb_LowBrGrZones_m_BGrid[14] //                                                                         
{0x0F12,0x0276},	//awbb_LowBrGrZones_m_BGrid[15] //                                                                         
{0x0F12,0x0168},	//awbb_LowBrGrZones_m_BGrid[16] //                                                                         
{0x0F12,0x0234},	//awbb_LowBrGrZones_m_BGrid[17] //                                                                         
{0x0F12,0x0190},	//awbb_LowBrGrZones_m_BGrid[18] //                                                                         
{0x0F12,0x01E6},	//awbb_LowBrGrZones_m_BGrid[19] //                                                                         
{0x0F12,0x0000},	//awbb_LowBrGrZones_m_BGrid[20] //                                                                         
{0x0F12,0x0000},	//awbb_LowBrGrZones_m_BGrid[21] //                                                                         
{0x0F12,0x0000},	//awbb_LowBrGrZones_m_BGrid[22] //                                                                         
{0x0F12,0x0000},	//awbb_LowBrGrZones_m_BGrid[23] //                                                                         
            
{0x0F12,0x0006},	//awbb_LowBrGrZones_m_Gridstep //	                                                                        
{0x002A,0x0D18},                                     
{0x0F12,0x00EE},	//awbb_LowBrGrZones_m_Boff //

{0x002A,0x0CA4},    
{0x0F12,0x021C},	//awbb_OutdoorGrZones_m_BGrid[0] // 		                                                                        
{0x0F12,0x0252},	//awbb_OutdoorGrZones_m_BGrid[1] //                                                                         
{0x0F12,0x0210},	//awbb_OutdoorGrZones_m_BGrid[2] //                                                                         
{0x0F12,0x0256},	//awbb_OutdoorGrZones_m_BGrid[3] //                                                                         
{0x0F12,0x0204},	//awbb_OutdoorGrZones_m_BGrid[4] //                                                                         
{0x0F12,0x025C},	//awbb_OutdoorGrZones_m_BGrid[5] //                                                                         
{0x0F12,0x01F6},	//awbb_OutdoorGrZones_m_BGrid[6] //                                                                         
{0x0F12,0x0252},	//awbb_OutdoorGrZones_m_BGrid[7] //                                                                         
{0x0F12,0x01EC},	//awbb_OutdoorGrZones_m_BGrid[8] //                                                                         
{0x0F12,0x0248},	//awbb_OutdoorGrZones_m_BGrid[9] //                                                                         
{0x0F12,0x01E8},	//awbb_OutdoorGrZones_m_BGrid[10] //                                                                        
{0x0F12,0x023E},	//awbb_OutdoorGrZones_m_BGrid[11] //                                                                        
{0x0F12,0x01E8},	//awbb_OutdoorGrZones_m_BGrid[12] //                                                                        
{0x0F12,0x022C},	//awbb_OutdoorGrZones_m_BGrid[13] //                                                                        
{0x0F12,0x01F2},	//awbb_OutdoorGrZones_m_BGrid[14] //                                                                        
{0x0F12,0x0214},	//awbb_OutdoorGrZones_m_BGrid[15] //                                                                        
{0x0F12,0x0000},	//awbb_OutdoorGrZones_m_BGrid[16] //                                                                        
{0x0F12,0x0000},	//awbb_OutdoorGrZones_m_BGrid[17] //                                                                        
{0x0F12,0x0000},	//awbb_OutdoorGrZones_m_BGrid[18] //                                                                        
{0x0F12,0x0000},	//awbb_OutdoorGrZones_m_BGrid[19] //                                                                        
{0x0F12,0x0000},	//awbb_OutdoorGrZones_m_BGrid[20] //                                                                        
{0x0F12,0x0000},	//awbb_OutdoorGrZones_m_BGrid[21] //                                                                        
{0x0F12,0x0000},	//awbb_OutdoorGrZones_m_BGrid[22] //                                                                        
{0x0F12,0x0000},	//awbb_OutdoorGrZones_m_BGrid[23] //                                                                        

{0x0F12,0x0004},	//awbb_OutdoorGrZones_m_Gridstep //       
{0x002A,0x0CD8},                                                          
{0x0F12,0x0008},               
{0x002A,0x0CDC},                                                          
{0x0F12,0x023A},	//awbb_OutdoorGrZones_m_Boff //                   
{0x002A,0x0D1C},                                                          
{0x0F12,0x037C},	//awbb_CrclLowT_R_c //                            
{0x002A,0x0D20},                                                          
{0x0F12,0x0157},	//awbb_CrclLowT_B_c //                            
{0x002A,0x0D24},                                                          
{0x0F12,0x3EB8},	//awbb_CrclLowT_Rad_c // 

{0x002A,0x0D2C},    
{0x0F12,0x014C},	//awbb_IntcR //	                                                                        
{0x0F12,0x00FB},	//awbb_IntcB //                                                                        
{0x002A,0x0D46},    
{0x0F12,0x04C0},	//0554//055D//0396//04A2//awbb_MvEq_RBthresh //	                                                                        
           
           
           
{0x002A,0x0D28},    //wp outdoor
{0x0F12,0x0270},	                                                                        
{0x0F12,0x0240},	                                                                        
                                                          
                              
{0x002A,0x0D5C},    
{0x0F12,0x7FFF},	                                                                        
{0x0F12,0x0050},	                                                                        
            
{0x002A,0x2316},    
{0x0F12,0x0006},	                                                                        
            
{0x002A,0x0E44},    
{0x0F12,0x0525},	                                                                        
{0x0F12,0x0400},	                                                                        
{0x0F12,0x078C},	                                                                        
           
{0x002A,0x0E36},    
{0x0F12,0x0028},	 //R OFFSET                                                                       
{0x0F12,0xFFD8},	 //B OFFSET                                                                       
{0x0F12,0x0000},	 //G OFFSET                                                                       

{0x002A,0x0DD4},    
{0x0F12,0x0000},	//awbb_GridCorr_R[0] //        	                                                                        
{0x0F12,0x0000},	//awbb_GridCorr_R[1] //    
{0x0F12,0x0000},	//awbb_GridCorr_R[2] //    
{0x0F12,0x0000},	//awbb_GridCorr_R[3] //    
{0x0F12,0xFFF0},	//awbb_GridCorr_R[4] //    
{0x0F12,0x0080},	//awbb_GridCorr_R[5] //    
      
{0x0F12,0x0000},	//awbb_GridCorr_R[6] //    
{0x0F12,0x0000},	//awbb_GridCorr_R[7] //    
{0x0F12,0x0000},	//awbb_GridCorr_R[8] //    
{0x0F12,0x0000},	//awbb_GridCorr_R[9] //    
{0x0F12,0xFFF0},	//awbb_GridCorr_R[10] //   
{0x0F12,0x0080},	//awbb_GridCorr_R[11] //   
     
{0x0F12,0x0000},	//awbb_GridCorr_R[12] //   
{0x0F12,0x0000},	//awbb_GridCorr_R[13] //   
{0x0F12,0x0000},	//awbb_GridCorr_R[14] //   
{0x0F12,0x0000},	//awbb_GridCorr_R[15] //   
{0x0F12,0xFFF0},	//awbb_GridCorr_R[16] //   
{0x0F12,0x0080},	//awbb_GridCorr_R[17] //   
      
{0x0F12,0x0000},	//awbb_GridCorr_B[0] ////                                                                        
{0x0F12,0x0000},	//awbb_GridCorr_B[1] //                                                                          
{0x0F12,0x0032},	//awbb_GridCorr_B[2] //                                                                          
{0x0F12,0x0000},	//awbb_GridCorr_B[3] //                                                                          
{0x0F12,0x0020},	//awbb_GridCorr_B[4] //                                                                          
{0x0F12,0xFFC0},	//awbb_GridCorr_B[5] //                                                                          
 
{0x0F12,0x0000},	//awbb_GridCorr_B[6] //                                                                          
{0x0F12,0x0000},	//awbb_GridCorr_B[7] //                                                                          
{0x0F12,0x0032},	//awbb_GridCorr_B[8] //                                                                          
{0x0F12,0x0000},	//awbb_GridCorr_B[9] //                                                                          
{0x0F12,0x0020},	//awbb_GridCorr_B[10] //                                                                         
{0x0F12,0xFFC0},	//awbb_GridCorr_B[11] //                                                                         
  
{0x0F12,0x0000},	//awbb_GridCorr_B[12] //                                                                         
{0x0F12,0x0000},	//awbb_GridCorr_B[13] //                                                                         
{0x0F12,0x0032},	//awbb_GridCorr_B[14] //                                                                         
{0x0F12,0x0000},	//awbb_GridCorr_B[15] //                                                                         
{0x0F12,0x0020},	//awbb_GridCorr_B[16] //   
{0x0F12,0xFFC0},	//awbb_GridCorr_B[17] //                 
      
{0x0F12,0x02D9},	//awbb_GridConst_1[0] //                                                                     
{0x0F12,0x0357},	//awbb_GridConst_1[1] //                                                                     
{0x0F12,0x03D1},	//awbb_GridConst_1[2] //                                                                     
      
      
{0x0F12,0x0DF6},	//0E4F//0DE9//0DE9//awbb_GridConst_2[0] //                                                   
{0x0F12,0x0ED8},	//0EDD//0EDD//0EDD//awbb_GridConst_2[1] //                                                   
{0x0F12,0x0F51},	//0F42//0F42//0F42//awbb_GridConst_2[2] //                                                   
{0x0F12,0x0F5C},	//0F4E//0F4E//0F54//awbb_GridConst_2[3] //                                                   
{0x0F12,0x0F8F},	//0F99//0F99//0FAE//awbb_GridConst_2[4] //                                                   
{0x0F12,0x1006},	//1006//1006//1011//awbb_GridConst_2[5] //                                                   
       
{0x0F12,0x00AC},	//00BA//awbb_GridCoeff_R_1                                                                   
{0x0F12,0x00BD},	//00AF//awbb_GridCoeff_B_1                                                                   
{0x0F12,0x0049},	//0049//awbb_GridCoeff_R_2                                                                   
{0x0F12,0x00F5},	//00F5//awbb_GridCoeff_B_2                                                                   
      
{0x002A,0x0E4A},                         
{0x0F12,0x0002},	//awbb_GridEnable//                                                                          
     
{0x002A,0x051A},                         
{0x0F12,0x010E},	//lt_uLimitHigh//
{0x0F12,0x00F5},	//lt_uLimitLow// 
      
      
{0x002A,0x0F76},                         
{0x0F12,0x0007},	//ae_statmode BLC off : 0x0F, on : 0x0D//  illumType On : 07 , Off : 0F
      
{0x002A,0x1034},                         
{0x0F12,0x00C0},	//saRR_IllumType[0] //                                                                       
{0x0F12,0x00E0},	//saRR_IllumType[1] //                                                                       
{0x0F12,0x0104},	//saRR_IllumType[2] //                                                                       
{0x0F12,0x0129},	//saRR_IllumType[3] //                                                                       
{0x0F12,0x0156},	//saRR_IllumType[4] //                                                                       
{0x0F12,0x017F},	//saRR_IllumType[5] //                                                                       
{0x0F12,0x018F},	//saRR_IllumType[6] //                                                                       
      
      
{0x0F12,0x0120},	//saRR_IllumTypeF[0] //                                                                      
{0x0F12,0x0120},	//saRR_IllumTypeF[1] //                                                                      
{0x0F12,0x0120},	//saRR_IllumTypeF[2] //                                                                      
{0x0F12,0x0100},	//saRR_IllumTypeF[3] //                                                                      
{0x0F12,0x0100},	//saRR_IllumTypeF[4] //                                                                      
{0x0F12,0x0100},	//saRR_IllumTypeF[5] //                                                                      
{0x0F12,0x0100},	//saRR_IllumTypeF[6] //                                                                      
           
           
           
{0x002A,0x3288},	//saRR_usDualGammaLutRGBIndoor  //               	                                                                        
{0x0F12,0x0000}, //	saRR_usDualGammaLutRGBIndoor[0] //[0] //                                                 
{0x0F12,0x0008}, //  saRR_usDualGammaLutRGBIndoor[0] //[1] //                                                       
{0x0F12,0x0013}, //  saRR_usDualGammaLutRGBIndoor[0] //[2] //                                                       
{0x0F12,0x002C}, //  saRR_usDualGammaLutRGBIndoor[0] //[3] //                                                       
{0x0F12,0x0062}, //  saRR_usDualGammaLutRGBIndoor[0] //[4] //                                                       
{0x0F12,0x00CD}, //  saRR_usDualGammaLutRGBIndoor[0] //[5] //                                                       
{0x0F12,0x0129}, //  saRR_usDualGammaLutRGBIndoor[0] //[6] //                                                       
{0x0F12,0x0151}, //  saRR_usDualGammaLutRGBIndoor[0] //[7] //                                                       
{0x0F12,0x0174}, //  saRR_usDualGammaLutRGBIndoor[0] //[8] //                                                       
{0x0F12,0x01AA}, //  saRR_usDualGammaLutRGBIndoor[0] //[9] //                                                       
{0x0F12,0x01D7}, //  saRR_usDualGammaLutRGBIndoor[0] //[10] //                                                      
{0x0F12,0x01FE}, //  saRR_usDualGammaLutRGBIndoor[0] //[11] //                                                      
{0x0F12,0x0221}, //  saRR_usDualGammaLutRGBIndoor[0] //[12] //                                                      
{0x0F12,0x025D}, //  saRR_usDualGammaLutRGBIndoor[0] //[13] //                                                      
{0x0F12,0x0291}, //  saRR_usDualGammaLutRGBIndoor[0] //[14] //                                                      
{0x0F12,0x02EB}, //  saRR_usDualGammaLutRGBIndoor[0] //[15] //                                                      
{0x0F12,0x033A}, //  saRR_usDualGammaLutRGBIndoor[0] //[16] //                                                      
{0x0F12,0x0380}, //  saRR_usDualGammaLutRGBIndoor[0] //[17] //                                                      
{0x0F12,0x03C2}, //  saRR_usDualGammaLutRGBIndoor[0] //[18] //                                                      
{0x0F12,0x03FF}, //  saRR_usDualGammaLutRGBIndoor[0] //[19] //                                                      
{0x0F12,0x0000}, //  saRR_usDualGammaLutRGBIndoor[1] //[0] //                                                       
{0x0F12,0x0008}, //  saRR_usDualGammaLutRGBIndoor[1] //[1] //                                                       
{0x0F12,0x0013}, //  saRR_usDualGammaLutRGBIndoor[1] //[2] //                                                       
{0x0F12,0x002C}, //  saRR_usDualGammaLutRGBIndoor[1] //[3] //                                                       
{0x0F12,0x0062}, //  saRR_usDualGammaLutRGBIndoor[1] //[4] //                                                       
{0x0F12,0x00CD}, //  saRR_usDualGammaLutRGBIndoor[1] //[5] //                                                       
{0x0F12,0x0129}, //  saRR_usDualGammaLutRGBIndoor[1] //[6] //                                                       
{0x0F12,0x0151}, //  saRR_usDualGammaLutRGBIndoor[1] //[7] //                                                       
{0x0F12,0x0174}, //  saRR_usDualGammaLutRGBIndoor[1] //[8] //                                                       
{0x0F12,0x01AA}, //  saRR_usDualGammaLutRGBIndoor[1] //[9] //                                                       
{0x0F12,0x01D7}, //  saRR_usDualGammaLutRGBIndoor[1] //[10] //                                                      
{0x0F12,0x01FE}, //  saRR_usDualGammaLutRGBIndoor[1] //[11] //                                                      
{0x0F12,0x0221}, //  saRR_usDualGammaLutRGBIndoor[1] //[12] //                                                      
{0x0F12,0x025D}, //  saRR_usDualGammaLutRGBIndoor[1] //[13] //                                                      
{0x0F12,0x0291}, //  saRR_usDualGammaLutRGBIndoor[1] //[14] //                                                      
{0x0F12,0x02EB}, //  saRR_usDualGammaLutRGBIndoor[1] //[15] //                                                      
{0x0F12,0x033A}, //  saRR_usDualGammaLutRGBIndoor[1] //[16] //                                                      
{0x0F12,0x0380}, //  saRR_usDualGammaLutRGBIndoor[1] //[17] //                                                      
{0x0F12,0x03C2}, //  saRR_usDualGammaLutRGBIndoor[1] //[18] //                                                      
{0x0F12,0x03FF}, //  saRR_usDualGammaLutRGBIndoor[1] //[19] //                                                      
{0x0F12,0x0000}, //  saRR_usDualGammaLutRGBIndoor[2] //[0] //                                                       
{0x0F12,0x0008}, //  saRR_usDualGammaLutRGBIndoor[2] //[1] //                                                       
{0x0F12,0x0013}, //  saRR_usDualGammaLutRGBIndoor[2] //[2] //                                                       
{0x0F12,0x002C}, //  saRR_usDualGammaLutRGBIndoor[2] //[3] //                                                       
{0x0F12,0x0062}, //  saRR_usDualGammaLutRGBIndoor[2] //[4] //                                                       
{0x0F12,0x00CD}, //  saRR_usDualGammaLutRGBIndoor[2] //[5] //                                                       
{0x0F12,0x0129}, //  saRR_usDualGammaLutRGBIndoor[2] //[6] //                                                       
{0x0F12,0x0151}, //  saRR_usDualGammaLutRGBIndoor[2] //[7] //                                                       
{0x0F12,0x0174}, //  saRR_usDualGammaLutRGBIndoor[2] //[8] //                                                       
{0x0F12,0x01AA}, //  saRR_usDualGammaLutRGBIndoor[2] //[9] //                                                       
{0x0F12,0x01D7}, //  saRR_usDualGammaLutRGBIndoor[2] //[10] //                                                      
{0x0F12,0x01FE}, //  saRR_usDualGammaLutRGBIndoor[2] //[11] //                                                      
{0x0F12,0x0221}, //  saRR_usDualGammaLutRGBIndoor[2] //[12] //                                                      
{0x0F12,0x025D}, //  saRR_usDualGammaLutRGBIndoor[2] //[13] //                                                      
{0x0F12,0x0291}, //  saRR_usDualGammaLutRGBIndoor[2] //[14] //                                                      
{0x0F12,0x02EB}, //  saRR_usDualGammaLutRGBIndoor[2] //[15] //                                                      
{0x0F12,0x033A}, //  saRR_usDualGammaLutRGBIndoor[2] //[16] //                                                      
{0x0F12,0x0380}, //  saRR_usDualGammaLutRGBIndoor[2] //[17] //                                                      
{0x0F12,0x03C2}, //  saRR_usDualGammaLutRGBIndoor[2] //[18] //                                                      
{0x0F12,0x03FF}, //  saRR_usDualGammaLutRGBIndoor[2] //[19] //                                                      
                                 
                                 
{0x0F12,0x0000},	//			saRR_usDualGammaLutRGBOutdoor[0] //[0] //
{0x0F12,0x0008},	//  saRR_usDualGammaLutRGBOutdoor[0] //[1] //                                                
{0x0F12,0x0013},	//  saRR_usDualGammaLutRGBOutdoor[0] //[2] //                                                
{0x0F12,0x002C},	//  saRR_usDualGammaLutRGBOutdoor[0] //[3] //                                                
{0x0F12,0x0062},	//  saRR_usDualGammaLutRGBOutdoor[0] //[4] //                                                
{0x0F12,0x00CD},	//  saRR_usDualGammaLutRGBOutdoor[0] //[5] //                                                
{0x0F12,0x0129},	//  saRR_usDualGammaLutRGBOutdoor[0] //[6] //                                                
{0x0F12,0x0151},	//  saRR_usDualGammaLutRGBOutdoor[0] //[7] //                                                
{0x0F12,0x0174},	//  saRR_usDualGammaLutRGBOutdoor[0] //[8] //                                                
{0x0F12,0x01AA},	//  saRR_usDualGammaLutRGBOutdoor[0] //[9] //                                                
{0x0F12,0x01D7},	//  saRR_usDualGammaLutRGBOutdoor[0] //[10] //                                               
{0x0F12,0x01FE},	//  saRR_usDualGammaLutRGBOutdoor[0] //[11] //                                               
{0x0F12,0x0221},	//  saRR_usDualGammaLutRGBOutdoor[0] //[12] //                                               
{0x0F12,0x025D},	//  saRR_usDualGammaLutRGBOutdoor[0] //[13] //                                               
{0x0F12,0x0291},	//  saRR_usDualGammaLutRGBOutdoor[0] //[14] //                                               
{0x0F12,0x02EB},	//  saRR_usDualGammaLutRGBOutdoor[0] //[15] //                                               
{0x0F12,0x033A},	//  saRR_usDualGammaLutRGBOutdoor[0] //[16] //                                               
{0x0F12,0x0380},	//  saRR_usDualGammaLutRGBOutdoor[0] //[17] //                                               
{0x0F12,0x03C2},	//  saRR_usDualGammaLutRGBOutdoor[0] //[18] //                                               
{0x0F12,0x03FF},	//  saRR_usDualGammaLutRGBOutdoor[0] //[19] //                                               
{0x0F12,0x0000},	//  saRR_usDualGammaLutRGBOutdoor[1] //[0] //                                                
{0x0F12,0x0008},	//  saRR_usDualGammaLutRGBOutdoor[1] //[1] //                                                
{0x0F12,0x0013},	//  saRR_usDualGammaLutRGBOutdoor[1] //[2] //                                                
{0x0F12,0x002C},	//  saRR_usDualGammaLutRGBOutdoor[1] //[3] //                                                
{0x0F12,0x0062},	//  saRR_usDualGammaLutRGBOutdoor[1] //[4] //                                                
{0x0F12,0x00CD},	//  saRR_usDualGammaLutRGBOutdoor[1] //[5] //                                                
{0x0F12,0x0129},	//  saRR_usDualGammaLutRGBOutdoor[1] //[6] //                                                
{0x0F12,0x0151},	//  saRR_usDualGammaLutRGBOutdoor[1] //[7] //                                                
{0x0F12,0x0174},	//  saRR_usDualGammaLutRGBOutdoor[1] //[8] //                                                
{0x0F12,0x01AA},	//  saRR_usDualGammaLutRGBOutdoor[1] //[9] //                                                
{0x0F12,0x01D7},	//  saRR_usDualGammaLutRGBOutdoor[1] //[10] //                                               
{0x0F12,0x01FE},	//  saRR_usDualGammaLutRGBOutdoor[1] //[11] //                                               
{0x0F12,0x0221},	//  saRR_usDualGammaLutRGBOutdoor[1] //[12] //                                               
{0x0F12,0x025D},	//  saRR_usDualGammaLutRGBOutdoor[1] //[13] //                                               
{0x0F12,0x0291},	//  saRR_usDualGammaLutRGBOutdoor[1] //[14] //                                               
{0x0F12,0x02EB},	//  saRR_usDualGammaLutRGBOutdoor[1] //[15] //                                               
{0x0F12,0x033A},	//  saRR_usDualGammaLutRGBOutdoor[1] //[16] //                                               
{0x0F12,0x0380},	//  saRR_usDualGammaLutRGBOutdoor[1] //[17] //                                               
{0x0F12,0x03C2},	//  saRR_usDualGammaLutRGBOutdoor[1] //[18] //                                               
{0x0F12,0x03FF},	//  saRR_usDualGammaLutRGBOutdoor[1] //[19] //                                               
{0x0F12,0x0000},	//  saRR_usDualGammaLutRGBOutdoor[2] //[0] //                                                
{0x0F12,0x0008},	//  saRR_usDualGammaLutRGBOutdoor[2] //[1] //                                                
{0x0F12,0x0013},	//  saRR_usDualGammaLutRGBOutdoor[2] //[2] //                                                
{0x0F12,0x002C},	//  saRR_usDualGammaLutRGBOutdoor[2] //[3] //                                                
{0x0F12,0x0062},	//  saRR_usDualGammaLutRGBOutdoor[2] //[4] //                                                
{0x0F12,0x00CD},	//  saRR_usDualGammaLutRGBOutdoor[2] //[5] //                                                
{0x0F12,0x0129},	//  saRR_usDualGammaLutRGBOutdoor[2] //[6] //                                                
{0x0F12,0x0151},	//  saRR_usDualGammaLutRGBOutdoor[2] //[7] //                                                
{0x0F12,0x0174},	//  saRR_usDualGammaLutRGBOutdoor[2] //[8] //                                                
{0x0F12,0x01AA},	//  saRR_usDualGammaLutRGBOutdoor[2] //[9] //                                                
{0x0F12,0x01D7},	//  saRR_usDualGammaLutRGBOutdoor[2] //[10] //                                               
{0x0F12,0x01FE},	//  saRR_usDualGammaLutRGBOutdoor[2] //[11] //                                               
{0x0F12,0x0221},	//  saRR_usDualGammaLutRGBOutdoor[2] //[12] //                                               
{0x0F12,0x025D},	//  saRR_usDualGammaLutRGBOutdoor[2] //[13] //                                               
{0x0F12,0x0291},	//  saRR_usDualGammaLutRGBOutdoor[2] //[14] //                                               
{0x0F12,0x02EB},	//  saRR_usDualGammaLutRGBOutdoor[2] //[15] //                                               
{0x0F12,0x033A},	//  saRR_usDualGammaLutRGBOutdoor[2] //[16] //                                               
{0x0F12,0x0380},	//  saRR_usDualGammaLutRGBOutdoor[2] //[17] //                                               
{0x0F12,0x03C2},	//  saRR_usDualGammaLutRGBOutdoor[2] //[18] //                                               
{0x0F12,0x03FF},	//  saRR_usDualGammaLutRGBOutdoor[2] //[19] //                                               
                                 
                                 
{0x002A,0x06A6},                           
{0x0F12,0x00C0},	//saRR_AwbCcmCord[0] //                      
{0x0F12,0x00E0},	//saRR_AwbCcmCord[1] //                      
{0x0F12,0x0110},	//saRR_AwbCcmCord[2] //                      
{0x0F12,0x0139},	//saRR_AwbCcmCord[3] //                      
{0x0F12,0x0166},	//saRR_AwbCcmCord[4] //                      
{0x0F12,0x019F},	//saRR_AwbCcmCord[5] //                      
           
{0x002A,0x33A4},    
{0x0F12,0x0181},	  //TVAR_wbt_pBaseCcmS[0] //                                                           
{0x0F12,0xFF88},   //TVAR_wbt_pBaseCcmS[1] //                                                        
{0x0F12,0xFF90},   //TVAR_wbt_pBaseCcmS[2] //                                                        
{0x0F12,0xFE6B},   //TVAR_wbt_pBaseCcmS[3] //                                                        
{0x0F12,0x0106},   //TVAR_wbt_pBaseCcmS[4] //                                                        
{0x0F12,0xFF0B},   //TVAR_wbt_pBaseCcmS[5] //                                                        
{0x0F12,0xFFDD},   //TVAR_wbt_pBaseCcmS[6] //                                                        
{0x0F12,0xFFEE},   //TVAR_wbt_pBaseCcmS[7] //                                                        
{0x0F12,0x01CB},   //TVAR_wbt_pBaseCcmS[8] //                                                        
{0x0F12,0x0187},   //TVAR_wbt_pBaseCcmS[9] //                                                        
{0x0F12,0x00A6},   //TVAR_wbt_pBaseCcmS[10] //                                                       
{0x0F12,0xFEBE},   //TVAR_wbt_pBaseCcmS[11] //                                                       
{0x0F12,0x021C},   //TVAR_wbt_pBaseCcmS[12] //                                                       
{0x0F12,0xFF5F},   //TVAR_wbt_pBaseCcmS[13] //                                                       
{0x0F12,0x0175},   //TVAR_wbt_pBaseCcmS[14] //                                                       
{0x0F12,0xFEE7},   //TVAR_wbt_pBaseCcmS[15] //                                                       
{0x0F12,0x0106},   //TVAR_wbt_pBaseCcmS[16] //                                                       
{0x0F12,0x00F3},   //TVAR_wbt_pBaseCcmS[17] //                                                       

{0x0F12,0x0181},   //TVAR_wbt_pBaseCcmS[18] //                                                       
{0x0F12,0xFF88},   //TVAR_wbt_pBaseCcmS[19] //                                                       
{0x0F12,0xFF90},   //TVAR_wbt_pBaseCcmS[20] //                                                       
{0x0F12,0xFE6B},   //TVAR_wbt_pBaseCcmS[21] //                                                       
{0x0F12,0x0106},   //TVAR_wbt_pBaseCcmS[22] //                                                       
{0x0F12,0xFF0B},   //TVAR_wbt_pBaseCcmS[23] //                                                       
{0x0F12,0xFFDD},   //TVAR_wbt_pBaseCcmS[24] //                                                       
{0x0F12,0xFFEE},   //TVAR_wbt_pBaseCcmS[25] //                                                       
{0x0F12,0x01CB},   //TVAR_wbt_pBaseCcmS[26] //                                                       
{0x0F12,0x0187},   //TVAR_wbt_pBaseCcmS[27] //                                                       
{0x0F12,0x00A6},   //TVAR_wbt_pBaseCcmS[28] //                                                       
{0x0F12,0xFEBE},   //TVAR_wbt_pBaseCcmS[29] //                                                       
{0x0F12,0x021C},   //TVAR_wbt_pBaseCcmS[30] //                                                       
{0x0F12,0xFF5F},   //TVAR_wbt_pBaseCcmS[31] //                                                       
{0x0F12,0x0175},   //TVAR_wbt_pBaseCcmS[32] //                                                       
{0x0F12,0xFEE7},   //TVAR_wbt_pBaseCcmS[33] //                                                       
{0x0F12,0x0106},   //TVAR_wbt_pBaseCcmS[34] //                                                       
{0x0F12,0x00F3},   //TVAR_wbt_pBaseCcmS[35] //                                                       

{0x0F12,0x0181},   //TVAR_wbt_pBaseCcmS[36] //                                                       
{0x0F12,0xFF88},   //TVAR_wbt_pBaseCcmS[37] //                                                       
{0x0F12,0xFF90},   //TVAR_wbt_pBaseCcmS[38] //                                                       
{0x0F12,0xFE6B},   //TVAR_wbt_pBaseCcmS[39] //                                                       
{0x0F12,0x0106},   //TVAR_wbt_pBaseCcmS[40] //                                                       
{0x0F12,0xFF0B},   //TVAR_wbt_pBaseCcmS[41] //                                                       
{0x0F12,0xFFDD},   //TVAR_wbt_pBaseCcmS[42] //                                                       
{0x0F12,0xFFEE},   //TVAR_wbt_pBaseCcmS[43] //                                                       
{0x0F12,0x01CB},   //TVAR_wbt_pBaseCcmS[44] //                                                       
{0x0F12,0x0187},   //TVAR_wbt_pBaseCcmS[45] //                                                       
{0x0F12,0x00A6},   //TVAR_wbt_pBaseCcmS[46] //                                                       
{0x0F12,0xFEBE},   //TVAR_wbt_pBaseCcmS[47] //                                                       
{0x0F12,0x021C},   //TVAR_wbt_pBaseCcmS[48] //                                                       
{0x0F12,0xFF5F},   //TVAR_wbt_pBaseCcmS[49] //                                                       
{0x0F12,0x0175},   //TVAR_wbt_pBaseCcmS[50] //                                                       
{0x0F12,0xFEE7},   //TVAR_wbt_pBaseCcmS[51] //                                                       
{0x0F12,0x0106},   //TVAR_wbt_pBaseCcmS[52] //                                                       
{0x0F12,0x00F3},   //TVAR_wbt_pBaseCcmS[53] //                                                                        

{0x0F12,0x01FD},  //01FA   //TVAR_wbt_pBaseCcmS[54] //                                                       
{0x0F12,0xFFAB},  //FF9B   //TVAR_wbt_pBaseCcmS[55] //                                                       
{0x0F12,0xFFED},  //FFFF   //TVAR_wbt_pBaseCcmS[56] //                                                       
{0x0F12,0xFEB5},  //FE9F   //TVAR_wbt_pBaseCcmS[57] //                                                       
{0x0F12,0x0112},  //010F   //TVAR_wbt_pBaseCcmS[58] //                                                       
{0x0F12,0xFEDC},  //FEF5   //TVAR_wbt_pBaseCcmS[59] //                                                       
{0x0F12,0xFFD2},  //FFD2   //TVAR_wbt_pBaseCcmS[60] //                                                       
{0x0F12,0x0015},  //0015   //TVAR_wbt_pBaseCcmS[61] //                                                       
{0x0F12,0x01A1},  //01A1   //TVAR_wbt_pBaseCcmS[62] //                                                       
{0x0F12,0x0111},  //0111   //TVAR_wbt_pBaseCcmS[63] //                                                       
{0x0F12,0x009D},  //009D   //TVAR_wbt_pBaseCcmS[64] //                                                       
{0x0F12,0xFECB},  //FECB   //TVAR_wbt_pBaseCcmS[65] //                                                       
{0x0F12,0x01FC},  //01FC   //TVAR_wbt_pBaseCcmS[66] //                                                       
{0x0F12,0xFF99},  //FF99   //TVAR_wbt_pBaseCcmS[67] //                                                       
{0x0F12,0x01A9},  //01A9   //TVAR_wbt_pBaseCcmS[68] //                                                       
{0x0F12,0xFF26},  //FF26   //TVAR_wbt_pBaseCcmS[69] //                                                       
{0x0F12,0x012B},  //012B   //TVAR_wbt_pBaseCcmS[70] //                                                       
{0x0F12,0x00DF},  //00DF   //TVAR_wbt_pBaseCcmS[71] //    

{0x0F12,0x01E3},  //01E2   //TVAR_wbt_pBaseCcmS[72] //                                                 
{0x0F12,0xFFA4},  //FF9A   //TVAR_wbt_pBaseCcmS[73] //                                                 
{0x0F12,0xFFDB},  //FFE7   //TVAR_wbt_pBaseCcmS[74] //                                                 
{0x0F12,0xFEB5},  //FE9F   //TVAR_wbt_pBaseCcmS[75] //                                                 
{0x0F12,0x0112},  //010F   //TVAR_wbt_pBaseCcmS[76] //                                                 
{0x0F12,0xFEDC},  //FEF5   //TVAR_wbt_pBaseCcmS[77] //                                                 
{0x0F12,0xFFD2},  //FFD2   //TVAR_wbt_pBaseCcmS[78] //                                                 
{0x0F12,0xFFFE},  //FFFE   //TVAR_wbt_pBaseCcmS[79] //                                                 
{0x0F12,0x01B7},  //01B7   //TVAR_wbt_pBaseCcmS[80] //                                                 
{0x0F12,0x00E8},  //00E8   //TVAR_wbt_pBaseCcmS[81] //                                                 
{0x0F12,0x0095},  //0095   //TVAR_wbt_pBaseCcmS[82] //                                                 
{0x0F12,0xFF0D},  //FF0D   //TVAR_wbt_pBaseCcmS[83] //                                                 
{0x0F12,0x0182},  //0182   //TVAR_wbt_pBaseCcmS[84] //                                                 
{0x0F12,0xFF29},  //FF29   //TVAR_wbt_pBaseCcmS[85] //                                                 
{0x0F12,0x0146},  //0146   //TVAR_wbt_pBaseCcmS[86] //                                                 
{0x0F12,0xFF26},  //FF26   //TVAR_wbt_pBaseCcmS[87] //                                                 
{0x0F12,0x012B},  //012B   //TVAR_wbt_pBaseCcmS[88] //                                                 
{0x0F12,0x00DF},  //00DF   //TVAR_wbt_pBaseCcmS[89] //                                                                         

{0x0F12,0x01E3},  //01E2   //TVAR_wbt_pBaseCcmS[90] //                                                 
{0x0F12,0xFFA4},  //FF9A   //TVAR_wbt_pBaseCcmS[91] //                                                 
{0x0F12,0xFFDB},  //FFE7   //TVAR_wbt_pBaseCcmS[92] //                                                 
{0x0F12,0xFEB5},  //FE9F   //TVAR_wbt_pBaseCcmS[93] //                                                 
{0x0F12,0x0112},  //010F   //TVAR_wbt_pBaseCcmS[94] //                                                 
{0x0F12,0xFEDC},  //FEF5   //TVAR_wbt_pBaseCcmS[95] //                                                 
{0x0F12,0xFFD2},  //FFD2   //TVAR_wbt_pBaseCcmS[96] //                                                 
{0x0F12,0xFFFE},  //FFFE   //TVAR_wbt_pBaseCcmS[97] //                                                 
{0x0F12,0x01B7},  //01B7   //TVAR_wbt_pBaseCcmS[98] //                                                 
{0x0F12,0x00E8},  //00E8   //TVAR_wbt_pBaseCcmS[99] //                                                 
{0x0F12,0x0095},  //0095   //TVAR_wbt_pBaseCcmS[100] //                                                
{0x0F12,0xFF0D},  //FF0D   //TVAR_wbt_pBaseCcmS[101] //                                                
{0x0F12,0x0182},  //0182   //TVAR_wbt_pBaseCcmS[102] //                                                
{0x0F12,0xFF29},  //FF29   //TVAR_wbt_pBaseCcmS[103] //                                                
{0x0F12,0x0146},  //0146   //TVAR_wbt_pBaseCcmS[104] //                                                
{0x0F12,0xFF26},  //FF26   //TVAR_wbt_pBaseCcmS[105] //                                                
{0x0F12,0x012B},  //012B   //TVAR_wbt_pBaseCcmS[106] //                                                
{0x0F12,0x00DF},  //00DF   //TVAR_wbt_pBaseCcmS[107] //                                                
                             
{0x002A,0x3380},  //12Ω√√÷¿˚     
{0x0F12,0x020E},  //0223   //0223  //01F3  //01F3  //TVAR_wbt_pOutdoorCcm[0] //  
{0x0F12,0xFF88},  //FF7C   //FF7C  //FFA4  //FFA4  //TVAR_wbt_pOutdoorCcm[1] //  
{0x0F12,0xFFCE},  //FFC5   //FFC5  //FFE4  //FFE4  //TVAR_wbt_pOutdoorCcm[2] //  
{0x0F12,0xFE3D},  //FE3D   //FE3D  //FE3D  //FE23  //TVAR_wbt_pOutdoorCcm[3] //  
{0x0F12,0x0158},  //0158   //0158  //0158  //017D  //TVAR_wbt_pOutdoorCcm[4] //  
{0x0F12,0xFF03},  //FF03   //FF03  //FF03  //FEF9  //TVAR_wbt_pOutdoorCcm[5] //  
{0x0F12,0xFF99},  //FF9F   //FF9F  //FF9F  //FF9F  //TVAR_wbt_pOutdoorCcm[6] //  
{0x0F12,0x0018},  //0011   //0011  //0011  //0011  //TVAR_wbt_pOutdoorCcm[7] //  
{0x0F12,0x0235},  //0237   //0237  //0237  //0237  //TVAR_wbt_pOutdoorCcm[8] //  
{0x0F12,0x0101},  //00EB   //00D1  //012A  //0143  //TVAR_wbt_pOutdoorCcm[9] //  
{0x0F12,0x0116},  //012A   //0125  //00CA  //00F6  //TVAR_wbt_pOutdoorCcm[10] // 
{0x0F12,0xFF00},  //FF02   //FEF5  //FEF6  //FEB1  //TVAR_wbt_pOutdoorCcm[11] // 
{0x0F12,0x01C5},  //01C5   //01C5  //01C5  //01C5  //TVAR_wbt_pOutdoorCcm[12] // 
{0x0F12,0xFF80},  //FF80   //FF80  //FF80  //FF80  //TVAR_wbt_pOutdoorCcm[13] // 
{0x0F12,0x019D},  //019D   //019D  //019D  //019D  //TVAR_wbt_pOutdoorCcm[14] // 
{0x0F12,0xFE7A},  //FE7A   //FE7A  //FE7A  //FE7A  //TVAR_wbt_pOutdoorCcm[15] // 
{0x0F12,0x0179},  //0179   //0179  //0179  //0179  //TVAR_wbt_pOutdoorCcm[16] // 
{0x0F12,0x0179},  //0179   //0179  //0179  //0179  //TVAR_wbt_pOutdoorCcm[17] // 
        
             
             
{0x002A,0x0764},       
{0x0F12,0x0049},	//afit_uNoiseIndInDoor[0] //                                                                         
{0x0F12,0x005F},	//afit_uNoiseIndInDoor[1] //                                                                         
{0x0F12,0x00CB},	//afit_uNoiseIndInDoor[2] // 203//                                                                   
{0x0F12,0x01E0},	//afit_uNoiseIndInDoor[3] // Indoor_NB below 1500 _Noise index 300-400d //                           
{0x0F12,0x0220},	//afit_uNoiseIndInDoor[4] // DNP NB 4600 _ Noisenidex :560d-230h //                                  
              
              
{0x002A,0x07C4},       
{0x0F12,0x0014},	//700007C4 //TVAR_afit_pBaseValS[0] // AFIT16_BRIGHTNESS                                                                                  
{0x0F12,0x0000},	//700007C6 //TVAR_afit_pBaseValS[1] // AFIT16_CONTRAST                                                                                    
{0x0F12,0x0014},	//700007C8 //TVAR_afit_pBaseValS[2] // AFIT16_SATURATION                                                                                  
{0x0F12,0x0000},	//700007CA //TVAR_afit_pBaseValS[3] // AFIT16_SHARP_BLUR                                                                                  
{0x0F12,0x0000},	//700007CC //TVAR_afit_pBaseValS[4] // AFIT16_GLAMOUR                                                                                     
{0x0F12,0x00C1},	//700007CE //TVAR_afit_pBaseValS[5] // AFIT16_sddd8a_edge_high                                                                            
{0x0F12,0x03FF},	//700007D0 //TVAR_afit_pBaseValS[6] // AFIT16_Demosaicing_iSatVal                                                                         
{0x0F12,0x009C},	//700007D2 //TVAR_afit_pBaseValS[7] // AFIT16_Sharpening_iReduceEdgeThresh                                                                
{0x0F12,0x0251},	//700007D4 //TVAR_afit_pBaseValS[8] // AFIT16_demsharpmix1_iRGBOffset                                                                     
{0x0F12,0x03FF},	//700007D6 //TVAR_afit_pBaseValS[9] // AFIT16_demsharpmix1_iDemClamp                                                                      
{0x0F12,0x000C},	//700007D8 //TVAR_afit_pBaseValS[10] //AFIT16_demsharpmix1_iLowThreshold                                                                  
{0x0F12,0x0010},	//700007DA //TVAR_afit_pBaseValS[11] //AFIT16_demsharpmix1_iHighThreshold                                                                 
{0x0F12,0x012C},	//700007DC //TVAR_afit_pBaseValS[12] //AFIT16_demsharpmix1_iLowBright                                                                     
{0x0F12,0x03E8},	//700007DE //TVAR_afit_pBaseValS[13] //AFIT16_demsharpmix1_iHighBright                                                                    
{0x0F12,0x0046},	//700007E0 //TVAR_afit_pBaseValS[14] //AFIT16_demsharpmix1_iLowSat                                                                        
{0x0F12,0x005A},	//700007E2 //TVAR_afit_pBaseValS[15] //AFIT16_demsharpmix1_iHighSat                                                                       
{0x0F12,0x0070},	//700007E4 //TVAR_afit_pBaseValS[16] //AFIT16_demsharpmix1_iTune                                                                          
{0x0F12,0x0000},	//700007E6 //TVAR_afit_pBaseValS[17] //AFIT16_demsharpmix1_iHystThLow                                                                     
{0x0F12,0x0000},	//700007E8 //TVAR_afit_pBaseValS[18] //AFIT16_demsharpmix1_iHystThHigh                                                                    
{0x0F12,0x01AA},	//700007EA //TVAR_afit_pBaseValS[19] //AFIT16_demsharpmix1_iHystCenter                                                                    
{0x0F12,0x003C},	//700007EC //TVAR_afit_pBaseValS[20] //AFIT16_YUV422_DENOISE_iUVLowThresh                                                                 
{0x0F12,0x003C},	//700007EE //TVAR_afit_pBaseValS[21] //AFIT16_YUV422_DENOISE_iUVHighThresh                                                                
{0x0F12,0x0000},	//700007F0 //TVAR_afit_pBaseValS[22] //AFIT16_YUV422_DENOISE_iYLowThresh                                                                  
{0x0F12,0x0000},	//700007F2 //TVAR_afit_pBaseValS[23] //AFIT16_YUV422_DENOISE_iYHighThresh                                                                 
{0x0F12,0x003E},	//700007F4 //TVAR_afit_pBaseValS[24] //AFIT16_Sharpening_iLowSharpClamp                                                                   
{0x0F12,0x0008},	//700007F6 //TVAR_afit_pBaseValS[25] //AFIT16_Sharpening_iHighSharpClamp                                                                  
{0x0F12,0x003C},	//700007F8 //TVAR_afit_pBaseValS[26] //AFIT16_Sharpening_iLowSharpClamp_Bin                                                               
{0x0F12,0x001E},	//700007FA //TVAR_afit_pBaseValS[27] //AFIT16_Sharpening_iHighSharpClamp_Bin                                                              
{0x0F12,0x003C},	//700007FC //TVAR_afit_pBaseValS[28] //AFIT16_Sharpening_iLowSharpClamp_sBin                                                              
{0x0F12,0x001E},	//700007FE //TVAR_afit_pBaseValS[29] //AFIT16_Sharpening_iHighSharpClamp_sBin                                                             
{0x0F12,0x0A24},	//70000800 //TVAR_afit_pBaseValS[30] //AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]                                     
{0x0F12,0x1701},	//70000802 //TVAR_afit_pBaseValS[31] //AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]                                      
{0x0F12,0x0229},	//70000804 //TVAR_afit_pBaseValS[32] //AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]                                            
{0x0F12,0x1403},	//70000806 //TVAR_afit_pBaseValS[33] //AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]                                 
{0x0F12,0x0000},	//70000808 //TVAR_afit_pBaseValS[34] //AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]                               
{0x0F12,0x0000},	//7000080A //TVAR_afit_pBaseValS[35] //AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]                             
{0x0F12,0x0000},	//7000080C //TVAR_afit_pBaseValS[36] //AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]                              
{0x0F12,0x00FF},	//7000080E //TVAR_afit_pBaseValS[37] //AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]                                       
{0x0F12,0x045A},	//70000810 //TVAR_afit_pBaseValS[38] //AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]                                
{0x0F12,0x1414},	//70000812 //TVAR_afit_pBaseValS[39] //AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]                  
{0x0F12,0x0301},	//70000814 //TVAR_afit_pBaseValS[40] //AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]                          
{0x0F12,0xFF07},	//70000816 //TVAR_afit_pBaseValS[41] //AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]                          
{0x0F12,0x081E},	//70000818 //TVAR_afit_pBaseValS[42] //AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]                     
{0x0F12,0x0A14},	//7000081A //TVAR_afit_pBaseValS[43] //AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]                        
{0x0F12,0x0F0F},	//7000081C //TVAR_afit_pBaseValS[44] //AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]                    
{0x0F12,0x0A00},	//7000081E //TVAR_afit_pBaseValS[45] //AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]                 
{0x0F12,0x0032},	//70000820 //TVAR_afit_pBaseValS[46] //AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]                             
{0x0F12,0x000E},	//70000822 //TVAR_afit_pBaseValS[47] //AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]                             
{0x0F12,0x0002},	//70000824 //TVAR_afit_pBaseValS[48] //AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]                      
{0x0F12,0x00FF},	//70000826 //TVAR_afit_pBaseValS[49] //AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]                     
{0x0F12,0x1102},	//70000828 //TVAR_afit_pBaseValS[50] //AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]                          
{0x0F12,0x001B},	//7000082A //TVAR_afit_pBaseValS[51] //AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]                            
{0x0F12,0x0900},	//7000082C //TVAR_afit_pBaseValS[52] //AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]                       
{0x0F12,0x0600},	//7000082E //TVAR_afit_pBaseValS[53] //AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]                      
{0x0F12,0x0504},	//70000830 //TVAR_afit_pBaseValS[54] //AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]                   
{0x0F12,0x0306},	//70000832 //TVAR_afit_pBaseValS[55] //AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]                      
{0x0F12,0x4603},	//70000834 //TVAR_afit_pBaseValS[56] //AFIT8_YUV422_DENOISE_iYSupport [7:0],   AFIT8_byr_cgras_iShadingPower [15:8]                       
{0x0F12,0x0480},	//70000836 //TVAR_afit_pBaseValS[57] //AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]                              
{0x0F12,0x003C},	//70000838 //TVAR_afit_pBaseValS[58] //AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]                                  
{0x0F12,0x0080},	//7000083A //TVAR_afit_pBaseValS[59] //AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]                                   
{0x0F12,0x0101},	//7000083C //TVAR_afit_pBaseValS[60] //AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]                             
{0x0F12,0x0707},	//7000083E //TVAR_afit_pBaseValS[61] //AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]                                
{0x0F12,0x4601},	//70000840 //TVAR_afit_pBaseValS[62] //AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]                                 
{0x0F12,0x4944},	//70000842 //TVAR_afit_pBaseValS[63] //AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]                                
{0x0F12,0x5044},	//70000844 //TVAR_afit_pBaseValS[64] //AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]                            
{0x0F12,0x0500},	//70000846 //TVAR_afit_pBaseValS[65] //AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]             
{0x0F12,0x0003},	//70000848 //TVAR_afit_pBaseValS[66] //AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]                   
{0x0F12,0x5400},	//7000084A //TVAR_afit_pBaseValS[67] //AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]                 
{0x0F12,0x0714},	//7000084C //TVAR_afit_pBaseValS[68] //AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]           
{0x0F12,0x32FF},	//7000084E //TVAR_afit_pBaseValS[69] //AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]                        
{0x0F12,0x5A04},	//70000850 //TVAR_afit_pBaseValS[70] //AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]                     
{0x0F12,0x201E},	//70000852 //TVAR_afit_pBaseValS[71] //AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]                     
{0x0F12,0x4012},	//70000854 //TVAR_afit_pBaseValS[72] //AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]                
{0x0F12,0x0204},	//70000856 //TVAR_afit_pBaseValS[73] //AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]                
{0x0F12,0x1403},	//70000858 //TVAR_afit_pBaseValS[74] //AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]                  
{0x0F12,0x0114},	//7000085A //TVAR_afit_pBaseValS[75] //AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]                      
{0x0F12,0x0101},	//7000085C //TVAR_afit_pBaseValS[76] //AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]                       
{0x0F12,0x4446},	//7000085E //TVAR_afit_pBaseValS[77] //AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]                           
{0x0F12,0x646E},	//70000860 //TVAR_afit_pBaseValS[78] //AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]                     
{0x0F12,0x0028},	//70000862 //TVAR_afit_pBaseValS[79] //AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]             
{0x0F12,0x030A},	//70000864 //TVAR_afit_pBaseValS[80] //AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]       
{0x0F12,0x0000},	//70000866 //TVAR_afit_pBaseValS[81] //AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]          
{0x0F12,0x141E},	//70000868 //TVAR_afit_pBaseValS[82] //AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]          
{0x0F12,0xFF07},	//7000086A //TVAR_afit_pBaseValS[83] //AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]       
{0x0F12,0x0432},	//7000086C //TVAR_afit_pBaseValS[84] //AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]              
{0x0F12,0x0000},	//7000086E //TVAR_afit_pBaseValS[85] //AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]            
{0x0F12,0x0F0F},	//70000870 //TVAR_afit_pBaseValS[86] //AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]              
{0x0F12,0x0440},	//70000872 //TVAR_afit_pBaseValS[87] //AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]       
{0x0F12,0x0302},	//70000874 //TVAR_afit_pBaseValS[88] //AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]       
{0x0F12,0x1414},	//70000876 //TVAR_afit_pBaseValS[89] //AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]                   
{0x0F12,0x0101},	//70000878 //TVAR_afit_pBaseValS[90] //AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]                      
{0x0F12,0x4601},	//7000087A //TVAR_afit_pBaseValS[91] //AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]                       
{0x0F12,0x6E44},	//7000087C //TVAR_afit_pBaseValS[92] //AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]                      
{0x0F12,0x2864},	//7000087E //TVAR_afit_pBaseValS[93] //AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]                  
{0x0F12,0x0A00},	//70000880 //TVAR_afit_pBaseValS[94] //AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]    
{0x0F12,0x0003},	//70000882 //TVAR_afit_pBaseValS[95] //AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]        
{0x0F12,0x1E00},	//70000884 //TVAR_afit_pBaseValS[96] //AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]       
{0x0F12,0x0714},	//70000886 //TVAR_afit_pBaseValS[97] //AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8]  
{0x0F12,0x32FF},	//70000888 //TVAR_afit_pBaseValS[98] //AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]              
{0x0F12,0x0004},	//7000088A //TVAR_afit_pBaseValS[99] //AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]           
{0x0F12,0x0F00},	//7000088C //TVAR_afit_pBaseValS[100] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]          /
{0x0F12,0x400F},	//7000088E //TVAR_afit_pBaseValS[101] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]      /
{0x0F12,0x0204},	//70000890 //TVAR_afit_pBaseValS[102] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]     /
{0x0F12,0x0003},	//70000892 //TVAR_afit_pBaseValS[103] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]                                                     /
{0x0F12,0x0000},	//70000894 //TVAR_afit_pBaseValS[104] /AFIT16_BRIGHTNESS                                                                                 /
{0x0F12,0x0000},	//70000896 //TVAR_afit_pBaseValS[105] /AFIT16_CONTRAST                                                                                   /
{0x0F12,0x0014},	//70000898 //TVAR_afit_pBaseValS[106] /AFIT16_SATURATION                                                                                 /
{0x0F12,0x0000},	//7000089A //TVAR_afit_pBaseValS[107] /AFIT16_SHARP_BLUR                                                                                 /
{0x0F12,0x0000},	//7000089C //TVAR_afit_pBaseValS[108] /AFIT16_GLAMOUR                                                                                    /
{0x0F12,0x00C1},	//7000089E //TVAR_afit_pBaseValS[109] /AFIT16_sddd8a_edge_high                                                                           /
{0x0F12,0x03FF},	//700008A0 //TVAR_afit_pBaseValS[110] /AFIT16_Demosaicing_iSatVal                                                                        /
{0x0F12,0x009C},	//700008A2 //TVAR_afit_pBaseValS[111] /AFIT16_Sharpening_iReduceEdgeThresh                                                               /
{0x0F12,0x0251},	//700008A4 //TVAR_afit_pBaseValS[112] /AFIT16_demsharpmix1_iRGBOffset                                                                    /
{0x0F12,0x03FF},	//700008A6 //TVAR_afit_pBaseValS[113] /AFIT16_demsharpmix1_iDemClamp                                                                     /
{0x0F12,0x000C},	//700008A8 //TVAR_afit_pBaseValS[114] /AFIT16_demsharpmix1_iLowThreshold                                                                 /
{0x0F12,0x0010},	//700008AA //TVAR_afit_pBaseValS[115] /AFIT16_demsharpmix1_iHighThreshold                                                                /
{0x0F12,0x012C},	//700008AC //TVAR_afit_pBaseValS[116] /AFIT16_demsharpmix1_iLowBright                                                                    /
{0x0F12,0x03E8},	//700008AE //TVAR_afit_pBaseValS[117] /AFIT16_demsharpmix1_iHighBright                                                                   /
{0x0F12,0x0046},	//700008B0 //TVAR_afit_pBaseValS[118] /AFIT16_demsharpmix1_iLowSat                                                                       /
{0x0F12,0x005A},	//700008B2 //TVAR_afit_pBaseValS[119] /AFIT16_demsharpmix1_iHighSat                                                                      /
{0x0F12,0x0070},	//700008B4 //TVAR_afit_pBaseValS[120] /AFIT16_demsharpmix1_iTune                                                                         /
{0x0F12,0x0000},	//700008B6 //TVAR_afit_pBaseValS[121] /AFIT16_demsharpmix1_iHystThLow                                                                    /
{0x0F12,0x0000},	//700008B8 //TVAR_afit_pBaseValS[122] /AFIT16_demsharpmix1_iHystThHigh                                                                   /
{0x0F12,0x01AE},	//700008BA //TVAR_afit_pBaseValS[123] /AFIT16_demsharpmix1_iHystCenter                                                                   /
{0x0F12,0x001E},	//700008BC //TVAR_afit_pBaseValS[124] /AFIT16_YUV422_DENOISE_iUVLowThresh                                                                /
{0x0F12,0x001E},	//700008BE //TVAR_afit_pBaseValS[125] /AFIT16_YUV422_DENOISE_iUVHighThresh                                                               /
{0x0F12,0x0000},	//700008C0 //TVAR_afit_pBaseValS[126] /AFIT16_YUV422_DENOISE_iYLowThresh                                                                 /
{0x0F12,0x0000},	//700008C2 //TVAR_afit_pBaseValS[127] /AFIT16_YUV422_DENOISE_iYHighThresh                                                                /
{0x0F12,0x003E},	//700008C4 //TVAR_afit_pBaseValS[128] /AFIT16_Sharpening_iLowSharpClamp                                                                  /
{0x0F12,0x0008},	//700008C6 //TVAR_afit_pBaseValS[129] /AFIT16_Sharpening_iHighSharpClamp                                                                 /
{0x0F12,0x003C},	//700008C8 //TVAR_afit_pBaseValS[130] /AFIT16_Sharpening_iLowSharpClamp_Bin                                                              /
{0x0F12,0x001E},	//700008CA //TVAR_afit_pBaseValS[131] /AFIT16_Sharpening_iHighSharpClamp_Bin                                                             /
{0x0F12,0x003C},	//700008CC //TVAR_afit_pBaseValS[132] /AFIT16_Sharpening_iLowSharpClamp_sBin                                                             /
{0x0F12,0x001E},	//700008CE //TVAR_afit_pBaseValS[133] /AFIT16_Sharpening_iHighSharpClamp_sBin                                                            /
{0x0F12,0x0A24},	//700008D0 //TVAR_afit_pBaseValS[134] /AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]                                    /
{0x0F12,0x1701},	//700008D2 //TVAR_afit_pBaseValS[135] /AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]                                     /
{0x0F12,0x0229},	//700008D4 //TVAR_afit_pBaseValS[136] /AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]                                           /
{0x0F12,0x1403},	//700008D6 //TVAR_afit_pBaseValS[137] /AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]                                /
{0x0F12,0x0000},	//700008D8 //TVAR_afit_pBaseValS[138] /AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]                              /
{0x0F12,0x0000},	//700008DA //TVAR_afit_pBaseValS[139] /AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]                            /
{0x0F12,0x0000},	//700008DC //TVAR_afit_pBaseValS[140] /AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]                             /
{0x0F12,0x00FF},	//700008DE //TVAR_afit_pBaseValS[141] /AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]                                      /
{0x0F12,0x045A},	//700008E0 //TVAR_afit_pBaseValS[142] /AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]                               /
{0x0F12,0x1414},	//700008E2 //TVAR_afit_pBaseValS[143] /AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]                 /
{0x0F12,0x0301},	//700008E4 //TVAR_afit_pBaseValS[144] /AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]                         /
{0x0F12,0xFF07},	//700008E6 //TVAR_afit_pBaseValS[145] /AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]                         /
{0x0F12,0x081E},	//700008E8 //TVAR_afit_pBaseValS[146] /AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]                    /
{0x0F12,0x0A14},	//700008EA //TVAR_afit_pBaseValS[147] /AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]                       /
{0x0F12,0x0F0F},	//700008EC //TVAR_afit_pBaseValS[148] /AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]                   /
{0x0F12,0x0A00},	//700008EE //TVAR_afit_pBaseValS[149] /AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]                /
{0x0F12,0x0032},	//700008F0 //TVAR_afit_pBaseValS[150] /AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]                            /
{0x0F12,0x000E},	//700008F2 //TVAR_afit_pBaseValS[151] /AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]                            /
{0x0F12,0x0002},	//700008F4 //TVAR_afit_pBaseValS[152] /AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]                     /
{0x0F12,0x00FF},	//700008F6 //TVAR_afit_pBaseValS[153] /AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]                    /
{0x0F12,0x1102},	//700008F8 //TVAR_afit_pBaseValS[154] /AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]                         /
{0x0F12,0x001B},	//700008FA //TVAR_afit_pBaseValS[155] /AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]                           /
{0x0F12,0x0900},	//700008FC //TVAR_afit_pBaseValS[156] /AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]                      /
{0x0F12,0x0600},	//700008FE //TVAR_afit_pBaseValS[157] /AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]                     /
{0x0F12,0x0504},	//70000900 //TVAR_afit_pBaseValS[158] /AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]                  /
{0x0F12,0x0306},	//70000902 //TVAR_afit_pBaseValS[159] /AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]                     /
{0x0F12,0x4603},	//70000904 //TVAR_afit_pBaseValS[160] /AFIT8_YUV422_DENOISE_iYSupport [7:0],   AFIT8_byr_cgras_iShadingPower [15:8]                      /
{0x0F12,0x0480},	//70000906 //TVAR_afit_pBaseValS[161] /AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]                             /
{0x0F12,0x0046},	//70000908 //TVAR_afit_pBaseValS[162] /AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]                                 /
{0x0F12,0x0080},	//7000090A //TVAR_afit_pBaseValS[163] /AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]                                  /
{0x0F12,0x0101},	//7000090C //TVAR_afit_pBaseValS[164] /AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]                            /
{0x0F12,0x0707},	//7000090E //TVAR_afit_pBaseValS[165] /AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]                               /
{0x0F12,0x1E01},	//70000910 //TVAR_afit_pBaseValS[166] /AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]                                /
{0x0F12,0x491E},	//70000912 //TVAR_afit_pBaseValS[167] /AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]                               /
{0x0F12,0x5044},	//70000914 //TVAR_afit_pBaseValS[168] /AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]                           /
{0x0F12,0x0500},	//70000916 //TVAR_afit_pBaseValS[169] /AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]            /
{0x0F12,0x0004},	//70000918 //TVAR_afit_pBaseValS[170] /AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]                  /
{0x0F12,0x3C0A},	//7000091A //TVAR_afit_pBaseValS[171] /AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]                /
{0x0F12,0x0714},	//7000091C //TVAR_afit_pBaseValS[172] /AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]          /
{0x0F12,0x3214},	//7000091E //TVAR_afit_pBaseValS[173] /AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]                       /
{0x0F12,0x5A03},	//70000920 //TVAR_afit_pBaseValS[174] /AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]                    /
{0x0F12,0x121E},	//70000922 //TVAR_afit_pBaseValS[175] /AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]                    /
{0x0F12,0x4012},	//70000924 //TVAR_afit_pBaseValS[176] /AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]               /
{0x0F12,0x0604},	//70000926 //TVAR_afit_pBaseValS[177] /AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]               /
{0x0F12,0x1E06},	//70000928 //TVAR_afit_pBaseValS[178] /AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]                 /
{0x0F12,0x011E},	//7000092A //TVAR_afit_pBaseValS[179] /AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]                     /
{0x0F12,0x0101},	//7000092C //TVAR_afit_pBaseValS[180] /AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]                      /
{0x0F12,0x3A3C},	//7000092E //TVAR_afit_pBaseValS[181] /AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]                          /
{0x0F12,0x585A},	//70000930 //TVAR_afit_pBaseValS[182] /AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]                    /
{0x0F12,0x0028},	//70000932 //TVAR_afit_pBaseValS[183] /AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]            /
{0x0F12,0x030A},	//70000934 //TVAR_afit_pBaseValS[184] /AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]      /
{0x0F12,0x0000},	//70000936 //TVAR_afit_pBaseValS[185] /AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]         /
{0x0F12,0x141E},	//70000938 //TVAR_afit_pBaseValS[186] /AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]         /
{0x0F12,0xFF07},	//7000093A //TVAR_afit_pBaseValS[187] /AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]      /
{0x0F12,0x0432},	//7000093C //TVAR_afit_pBaseValS[188] /AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]             /
{0x0F12,0x0000},	//7000093E //TVAR_afit_pBaseValS[189] /AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]           /
{0x0F12,0x0F0F},	//70000940 //TVAR_afit_pBaseValS[190] /AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]             /
{0x0F12,0x0440},	//70000942 //TVAR_afit_pBaseValS[191] /AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]      /
{0x0F12,0x0302},	//70000944 //TVAR_afit_pBaseValS[192] /AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]      /
{0x0F12,0x1E1E},	//70000946 //TVAR_afit_pBaseValS[193] /AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]                  /
{0x0F12,0x0101},	//70000948 //TVAR_afit_pBaseValS[194] /AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]                     /
{0x0F12,0x3C01},	//7000094A //TVAR_afit_pBaseValS[195] /AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]                      /
{0x0F12,0x5A3A},	//7000094C //TVAR_afit_pBaseValS[196] /AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]                     /
{0x0F12,0x2858},	//7000094E //TVAR_afit_pBaseValS[197] /AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]                 /
{0x0F12,0x0A00},	//70000950 //TVAR_afit_pBaseValS[198] /AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]   /
{0x0F12,0x0003},	//70000952 //TVAR_afit_pBaseValS[199] /AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]       /
{0x0F12,0x1E00},	//70000954 //TVAR_afit_pBaseValS[200] /AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]      /
{0x0F12,0x0714},	//70000956 //TVAR_afit_pBaseValS[201] /AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8] /
{0x0F12,0x32FF},	//70000958 //TVAR_afit_pBaseValS[202] /AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]             /
{0x0F12,0x0004},	//7000095A //TVAR_afit_pBaseValS[203] /AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]          /
{0x0F12,0x0F00},	//7000095C //TVAR_afit_pBaseValS[204] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]          /
{0x0F12,0x400F},	//7000095E //TVAR_afit_pBaseValS[205] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]      /
{0x0F12,0x0204},	//70000960 //TVAR_afit_pBaseValS[206] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]     /
{0x0F12,0x0003},	//70000962 //TVAR_afit_pBaseValS[207] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]                                                     /
{0x0F12,0x0000},	//70000964 //TVAR_afit_pBaseValS[208] /AFIT16_BRIGHTNESS                                                                                 /
{0x0F12,0x0000},	//70000966 //TVAR_afit_pBaseValS[209] /AFIT16_CONTRAST                                                                                   /
{0x0F12,0x0000},	//70000968 //TVAR_afit_pBaseValS[210] /AFIT16_SATURATION                                                                                 /
{0x0F12,0x0000},	//7000096A //TVAR_afit_pBaseValS[211] /AFIT16_SHARP_BLUR                                                                                 /
{0x0F12,0x0000},	//7000096C //TVAR_afit_pBaseValS[212] /AFIT16_GLAMOUR                                                                                    /
{0x0F12,0x00C1},	//7000096E //TVAR_afit_pBaseValS[213] /AFIT16_sddd8a_edge_high                                                                           /
{0x0F12,0x03FF},	//70000970 //TVAR_afit_pBaseValS[214] /AFIT16_Demosaicing_iSatVal                                                                        /
{0x0F12,0x009C},	//70000972 //TVAR_afit_pBaseValS[215] /AFIT16_Sharpening_iReduceEdgeThresh                                                               /
{0x0F12,0x0251},	//70000974 //TVAR_afit_pBaseValS[216] /AFIT16_demsharpmix1_iRGBOffset                                                                    /
{0x0F12,0x03FF},	//70000976 //TVAR_afit_pBaseValS[217] /AFIT16_demsharpmix1_iDemClamp                                                                     /
{0x0F12,0x000C},	//70000978 //TVAR_afit_pBaseValS[218] /AFIT16_demsharpmix1_iLowThreshold                                                                 /
{0x0F12,0x0010},	//7000097A //TVAR_afit_pBaseValS[219] /AFIT16_demsharpmix1_iHighThreshold                                                                /
{0x0F12,0x012C},	//7000097C //TVAR_afit_pBaseValS[220] /AFIT16_demsharpmix1_iLowBright                                                                    /
{0x0F12,0x03E8},	//7000097E //TVAR_afit_pBaseValS[221] /AFIT16_demsharpmix1_iHighBright                                                                   /
{0x0F12,0x0046},	//70000980 //TVAR_afit_pBaseValS[222] /AFIT16_demsharpmix1_iLowSat                                                                       /
{0x0F12,0x005A},	//70000982 //TVAR_afit_pBaseValS[223] /AFIT16_demsharpmix1_iHighSat                                                                      /
{0x0F12,0x0070},	//70000984 //TVAR_afit_pBaseValS[224] /AFIT16_demsharpmix1_iTune                                                                         /
{0x0F12,0x0000},	//70000986 //TVAR_afit_pBaseValS[225] /AFIT16_demsharpmix1_iHystThLow                                                                    /
{0x0F12,0x0000},	//70000988 //TVAR_afit_pBaseValS[226] /AFIT16_demsharpmix1_iHystThHigh                                                                   /
{0x0F12,0x0226},	//7000098A //TVAR_afit_pBaseValS[227] /AFIT16_demsharpmix1_iHystCenter                                                                   /
{0x0F12,0x001E},	//7000098C //TVAR_afit_pBaseValS[228] /AFIT16_YUV422_DENOISE_iUVLowThresh                                                                /
{0x0F12,0x001E},	//7000098E //TVAR_afit_pBaseValS[229] /AFIT16_YUV422_DENOISE_iUVHighThresh                                                               /
{0x0F12,0x0000},	//70000990 //TVAR_afit_pBaseValS[230] /AFIT16_YUV422_DENOISE_iYLowThresh                                                                 /
{0x0F12,0x0000},	//70000992 //TVAR_afit_pBaseValS[231] /AFIT16_YUV422_DENOISE_iYHighThresh                                                                /
{0x0F12,0x004E},	//70000994 //TVAR_afit_pBaseValS[232] /AFIT16_Sharpening_iLowSharpClamp                                                                  /
{0x0F12,0x0000},	//70000996 //TVAR_afit_pBaseValS[233] /AFIT16_Sharpening_iHighSharpClamp                                                                 /
{0x0F12,0x003C},	//70000998 //TVAR_afit_pBaseValS[234] /AFIT16_Sharpening_iLowSharpClamp_Bin                                                              /
{0x0F12,0x001E},	//7000099A //TVAR_afit_pBaseValS[235] /AFIT16_Sharpening_iHighSharpClamp_Bin                                                             /
{0x0F12,0x003C},	//7000099C //TVAR_afit_pBaseValS[236] /AFIT16_Sharpening_iLowSharpClamp_sBin                                                             /
{0x0F12,0x001E},	//7000099E //TVAR_afit_pBaseValS[237] /AFIT16_Sharpening_iHighSharpClamp_sBin                                                            /
{0x0F12,0x0A24},	//700009A0 //TVAR_afit_pBaseValS[238] /AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]                                    /
{0x0F12,0x1701},	//700009A2 //TVAR_afit_pBaseValS[239] /AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]                                     /
{0x0F12,0x0229},	//700009A4 //TVAR_afit_pBaseValS[240] /AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]                                           /
{0x0F12,0x1403},	//700009A6 //TVAR_afit_pBaseValS[241] /AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]                                /
{0x0F12,0x0000},	//700009A8 //TVAR_afit_pBaseValS[242] /AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]                              /
{0x0F12,0x0000},	//700009AA //TVAR_afit_pBaseValS[243] /AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]                            /
{0x0F12,0x0906},	//700009AC //TVAR_afit_pBaseValS[244] /AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]                             /
{0x0F12,0x00FF},	//700009AE //TVAR_afit_pBaseValS[245] /AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]                                      /
{0x0F12,0x045A},	//700009B0 //TVAR_afit_pBaseValS[246] /AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]                               /
{0x0F12,0x1414},	//700009B2 //TVAR_afit_pBaseValS[247] /AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]                 /
{0x0F12,0x0301},	//700009B4 //TVAR_afit_pBaseValS[248] /AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]                         /
{0x0F12,0xFF07},	//700009B6 //TVAR_afit_pBaseValS[249] /AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]                         /
{0x0F12,0x081E},	//700009B8 //TVAR_afit_pBaseValS[250] /AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]                    /
{0x0F12,0x0A14},	//700009BA //TVAR_afit_pBaseValS[251] /AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]                       /
{0x0F12,0x0F0F},	//700009BC //TVAR_afit_pBaseValS[252] /AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]                   /
{0x0F12,0x0A00},	//700009BE //TVAR_afit_pBaseValS[253] /AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]                /
{0x0F12,0x0090},	//700009C0 //TVAR_afit_pBaseValS[254] /AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]                            /
{0x0F12,0x000A},	//700009C2 //TVAR_afit_pBaseValS[255] /AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]                            /
{0x0F12,0x0002},	//700009C4 //TVAR_afit_pBaseValS[256] /AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]                     /
{0x0F12,0x00FF},	//700009C6 //TVAR_afit_pBaseValS[257] /AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]                    /
{0x0F12,0x1102},	//700009C8 //TVAR_afit_pBaseValS[258] /AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]                         /
{0x0F12,0x001B},	//700009CA //TVAR_afit_pBaseValS[259] /AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]                           /
{0x0F12,0x0900},	//700009CC //TVAR_afit_pBaseValS[260] /AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]                      /
{0x0F12,0x0600},	//700009CE //TVAR_afit_pBaseValS[261] /AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]                     /
{0x0F12,0x0504},	//700009D0 //TVAR_afit_pBaseValS[262] /AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]                  /
{0x0F12,0x0306},	//700009D2 //TVAR_afit_pBaseValS[263] /AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]                     /
{0x0F12,0x4602},	//700009D4 //TVAR_afit_pBaseValS[264] /AFIT8_YUV422_DENOISE_iYSupport [7:0],   AFIT8_byr_cgras_iShadingPower [15:8]                      /
{0x0F12,0x0880},	//700009D6 //TVAR_afit_pBaseValS[265] /AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]                             /
{0x0F12,0x0080},	//700009D8 //TVAR_afit_pBaseValS[266] /AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]                                 /
{0x0F12,0x0080},	//700009DA //TVAR_afit_pBaseValS[267] /AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]                                  /
{0x0F12,0x0101},	//700009DC //TVAR_afit_pBaseValS[268] /AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]                            /
{0x0F12,0x0707},	//700009DE //TVAR_afit_pBaseValS[269] /AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]                               /
{0x0F12,0x1E01},	//700009E0 //TVAR_afit_pBaseValS[270] /AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]                                /
{0x0F12,0x3C1E},	//700009E2 //TVAR_afit_pBaseValS[271] /AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]                               /
{0x0F12,0x5028},	//700009E4 //TVAR_afit_pBaseValS[272] /AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]                           /
{0x0F12,0x0500},	//700009E6 //TVAR_afit_pBaseValS[273] /AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]            /
{0x0F12,0x1A04},	//700009E8 //TVAR_afit_pBaseValS[274] /AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]                  /
{0x0F12,0x280A},	//700009EA //TVAR_afit_pBaseValS[275] /AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]                /
{0x0F12,0x080C},	//700009EC //TVAR_afit_pBaseValS[276] /AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]          /
{0x0F12,0x1414},	//700009EE //TVAR_afit_pBaseValS[277] /AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]                       /
{0x0F12,0x6A03},	//700009F0 //TVAR_afit_pBaseValS[278] /AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]                    /
{0x0F12,0x121E},	//700009F2 //TVAR_afit_pBaseValS[279] /AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]                    /
{0x0F12,0x4012},	//700009F4 //TVAR_afit_pBaseValS[280] /AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]               /
{0x0F12,0x0604},	//700009F6 //TVAR_afit_pBaseValS[281] /AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]               /
{0x0F12,0x2806},	//700009F8 //TVAR_afit_pBaseValS[282] /AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]                 /
{0x0F12,0x0128},	//700009FA //TVAR_afit_pBaseValS[283] /AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]                     /
{0x0F12,0x0101},	//700009FC //TVAR_afit_pBaseValS[284] /AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]                      /
{0x0F12,0x2224},	//700009FE //TVAR_afit_pBaseValS[285] /AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]                          /
{0x0F12,0x3236},	//70000A00 //TVAR_afit_pBaseValS[286] /AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]                    /
{0x0F12,0x0028},	//70000A02 //TVAR_afit_pBaseValS[287] /AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]            /
{0x0F12,0x030A},	//70000A04 //TVAR_afit_pBaseValS[288] /AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]      /
{0x0F12,0x0410},	//70000A06 //TVAR_afit_pBaseValS[289] /AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]         /
{0x0F12,0x141E},	//70000A08 //TVAR_afit_pBaseValS[290] /AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]         /
{0x0F12,0xFF07},	//70000A0A //TVAR_afit_pBaseValS[291] /AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]      /
{0x0F12,0x0432},	//70000A0C //TVAR_afit_pBaseValS[292] /AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]             /
{0x0F12,0x4050},	//70000A0E //TVAR_afit_pBaseValS[293] /AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]           /
{0x0F12,0x0F0F},	//70000A10 //TVAR_afit_pBaseValS[294] /AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]             /
{0x0F12,0x0440},	//70000A12 //TVAR_afit_pBaseValS[295] /AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]      /
{0x0F12,0x0302},	//70000A14 //TVAR_afit_pBaseValS[296] /AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]      /
{0x0F12,0x2828},	//70000A16 //TVAR_afit_pBaseValS[297] /AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]                  /
{0x0F12,0x0101},	//70000A18 //TVAR_afit_pBaseValS[298] /AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]                     /
{0x0F12,0x2401},	//70000A1A //TVAR_afit_pBaseValS[299] /AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]                      /
{0x0F12,0x3622},	//70000A1C //TVAR_afit_pBaseValS[300] /AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]                     /
{0x0F12,0x2832},	//70000A1E //TVAR_afit_pBaseValS[301] /AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]                 /
{0x0F12,0x0A00},	//70000A20 //TVAR_afit_pBaseValS[302] /AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]   /
{0x0F12,0x1003},	//70000A22 //TVAR_afit_pBaseValS[303] /AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]       /
{0x0F12,0x1E04},	//70000A24 //TVAR_afit_pBaseValS[304] /AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]      /
{0x0F12,0x0714},	//70000A26 //TVAR_afit_pBaseValS[305] /AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8] /
{0x0F12,0x32FF},	//70000A28 //TVAR_afit_pBaseValS[306] /AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]             /
{0x0F12,0x5004},	//70000A2A //TVAR_afit_pBaseValS[307] /AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]          /
{0x0F12,0x0F40},	//70000A2C //TVAR_afit_pBaseValS[308] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]          /
{0x0F12,0x400F},	//70000A2E //TVAR_afit_pBaseValS[309] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]      /
{0x0F12,0x0204},	//70000A30 //TVAR_afit_pBaseValS[310] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]     /
{0x0F12,0x0003},	//70000A32 //TVAR_afit_pBaseValS[311] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]                                                     /
{0x0F12,0x0000},	//70000A34 //TVAR_afit_pBaseValS[312] /AFIT16_BRIGHTNESS                                                                                 /
{0x0F12,0x0000},	//70000A36 //TVAR_afit_pBaseValS[313] /AFIT16_CONTRAST                                                                                   /
{0x0F12,0x0000},	//70000A38 //TVAR_afit_pBaseValS[314] /AFIT16_SATURATION                                                                                 /
{0x0F12,0x0000},	//70000A3A //TVAR_afit_pBaseValS[315] /AFIT16_SHARP_BLUR                                                                                 /
{0x0F12,0x0000},	//70000A3C //TVAR_afit_pBaseValS[316] /AFIT16_GLAMOUR                                                                                    /
{0x0F12,0x00C1},	//70000A3E //TVAR_afit_pBaseValS[317] /AFIT16_sddd8a_edge_high                                                                           /
{0x0F12,0x03FF},	//70000A40 //TVAR_afit_pBaseValS[318] /AFIT16_Demosaicing_iSatVal                                                                        /
{0x0F12,0x009C},	//70000A42 //TVAR_afit_pBaseValS[319] /AFIT16_Sharpening_iReduceEdgeThresh                                                               /
{0x0F12,0x0251},	//70000A44 //TVAR_afit_pBaseValS[320] /AFIT16_demsharpmix1_iRGBOffset                                                                    /
{0x0F12,0x03FF},	//70000A46 //TVAR_afit_pBaseValS[321] /AFIT16_demsharpmix1_iDemClamp                                                                     /
{0x0F12,0x000C},	//70000A48 //TVAR_afit_pBaseValS[322] /AFIT16_demsharpmix1_iLowThreshold                                                                 /
{0x0F12,0x0010},	//70000A4A //TVAR_afit_pBaseValS[323] /AFIT16_demsharpmix1_iHighThreshold                                                                /
{0x0F12,0x00C8},	//70000A4C //TVAR_afit_pBaseValS[324] /AFIT16_demsharpmix1_iLowBright                                                                    /
{0x0F12,0x03E8},	//70000A4E //TVAR_afit_pBaseValS[325] /AFIT16_demsharpmix1_iHighBright                                                                   /
{0x0F12,0x0046},	//70000A50 //TVAR_afit_pBaseValS[326] /AFIT16_demsharpmix1_iLowSat                                                                       /
{0x0F12,0x0050},	//70000A52 //TVAR_afit_pBaseValS[327] /AFIT16_demsharpmix1_iHighSat                                                                      /
{0x0F12,0x0070},	//70000A54 //TVAR_afit_pBaseValS[328] /AFIT16_demsharpmix1_iTune                                                                         /
{0x0F12,0x0000},	//70000A56 //TVAR_afit_pBaseValS[329] /AFIT16_demsharpmix1_iHystThLow                                                                    /
{0x0F12,0x0000},	//70000A58 //TVAR_afit_pBaseValS[330] /AFIT16_demsharpmix1_iHystThHigh                                                                   /
{0x0F12,0x0226},	//70000A5A //TVAR_afit_pBaseValS[331] /AFIT16_demsharpmix1_iHystCenter                                                                   /
{0x0F12,0x0014},	//70000A5C //TVAR_afit_pBaseValS[332] /AFIT16_YUV422_DENOISE_iUVLowThresh                                                                /
{0x0F12,0x0014},	//70000A5E //TVAR_afit_pBaseValS[333] /AFIT16_YUV422_DENOISE_iUVHighThresh                                                               /
{0x0F12,0x0000},	//70000A60 //TVAR_afit_pBaseValS[334] /AFIT16_YUV422_DENOISE_iYLowThresh                                                                 /
{0x0F12,0x0000},	//70000A62 //TVAR_afit_pBaseValS[335] /AFIT16_YUV422_DENOISE_iYHighThresh                                                                /
{0x0F12,0x004E},	//70000A64 //TVAR_afit_pBaseValS[336] /AFIT16_Sharpening_iLowSharpClamp                                                                  /
{0x0F12,0x0000},	//70000A66 //TVAR_afit_pBaseValS[337] /AFIT16_Sharpening_iHighSharpClamp                                                                 /
{0x0F12,0x002D},	//70000A68 //TVAR_afit_pBaseValS[338] /AFIT16_Sharpening_iLowSharpClamp_Bin                                                              /
{0x0F12,0x0019},	//70000A6A //TVAR_afit_pBaseValS[339] /AFIT16_Sharpening_iHighSharpClamp_Bin                                                             /
{0x0F12,0x002D},	//70000A6C //TVAR_afit_pBaseValS[340] /AFIT16_Sharpening_iLowSharpClamp_sBin                                                             /
{0x0F12,0x0019},	//70000A6E //TVAR_afit_pBaseValS[341] /AFIT16_Sharpening_iHighSharpClamp_sBin                                                            /
{0x0F12,0x0A24},	//70000A70 //TVAR_afit_pBaseValS[342] /AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]                                    /
{0x0F12,0x1701},	//70000A72 //TVAR_afit_pBaseValS[343] /AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]                                     /
{0x0F12,0x0229},	//70000A74 //TVAR_afit_pBaseValS[344] /AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]                                           /
{0x0F12,0x1403},	//70000A76 //TVAR_afit_pBaseValS[345] /AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]                                /
{0x0F12,0x0000},	//70000A78 //TVAR_afit_pBaseValS[346] /AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]                              /
{0x0F12,0x0000},	//70000A7A //TVAR_afit_pBaseValS[347] /AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]                            /
{0x0F12,0x0906},	//70000A7C //TVAR_afit_pBaseValS[348] /AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]                             /
{0x0F12,0x00FF},	//70000A7E //TVAR_afit_pBaseValS[349] /AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]                                      /
{0x0F12,0x045A},	//70000A80 //TVAR_afit_pBaseValS[350] /AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]                               /
{0x0F12,0x1414},	//70000A82 //TVAR_afit_pBaseValS[351] /AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]                 /
{0x0F12,0x0301},	//70000A84 //TVAR_afit_pBaseValS[352] /AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]                         /
{0x0F12,0xFF07},	//70000A86 //TVAR_afit_pBaseValS[353] /AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]                         /
{0x0F12,0x081E},	//70000A88 //TVAR_afit_pBaseValS[354] /AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]                    /
{0x0F12,0x0A14},	//70000A8A //TVAR_afit_pBaseValS[355] /AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]                       /
{0x0F12,0x0F0F},	//70000A8C //TVAR_afit_pBaseValS[356] /AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]                   /
{0x0F12,0x0A00},	//70000A8E //TVAR_afit_pBaseValS[357] /AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]                /
{0x0F12,0x0090},	//70000A90 //TVAR_afit_pBaseValS[358] /AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]                            /
{0x0F12,0x000A},	//70000A92 //TVAR_afit_pBaseValS[359] /AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]                            /
{0x0F12,0x0001},	//70000A94 //TVAR_afit_pBaseValS[360] /AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]                     /
{0x0F12,0x00FF},	//70000A96 //TVAR_afit_pBaseValS[361] /AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]                    /
{0x0F12,0x1002},	//70000A98 //TVAR_afit_pBaseValS[362] /AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]                         /
{0x0F12,0x001E},	//70000A9A //TVAR_afit_pBaseValS[363] /AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]                           /
{0x0F12,0x0900},	//70000A9C //TVAR_afit_pBaseValS[364] /AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]                      /
{0x0F12,0x0600},	//70000A9E //TVAR_afit_pBaseValS[365] /AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]                     /
{0x0F12,0x0504},	//70000AA0 //TVAR_afit_pBaseValS[366] /AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]                  /
{0x0F12,0x0307},	//70000AA2 //TVAR_afit_pBaseValS[367] /AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]                     /
{0x0F12,0x5002},	//70000AA4 //TVAR_afit_pBaseValS[368] /AFIT8_YUV422_DENOISE_iYSupport [7:0],   AFIT8_byr_cgras_iShadingPower [15:8]                      /
{0x0F12,0x0080},	//70000AA6 //TVAR_afit_pBaseValS[369] /AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]                             /
{0x0F12,0x0080},	//70000AA8 //TVAR_afit_pBaseValS[370] /AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]                                 /
{0x0F12,0x0080},	//70000AAA //TVAR_afit_pBaseValS[371] /AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]                                  /
{0x0F12,0x0101},	//70000AAC //TVAR_afit_pBaseValS[372] /AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]                            /
{0x0F12,0x0707},	//70000AAE //TVAR_afit_pBaseValS[373] /AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]                               /
{0x0F12,0x1E01},	//70000AB0 //TVAR_afit_pBaseValS[374] /AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]                                /
{0x0F12,0x2A1E},	//70000AB2 //TVAR_afit_pBaseValS[375] /AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]                               /
{0x0F12,0x5020},	//70000AB4 //TVAR_afit_pBaseValS[376] /AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]                           /
{0x0F12,0x0500},	//70000AB6 //TVAR_afit_pBaseValS[377] /AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]            /
{0x0F12,0x1A04},	//70000AB8 //TVAR_afit_pBaseValS[378] /AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]                  /
{0x0F12,0x280A},	//70000ABA //TVAR_afit_pBaseValS[379] /AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]                /
{0x0F12,0x080C},	//70000ABC //TVAR_afit_pBaseValS[380] /AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]          /
{0x0F12,0x1414},	//70000ABE //TVAR_afit_pBaseValS[381] /AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]                       /
{0x0F12,0x6A03},	//70000AC0 //TVAR_afit_pBaseValS[382] /AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]                    /
{0x0F12,0x121E},	//70000AC2 //TVAR_afit_pBaseValS[383] /AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]                    /
{0x0F12,0x4012},	//70000AC4 //TVAR_afit_pBaseValS[384] /AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]               /
{0x0F12,0x0604},	//70000AC6 //TVAR_afit_pBaseValS[385] /AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]               /
{0x0F12,0x3C06},	//70000AC8 //TVAR_afit_pBaseValS[386] /AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]                 /
{0x0F12,0x013C},	//70000ACA //TVAR_afit_pBaseValS[387] /AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]                     /
{0x0F12,0x0101},	//70000ACC //TVAR_afit_pBaseValS[388] /AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]                      /
{0x0F12,0x1C1E},	//70000ACE //TVAR_afit_pBaseValS[389] /AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]                          /
{0x0F12,0x1E22},	//70000AD0 //TVAR_afit_pBaseValS[390] /AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]                    /
{0x0F12,0x0028},	//70000AD2 //TVAR_afit_pBaseValS[391] /AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]            /
{0x0F12,0x030A},	//70000AD4 //TVAR_afit_pBaseValS[392] /AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]      /
{0x0F12,0x0214},	//70000AD6 //TVAR_afit_pBaseValS[393] /AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]         /
{0x0F12,0x0E14},	//70000AD8 //TVAR_afit_pBaseValS[394] /AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]         /
{0x0F12,0xFF06},	//70000ADA //TVAR_afit_pBaseValS[395] /AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]      /
{0x0F12,0x0432},	//70000ADC //TVAR_afit_pBaseValS[396] /AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]             /
{0x0F12,0x4052},	//70000ADE //TVAR_afit_pBaseValS[397] /AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]           /
{0x0F12,0x150C},	//70000AE0 //TVAR_afit_pBaseValS[398] /AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]             /
{0x0F12,0x0440},	//70000AE2 //TVAR_afit_pBaseValS[399] /AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]      /
{0x0F12,0x0302},	//70000AE4 //TVAR_afit_pBaseValS[400] /AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]      /
{0x0F12,0x3C3C},	//70000AE6 //TVAR_afit_pBaseValS[401] /AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]                  /
{0x0F12,0x0101},	//70000AE8 //TVAR_afit_pBaseValS[402] /AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]                     /
{0x0F12,0x1E01},	//70000AEA //TVAR_afit_pBaseValS[403] /AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]                      /
{0x0F12,0x221C},	//70000AEC //TVAR_afit_pBaseValS[404] /AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]                     /
{0x0F12,0x281E},	//70000AEE //TVAR_afit_pBaseValS[405] /AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]                 /
{0x0F12,0x0A00},	//70000AF0 //TVAR_afit_pBaseValS[406] /AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]   /
{0x0F12,0x1403},	//70000AF2 //TVAR_afit_pBaseValS[407] /AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]       /
{0x0F12,0x1402},	//70000AF4 //TVAR_afit_pBaseValS[408] /AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]      /
{0x0F12,0x060E},	//70000AF6 //TVAR_afit_pBaseValS[409] /AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8] /
{0x0F12,0x32FF},	//70000AF8 //TVAR_afit_pBaseValS[410] /AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]             /
{0x0F12,0x5204},	//70000AFA //TVAR_afit_pBaseValS[411] /AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]          /
{0x0F12,0x0C40},	//70000AFC //TVAR_afit_pBaseValS[412] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]          /
{0x0F12,0x4015},	//70000AFE //TVAR_afit_pBaseValS[413] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]      /
{0x0F12,0x0204},	//70000B00 //TVAR_afit_pBaseValS[414] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]     /
{0x0F12,0x0003},	//70000B02 //TVAR_afit_pBaseValS[415] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]                                                     /
{0x0F12,0x0000},	//70000B04 //TVAR_afit_pBaseValS[416] /AFIT16_BRIGHTNESS                                                                                 /
{0x0F12,0x0000},	//70000B06 //TVAR_afit_pBaseValS[417] /AFIT16_CONTRAST                                                                                   /
{0x0F12,0x0000},	//70000B08 //TVAR_afit_pBaseValS[418] /AFIT16_SATURATION                                                                                 /
{0x0F12,0x0000},	//70000B0A //TVAR_afit_pBaseValS[419] /AFIT16_SHARP_BLUR                                                                                 /
{0x0F12,0x0000},	//70000B0C //TVAR_afit_pBaseValS[420] /AFIT16_GLAMOUR                                                                                    /
{0x0F12,0x00C1},	//70000B0E //TVAR_afit_pBaseValS[421] /AFIT16_sddd8a_edge_high                                                                           /
{0x0F12,0x03FF},	//70000B10 //TVAR_afit_pBaseValS[422] /AFIT16_Demosaicing_iSatVal                                                                        /
{0x0F12,0x009C},	//70000B12 //TVAR_afit_pBaseValS[423] /AFIT16_Sharpening_iReduceEdgeThresh                                                               /
{0x0F12,0x0251},	//70000B14 //TVAR_afit_pBaseValS[424] /AFIT16_demsharpmix1_iRGBOffset                                                                    /
{0x0F12,0x03FF},	//70000B16 //TVAR_afit_pBaseValS[425] /AFIT16_demsharpmix1_iDemClamp                                                                     /
{0x0F12,0x000C},	//70000B18 //TVAR_afit_pBaseValS[426] /AFIT16_demsharpmix1_iLowThreshold                                                                 /
{0x0F12,0x0010},	//70000B1A //TVAR_afit_pBaseValS[427] /AFIT16_demsharpmix1_iHighThreshold                                                                /
{0x0F12,0x0032},	//70000B1C //TVAR_afit_pBaseValS[428] /AFIT16_demsharpmix1_iLowBright                                                                    /
{0x0F12,0x028A},	//70000B1E //TVAR_afit_pBaseValS[429] /AFIT16_demsharpmix1_iHighBright                                                                   /
{0x0F12,0x0032},	//70000B20 //TVAR_afit_pBaseValS[430] /AFIT16_demsharpmix1_iLowSat                                                                       /
{0x0F12,0x01F4},	//70000B22 //TVAR_afit_pBaseValS[431] /AFIT16_demsharpmix1_iHighSat                                                                      /
{0x0F12,0x0070},	//70000B24 //TVAR_afit_pBaseValS[432] /AFIT16_demsharpmix1_iTune                                                                         /
{0x0F12,0x0000},	//70000B26 //TVAR_afit_pBaseValS[433] /AFIT16_demsharpmix1_iHystThLow                                                                    /
{0x0F12,0x0000},	//70000B28 //TVAR_afit_pBaseValS[434] /AFIT16_demsharpmix1_iHystThHigh                                                                   /
{0x0F12,0x01AA},	//70000B2A //TVAR_afit_pBaseValS[435] /AFIT16_demsharpmix1_iHystCenter                                                                   /
{0x0F12,0x003C},	//70000B2C //TVAR_afit_pBaseValS[436] /AFIT16_YUV422_DENOISE_iUVLowThresh                                                                /
{0x0F12,0x0050},	//70000B2E //TVAR_afit_pBaseValS[437] /AFIT16_YUV422_DENOISE_iUVHighThresh                                                               /
{0x0F12,0x0000},	//70000B30 //TVAR_afit_pBaseValS[438] /AFIT16_YUV422_DENOISE_iYLowThresh                                                                 /
{0x0F12,0x0000},	//70000B32 //TVAR_afit_pBaseValS[439] /AFIT16_YUV422_DENOISE_iYHighThresh                                                                /
{0x0F12,0x0044},	//70000B34 //TVAR_afit_pBaseValS[440] /AFIT16_Sharpening_iLowSharpClamp                                                                  /
{0x0F12,0x0014},	//70000B36 //TVAR_afit_pBaseValS[441] /AFIT16_Sharpening_iHighSharpClamp                                                                 /
{0x0F12,0x0046},	//70000B38 //TVAR_afit_pBaseValS[442] /AFIT16_Sharpening_iLowSharpClamp_Bin                                                              /
{0x0F12,0x0019},	//70000B3A //TVAR_afit_pBaseValS[443] /AFIT16_Sharpening_iHighSharpClamp_Bin                                                             /
{0x0F12,0x0046},	//70000B3C //TVAR_afit_pBaseValS[444] /AFIT16_Sharpening_iLowSharpClamp_sBin                                                             /
{0x0F12,0x0019},	//70000B3E //TVAR_afit_pBaseValS[445] /AFIT16_Sharpening_iHighSharpClamp_sBin                                                            /
{0x0F12,0x0A24},	//70000B40 //TVAR_afit_pBaseValS[446] /AFIT8_sddd8a_edge_low [7:0],   AFIT8_sddd8a_repl_thresh [15:8]                                    /
{0x0F12,0x1701},	//70000B42 //TVAR_afit_pBaseValS[447] /AFIT8_sddd8a_repl_force [7:0],  AFIT8_sddd8a_sat_level [15:8]                                     /
{0x0F12,0x0229},	//70000B44 //TVAR_afit_pBaseValS[448] /AFIT8_sddd8a_sat_thr[7:0],  AFIT8_sddd8a_sat_mpl [15:8]                                           /
{0x0F12,0x0503},	//70000B46 //TVAR_afit_pBaseValS[449] /AFIT8_sddd8a_sat_noise[7:0],  AFIT8_sddd8a_iMaxSlopeAllowed [15:8]                                /
{0x0F12,0x080F},	//70000B48 //TVAR_afit_pBaseValS[450] /AFIT8_sddd8a_iHotThreshHigh[7:0],  AFIT8_sddd8a_iHotThreshLow [15:8]                              /
{0x0F12,0x0808},	//70000B4A //TVAR_afit_pBaseValS[451] /AFIT8_sddd8a_iColdThreshHigh[7:0],  AFIT8_sddd8a_iColdThreshLow [15:8]                            /
{0x0F12,0x0000},	//70000B4C //TVAR_afit_pBaseValS[452] /AFIT8_sddd8a_AddNoisePower1[7:0],  AFIT8_sddd8a_AddNoisePower2 [15:8]                             /
{0x0F12,0x00FF},	//70000B4E //TVAR_afit_pBaseValS[453] /AFIT8_sddd8a_iSatSat[7:0],   AFIT8_sddd8a_iRadialTune [15:8]                                      /
{0x0F12,0x012D},	//70000B50 //TVAR_afit_pBaseValS[454] /AFIT8_sddd8a_iRadialLimit [7:0],   AFIT8_sddd8a_iRadialPower [15:8]                               /
{0x0F12,0x1414},	//70000B52 //TVAR_afit_pBaseValS[455] /AFIT8_sddd8a_iLowMaxSlopeAllowed [7:0],  AFIT8_sddd8a_iHighMaxSlopeAllowed [15:8]                 /
{0x0F12,0x0301},	//70000B54 //TVAR_afit_pBaseValS[456] /AFIT8_sddd8a_iLowSlopeThresh[7:0],   AFIT8_sddd8a_iHighSlopeThresh [15:8]                         /
{0x0F12,0xFF07},	//70000B56 //TVAR_afit_pBaseValS[457] /AFIT8_sddd8a_iSquaresRounding [7:0],   AFIT8_Demosaicing_iCentGrad [15:8]                         /
{0x0F12,0x061E},	//70000B58 //TVAR_afit_pBaseValS[458] /AFIT8_Demosaicing_iMonochrom [7:0],   AFIT8_Demosaicing_iDecisionThresh [15:8]                    /
{0x0F12,0x0A1E},	//70000B5A //TVAR_afit_pBaseValS[459] /AFIT8_Demosaicing_iDesatThresh [7:0],   AFIT8_Demosaicing_iEnhThresh [15:8]                       /
{0x0F12,0x0606},	//70000B5C //TVAR_afit_pBaseValS[460] /AFIT8_Demosaicing_iGRDenoiseVal [7:0],   AFIT8_Demosaicing_iGBDenoiseVal [15:8]                   /
{0x0F12,0x0A01},	//70000B5E //TVAR_afit_pBaseValS[461] /AFIT8_Demosaicing_iNearGrayDesat[7:0],   AFIT8_Demosaicing_iDFD_ReduceCoeff [15:8]                /
{0x0F12,0x378B},	//70000B60 //TVAR_afit_pBaseValS[462] /AFIT8_Sharpening_iMSharpen [7:0],   AFIT8_Sharpening_iMShThresh [15:8]                            /
{0x0F12,0x1028},	//70000B62 //TVAR_afit_pBaseValS[463] /AFIT8_Sharpening_iWSharpen [7:0],   AFIT8_Sharpening_iWShThresh [15:8]                            /
{0x0F12,0x0001},	//70000B64 //TVAR_afit_pBaseValS[464] /AFIT8_Sharpening_nSharpWidth [7:0],   AFIT8_Sharpening_iReduceNegative [15:8]                     /
{0x0F12,0x00FF},	//70000B66 //TVAR_afit_pBaseValS[465] /AFIT8_Sharpening_iShDespeckle [7:0],  AFIT8_demsharpmix1_iRGBMultiplier [15:8]                    /
{0x0F12,0x1002},	//70000B68 //TVAR_afit_pBaseValS[466] /AFIT8_demsharpmix1_iFilterPower [7:0],  AFIT8_demsharpmix1_iBCoeff [15:8]                         /
{0x0F12,0x001E},	//70000B6A //TVAR_afit_pBaseValS[467] /AFIT8_demsharpmix1_iGCoeff [7:0],   AFIT8_demsharpmix1_iWideMult [15:8]                           /
{0x0F12,0x0900},	//70000B6C //TVAR_afit_pBaseValS[468] /AFIT8_demsharpmix1_iNarrMult [7:0],   AFIT8_demsharpmix1_iHystFalloff [15:8]                      /
{0x0F12,0x0600},	//70000B6E //TVAR_afit_pBaseValS[469] /AFIT8_demsharpmix1_iHystMinMult [7:0],   AFIT8_demsharpmix1_iHystWidth [15:8]                     /
{0x0F12,0x0504},	//70000B70 //TVAR_afit_pBaseValS[470] /AFIT8_demsharpmix1_iHystFallLow [7:0],   AFIT8_demsharpmix1_iHystFallHigh [15:8]                  /
{0x0F12,0x0307},	//70000B72 //TVAR_afit_pBaseValS[471] /AFIT8_demsharpmix1_iHystTune [7:0],  * AFIT8_YUV422_DENOISE_iUVSupport [15:8]                     /
{0x0F12,0x5001},	//70000B74 //TVAR_afit_pBaseValS[472] /AFIT8_YUV422_DENOISE_iYSupport [7:0],   AFIT8_byr_cgras_iShadingPower [15:8]                      /
{0x0F12,0x0080},	//70000B76 //TVAR_afit_pBaseValS[473] /AFIT8_RGBGamma2_iLinearity [7:0],  AFIT8_RGBGamma2_iDarkReduce [15:8]                             /
{0x0F12,0x0080},	//70000B78 //TVAR_afit_pBaseValS[474] /AFIT8_ccm_oscar_iSaturation[7:0],   AFIT8_RGB2YUV_iYOffset [15:8]                                 /
{0x0F12,0x0080},	//70000B7A //TVAR_afit_pBaseValS[475] /AFIT8_RGB2YUV_iRGBGain [7:0],   AFIT8_RGB2YUV_iSaturation [15:8]                                  /
{0x0F12,0x5050},	//70000B7C //TVAR_afit_pBaseValS[476] /AFIT8_sddd8a_iClustThresh_H [7:0],  AFIT8_sddd8a_iClustThresh_C [15:8]                            /
{0x0F12,0x0101},	//70000B7E //TVAR_afit_pBaseValS[477] /AFIT8_sddd8a_iClustMulT_H [7:0],   AFIT8_sddd8a_iClustMulT_C [15:8]                               /
{0x0F12,0x3201},	//70000B80 //TVAR_afit_pBaseValS[478] /AFIT8_sddd8a_nClustLevel_H [7:0],   AFIT8_sddd8a_DispTH_Low [15:8]                                /
{0x0F12,0x1832},	//70000B82 //TVAR_afit_pBaseValS[479] /AFIT8_sddd8a_DispTH_High [7:0],   AFIT8_sddd8a_iDenThreshLow [15:8]                               /        iden 12
{0x0F12,0x210C},	//70000B84 //TVAR_afit_pBaseValS[480] /AFIT8_sddd8a_iDenThreshHigh[7:0],   AFIT8_Demosaicing_iEdgeDesat [15:8]                           /
{0x0F12,0x0A00},	//70000B86 //TVAR_afit_pBaseValS[481] /AFIT8_Demosaicing_iEdgeDesatThrLow [7:0],   AFIT8_Demosaicing_iEdgeDesatThrHigh [15:8]            /
{0x0F12,0x1E04},	//70000B88 //TVAR_afit_pBaseValS[482] /AFIT8_Demosaicing_iEdgeDesatLimit[7:0],  AFIT8_Demosaicing_iDemSharpenLow [15:8]                  /    1E demhsharplow
{0x0F12,0x0A08},	//70000B8A //TVAR_afit_pBaseValS[483] /AFIT8_Demosaicing_iDemSharpenHigh[7:0],   AFIT8_Demosaicing_iDemSharpThresh [15:8]                /
{0x0F12,0x070C},	//70000B8C //TVAR_afit_pBaseValS[484] /AFIT8_Demosaicing_iDemShLowLimit [7:0],   AFIT8_Demosaicing_iDespeckleForDemsharp [15:8]          /    c8 clamp
{0x0F12,0x3264},	//70000B8E //TVAR_afit_pBaseValS[485] /AFIT8_Demosaicing_iDemBlurLow[7:0],   AFIT8_Demosaicing_iDemBlurHigh [15:8]                       /
{0x0F12,0x5A02},	//70000B90 //TVAR_afit_pBaseValS[486] /AFIT8_Demosaicing_iDemBlurRange[7:0],   AFIT8_Sharpening_iLowSharpPower [15:8]                    /
{0x0F12,0x1040},	//70000B92 //TVAR_afit_pBaseValS[487] /AFIT8_Sharpening_iHighSharpPower[7:0],   AFIT8_Sharpening_iLowShDenoise [15:8]                    /
{0x0F12,0x4012},	//70000B94 //TVAR_afit_pBaseValS[488] /AFIT8_Sharpening_iHighShDenoise [7:0],   AFIT8_Sharpening_iReduceEdgeMinMult [15:8]               /
{0x0F12,0x0604},	//70000B96 //TVAR_afit_pBaseValS[489] /AFIT8_Sharpening_iReduceEdgeSlope [7:0],  AFIT8_demsharpmix1_iWideFiltReduce [15:8]               /
{0x0F12,0x4606},	//70000B98 //TVAR_afit_pBaseValS[490] /AFIT8_demsharpmix1_iNarrFiltReduce [7:0],  AFIT8_sddd8a_iClustThresh_H_Bin [15:8]                 /
{0x0F12,0x0146},	//70000B9A //TVAR_afit_pBaseValS[491] /AFIT8_sddd8a_iClustThresh_C_Bin [7:0],   AFIT8_sddd8a_iClustMulT_H_Bin [15:8]                     /
{0x0F12,0x0101},	//70000B9C //TVAR_afit_pBaseValS[492] /AFIT8_sddd8a_iClustMulT_C_Bin [7:0],   AFIT8_sddd8a_nClustLevel_H_Bin [15:8]                      /
{0x0F12,0x1C18},	//70000B9E //TVAR_afit_pBaseValS[493] /AFIT8_sddd8a_DispTH_Low_Bin [7:0],   AFIT8_sddd8a_DispTH_High_Bin [15:8]                          /
{0x0F12,0x1819},	//70000BA0 //TVAR_afit_pBaseValS[494] /AFIT8_sddd8a_iDenThreshLow_Bin [7:0],   AFIT8_sddd8a_iDenThreshHigh_Bin [15:8]                    /
{0x0F12,0x0028},	//70000BA2 //TVAR_afit_pBaseValS[495] /AFIT8_Demosaicing_iEdgeDesat_Bin[7:0],   AFIT8_Demosaicing_iEdgeDesatThrLow_Bin [15:8]            /
{0x0F12,0x030A},	//70000BA4 //TVAR_afit_pBaseValS[496] /AFIT8_Demosaicing_iEdgeDesatThrHigh_Bin [7:0],  AFIT8_Demosaicing_iEdgeDesatLimit_Bin [15:8]      /
{0x0F12,0x0514},	//70000BA6 //TVAR_afit_pBaseValS[497] /AFIT8_Demosaicing_iDemSharpenLow_Bin [7:0],  AFIT8_Demosaicing_iDemSharpenHigh_Bin [15:8]         /
{0x0F12,0x0C14},	//70000BA8 //TVAR_afit_pBaseValS[498] /AFIT8_Demosaicing_iDemSharpThresh_Bin [7:0],  AFIT8_Demosaicing_iDemShLowLimit_Bin [15:8]         /
{0x0F12,0xFF05},	//70000BAA //TVAR_afit_pBaseValS[499] /AFIT8_Demosaicing_iDespeckleForDemsharp_Bin [7:0],  AFIT8_Demosaicing_iDemBlurLow_Bin [15:8]      /
{0x0F12,0x0432},	//70000BAC //TVAR_afit_pBaseValS[500] /AFIT8_Demosaicing_iDemBlurHigh_Bin [7:0],  AFIT8_Demosaicing_iDemBlurRange_Bin [15:8]             /
{0x0F12,0x4052},	//70000BAE //TVAR_afit_pBaseValS[501] /AFIT8_Sharpening_iLowSharpPower_Bin [7:0],  AFIT8_Sharpening_iHighSharpPower_Bin [15:8]           /
{0x0F12,0x1514},	//70000BB0 //TVAR_afit_pBaseValS[502] /AFIT8_Sharpening_iLowShDenoise_Bin [7:0],  AFIT8_Sharpening_iHighShDenoise_Bin [15:8]             /
{0x0F12,0x0440},	//70000BB2 //TVAR_afit_pBaseValS[503] /AFIT8_Sharpening_iReduceEdgeMinMult_Bin [7:0],  AFIT8_Sharpening_iReduceEdgeSlope_Bin [15:8]      /
{0x0F12,0x0302},	//70000BB4 //TVAR_afit_pBaseValS[504] /AFIT8_demsharpmix1_iWideFiltReduce_Bin [7:0],  AFIT8_demsharpmix1_iNarrFiltReduce_Bin [15:8]      /
{0x0F12,0x4646},	//70000BB6 //TVAR_afit_pBaseValS[505] /AFIT8_sddd8a_iClustThresh_H_sBin[7:0],   AFIT8_sddd8a_iClustThresh_C_sBin [15:8]                  /
{0x0F12,0x0101},	//70000BB8 //TVAR_afit_pBaseValS[506] /AFIT8_sddd8a_iClustMulT_H_sBin [7:0],   AFIT8_sddd8a_iClustMulT_C_sBin [15:8]                     /
{0x0F12,0x1801},	//70000BBA //TVAR_afit_pBaseValS[507] /AFIT8_sddd8a_nClustLevel_H_sBin [7:0],   AFIT8_sddd8a_DispTH_Low_sBin [15:8]                      /
{0x0F12,0x191C},	//70000BBC //TVAR_afit_pBaseValS[508] /AFIT8_sddd8a_DispTH_High_sBin [7:0],   AFIT8_sddd8a_iDenThreshLow_sBin [15:8]                     /
{0x0F12,0x2818},	//70000BBE //TVAR_afit_pBaseValS[509] /AFIT8_sddd8a_iDenThreshHigh_sBin[7:0],   AFIT8_Demosaicing_iEdgeDesat_sBin [15:8]                 /
{0x0F12,0x0A00},	//70000BC0 //TVAR_afit_pBaseValS[510] /AFIT8_Demosaicing_iEdgeDesatThrLow_sBin [7:0],  AFIT8_Demosaicing_iEdgeDesatThrHigh_sBin [15:8]   /
{0x0F12,0x1403},	//70000BC2 //TVAR_afit_pBaseValS[511] /AFIT8_Demosaicing_iEdgeDesatLimit_sBin [7:0],  AFIT8_Demosaicing_iDemSharpenLow_sBin [15:8]       /
{0x0F12,0x1405},	//70000BC4 //TVAR_afit_pBaseValS[512] /AFIT8_Demosaicing_iDemSharpenHigh_sBin [7:0],  AFIT8_Demosaicing_iDemSharpThresh_sBin [15:8]      /
{0x0F12,0x050C},	//70000BC6 //TVAR_afit_pBaseValS[513] /AFIT8_Demosaicing_iDemShLowLimit_sBin [7:0],  AFIT8_Demosaicing_iDespeckleForDemsharp_sBin [15:8] /
{0x0F12,0x32FF},	//70000BC8 //TVAR_afit_pBaseValS[514] /AFIT8_Demosaicing_iDemBlurLow_sBin [7:0],  AFIT8_Demosaicing_iDemBlurHigh_sBin [15:8]             /
{0x0F12,0x5204},	//70000BCA //TVAR_afit_pBaseValS[515] /AFIT8_Demosaicing_iDemBlurRange_sBin [7:0],  AFIT8_Sharpening_iLowSharpPower_sBin [15:8]          /
{0x0F12,0x1440},	//70000BCC //TVAR_afit_pBaseValS[516] /AFIT8_Sharpening_iHighSharpPower_sBin [7:0],  AFIT8_Sharpening_iLowShDenoise_sBin [15:8]          /
{0x0F12,0x4015},	//70000BCE //TVAR_afit_pBaseValS[517] /AFIT8_Sharpening_iHighShDenoise_sBin [7:0],  AFIT8_Sharpening_iReduceEdgeMinMult_sBin [15:8]      /
{0x0F12,0x0204},	//70000BD0 //TVAR_afit_pBaseValS[518] /AFIT8_Sharpening_iReduceEdgeSlope_sBin [7:0],  AFIT8_demsharpmix1_iWideFiltReduce_sBin [15:8]     /
{0x0F12,0x0003},	//70000BD2 //TVAR_afit_pBaseValS[519] /AFIT8_demsharpmix1_iNarrFiltReduce_sBin [7:0]                                                     /
              
{0x0F12,0x7DFA},	//afit_pConstBaseValS[0] //                                                                          
{0x0F12,0xFFBD},	//afit_pConstBaseValS[1] //                                                                          
{0x0F12,0x26FE},	//afit_pConstBaseValS[2] //                                                                          
{0x0F12,0xF7BC},	//afit_pConstBaseValS[3] //                                                                          
{0x0F12,0x7E06},	//afit_pConstBaseValS[4] //                                                                          
{0x0F12,0x00D3},	//afit_pConstBaseValS[5] //                                                                          

          
{0x002A,0x2CE8},    
{0x0F12,0x0007},	// Modify LSB to control AWBB_YThreshLow //    	                                                                        
{0x0F12,0x00E2},	//                                         
{0x0F12,0x0005},	// Modify LSB to control AWBB_YThreshLowBrLow//                                                                        

{0x0F12,0x00e2},	// 
{0x002A,0x337A},
{0x0F12,0x0006},	// #Tune_TP_atop_dbus_reg // 6 is the default HW value    
//===========================================================
// IP-FE Setting END
//===========================================================
        
//===============================================================================================
// ST PLL
//===============================================================================================
// Hw to set
// 1 MCLK
//he(CLK you want) * 1000)
// 2 System CLK
//he((CLK you want) * 1000 / 4)
// 3 PCLK
//he((CLK you want) * 1000 / 4)
//===============================================================================================
{0x0028,0x7000},
{0x002A,0x01CC},
{0x0F12,0x5DC0},	// 5DC0=24MCLK #REG_TC_IPRM_InClockLSBs
{0x0F12,0x0000},	// #REG_TC_IPRM_InClockMSBs
{0x002A,0x01EE},
{0x0F12,0x0002},	// #REG_TC_IPRM_UseNPviClocks // Number of PLL setting
{0x002A,0x01F6},
{0x0F12,0x2710},	//40M #REG_TC_IPRM_OpClk4KHz_0
{0x0F12,0x2ee0},	//48M #REG_TC_IPRM_MinOutRate4KHz_0
{0x0F12,0x2ee0},	//48M #REG_TC_IPRM_MaxOutRate4KHz_0
{0x0F12,0x2ee0},	//48M #REG_TC_IPRM_OpClk4KHz_1
{0x0F12,0x2ee0},	//48M #REG_TC_IPRM_MinOutRate4KHz_1
{0x0F12,0x2ee0},	//48M #REG_TC_IPRM_MaxOutRate4KHz_1
{0x002A,0x0208},
{0x0F12,0x0001},	// #REG_TC_IPRM_InitParamsUpdated
//===========================================================
// Fame rate setting 
//===========================================================
// Hw to set
// 1 Exposure value
// dc2hex((1 / (frame rate you want(ms))) * 100d * 4d)
// 2 Analog Digital gain
// dc2hex((Analog gain you want) * 256d)
//===========================================================
//Se preview exposure time
{0x002A,0x0530},                             
{0x0F12,0x5DC0},	// #lt_uMaxExp1 			60ms                             
{0x0F12,0x0000},	                           
{0x0F12,0x6D60},	// #lt_uMaxExp2 			70ms                            
{0x0F12,0x0000},	                           
{0x002A,0x167C},	                           
{0x0F12,0x9C40},	// #evt1_lt_uMaxExp3 	100ms                             
{0x0F12,0x0000},	                            
{0x0F12,0x3880},	// #evt1_lt_uMaxExp4 	120ms                            
{0x0F12,0x0001},                             
//Se capture exposure time                            
{0x002A,0x0538},                            
{0x0F12,0x5DC0},	// #lt_uCapMaxExp1 60ms                             
{0x0F12,0x0000},	                                                         
{0x0F12,0x6D60},	// #lt_uCapMaxExp2      70ms                             
{0x0F12,0x0000},	                                                         
{0x002A,0x1684},	                                                         
{0x0F12,0x9C40},	// #evt1_lt_uCapMaxExp3 100ms                            
{0x0F12,0x0000},	                                                          
{0x0F12,0x3880},	// #evt1_lt_uCapMaxExp4 120ms                            
{0x0F12,0x0001},	                           
//Se gain                            
{0x002A,0x0540},                             
{0x0F12,0x0200},	// #lt_uMaxAnGain1                            
{0x0F12,0x0380},	// #lt_uMaxAnGain2                            
{0x002A,0x168C},	                           
{0x0F12,0x0420},	// #evt1_lt_uMaxAnGain3                            
{0x0F12,0x0700},	// #evt1_lt_uMaxAnGain4                             
{0x002A,0x0544},	
{0x0F12,0x0100},	// #lt_uMaxDigGain
{0x0F12,0x1000},	// #lt_uMaxTotGain
{0x002A,0x1694},	
{0x0F12,0x0001},	// #evt1_senHal_bExpandForbid
{0x002A,0x051A},	
{0x0F12,0x0111},	// #lt_uLimitHigh 
{0x0F12,0x00F0},	// #lt_uLimitLow
//===============================================================================================
// ST PREVIEW CONFIGURATION_0
// #Foramt : YUV422
// #Size: 1024x768
// #FPS : 20fps for normal mode
//===============================================================================================
{0x002A,0x026C},
{0x0F12,0x0400},	//#REG_0TC_PCFG_usWidth  
{0x0F12,0x0300},	//#REG_0TC_PCFG_usHeight    026E 
{0x0F12,0x0005},	//#REG_0TC_PCFG_Format            0270
{0x0F12,0x2ee0},	//#REG_0TC_PCFG_usMaxOut4KHzRate  0272
{0x0F12,0x2ee0},	//#REG_0TC_PCFG_usMinOut4KHzRate  0274
{0x0F12,0x0100},	//#REG_0TC_PCFG_OutClkPerPix88    0276
{0x0F12,0x0800},	//#REG_0TC_PCFG_uMaxBpp88         027  
{0x0F12,0x0052},	//#REG_0TC_PCFG_PVIMask //s0050 = FALSE in MSM6290 : s0052 = TRUE in MSM6800 //reg 027A
{0x0F12,0x4000},	//#REG_0TC_PCFG_OIFMask
{0x0F12,0x01E0},	//#REG_0TC_PCFG_usJpegPacketSize
{0x0F12,0x0000},	//#REG_0TC_PCFG_usJpegTotalPackets
{0x0F12,0x0000},	//#REG_0TC_PCFG_uClockInd
{0x0F12,0x0002},	//#REG_0TC_PCFG_usFrTimeType
{0x0F12,0x0001},	//#REG_0TC_PCFG_FrRateQualityType
{0x0F12,0x01f0},	//#REG_0TC_PCFG_usMaxFrTimeMsecMult10 //10fps
{0x0F12,0x0000},	//#REG_0TC_PCFG_usMinFrTimeMsecMult10 //22fps
{0x0F12,0x0000},	//#REG_0TC_PCFG_bSmearOutput
{0x0F12,0x0000},	//#REG_0TC_PCFG_sSaturation
{0x0F12,0x0000},	//#REG_0TC_PCFG_sSharpBlur
{0x0F12,0x0000},	//#REG_0TC_PCFG_sColorTemp
{0x0F12,0x0000},	//#REG_0TC_PCFG_uDeviceGammaIndex
{0x0F12,0x0000},	//#REG_0TC_PCFG_uPrevMirror
{0x0F12,0x0000},	//#REG_0TC_PCFG_uCaptureMirror
{0x0F12,0x0000},	//#REG_0TC_PCFG_uRotation
//===============================================================================================
// APLY PREVIEW CONFIGURATION & RUN PREVIEW
//===============================================================================================
{0x002A,0x023C},
{0x0F12,0x0000},	// #REG_TC_GP_ActivePrevConfig // Select preview configuration_0
{0x002A,0x0240},	
{0x0F12,0x0001},	// #REG_TC_GP_PrevOpenAfterChange
{0x002A,0x0230},	
{0x0F12,0x0001},	// #REG_TC_GP_NewConfigSync // Update preview configuration
{0x002A,0x023E},	
{0x0F12,0x0001},	// #REG_TC_GP_PrevConfigChanged
{0x002A,0x0220},	
{0x0F12,0x0001},	// #REG_TC_GP_EnablePreview // Start preview
{0x0F12,0x0001},	// #REG_TC_GP_EnablePreviewChanged
//================================================================================================
// SET CAPTURE CONFIGURATION_0
// # Foramt :YUV
// # Size: 2048*1536
// # FPS : 13 ~ 15fps
//================================================================================================
{0x002A,0x035C}, 
{0x0F12,0x0000},	//#REG_0TC_CCFG_uCaptureModeJpEG
{0x0F12,0x0800},	//#REG_0TC_CCFG_usWidth 
{0x0F12,0x0600},	//#REG_0TC_CCFG_usHeight
{0x0F12,0x0005},	//#REG_0TC_CCFG_Format//5:YUV9:JPEG 
{0x0F12,0x2ee0},	//#REG_0TC_CCFG_usMaxOut4KHzRate
{0x0F12,0x2e90},	//#REG_0TC_CCFG_usMinOut4KHzRate
{0x0F12,0x0100},	//#REG_0TC_CCFG_OutClkPerPix88
{0x0F12,0x0800},	//#REG_0TC_CCFG_uMaxBpp88 
{0x0F12,0x0052},	//#REG_0TC_CCFG_PVIMask 
{0x0F12,0x0050},	//#REG_0TC_CCFG_OIFMask   edison
{0x0F12,0x01E0},	//#REG_0TC_CCFG_usJpegPacketSize
{0x0F12,0x08fc},	//#REG_0TC_CCFG_usJpegTotalPackets
{0x0F12,0x0001},	//#REG_0TC_CCFG_uClockInd 
{0x0F12,0x0002},	//#REG_0TC_CCFG_usFrTimeType
{0x0F12,0x0000},	//#REG_0TC_CCFG_FrRateQualityType 
{0x0F12,0x0588},	//#REG_0TC_CCFG_usMaxFrTimeMsecMult10 //7fps
{0x0F12,0x0000},	//#REG_0TC_CCFG_usMinFrTimeMsecMult10 
{0x0F12,0x0000},	//#REG_0TC_CCFG_bSmearOutput
{0x0F12,0x0000},	//#REG_0TC_CCFG_sSaturation 
{0x0F12,0x0000},	//#REG_0TC_CCFG_sSharpBlur
{0x0F12,0x0000},	//#REG_0TC_CCFG_sColorTemp
{0x0F12,0x0000},	//#REG_0TC_CCFG_uDeviceGammaIndex 
    
{0x0028,0xD000},
{0x002A,0x1000},
{0x0F12,0x0001},
{0x0028,0x7000},
 //#endif

};
*/

struct cam_preview priview_arr[]={
{4,{0xFC,0xFC,0xD0,0x00}},
{4,{0x00,0x10,0x00,0x01}},
{4,{0x10,0x30,0x00,0x00}},
{4,{0x00,0x14,0x00,0x01}},
{-2,{0x0}},    
{4,{0x00,0x28,0xD0,0x00}},
{4,{0x00,0x2A,0x10,0x82}},
{10,{0x0F,0x12,0x03,0xff,0x03,0xff,0x15,0x55,0x03,0xff}},
{4,{0x00,0x28,0x70,0x00}},
{4,{0x00,0x2A,0x2C,0xF8}},
{870,{0x0F,0x12,0xB5,0x10,0x49,0x0F,0x20,0x00,0x80,0x48,0x80,0x88,0x49,0x0E,0x48,0x0E,0xF0,0x00,0xF9,0x49,0x49,0x0E,0x48,0x0E,0x63,0x41,0x49,0x0E,0x38,0xC0,0x63,0xC1,0x49,0x0E,0x63,0x01,0x49,0x0E,0x30,0x40,0x61,0x81,0x49,0x0D,0x48,0x0E,0xF0,0x00,0xF9,0x3A,0x49,0x0D,0x48,
0x0E,0xF0,0x00,0xF9,0x36,0xBC,0x10,0xBC,0x08,0x47,0x18,0x00,0x00,0x10,0x80,0xD0,0x00,0x2D,0x69,0x70,0x00,0x89,0xA9,0x00,0x00,0x2D,0xBB,0x70,0x00,0x01,0x40,0x70,0x00,0x2D,0xED,0x70,0x00,0x2E,0x65,0x70,0x00,0x2E,0x79,0x70,0x00,0x2E,0x4D,0x70,0x00,0x01,0x3D,0x00,0x01,0x2F,
0x03,0x70,0x00,0x58,0x23,0x00,0x00,0xB5,0x70,0x68,0x04,0x68,0x45,0x68,0x81,0x68,0x40,0x29,0x00,0x68,0x80,0xD0,0x07,0x49,0x76,0x89,0x49,0x08,0x4A,0x18,0x80,0xF0,0x00,0xF9,0x14,0x80,0xA0,0xE0,0x00,0x80,0xA0,0x88,0xA0,0x28,0x00,0xD0,0x10,0x68,0xA9,0x68,0x28,0x08,0x4A,0x18,
0x80,0xF0,0x00,0xF9,0x08,0x80,0x20,0x1D,0x2D,0xCD,0x03,0x08,0x4A,0x18,0x80,0xF0,0x00,0xF9,0x01,0x80,0x60,0xBC,0x70,0xBC,0x08,0x47,0x18,0x20,0x00,0x80,0x60,0x80,0x20,0xE7,0xF8,0xB5,0x10,0xF0,0x00,0xF8,0xFC,0x48,0x65,0x49,0x66,0x88,0x00,0x4A,0x66,0x28,0x05,0xD0,0x03,0x4B,
0x65,0x79,0x5B,0x2B,0x00,0xD0,0x05,0x20,0x01,0x80,0x08,0x80,0x10,0xBC,0x10,0xBC,0x08,0x47,0x18,0x28,0x00,0xD1,0xFA,0x20,0x00,0x80,0x08,0x80,0x10,0xE7,0xF6,0xB5,0x70,0x00,0x04,0x48,0x5D,0x2C,0x00,0x8D,0x00,0xD0,0x01,0x25,0x01,0xE0,0x00,0x25,0x00,0x4E,0x5B,0x43,0x28,0x80,
0x30,0x20,0x7D,0x00,0xC0,0xF0,0x00,0xF8,0xDE,0x48,0x58,0x2C,0x00,0x8C,0x40,0x03,0x29,0x43,0x08,0x81,0x30,0x48,0x56,0x2C,0x00,0x8A,0x40,0x01,0xA9,0x43,0x08,0x80,0xB0,0x2C,0x00,0xD0,0x0B,0x48,0x53,0x8A,0x01,0x48,0x53,0xF0,0x00,0xF8,0xBD,0x49,0x53,0x88,0x09,0x43,0x48,0x04,
0x00,0x0C,0x00,0xF0,0x00,0xF8,0xC4,0x00,0x20,0xF0,0x00,0xF8,0xC9,0x48,0x4F,0x70,0x04,0xE7,0xAF,0xB5,0x10,0x00,0x04,0xF0,0x00,0xF8,0xCA,0x60,0x20,0x49,0x4C,0x8B,0x49,0x07,0x89,0xD0,0xBD,0x00,0x40,0x60,0x20,0xE7,0xBA,0xB5,0x10,0xF0,0x00,0xF8,0xC7,0x48,0x48,0x88,0x80,0x06,
0x01,0x48,0x40,0x16,0x09,0x82,0x81,0xE7,0xB0,0xB5,0xF8,0x00,0x0F,0x4C,0x3A,0x34,0x20,0x25,0x00,0x57,0x65,0x00,0x39,0xF0,0x00,0xF8,0xBF,0x90,0x00,0x26,0x00,0x57,0xA6,0x4C,0x38,0x42,0xAE,0xD0,0x1B,0x4D,0x3D,0x8A,0xE8,0x28,0x00,0xD0,0x13,0x48,0x32,0x8A,0x01,0x8B,0x80,0x43,
0x78,0xF0,0x00,0xF8,0x81,0x89,0xA9,0x1A,0x41,0x48,0x37,0x38,0x20,0x8A,0xC0,0x43,0x48,0x17,0xC1,0x0D,0x89,0x18,0x08,0x12,0x80,0x8A,0xA1,0x1A,0x08,0x82,0xA0,0xE0,0x03,0x88,0xA8,0x06,0x00,0x16,0x00,0x82,0xA0,0x20,0x14,0x5E,0x20,0x42,0xB0,0xD0,0x11,0xF0,0x00,0xF8,0x9F,0x1D,
0x40,0x00,0xC3,0x1A,0x18,0x21,0x4B,0xF0,0x00,0xF8,0x63,0x21,0x1F,0xF0,0x00,0xF8,0x9E,0x21,0x14,0x5E,0x61,0x0F,0xC9,0x01,0x49,0x43,0x01,0x48,0x26,0x81,0xC1,0x98,0x00,0xBC,0xF8,0xBC,0x08,0x47,0x18,0xB5,0xF1,0xB0,0x82,0x25,0x00,0x48,0x22,0x90,0x01,0x24,0x00,0x20,0x28,0x43,
0x68,0x4A,0x21,0x49,0x17,0x18,0x82,0x39,0xE0,0x18,0x47,0x92,0x00,0x00,0x66,0x19,0xB8,0x9A,0x01,0x30,0x60,0x8B,0x01,0x5B,0xB8,0x88,0x12,0xF0,0x00,0xF8,0x84,0x99,0x00,0x53,0x88,0x1C,0x64,0x2C,0x14,0xDB,0xF1,0x1C,0x6D,0x2D,0x03,0xDB,0xE5,0x98,0x02,0x68,0x00,0x06,0x00,0x0E,
0x00,0xF0,0x00,0xF8,0x7E,0xBC,0xFE,0xBC,0x08,0x47,0x18,0x00,0x00,0x0C,0x3C,0x70,0x00,0x26,0xE8,0x70,0x00,0x61,0x00,0xD0,0x00,0x65,0x00,0xD0,0x00,0x1A,0x7C,0x70,0x00,0x2C,0x2C,0x70,0x00,0xF4,0x00,0xD0,0x00,0x16,0x7C,0x70,0x00,0x33,0x68,0x70,0x00,0x1D,0x6C,0x70,0x00,0x40,
0xA0,0x00,0xDD,0xF5,0x20,0xD0,0x00,0x2C,0x29,0x70,0x00,0x1A,0x54,0x70,0x00,0x15,0x64,0x70,0x00,0xF2,0xA0,0xD0,0x00,0x24,0x40,0x70,0x00,0x05,0xA0,0x70,0x00,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0x1A,0x3F,0x00,0x01,0x47,0x78,0x46,0xC0,0xF0,0x04,0xE5,
0x1F,0x1F,0x48,0x00,0x01,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0x24,0xBD,0x00,0x00,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0xF5,0x3F,0x00,0x00,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0xF5,0xD9,0x00,0x00,0x47,
0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0x01,0x3D,0x00,0x01,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0xF5,0xC9,0x00,0x00,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0xFA,0xA9,0x00,0x00,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,
0x9F,0xFF,0x1C,0xE1,0x2F,0x36,0xDD,0x00,0x00,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0x36,0xED,0x00,0x00,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0x37,0x23,0x00,0x00,0x47,0x78,0x46,0xC0,0xC0,0x00,0xE5,0x9F,0xFF,0x1C,0xE1,0x2F,0x58,
0x23,0x00,0x00,0x7D,0x3E,0x00,0x00}},
{4,{0x00,0x28,0x70,0x00}},
{4,{0x00,0x2A,0x15,0x7A}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x15,0x78}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x15,0x76}},
{4,{0x0F,0x12,0x00,0x20}},
{4,{0x00,0x2A,0x15,0x74}},
{4,{0x0F,0x12,0x00,0x06}},
{4,{0x00,0x2A,0x15,0x6E}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x15,0x68}},
{4,{0x0F,0x12,0x00,0xFC}},
{4,{0x00,0x2A,0x15,0x5A}},
{4,{0x0F,0x12,0x01,0xCC}},
{4,{0x00,0x2A,0x15,0x7E}},
{6,{0x0F,0x12,0x0C,0x80,0x05,0x78}},
{4,{0x00,0x2A,0x15,0x7C}},
{4,{0x0F,0x12,0x01,0x90}},
{4,{0x00,0x2A,0x15,0x70}},
{6,{0x0F,0x12,0x00,0xA0,0x00,0x10}},
{4,{0x00,0x2A,0x12,0xC4}},
{4,{0x0F,0x12,0x00,0x6A}},
{4,{0x00,0x2A,0x12,0xC8}},
{6,{0x0F,0x12,0x08,0xAC,0x00,0x50}},
{4,{0x00,0x2A,0x16,0x96}},
{10,{0x0F,0x12,0x00,0x00,0x00,0x00,0x00,0xC6,0x00,0xC6}},
{4,{0x00,0x2A,0x16,0x90}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x12,0xB0}},
{6,{0x0F,0x12,0x00,0x55,0x00,0x5A}},
{4,{0x00,0x2A,0x33,0x7A}},
{4,{0x0F,0x12,0x00,0x06}},
{4,{0x00,0x2A,0x16,0x9E}},
{4,{0x0F,0x12,0x00,0x0A}},
{4,{0x00,0x28,0xD0,0x00}},
{4,{0x00,0x2A,0xF4,0x06}},
{4,{0x0F,0x12,0x10,0x00}},
{4,{0x00,0x2A,0xF4,0x0A}},
{4,{0x0F,0x12,0x69,0x98}},
{4,{0x00,0x2A,0xF4,0x18}},
{6,{0x0F,0x12,0x00,0x78,0x04,0xFE}},
{4,{0x00,0x2A,0xF5,0x2C}},
{4,{0x0F,0x12,0x88,0x00}},
{4,{0x00,0x28,0x70,0x00}},
{4,{0x00,0x2A,0x12,0xD2}},
{642,{0x0F,0x12,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x08,0x84,0x08,0xCF,0x05,0x00,0x05,0x4B,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x08,0x85,0x04,0x67,0x05,0x01,0x02,0xA5,0x00,0x01,0x04,0x6A,0x00,0x01,0x02,0xA8,0x08,0x85,0x08,0xD0,0x05,0x01,0x05,0x4C,0x00,0x06,0x00,
0x20,0x00,0x06,0x00,0x20,0x08,0x81,0x04,0x63,0x04,0xFD,0x02,0xA1,0x00,0x06,0x04,0x89,0x00,0x06,0x02,0xC7,0x08,0x81,0x08,0xCC,0x04,0xFD,0x05,0x48,0x03,0xA2,0x01,0xD3,0x01,0xE0,0x00,0xF2,0x03,0xF2,0x02,0x23,0x02,0x30,0x01,0x42,0x03,0xA2,0x06,0x3C,0x01,0xE0,0x03,0x99,0x03,
0xF2,0x06,0x8C,0x02,0x30,0x03,0xE9,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x01,0xD3,0x01,0xD3,0x00,0xF2,0x00,0xF2,0x02,0x0B,0x02,0x4A,0x01,0x2A,0x01,0x69,0x00,0x02,0x04,0x6B,0x00,0x02,0x02,0xA9,0x04,0x19,0x04,0xA5,0x02,0x57,0x02,
0xE3,0x06,0x30,0x06,0x3C,0x03,0x8D,0x03,0x99,0x06,0x68,0x06,0xB3,0x03,0xC5,0x04,0x10,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x03,0xA2,0x01,0xD3,0x01,0xE0,0x00,0xF2,0x00,0x00,0x04,0x61,0x00,0x00,0x02,0x9F,0x00,0x00,0x06,0x3C,0x00,0x00,0x03,0x99,0x00,0x3D,0x00,0x3D,0x00,
0x3D,0x00,0x3D,0x01,0xD0,0x01,0xD0,0x00,0xEF,0x00,0xEF,0x02,0x0C,0x02,0x4B,0x01,0x2B,0x01,0x6A,0x03,0x9F,0x04,0x5E,0x01,0xDD,0x02,0x9C,0x04,0x1A,0x04,0xA6,0x02,0x58,0x02,0xE4,0x06,0x2D,0x06,0x39,0x03,0x8A,0x03,0x96,0x06,0x69,0x06,0xB4,0x03,0xC6,0x04,0x11,0x08,0x7C,0x08,
0xC7,0x04,0xF8,0x05,0x43,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x01,0xD0,0x01,0xD0,0x00,0xEF,0x00,0xEF,0x02,0x0F,0x02,0x4E,0x01,0x2E,0x01,0x6D,0x03,0x9F,0x04,0x5E,0x01,0xDD,0x02,0x9C,0x04,0x1D,0x04,0xA9,0x02,0x5B,0x02,0xE7,0x06,0x2D,0x06,0x39,0x03,0x8A,0x03,0x96,0x06,
0x6C,0x06,0xB7,0x03,0xC9,0x04,0x14,0x08,0x7C,0x08,0xC7,0x04,0xF8,0x05,0x43,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x01,0xD0,0x01,0xD0,0x00,0xEF,0x00,0xEF,0x02,0x0F,0x02,0x4E,0x01,0x2E,0x01,0x6D,0x03,0x9F,0x04,0x5E,0x01,0xDD,0x02,0x9C,0x04,0x1D,0x04,0xA9,0x02,0x5B,0x02,
0xE7,0x06,0x2D,0x06,0x39,0x03,0x8A,0x03,0x96,0x06,0x6C,0x06,0xB7,0x03,0xC9,0x04,0x14,0x08,0x7C,0x08,0xC7,0x04,0xF8,0x05,0x43,0x00,0x3D,0x00,0x3D,0x00,0x3D,0x00,0x3D,0x01,0xD2,0x01,0xD2,0x00,0xF1,0x00,0xF1,0x02,0x0C,0x02,0x4B,0x01,0x2B,0x01,0x6A,0x03,0xA1,0x04,0x60,0x01,
0xDF,0x02,0x9E,0x04,0x1A,0x04,0xA6,0x02,0x58,0x02,0xE4,0x06,0x2F,0x06,0x3B,0x03,0x8C,0x03,0x98,0x06,0x69,0x06,0xB4,0x03,0xC6,0x04,0x11,0x08,0x7E,0x08,0xC9,0x04,0xFA,0x05,0x45,0x03,0xA2,0x01,0xD3,0x01,0xE0,0x00,0xF2,0x03,0xAF,0x01,0xE0,0x01,0xED,0x00,0xFF,0x00,0x00,0x04,
0x61,0x00,0x00,0x02,0x9F,0x00,0x00,0x04,0x6E,0x00,0x00,0x02,0xAC,0x00,0x00,0x06,0x3C,0x00,0x00,0x03,0x99,0x00,0x00,0x06,0x49,0x00,0x00,0x03,0xA6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xAA,0x01,0xDB,0x01,0xE8,0x00,0xFA,0x03,
0xB7,0x01,0xE8,0x01,0xF5,0x01,0x07,0x00,0x00,0x04,0x69,0x00,0x00,0x02,0xA7,0x00,0x00,0x04,0x76,0x00,0x00,0x02,0xB4,0x00,0x00,0x06,0x44,0x00,0x00,0x03,0xA1,0x00,0x00,0x06,0x51,0x00,0x00,0x03,0xAE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x05,0xAD,0x03,0xDE,0x03,0x0A,0x02,0x1C,0x06,0x2F,0x04,0x60,0x03,0x8C,0x02,0x9E,0x07,0xFC,0x08,0x47,0x04,0x78,0x04,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{4,{0x00,0x2A,0x15,0x8A}},
{4,{0x0F,0x12,0xEA,0xF0}},
{4,{0x00,0x2A,0x15,0xC6}},
{6,{0x0F,0x12,0x00,0x20,0x00,0x60}},
{4,{0x00,0x2A,0x15,0xBC}},
{4,{0x0F,0x12,0x02,0x00}},
{4,{0x00,0x2A,0x16,0x08}},
{10,{0x0F,0x12,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00}},
{4,{0x00,0x2A,0x0F,0x70}},
{4,{0x0F,0x12,0x00,0x40}},
{4,{0x00,0x2A,0x05,0x30}},
{4,{0x0F,0x12,0x34,0x15}},
{4,{0x00,0x2A,0x05,0x34}},
{4,{0x0F,0x12,0x68,0x2A}},
{4,{0x00,0x2A,0x16,0x7C}},
{4,{0x0F,0x12,0x82,0x35}},
{4,{0x00,0x2A,0x16,0x80}},
{4,{0x0F,0x12,0xC3,0x50}},
{4,{0x00,0x2A,0x05,0x38}},
{4,{0x0F,0x12,0x34,0x15}},
{4,{0x00,0x2A,0x05,0x3C}},
{4,{0x0F,0x12,0x68,0x2A}},
{4,{0x00,0x2A,0x16,0x84}},
{4,{0x0F,0x12,0x82,0x35}},
{4,{0x00,0x2A,0x16,0x88}},
{4,{0x0F,0x12,0xC3,0x50}},
{4,{0x00,0x2A,0x05,0x40}},
{6,{0x0F,0x12,0x01,0xB3,0x01,0xB3}},
{4,{0x00,0x2A,0x16,0x8C}},
{6,{0x0F,0x12,0x02,0xA0,0x07,0x10}},
{4,{0x00,0x2A,0x05,0x44}},
{6,{0x0F,0x12,0x01,0x00,0x80,0x00}},
{4,{0x00,0x2A,0x16,0x94}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x02,0x1A}},
{4,{0x0F,0x12,0x00,0x00}},
{4,{0x00,0x2A,0x04,0xD2}},
{4,{0x0F,0x12,0x06,0x5F}},
{4,{0x00,0x2A,0x04,0xBA}},
{6,{0x0F,0x12,0x00,0x01,0x00,0x01}},
{4,{0x00,0x2A,0x06,0xCE}},
{130,{0x0F,0x12,0x01,0x20,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x30,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0xF8,0x01,0x0A,0x01,0x0A,0x01,0x00,0x01,0x01,0x01,0x03,0x01,0x03,0x01,0x01,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0xf8,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x1F,0x00,0x20,0x00,0x00}},
{4,{0x00,0x2A,0x07,0x5A}},
{12,{0x0F,0x12,0x00,0x00,0x04,0x00,0x03,0x00,0x00,0x10,0x00,0x11}},
{4,{0x00,0x2A,0x34,0x7C}},
{1146,{0x0F,0x12,0x01,0xAE,0x01,0x70,0x01,0x45,0x01,0x1E,0x00,0xFE,0x00,0xE6,0x00,0xDC,0x00,0xE0,0x00,0xF0,0x01,0x10,0x01,0x37,0x01,0x69,0x01,0xB4,0x01,0x86,0x01,0x54,0x01,0x20,0x00,0xED,0x00,0xC2,0x00,0xA8,0x00,0x9C,0x00,0xA3,0x00,0xB9,0x00,0xE3,0x01,0x19,0x01,0x53,0x01,
0x8C,0x01,0x56,0x01,0x22,0x00,0xE4,0x00,0xA9,0x00,0x7B,0x00,0x60,0x00,0x55,0x00,0x5C,0x00,0x76,0x00,0xA4,0x00,0xE1,0x01,0x25,0x01,0x5E,0x01,0x2E,0x00,0xF7,0x00,0xB2,0x00,0x74,0x00,0x46,0x00,0x2A,0x00,0x21,0x00,0x29,0x00,0x44,0x00,0x74,0x00,0xB4,0x00,0xFF,0x01,0x3D,0x01,
0x1D,0x00,0xE0,0x00,0x95,0x00,0x56,0x00,0x28,0x00,0x0F,0x00,0x08,0x00,0x10,0x00,0x29,0x00,0x5B,0x00,0x9E,0x00,0xEC,0x01,0x30,0x01,0x18,0x00,0xD8,0x00,0x8C,0x00,0x4C,0x00,0x1E,0x00,0x07,0x00,0x00,0x00,0x08,0x00,0x22,0x00,0x54,0x00,0x99,0x00,0xE9,0x01,0x31,0x01,0x16,0x00,
0xDA,0x00,0x90,0x00,0x51,0x00,0x23,0x00,0x0C,0x00,0x05,0x00,0x0E,0x00,0x29,0x00,0x5B,0x00,0xA0,0x00,0xEF,0x01,0x32,0x01,0x24,0x00,0xED,0x00,0xA7,0x00,0x6A,0x00,0x3D,0x00,0x24,0x00,0x1D,0x00,0x27,0x00,0x43,0x00,0x76,0x00,0xB9,0x01,0x04,0x01,0x43,0x01,0x41,0x01,0x11,0x00,
0xD2,0x00,0x97,0x00,0x6C,0x00,0x53,0x00,0x4B,0x00,0x56,0x00,0x73,0x00,0xA4,0x00,0xE4,0x01,0x27,0x01,0x61,0x01,0x6D,0x01,0x39,0x01,0x05,0x00,0xD2,0x00,0xAA,0x00,0x93,0x00,0x8C,0x00,0x96,0x00,0xB1,0x00,0xDE,0x01,0x15,0x01,0x4D,0x01,0x89,0x01,0x9E,0x01,0x60,0x01,0x31,0x01,
0x0C,0x00,0xED,0x00,0xD9,0x00,0xD4,0x00,0xDC,0x00,0xF3,0x01,0x16,0x01,0x40,0x01,0x71,0x01,0xB6,0x01,0x67,0x01,0x26,0x01,0x03,0x00,0xE2,0x00,0xC5,0x00,0xB0,0x00,0xA6,0x00,0xA9,0x00,0xB8,0x00,0xD2,0x00,0xF0,0x01,0x19,0x01,0x5E,0x01,0x42,0x01,0x0C,0x00,0xE4,0x00,0xBA,0x00,
0x96,0x00,0x7F,0x00,0x75,0x00,0x7A,0x00,0x8D,0x00,0xAE,0x00,0xD6,0x01,0x03,0x01,0x3E,0x01,0x1B,0x00,0xE7,0x00,0xB8,0x00,0x88,0x00,0x62,0x00,0x49,0x00,0x40,0x00,0x46,0x00,0x5C,0x00,0x7F,0x00,0xAE,0x00,0xE1,0x01,0x17,0x00,0xFA,0x00,0xC5,0x00,0x90,0x00,0x5E,0x00,0x38,0x00,
0x20,0x00,0x18,0x00,0x1F,0x00,0x35,0x00,0x5B,0x00,0x8C,0x00,0xC5,0x00,0xFE,0x00,0xEB,0x00,0xB3,0x00,0x7A,0x00,0x47,0x00,0x21,0x00,0x0B,0x00,0x05,0x00,0x0C,0x00,0x22,0x00,0x49,0x00,0x7C,0x00,0xB8,0x00,0xF4,0x00,0xE6,0x00,0xAD,0x00,0x73,0x00,0x40,0x00,0x1A,0x00,0x05,0x00,
0x00,0x00,0x08,0x00,0x1E,0x00,0x45,0x00,0x79,0x00,0xB7,0x00,0xF5,0x00,0xE3,0x00,0xAE,0x00,0x76,0x00,0x44,0x00,0x1E,0x00,0x0A,0x00,0x05,0x00,0x0D,0x00,0x25,0x00,0x4C,0x00,0x80,0x00,0xBC,0x00,0xF8,0x00,0xF0,0x00,0xBD,0x00,0x89,0x00,0x59,0x00,0x34,0x00,0x1F,0x00,0x19,0x00,
0x23,0x00,0x3B,0x00,0x63,0x00,0x96,0x00,0xCE,0x01,0x05,0x01,0x0A,0x00,0xDA,0x00,0xAB,0x00,0x7E,0x00,0x5B,0x00,0x46,0x00,0x40,0x00,0x4A,0x00,0x62,0x00,0x88,0x00,0xB9,0x00,0xEB,0x01,0x1E,0x01,0x2E,0x00,0xF8,0x00,0xD1,0x00,0xA9,0x00,0x89,0x00,0x77,0x00,0x72,0x00,0x7C,0x00,
0x93,0x00,0xB6,0x00,0xDE,0x01,0x08,0x01,0x3F,0x01,0x5D,0x01,0x1D,0x00,0xF6,0x00,0xD7,0x00,0xBE,0x00,0xAF,0x00,0xAC,0x00,0xB5,0x00,0xCA,0x00,0xE5,0x01,0x04,0x01,0x29,0x01,0x68,0x01,0x5B,0x01,0x1A,0x00,0xF7,0x00,0xD9,0x00,0xC1,0x00,0xB2,0x00,0xAF,0x00,0xB9,0x00,0xCE,0x00,
0xED,0x01,0x0E,0x01,0x39,0x01,0x82,0x01,0x36,0x00,0xFF,0x00,0xD9,0x00,0xB2,0x00,0x92,0x00,0x80,0x00,0x7B,0x00,0x86,0x00,0x9F,0x00,0xC4,0x00,0xF0,0x01,0x1F,0x01,0x5A,0x01,0x14,0x00,0xE0,0x00,0xB2,0x00,0x84,0x00,0x62,0x00,0x4D,0x00,0x47,0x00,0x52,0x00,0x6B,0x00,0x92,0x00,
0xC4,0x00,0xF9,0x01,0x30,0x00,0xF7,0x00,0xC2,0x00,0x8E,0x00,0x5E,0x00,0x3A,0x00,0x24,0x00,0x1F,0x00,0x28,0x00,0x42,0x00,0x6A,0x00,0x9D,0x00,0xD7,0x01,0x10,0x00,0xE8,0x00,0xB0,0x00,0x78,0x00,0x47,0x00,0x22,0x00,0x0D,0x00,0x08,0x00,0x10,0x00,0x28,0x00,0x50,0x00,0x83,0x00,
0xBF,0x00,0xFB,0x00,0xE9,0x00,0xAE,0x00,0x75,0x00,0x42,0x00,0x1C,0x00,0x08,0x00,0x02,0x00,0x0A,0x00,0x21,0x00,0x48,0x00,0x7C,0x00,0xB8,0x00,0xF6,0x00,0xEF,0x00,0xB7,0x00,0x7F,0x00,0x4C,0x00,0x25,0x00,0x0F,0x00,0x08,0x00,0x10,0x00,0x27,0x00,0x4E,0x00,0x80,0x00,0xBA,0x00,
0xF5,0x01,0x06,0x00,0xD1,0x00,0x9B,0x00,0x69,0x00,0x41,0x00,0x29,0x00,0x20,0x00,0x27,0x00,0x3F,0x00,0x65,0x00,0x96,0x00,0xCC,0x01,0x04,0x01,0x25,0x00,0xF2,0x00,0xC2,0x00,0x92,0x00,0x6C,0x00,0x52,0x00,0x48,0x00,0x4E,0x00,0x64,0x00,0x87,0x00,0xB4,0x00,0xE3,0x01,0x18,0x01,
0x52,0x01,0x19,0x00,0xF0,0x00,0xC6,0x00,0xA3,0x00,0x8A,0x00,0x7F,0x00,0x84,0x00,0x96,0x00,0xB5,0x00,0xDA,0x01,0x02,0x01,0x3D,0x01,0x8F,0x01,0x4A,0x01,0x21,0x01,0x01,0x00,0xE4,0x00,0xCD,0x00,0xC2,0x00,0xC4,0x00,0xD3,0x00,0xEA,0x01,0x04,0x01,0x29,0x01,0x6C,0x01,0x26,0x00,
0xF5,0x00,0xD5,0x00,0xBD,0x00,0xA9,0x00,0x9D,0x00,0x9A,0x00,0xA2,0x00,0xB3,0x00,0xCE,0x00,0xEC,0x01,0x16,0x01,0x50,0x01,0x04,0x00,0xDE,0x00,0xBD,0x00,0x9D,0x00,0x83,0x00,0x74,0x00,0x71,0x00,0x78,0x00,0x8C,0x00,0xAC,0x00,0xD3,0x00,0xFF,0x01,0x2E,0x00,0xE0,0x00,0xBD,0x00,
0x95,0x00,0x71,0x00,0x55,0x00,0x45,0x00,0x41,0x00,0x48,0x00,0x5D,0x00,0x7E,0x00,0xA9,0x00,0xDA,0x01,0x05,0x00,0xC3,0x00,0x9F,0x00,0x73,0x00,0x4D,0x00,0x30,0x00,0x1F,0x00,0x1B,0x00,0x22,0x00,0x36,0x00,0x57,0x00,0x83,0x00,0xB8,0x00,0xE4,0x00,0xB2,0x00,0x8C,0x00,0x5D,0x00,
0x36,0x00,0x18,0x00,0x0A,0x00,0x05,0x00,0x0B,0x00,0x1D,0x00,0x3E,0x00,0x6A,0x00,0x9F,0x00,0xCE,0x00,0xB0,0x00,0x89,0x00,0x59,0x00,0x31,0x00,0x12,0x00,0x04,0x00,0x00,0x00,0x05,0x00,0x16,0x00,0x36,0x00,0x62,0x00,0x97,0x00,0xC7,0x00,0xB4,0x00,0x8F,0x00,0x61,0x00,0x38,0x00,
0x19,0x00,0x09,0x00,0x04,0x00,0x09,0x00,0x1B,0x00,0x3A,0x00,0x65,0x00,0x98,0x00,0xC7,0x00,0xC9,0x00,0xA6,0x00,0x79,0x00,0x52,0x00,0x33,0x00,0x21,0x00,0x1A,0x00,0x1F,0x00,0x30,0x00,0x4F,0x00,0x79,0x00,0xA9,0x00,0xD3,0x00,0xE6,0x00,0xC5,0x00,0x9D,0x00,0x77,0x00,0x59,0x00,
0x46,0x00,0x3E,0x00,0x42,0x00,0x52,0x00,0x6E,0x00,0x94,0x00,0xBF,0x00,0xE8,0x01,0x14,0x00,0xED,0x00,0xC9,0x00,0xA8,0x00,0x8B,0x00,0x7A,0x00,0x71,0x00,0x73,0x00,0x80,0x00,0x99,0x00,0xB8,0x00,0xDC,0x01,0x0B,0x01,0x4D,0x01,0x19,0x00,0xF6,0x00,0xDB,0x00,0xC4,0x00,0xB4,0x00,
0xAB,0x00,0xAB,0x00,0xB5,0x00,0xC6,0x00,0xDD,0x00,0xFF,0x01,0x32}},
{4,{0x00,0x2A,0x07,0x4E}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x0D,0x30}},
{6,{0x0F,0x12,0x02,0xA8,0x03,0x47}},
{4,{0x00,0x2A,0x06,0xB8}},
{16,{0x0F,0x12,0x00,0xDF,0x01,0x01,0x01,0x2A,0x01,0x3E,0x01,0x76,0x01,0xA6,0x01,0xB6}},
{4,{0x00,0x2A,0x06,0x64}},
{4,{0x0F,0x12,0x01,0x3E}},
{4,{0x00,0x2A,0x06,0xC6}},
{10,{0x0F,0x12,0x01,0x0B,0x01,0x03,0x00,0xFC,0x01,0x0C}},
{4,{0x00,0x2A,0x0C,0x48}},
{84,{0x0F,0x12,0x03,0x90,0x03,0xA6,0x03,0x3A,0x03,0xB2,0x02,0xFC,0x03,0xA4,0x02,0xCA,0x03,0x68,0x02,0x90,0x03,0x30,0x02,0x5A,0x03,0x02,0x02,0x3E,0x02,0xCE,0x02,0x22,0x02,0x88,0x02,0x0E,0x02,0x6A,0x01,0xF8,0x02,0x60,0x01,0xE6,0x02,0x46,0x01,0xD0,0x02,0x34,0x01,0xB6,0x02,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05}},
{4,{0x00,0x2A,0x0C,0x9C}},
{4,{0x0F,0x12,0x00,0x0C}},
{4,{0x00,0x2A,0x0C,0xA0}},
{4,{0x0F,0x12,0x01,0x38}},
{4,{0x00,0x2A,0x0C,0xE0}},
{52,{0x0F,0x12,0x03,0x82,0x03,0xEC,0x03,0x0A,0x03,0xE6,0x02,0xA0,0x03,0xDA,0x02,0x4A,0x03,0x9E,0x02,0x0C,0x03,0x44,0x01,0xDC,0x02,0xF4,0x01,0xAE,0x02,0xA4,0x01,0x7C,0x02,0x76,0x01,0x68,0x02,0x34,0x01,0x90,0x01,0xE6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06}},
{4,{0x00,0x2A,0x0D,0x18}},
{4,{0x0F,0x12,0x00,0xEE}},
{4,{0x00,0x2A,0x0C,0xA4}},
{52,{0x0F,0x12,0x02,0x1C,0x02,0x52,0x02,0x10,0x02,0x56,0x02,0x04,0x02,0x5C,0x01,0xF6,0x02,0x52,0x01,0xEC,0x02,0x48,0x01,0xE8,0x02,0x3E,0x01,0xE8,0x02,0x2C,0x01,0xF2,0x02,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04}},
{4,{0x00,0x2A,0x0C,0xD8}},
{4,{0x0F,0x12,0x00,0x08}},
{4,{0x00,0x2A,0x0C,0xDC}},
{4,{0x0F,0x12,0x02,0x3A}},
{4,{0x00,0x2A,0x0D,0x1C}},
{4,{0x0F,0x12,0x03,0x7C}},
{4,{0x00,0x2A,0x0D,0x20}},
{4,{0x0F,0x12,0x01,0x57}},
{4,{0x00,0x2A,0x0D,0x24}},
{4,{0x0F,0x12,0x3E,0xB8}},
{4,{0x00,0x2A,0x0D,0x2C}},
{6,{0x0F,0x12,0x01,0x4C,0x00,0xFB}},
{4,{0x00,0x2A,0x0D,0x46}},
{4,{0x0F,0x12,0x04,0xC0}},
{4,{0x00,0x2A,0x0D,0x28}},
{6,{0x0F,0x12,0x02,0x70,0x02,0x40}},
{4,{0x00,0x2A,0x0D,0x5C}},
{6,{0x0F,0x12,0x7F,0xFF,0x00,0x50}},
{4,{0x00,0x2A,0x23,0x16}},
{4,{0x0F,0x12,0x00,0x06}},
{4,{0x00,0x2A,0x0E,0x44}},
{8,{0x0F,0x12,0x05,0x25,0x04,0x00,0x07,0x8C}},
{4,{0x00,0x2A,0x0E,0x36}},
{8,{0x0F,0x12,0x00,0x28,0xFF,0xD8,0x00,0x00}},
{4,{0x00,0x2A,0x0D,0xD4}},
{100,{0x0F,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xF0,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xF0,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xF0,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x20,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x20,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x20,0xFF,0xC0,0x02,0xD9,0x03,0x57,0x03,0xD1,0x0D,0xF6,0x0E,0xD8,0x0F,0x51,0x0F,0x5C,0x0F,0x8F,0x10,0x06,0x00,0xAC,0x00,0xBD,0x00,0x49,0x00,0xF5}},
{4,{0x00,0x2A,0x0E,0x4A}},
{4,{0x0F,0x12,0x00,0x02}},
{4,{0x00,0x2A,0x05,0x1A}},
{6,{0x0F,0x12,0x01,0x0E,0x00,0xF5}},
{4,{0x00,0x2A,0x0F,0x76}},
{4,{0x0F,0x12,0x00,0x07}},
{4,{0x00,0x2A,0x10,0x34}},
{30,{0x0F,0x12,0x00,0xC0,0x00,0xE0,0x01,0x04,0x01,0x29,0x01,0x56,0x01,0x7F,0x01,0x8F,0x01,0x20,0x01,0x20,0x01,0x20,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00}},
{4,{0x00,0x2A,0x32,0x88}},
{242,{0x0F,0x12,0x00,0x00,0x00,0x08,0x00,0x13,0x00,0x2C,0x00,0x62,0x00,0xCD,0x01,0x29,0x01,0x51,0x01,0x74,0x01,0xAA,0x01,0xD7,0x01,0xFE,0x02,0x21,0x02,0x5D,0x02,0x91,0x02,0xEB,0x03,0x3A,0x03,0x80,0x03,0xC2,0x03,0xFF,0x00,0x00,0x00,0x08,0x00,0x13,0x00,0x2C,0x00,0x62,
0x00,0xCD,0x01,0x29,0x01,0x51,0x01,0x74,0x01,0xAA,0x01,0xD7,0x01,0xFE,0x02,0x21,0x02,0x5D,0x02,0x91,0x02,0xEB,0x03,0x3A,0x03,0x80,0x03,0xC2,0x03,0xFF,0x00,0x00,0x00,0x08,0x00,0x13,0x00,0x2C,0x00,0x62,0x00,0xCD,0x01,0x29,0x01,0x51,0x01,0x74,0x01,0xAA,0x01,0xD7,0x01,
0xFE,0x02,0x21,0x02,0x5D,0x02,0x91,0x02,0xEB,0x03,0x3A,0x03,0x80,0x03,0xC2,0x03,0xFF,0x00,0x00,0x00,0x08,0x00,0x13,0x00,0x2C,0x00,0x62,0x00,0xCD,0x01,0x29,0x01,0x51,0x01,0x74,0x01,0xAA,0x01,0xD7,0x01,0xFE,0x02,0x21,0x02,0x5D,0x02,0x91,0x02,0xEB,0x03,0x3A,0x03,0x80,
0x03,0xC2,0x03,0xFF,0x00,0x00,0x00,0x08,0x00,0x13,0x00,0x2C,0x00,0x62,0x00,0xCD,0x01,0x29,0x01,0x51,0x01,0x74,0x01,0xAA,0x01,0xD7,0x01,0xFE,0x02,0x21,0x02,0x5D,0x02,0x91,0x02,0xEB,0x03,0x3A,0x03,0x80,0x03,0xC2,0x03,0xFF,0x00,0x00,0x00,0x08,0x00,0x13,0x00,0x2C,0x00,
0x62,0x00,0xCD,0x01,0x29,0x01,0x51,0x01,0x74,0x01,0xAA,0x01,0xD7,0x01,0xFE,0x02,0x21,0x02,0x5D,0x02,0x91,0x02,0xEB,0x03,0x3A,0x03,0x80,0x03,0xC2,0x03,0xFF}},
{4,{0x00,0x2A,0x06,0xA6}},
{14,{0x0F,0x12,0x00,0xC0,0x00,0xE0,0x01,0x10,0x01,0x39,0x01,0x66,0x01,0x9F}},
{4,{0x00,0x2A,0x33,0xA4}},
{218,{0x0F,0x12,0x01,0x81,0xFF,0x88,0xFF,0x90,0xFE,0x6B,0x01,0x06,0xFF,0x0B,0xFF,0xDD,0xFF,0xEE,0x01,0xCB,0x01,0x87,0x00,0xA6,0xFE,0xBE,0x02,0x1C,0xFF,0x5F,0x01,0x75,0xFE,0xE7,0x01,0x06,0x00,0xF3,0x01,0x81,0xFF,0x88,0xFF,0x90,0xFE,0x6B,0x01,0x06,0xFF,0x0B,0xFF,0xDD,
0xFF,0xEE,0x01,0xCB,0x01,0x87,0x00,0xA6,0xFE,0xBE,0x02,0x1C,0xFF,0x5F,0x01,0x75,0xFE,0xE7,0x01,0x06,0x00,0xF3,0x01,0x81,0xFF,0x88,0xFF,0x90,0xFE,0x6B,0x01,0x06,0xFF,0x0B,0xFF,0xDD,0xFF,0xEE,0x01,0xCB,0x01,0x87,0x00,0xA6,0xFE,0xBE,0x02,0x1C,0xFF,0x5F,0x01,0x75,0xFE,
0xE7,0x01,0x06,0x00,0xF3,0x01,0xFD,0xFF,0xAB,0xFF,0xED,0xFE,0xB5,0x01,0x12,0xFE,0xDC,0xFF,0xD2,0x00,0x15,0x01,0xA1,0x01,0x11,0x00,0x9D,0xFE,0xCB,0x01,0xFC,0xFF,0x99,0x01,0xA9,0xFF,0x26,0x01,0x2B,0x00,0xDF,0x01,0xE3,0xFF,0xA4,0xFF,0xDB,0xFE,0xB5,0x01,0x12,0xFE,0xDC,
0xFF,0xD2,0xFF,0xFE,0x01,0xB7,0x00,0xE8,0x00,0x95,0xFF,0x0D,0x01,0x82,0xFF,0x29,0x01,0x46,0xFF,0x26,0x01,0x2B,0x00,0xDF,0x01,0xE3,0xFF,0xA4,0xFF,0xDB,0xFE,0xB5,0x01,0x12,0xFE,0xDC,0xFF,0xD2,0xFF,0xFE,0x01,0xB7,0x00,0xE8,0x00,0x95,0xFF,0x0D,0x01,0x82,0xFF,0x29,0x01,
0x46,0xFF,0x26,0x01,0x2B,0x00,0xDF}},
{4,{0x00,0x2A,0x33,0x80}},
{38,{0x0F,0x12,0x02,0x0E,0xFF,0x88,0xFF,0xCE,0xFE,0x3D,0x01,0x58,0xFF,0x03,0xFF,0x99,0x00,0x18,0x02,0x35,0x01,0x01,0x01,0x16,0xFF,0x00,0x01,0xC5,0xFF,0x80,0x01,0x9D,0xFE,0x7A,0x01,0x79,0x01,0x79}},
{4,{0x00,0x2A,0x07,0x64}},
{12,{0x0F,0x12,0x00,0x49,0x00,0x5F,0x00,0xCB,0x01,0xE0,0x02,0x20}},
{4,{0x00,0x2A,0x07,0xC4}},
{1054,{0x0F,0x12,0x00,0x14,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0xC1,0x03,0xFF,0x00,0x9C,0x02,0x51,0x03,0xFF,0x00,0x0C,0x00,0x10,0x01,0x2C,0x03,0xE8,0x00,0x46,0x00,0x5A,0x00,0x70,0x00,0x00,0x00,0x00,0x01,0xAA,0x00,0x3C,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x3E,
0x00,0x08,0x00,0x3C,0x00,0x1E,0x00,0x3C,0x00,0x1E,0x0A,0x24,0x17,0x01,0x02,0x29,0x14,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x04,0x5A,0x14,0x14,0x03,0x01,0xFF,0x07,0x08,0x1E,0x0A,0x14,0x0F,0x0F,0x0A,0x00,0x00,0x32,0x00,0x0E,0x00,0x02,0x00,0xFF,0x11,0x02,0x00,
0x1B,0x09,0x00,0x06,0x00,0x05,0x04,0x03,0x06,0x46,0x03,0x04,0x80,0x00,0x3C,0x00,0x80,0x01,0x01,0x07,0x07,0x46,0x01,0x49,0x44,0x50,0x44,0x05,0x00,0x00,0x03,0x54,0x00,0x07,0x14,0x32,0xFF,0x5A,0x04,0x20,0x1E,0x40,0x12,0x02,0x04,0x14,0x03,0x01,0x14,0x01,0x01,0x44,0x46,
0x64,0x6E,0x00,0x28,0x03,0x0A,0x00,0x00,0x14,0x1E,0xFF,0x07,0x04,0x32,0x00,0x00,0x0F,0x0F,0x04,0x40,0x03,0x02,0x14,0x14,0x01,0x01,0x46,0x01,0x6E,0x44,0x28,0x64,0x0A,0x00,0x00,0x03,0x1E,0x00,0x07,0x14,0x32,0xFF,0x00,0x04,0x0F,0x00,0x40,0x0F,0x02,0x04,0x00,0x03,0x00,
0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0xC1,0x03,0xFF,0x00,0x9C,0x02,0x51,0x03,0xFF,0x00,0x0C,0x00,0x10,0x01,0x2C,0x03,0xE8,0x00,0x46,0x00,0x5A,0x00,0x70,0x00,0x00,0x00,0x00,0x01,0xAE,0x00,0x1E,0x00,0x1E,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x08,0x00,0x3C,
0x00,0x1E,0x00,0x3C,0x00,0x1E,0x0A,0x24,0x17,0x01,0x02,0x29,0x14,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x04,0x5A,0x14,0x14,0x03,0x01,0xFF,0x07,0x08,0x1E,0x0A,0x14,0x0F,0x0F,0x0A,0x00,0x00,0x32,0x00,0x0E,0x00,0x02,0x00,0xFF,0x11,0x02,0x00,0x1B,0x09,0x00,0x06,
0x00,0x05,0x04,0x03,0x06,0x46,0x03,0x04,0x80,0x00,0x46,0x00,0x80,0x01,0x01,0x07,0x07,0x1E,0x01,0x49,0x1E,0x50,0x44,0x05,0x00,0x00,0x04,0x3C,0x0A,0x07,0x14,0x32,0x14,0x5A,0x03,0x12,0x1E,0x40,0x12,0x06,0x04,0x1E,0x06,0x01,0x1E,0x01,0x01,0x3A,0x3C,0x58,0x5A,0x00,0x28,
0x03,0x0A,0x00,0x00,0x14,0x1E,0xFF,0x07,0x04,0x32,0x00,0x00,0x0F,0x0F,0x04,0x40,0x03,0x02,0x1E,0x1E,0x01,0x01,0x3C,0x01,0x5A,0x3A,0x28,0x58,0x0A,0x00,0x00,0x03,0x1E,0x00,0x07,0x14,0x32,0xFF,0x00,0x04,0x0F,0x00,0x40,0x0F,0x02,0x04,0x00,0x03,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0xC1,0x03,0xFF,0x00,0x9C,0x02,0x51,0x03,0xFF,0x00,0x0C,0x00,0x10,0x01,0x2C,0x03,0xE8,0x00,0x46,0x00,0x5A,0x00,0x70,0x00,0x00,0x00,0x00,0x02,0x26,0x00,0x1E,0x00,0x1E,0x00,0x00,0x00,0x00,0x00,0x4E,0x00,0x00,0x00,0x3C,0x00,0x1E,0x00,0x3C,
0x00,0x1E,0x0A,0x24,0x17,0x01,0x02,0x29,0x14,0x03,0x00,0x00,0x00,0x00,0x09,0x06,0x00,0xFF,0x04,0x5A,0x14,0x14,0x03,0x01,0xFF,0x07,0x08,0x1E,0x0A,0x14,0x0F,0x0F,0x0A,0x00,0x00,0x90,0x00,0x0A,0x00,0x02,0x00,0xFF,0x11,0x02,0x00,0x1B,0x09,0x00,0x06,0x00,0x05,0x04,0x03,
0x06,0x46,0x02,0x08,0x80,0x00,0x80,0x00,0x80,0x01,0x01,0x07,0x07,0x1E,0x01,0x3C,0x1E,0x50,0x28,0x05,0x00,0x1A,0x04,0x28,0x0A,0x08,0x0C,0x14,0x14,0x6A,0x03,0x12,0x1E,0x40,0x12,0x06,0x04,0x28,0x06,0x01,0x28,0x01,0x01,0x22,0x24,0x32,0x36,0x00,0x28,0x03,0x0A,0x04,0x10,
0x14,0x1E,0xFF,0x07,0x04,0x32,0x40,0x50,0x0F,0x0F,0x04,0x40,0x03,0x02,0x28,0x28,0x01,0x01,0x24,0x01,0x36,0x22,0x28,0x32,0x0A,0x00,0x10,0x03,0x1E,0x04,0x07,0x14,0x32,0xFF,0x50,0x04,0x0F,0x40,0x40,0x0F,0x02,0x04,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0xC1,0x03,0xFF,0x00,0x9C,0x02,0x51,0x03,0xFF,0x00,0x0C,0x00,0x10,0x00,0xC8,0x03,0xE8,0x00,0x46,0x00,0x50,0x00,0x70,0x00,0x00,0x00,0x00,0x02,0x26,0x00,0x14,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x4E,0x00,0x00,0x00,0x2D,0x00,0x19,0x00,0x2D,0x00,0x19,0x0A,0x24,
0x17,0x01,0x02,0x29,0x14,0x03,0x00,0x00,0x00,0x00,0x09,0x06,0x00,0xFF,0x04,0x5A,0x14,0x14,0x03,0x01,0xFF,0x07,0x08,0x1E,0x0A,0x14,0x0F,0x0F,0x0A,0x00,0x00,0x90,0x00,0x0A,0x00,0x01,0x00,0xFF,0x10,0x02,0x00,0x1E,0x09,0x00,0x06,0x00,0x05,0x04,0x03,0x07,0x50,0x02,0x00,
0x80,0x00,0x80,0x00,0x80,0x01,0x01,0x07,0x07,0x1E,0x01,0x2A,0x1E,0x50,0x20,0x05,0x00,0x1A,0x04,0x28,0x0A,0x08,0x0C,0x14,0x14,0x6A,0x03,0x12,0x1E,0x40,0x12,0x06,0x04,0x3C,0x06,0x01,0x3C,0x01,0x01,0x1C,0x1E,0x1E,0x22,0x00,0x28,0x03,0x0A,0x02,0x14,0x0E,0x14,0xFF,0x06,
0x04,0x32,0x40,0x52,0x15,0x0C,0x04,0x40,0x03,0x02,0x3C,0x3C,0x01,0x01,0x1E,0x01,0x22,0x1C,0x28,0x1E,0x0A,0x00,0x14,0x03,0x14,0x02,0x06,0x0E,0x32,0xFF,0x52,0x04,0x0C,0x40,0x40,0x15,0x02,0x04,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC1,0x03,
0xFF,0x00,0x9C,0x02,0x51,0x03,0xFF,0x00,0x0C,0x00,0x10,0x00,0x32,0x02,0x8A,0x00,0x32,0x01,0xF4,0x00,0x70,0x00,0x00,0x00,0x00,0x01,0xAA,0x00,0x3C,0x00,0x50,0x00,0x00,0x00,0x00,0x00,0x44,0x00,0x14,0x00,0x46,0x00,0x19,0x00,0x46,0x00,0x19,0x0A,0x24,0x17,0x01,0x02,0x29,
0x05,0x03,0x08,0x0F,0x08,0x08,0x00,0x00,0x00,0xFF,0x01,0x2D,0x14,0x14,0x03,0x01,0xFF,0x07,0x06,0x1E,0x0A,0x1E,0x06,0x06,0x0A,0x01,0x37,0x8B,0x10,0x28,0x00,0x01,0x00,0xFF,0x10,0x02,0x00,0x1E,0x09,0x00,0x06,0x00,0x05,0x04,0x03,0x07,0x50,0x01,0x00,0x80,0x00,0x80,0x00,
0x80,0x50,0x50,0x01,0x01,0x32,0x01,0x18,0x32,0x21,0x0C,0x0A,0x00,0x1E,0x04,0x0A,0x08,0x07,0x0C,0x32,0x64,0x5A,0x02,0x10,0x40,0x40,0x12,0x06,0x04,0x46,0x06,0x01,0x46,0x01,0x01,0x1C,0x18,0x18,0x19,0x00,0x28,0x03,0x0A,0x05,0x14,0x0C,0x14,0xFF,0x05,0x04,0x32,0x40,0x52,
0x15,0x14,0x04,0x40,0x03,0x02,0x46,0x46,0x01,0x01,0x18,0x01,0x19,0x1C,0x28,0x18,0x0A,0x00,0x14,0x03,0x14,0x05,0x05,0x0C,0x32,0xFF,0x52,0x04,0x14,0x40,0x40,0x15,0x02,0x04,0x00,0x03,0x7D,0xFA,0xFF,0xBD,0x26,0xFE,0xF7,0xBC,0x7E,0x06,0x00,0xD3}},
{4,{0x00,0x2A,0x2C,0xE8}},
{10,{0x0F,0x12,0x00,0x07,0x00,0xE2,0x00,0x05,0x00,0xe2}},
{4,{0x00,0x2A,0x33,0x7A}},
{4,{0x0F,0x12,0x00,0x06}},
{4,{0x00,0x28,0x70,0x00}},
{4,{0x00,0x2A,0x01,0xCC}},
{6,{0x0F,0x12,0x5D,0xC0,0x00,0x00}},
{4,{0x00,0x2A,0x01,0xEE}},
{4,{0x0F,0x12,0x00,0x02}},
{4,{0x00,0x2A,0x01,0xF6}},
{14,{0x0F,0x12,0x27,0x10,0x2e,0xe0,0x2e,0xe0,0x2e,0xe0,0x2e,0xe0,0x2e,0xe0}},
{4,{0x00,0x2A,0x02,0x08}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x05,0x30}},
{10,{0x0F,0x12,0x5D,0xC0,0x00,0x00,0x6D,0x60,0x00,0x00}},
{4,{0x00,0x2A,0x16,0x7C}},
{10,{0x0F,0x12,0x9C,0x40,0x00,0x00,0x38,0x80,0x00,0x01}},
{4,{0x00,0x2A,0x05,0x38}},
{10,{0x0F,0x12,0x5D,0xC0,0x00,0x00,0x6D,0x60,0x00,0x00}},
{4,{0x00,0x2A,0x16,0x84}},
{10,{0x0F,0x12,0x9C,0x40,0x00,0x00,0x38,0x80,0x00,0x01}},
{4,{0x00,0x2A,0x05,0x40}},
{6,{0x0F,0x12,0x02,0x00,0x03,0x80}},
{4,{0x00,0x2A,0x16,0x8C}},
{6,{0x0F,0x12,0x04,0x20,0x07,0x00}},
{4,{0x00,0x2A,0x05,0x44}},
{6,{0x0F,0x12,0x01,0x00,0x10,0x00}},
{4,{0x00,0x2A,0x16,0x94}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x05,0x1A}},
{6,{0x0F,0x12,0x01,0x11,0x00,0xF0}},
{4,{0x00,0x2A,0x02,0x6C}},
{50,{0x0F,0x12,0x04,0x00,0x03,0x00,0x00,0x05,0x2e,0xe0,0x2e,0xe0,0x01,0x00,0x08,0x00,0x00,0x52,0x40,0x00,0x01,0xE0,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{4,{0x00,0x2A,0x02,0x3C}},
{4,{0x0F,0x12,0x00,0x00}},
{4,{0x00,0x2A,0x02,0x40}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x02,0x30}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x02,0x3E}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x2A,0x02,0x20}},
{6,{0x0F,0x12,0x00,0x01,0x00,0x01}},
{4,{0x00,0x2A,0x03,0x5C}},
{46,{0x0F,0x12,0x00,0x00,0x08,0x00,0x06,0x00,0x00,0x05,0x2e,0xe0,0x2e,0x90,0x01,0x00,0x08,0x00,0x00,0x52,0x00,0x50,0x01,0xE0,0x08,0xfc,0x00,0x01,0x00,0x02,0x00,0x00,0x05,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{4,{0x00,0x28,0xD0,0x00}},
{4,{0x00,0x2A,0x10,0x00}},
{4,{0x0F,0x12,0x00,0x01}},
{4,{0x00,0x28,0x70,0x00}},
{-1,{0x0}}
};


const u32 preview_tbl_1[][2] =
{

	// preview 
	{0x0028,0x7000},
	{0x002A,0x023C}, 
	{0x0F12,0x0000}, // #REG_TC_GP_ActivePrevConfig
	{0x002A,0x0240}, 
	{0x0F12,0x0001}, // #REG_TC_GP_PrevOpenAfterChange
	{0x002A,0x0230},	
	{0x0F12,0x0001},	// #REG_TC_GP_NewConfigSync 
	{0x002A,0x023E},
	{0x0F12,0x0001},	// #REG_TC_GP_PrevConfigChanged
	{0x002A,0x0220},
	{0x0F12,0x0001}, // #REG_TC_GP_EnablePreview // Start preview
	{0x0F12,0x0001},	// #REG_TC_GP_EnablePreviewChanged
		
};

const u32 capture_tbl_1[][2] =
{
	{0x0028,0x7000},
	{0x002A,0x0244}, //#REG_TC_GP_ActiveCapConfig
	{0x0F12,0x0000},
	{0x002A,0x0230}, //#REG_TC_GP_NewConfigSync
	{0x0F12,0x0001},
	{0x002A,0x0246},	//#REG_TC_GP_CapConfigChanged
	{0x0F12,0x0001},	
	{0x002A,0x0224},//#REG_TC_GP_EnableCapture
	{0x0F12,0x0001},	
	{0x002A,0x0226},//#REG_TC_GP_EnableCaptureChanged
	{0x0F12,0x0001},	
};


const u32 capture_tbl_2[][2] =
{

};
/*
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//saturation

const u32 s5k5cag_saturation_default_lv4_tbl[][2] =
{
	//@@ x1 (Default)              
	{0x5001,0xff}, //80 80  SDE_En       
	{0x5583,0x40},              
	{0x5584,0x40},              
	{0x5580,0x00},             
	{0x5588,0x41},                  
};

const u32 s5k5cag_saturation_lv0_tbl[][2] =
{
	//@@ x0.25              
	{0x5001,0xff},        
	{0x5583,0x00},              
	{0x5584,0x00},              
	{0x5580,0x02},              
	{0x5588,0x41},  

};

const u32 s5k5cag_saturation_lv1_tbl[][2] =
{
	//@@ x0.25              
	{0x5001,0xff},        
	{0x5583,0x10},              
	{0x5584,0x10},              
	{0x5580,0x02},               
	{0x5588,0x41}, 

};

const u32 s5k5cag_saturation_lv2_tbl[][2] =
{
	//@@ x0.25              
	{0x5001,0xff},       
	{0x5583,0x20},              
	{0x5584,0x20},              
	{0x5580,0x02},               
	{0x5588,0x41}, 

};

const u32 s5k5cag_saturation_lv3_tbl[][2] =
{
	//@@ x0.25              
	{0x5001,0xff},       
	{0x5583,0x30},              
	{0x5584,0x30},              
	{0x5580,0x02},             
	{0x5588,0x41},  

};

const u32 s5k5cag_saturation_lv5_tbl[][2] =
{
	//@@ x0.25              
	{0x5001,0xff},        
	{0x5583,0x50},              
	{0x5584,0x50},              
	{0x5580,0x02},          
	{0x5588,0x41}, 

};

const u32 s5k5cag_saturation_lv6_tbl[][2] =
{
	//@@ x0.25              
	{0x5001,0xff},       
	{0x5583,0x60},              
	{0x5584,0x60},              
	{0x5580,0x02},            
	{0x5588,0x41}, 

};

const u32 s5k5cag_saturation_lv7_tbl[][2] =
{
	//@@ x0.25              
	{0x5001,0xff},      
	{0x5583,0x70},              
	{0x5584,0x70},              
	{0x5580,0x02},              
	{0x5588,0x41}, 

};

const u32 s5k5cag_saturation_lv8_tbl[][2] =
{
	//@@ x0.25              
	{0x5001,0xff},      
	{0x5583,0x80},              
	{0x5584,0x80},              
	{0x5580,0x02},            
	{0x5588,0x41}, 

};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//sharpness

const u32 s5k5cag_sharpness_lv0_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                              
	{0x5502,0x00}, 

};

const u32 s5k5cag_sharpness_lv1_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                              
	{0x5502,0x02}, 

};

const u32 s5k5cag_sharpness_default_lv2_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                              
	{0x5502,0x04}, 

};

const u32 s5k5cag_sharpness_lv3_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                            
	{0x5502,0x08}, 

};

const u32 s5k5cag_sharpness_lv4_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                             
	{0x5502,0x0c}, 

};

const u32 s5k5cag_sharpness_lv5_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                           
	{0x5502,0x10}, 

};

const u32 s5k5cag_sharpness_lv6_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                              
	{0x5502,0x14}, 

};

const u32 s5k5cag_sharpness_lv7_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                              
	{0x5502,0x18}, 

};

const u32 s5k5cag_sharpness_lv8_tbl[][2] =
{
	//@@ @@ Sharpness OFF                             
	{0x5308,0x65},                             
	{0x5502,0x20}, 

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//Contrast

const u32 s5k5cag_contrast_lv0_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                         
	{0x5580,0x04}, 
	{0x5586,0x10},                           
	{0x5585,0x10},                         
	{0x5588,0x41},  
};

const u32 s5k5cag_contrast_lv1_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                        
	{0x5580,0x04}, 
	{0x5586,0x14},                              
	{0x5585,0x14},                         
	{0x5588,0x41},    
};

const u32 s5k5cag_contrast_lv2_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                              
	{0x5580,0x04}, 
	{0x5586,0x18},                             
	{0x5585,0x18},                          
	{0x5588,0x41},   
};

const u32 s5k5cag_contrast_lv3_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                             
	{0x5580,0x04}, 
	{0x5586,0x1c},                              
	{0x5585,0x1c},                           
	{0x5588,0x41},  
};

const u32 s5k5cag_contrast_default_lv4_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                              
	{0x5580,0x04}, 
	{0x5586,0x20},                              
	{0x5585,0x20},                            
	{0x5588,0x41},    
};

const u32 s5k5cag_contrast_lv5_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                              
	{0x5580,0x04}, 
	{0x5586,0x24},                            
	{0x5585,0x24},                            
	{0x5588,0x41},    
};

const u32 s5k5cag_contrast_lv6_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                           
	{0x5580,0x04}, 
	{0x5586,0x28},                            
	{0x5585,0x28},                            
	{0x5588,0x41},   
};

const u32 s5k5cag_contrast_lv7_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                             
	{0x5580,0x04}, 
	{0x5586,0x2c},                            
	{0x5585,0x2c},                            
	{0x5588,0x41},   
};

const u32 s5k5cag_contrast_lv8_tbl[][2] =
{
	//@@ Contrast +3                             
	{0x5001,0xff},                           
	{0x5580,0x04}, 
	{0x5586,0x30},                            
	{0x5585,0x30},                            
	{0x5588,0x41},   
};


*/





#endif /* CAMSENSOR_S5K5CAG */
