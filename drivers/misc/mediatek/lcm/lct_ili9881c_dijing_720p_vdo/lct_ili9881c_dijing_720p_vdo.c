/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include "lcm_drv.h"
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */

#define FRAME_WIDTH									(720)
#define FRAME_HEIGHT								(1280)

#define REGFLAG_DELAY								 0xFEF
#define REGFLAG_END_OF_TABLE						 0xFFF	/* END OF REGISTERS MARKER */

#define LCM_DSI_CMD_MODE							  0
#define LCT_LCM_MAPP_BACKLIGHT						  1
/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)	 lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)					 lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)		 lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg					 lcm_util.dsi_read_reg()

#ifdef CONFIG_LCT_LCM_GPIO_UTIL
	#define set_gpio_lcd_enp(cmd) lcm_util.set_gpio_lcd_enp_bias(cmd)
	#define set_gpio_lcd_enn(cmd) lcm_util.set_gpio_lcd_enn_bias(cmd)
	#define set_gpio_led_en(cmd) lcm_util.set_gpio_led_en_bias(cmd)
#else
	#define set_gpio_lcd_enp(cmd) 
	#define set_gpio_lcd_enn(cmd) 
#endif
#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define I2C_ID_NAME "I2C_LCD_BIAS"
#define TPS_ADDR 0x3E
#define TPS_I2C_BUSNUM 2
/***************************************************************************** 
* GLobal Variable
*****************************************************************************/
static struct i2c_board_info __initdata tps65132_board_info = {I2C_BOARD_INFO(I2C_ID_NAME, TPS_ADDR)};
static struct i2c_client *tps65132_i2c_client = NULL;


/***************************************************************************** 
* Function Prototype
*****************************************************************************/ 
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tps65132_remove(struct i2c_client *client);
/***************************************************************************** 
* Data Structure
*****************************************************************************/

struct tps65132_dev	{	
struct i2c_client	*client;

};

static const struct i2c_device_id tps65132_id[] = {
{ I2C_ID_NAME, 0 },
{ }
};


static struct i2c_driver tps65132_iic_driver = {
    .id_table	= tps65132_id,
    .probe		= tps65132_probe,
    .remove		= tps65132_remove,
    .driver		= {
    .owner	= THIS_MODULE,
    .name	= "tps65132",
},
};
/***************************************************************************** 
* Extern Area
*****************************************************************************/ 

/***************************************************************************** 
* Function
*****************************************************************************/ 
static int tps65132_probe(struct i2c_client *client, const struct i2c_device_id *id)
{  

    tps65132_i2c_client  = client;		
    return 0;      
}


static int tps65132_remove(struct i2c_client *client)
{  	
    tps65132_i2c_client = NULL;
    i2c_unregister_device(client);
    return 0;
}


static int tps65132_write_bytes(unsigned char addr, unsigned char value)
{	
	int ret = 0;
    char write_data[2];	
	struct i2c_client *client = tps65132_i2c_client;
	if(client == NULL)
	{
		return 0;
	}
	
    
    write_data[0]= addr;
    write_data[1] = value;
    ret=i2c_master_send(client, write_data, 2);
    return ret ;
}
//EXPORT_SYMBOL_GPL(tps65132_write_bytes);


static int __init tps65132_iic_init(void)
{
	int ret = 0;
	ret = i2c_register_board_info(TPS_I2C_BUSNUM, &tps65132_board_info, 1);
	i2c_add_driver(&tps65132_iic_driver);
	return 0;
}

static void __exit tps65132_iic_exit(void)
{
    i2c_del_driver(&tps65132_iic_driver);  
}

module_init(tps65132_iic_init);
module_exit(tps65132_iic_exit);

MODULE_AUTHOR("Xiaokuan Shi");
MODULE_DESCRIPTION("MTK TPS65132 I2C Driver");
MODULE_LICENSE("GPL"); 


struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {

{0xFF,3,{0x98,0x81,0x03}},
{0x01,1,{0x00}},
{0x02,1,{0x00}},
{0x03,1,{0x53}},
{0x04,1,{0x53}},
{0x05,1,{0x13}},
{0x06,1,{0x04}},
{0x07,1,{0x02}},
{0x08,1,{0x02}},
{0x09,1,{0x00}},
{0x0a,1,{0x00}},
{0x0b,1,{0x00}},
{0x0c,1,{0x00}},
{0x0d,1,{0x00}},
{0x0e,1,{0x00}},
{0x0f,1,{0x00}},
{0x10,1,{0x00}},
{0x11,1,{0x00}},
{0x12,1,{0x00}},
{0x13,1,{0x00}},
{0x14,1,{0x00}},
{0x15,1,{0x00}},
{0x16,1,{0x00}},
{0x17,1,{0x00}},
{0x18,1,{0x00}},
{0x19,1,{0x00}},
{0x1a,1,{0x00}},
{0x1b,1,{0x00}},
{0x1c,1,{0x00}},
{0x1d,1,{0x00}},
{0x1e,1,{0xC0}},
{0x1f,1,{0x80}},
{0x20,1,{0x02}},
{0x21,1,{0x09}},
{0x22,1,{0x00}},
{0x23,1,{0x00}},
{0x24,1,{0x00}},
{0x25,1,{0x00}},
{0x26,1,{0x00}},
{0x27,1,{0x00}},
{0x28,1,{0x55}},
{0x29,1,{0x03}},
{0x2a,1,{0x00}},
{0x2b,1,{0x00}},
{0x2c,1,{0x00}},
{0x2d,1,{0x00}},
{0x2e,1,{0x00}},
{0x2f,1,{0x00}},
{0x30,1,{0x00}},
{0x31,1,{0x00}},
{0x32,1,{0x00}},
{0x33,1,{0x00}},
{0x34,1,{0x03}},
{0x35,1,{0x00}},
{0x36,1,{0x05}},
{0x37,1,{0x00}},
{0x38,1,{0x3C}},
{0x39,1,{0x00}},
{0x3a,1,{0x00}},
{0x3b,1,{0x00}},
{0x3c,1,{0x00}},
{0x3d,1,{0x00}},
{0x3e,1,{0x00}},
{0x3f,1,{0x00}},
{0x40,1,{0x00}},
{0x41,1,{0x00}},
{0x42,1,{0x00}},
{0x43,1,{0x00}},
{0x44,1,{0x00}},
{0x50,1,{0x01}},
{0x51,1,{0x23}},
{0x52,1,{0x45}},
{0x53,1,{0x67}},
{0x54,1,{0x89}},
{0x55,1,{0xab}},
{0x56,1,{0x01}},
{0x57,1,{0x23}},
{0x58,1,{0x45}},
{0x59,1,{0x67}},
{0x5a,1,{0x89}},
{0x5b,1,{0xab}},
{0x5c,1,{0xcd}},
{0x5d,1,{0xef}},
{0x5e,1,{0x01}},
{0x5f,1,{0x14}},
{0x60,1,{0x15}},
{0x61,1,{0x0C}},
{0x62,1,{0x0D}},
{0x63,1,{0x0E}},
{0x64,1,{0x0F}},
{0x65,1,{0x10}},
{0x66,1,{0x11}},
{0x67,1,{0x08}},
{0x68,1,{0x02}},
{0x69,1,{0x0A}},
{0x6a,1,{0x02}},
{0x6b,1,{0x02}},
{0x6c,1,{0x02}},
{0x6d,1,{0x02}},
{0x6e,1,{0x02}},
{0x6f,1,{0x02}},
{0x70,1,{0x02}},
{0x71,1,{0x02}},
{0x72,1,{0x06}},
{0x73,1,{0x02}},
{0x74,1,{0x02}},
{0x75,1,{0x14}},
{0x76,1,{0x15}},
{0x77,1,{0x0f}},
{0x78,1,{0x0e}},
{0x79,1,{0x0d}},
{0x7a,1,{0x0c}},
{0x7b,1,{0x11}},
{0x7c,1,{0x10}},
{0x7d,1,{0x06}},
{0x7e,1,{0x02}},
{0x7f,1,{0x0A}},
{0x80,1,{0x02}},
{0x81,1,{0x02}},
{0x82,1,{0x02}},
{0x83,1,{0x02}},
{0x84,1,{0x02}},
{0x85,1,{0x02}},
{0x86,1,{0x02}},
{0x87,1,{0x02}},
{0x88,1,{0x08}},
{0x89,1,{0x02}},
{0x8A,1,{0x02}},
{0xFF,3,{0x98,0x81,0x04}},
{0x00,1,{0x00}}, 
{0x6C,1,{0x15}},                //Set VCORE voltage =1.5V);
{0x6E,1,{0x3b}},              //di_pwr_reg=0 for power mode 
{0x6F,1,{0x53}},                // reg vcl + pumping ratio 
{0x3A,1,{0xA4}},                //POWER SAVING);
{0x8D,1,{0x15}},              //VGL clamp -10
{0x87,1,{0xBA}},               //ESD               );
{0x26,1,{0x76}},            
{0xB2,1,{0xD1}},
{0x88,1,{0x0B}},
{0xFF,3,{0x98,0x81,0x02}},
{0x06,1,{0x40}}, 
{0x07,1,{0x04}}, 
{0xFF,3,{0x98,0x81,0x01}},
{0x22,1,{0x0A}},               //BGR,0x SS);
{0x31,1,{0x00}},               //column inversion);
{0x53,1,{0x72}},               //VCOM1);
{0x50,1,{0xC0}},               // VREG1OUT=4.7V);
{0x51,1,{0xC0}},               // VREG2OUT=-4.7V);
{0x60,1,{0x14}},               //SDT);
{0xA0,1,{0x08}},               //VP255 Gamma P);
{0xA1,1,{0x1E}},               //VP251);
{0xA2,1,{0x2C}},               //VP247);
{0xA3,1,{0x15}},               //VP243);
{0xA4,1,{0x18}},               //VP239);
{0xA5,1,{0x2A}},               //VP231);
{0xA6,1,{0x1F}},               //VP219);
{0xA7,1,{0x1F}},               //VP203);
{0xA8,1,{0x85}},               //VP175);
{0xA9,1,{0x1C}},               //VP144);
{0xAA,1,{0x2A}},               //VP111);
{0xAB,1,{0x72}},              //VP80);
{0xAC,1,{0x1A}},              //VP52);
{0xAD,1,{0x18}},              //VP36);
{0xAE,1,{0x4C}},              //VP24);
{0xAF,1,{0x20}},              //VP16);
{0xB0,1,{0x26}},               //VP12);
{0xB1,1,{0x4A}},               //VP8);
{0xB2,1,{0x57}},              //VP4);
{0xB3,1,{0x2C}},              //VP0);
{0xC0,1,{0x08}},              //VN255 GAMMA N);
{0xC1,1,{0x1B}},              //VN251);
{0xC2,1,{0x27}},              //VN247);
{0xC3,1,{0x12}},              //VN243);
{0xC4,1,{0x14}},              //VN239);
{0xC5,1,{0x25}},              //VN231);
{0xC6,1,{0x1A}},              //VN219);
{0xC7,1,{0x1D}},              //VN203);
{0xC8,1,{0x7A}},              //VN175); 
{0xC9,1,{0x1A}},               //VN144);
{0xCA,1,{0x28}},               //VN111);
{0xCB,1,{0x6B}},               //VN80);
{0xCC,1,{0x1F}},               //VN52);
{0xCD,1,{0x1D}},               //VN36);
{0xCE,1,{0x52}},               //VN24);
{0xCF,1,{0x24}},               //VN16);
{0xD0,1,{0x2D}},               //VN12);
{0xD1,1,{0x47}},               //VN8);
{0xD2,1,{0x55}},               //VN4);
{0xD3,1,{0x2C}},
{0xFF,3,{0x98,0x81,0x00}},
{0x68,1,{0x05}},
{0x11,0,{}},  
{REGFLAG_DELAY, 120, {}},            
{0x29,0,{}},       
{REGFLAG_DELAY, 10, {}},     
{0x35,1,{0x00}},   // te on
	{0x51,2,{0x0f,0xff}},
	{0x53,1,{0x2C}},
	{0x55,1,{0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	/* Sleep Mode On */
	{ 0x28, 0, {} },
	{ REGFLAG_DELAY, 10, {} },
	{ 0x10, 0, {} },
	{ REGFLAG_DELAY, 120, {} },
	{ REGFLAG_END_OF_TABLE, 0x00, {} }
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{ 0x51, 2, {0x0f,0xFF} },
	{ REGFLAG_END_OF_TABLE, 0x00, {} }
};


static void push_table(struct LCM_setting_table *table, unsigned int count,
		       unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {

		unsigned cmd;

		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}

}


/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
#else
	params->dsi.mode = BURST_VDO_MODE;
	//params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
#endif

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_THREE_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	/* Not support in MT6573 */
	params->dsi.packet_size = 512;
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 6;//15
	params->dsi.vertical_backporch					= 16;//20
	params->dsi.vertical_frontporch					= 16; //20
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 8; //20
	params->dsi.horizontal_backporch				= 80;  //140
	params->dsi.horizontal_frontporch				= 80;  //100
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

#if (LCM_DSI_CMD_MODE)
	params->dsi.PLL_CLOCK = 150;	/* this value must be in MTK suggested table */
#else
	params->dsi.PLL_CLOCK = 275;	/* this value must be in MTK suggested table */
#endif
	params->dsi.cont_clock = 0;
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd = 0x53;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x24;
}

static void tps65132_enable(char en)
{
	if (en)
	{
		set_gpio_led_en(1);
		MDELAY(5);
		set_gpio_lcd_enp(1);
		MDELAY(12);
		set_gpio_lcd_enn(1);
		MDELAY(12);
		tps65132_write_bytes(0x00, 0x0f);
		tps65132_write_bytes(0x01, 0x0f);
	}
	else
	{
		tps65132_write_bytes(0x03, 0x40);
		set_gpio_led_en(0);
		MDELAY(5);
		set_gpio_lcd_enp(0);
		MDELAY(12);
		set_gpio_lcd_enn(0);
		MDELAY(12);

	}	
}

static void lcm_init(void)
{
	tps65132_enable(1);
	SET_RESET_PIN(1);
	MDELAY(20);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);
	push_table(lcm_initialization_setting,
		   sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting,
		   sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	tps65132_enable(0);
}


static void lcm_resume(void)
{
	MDELAY(10);
	lcm_init();
	MDELAY(10);
}


static void lcm_setbacklight(unsigned int level)
{
	
    #if(LCT_LCM_MAPP_BACKLIGHT)
	unsigned int mapped_level = 0;
	if (level > 255)
		level = 200;
	if (level > 102)
		mapped_level = (641*level + 36667)/(1000);
	else
		mapped_level=level;
	#endif
	lcm_backlight_level_setting[0].para_list[1] = mapped_level;
	push_table(lcm_backlight_level_setting,
		   sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}


LCM_DRIVER lct_ili9881c_dijing_720p_vdo_lcm_drv = {

	.name = "lct_ili9881c_dijing_720p_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.set_backlight = lcm_setbacklight,

};