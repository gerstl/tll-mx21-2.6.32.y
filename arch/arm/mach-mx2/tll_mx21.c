/*
 * arch/arm/mach-imx21/tll_mx21.c
 *
 * Port to 2.6.32, 2010
 * Brian Dodge <bdodge09@comcast.net>
 *
 * Modified Sept 24, 2006
 * Tarun Tuli <tarun@virtualcogs.com>
 *
 * Originally based on:
 *	linux-2.6.7-imx/arch/arm/ mach-imx /scb9328.c
 *	Copyright (c) 2004 Sascha Hauer <sascha@saschahauer.de>
 * 	2004 (c) MontaVista Software, Inc.
 *
 * 	csb535fs.c
 * 	Ron Melvin (ron.melvin@timesys.com)
 * 	Copyright (c) 2005 TimeSys Corporation 
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
 * ttcl-vcmx
 */

#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/smsc911x.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>

#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/i2c.h>
#include <mach/irqs.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <mach/imx-uart.h>
#include <mach/mxc_nand.h>
#include <mach/spi.h>

#include <mach/tll_mx21.h>

#include "devices.h"

#if defined (CONFIG_I2C_IMX)
#include <linux/i2c.h>
#endif


#if defined (CONFIG_SPI_IMX)
#include <linux/spi/spi.h>
#endif



#if defined (CONFIG_USB_IMX21_HCD)
#include <asm/arch/imx21-hcd.h>
#define OTG_TXCVR_VENDOR_ID_REG0                0x00
#define OTG_TXCVR_VENDOR_ID_REG1                0x01
#define OTG_TXCVR_PRODUCT_ID_REG0               0x02
#define OTG_TXCVR_PRODUCT_ID_REG1               0x03
#define OTG_TXCVR_MODE_REG1_SET                 0x04
#define OTG_TXCVR_MODE_REG1_CLR                 0x05
#define OTG_TXCVR_CTRL_REG1_SET                 0x06
#define OTG_TXCVR_CTRL_REG1_CLR                 0x07
#define OTG_TXCVR_INT_SRC_REG                   0x08
#define OTG_TXCVR_INT_LAT_REG_SET               0x0a
#define OTG_TXCVR_INT_LAT_REG_CLR               0x0b
#define OTG_TXCVR_INT_FALSE_REG_SET             0x0c
#define OTG_TXCVR_INT_FALSE_REG_CLR             0x0d
#define OTG_TXCVR_INT_TRUE_REG_SET              0x0e
#define OTG_TXCVR_INT_TRUE_REG_CLR              0x0f
#define OTG_TXCVR_CTRL_REG2_SET                 0x10
//#define OTG_TXCVR_CTRL_REG2_CLR                 0x11
#define OTG_TXCVR_MODE_REG2_SET                 0x12
#define OTG_TXCVR_MODE_REG2_CLR                 0x13
#define OTG_TXCVR_BCD_DEV_REG0                  0x14
#define OTG_TXCVR_BCD_DEV_REG1                  0x15

#define OTG_TXCVR_DEV_WRITE_ADDR 0x2D
#define OTG_TXCVR_DEV_READ_ADDR (0x2D | (1<<7))

#define ISP1301_ID_GROUNDED  (1<<3)
#endif

static unsigned int tll_mx21_pins[] = {
#if defined(CONFIG_USB_IMX21_HCD)
	(11 | GPIO_PORTC | GPIO_PF | GPIO_OUT ),   // USBG_TXDP (G13)
	(20 | GPIO_PORTC | GPIO_PF | GPIO_OUT ),   // USBG_TXDM (B13)
	(13 | GPIO_PORTC | GPIO_PF | GPIO_IN ),    // USBG_RXDP (D14)
	(12 | GPIO_PORTC | GPIO_PF | GPIO_IN ),    // USBG_RXDM (C13)
	(8  | GPIO_PORTC | GPIO_PF | GPIO_OUT ),   // USBG FS  (A13)
	(9  | GPIO_PORTC | GPIO_PF | GPIO_OUT ),   // USBG_OE (H13)
	(13 | GPIO_PORTC | GPIO_PF | GPIO_OUT ),   // USBG_ON (D13)
#endif

#if defined (CONFIG_I2C_IMX)
	(18 | GPIO_PORTD | GPIO_PF ), //I2C_CLK (C18)
	(17 | GPIO_PORTD | GPIO_PF ), //I2C_DATA (B18)
#endif

#if defined (CONFIG_SPI_IMX)
	(24 | GPIO_PORTD | GPIO_PF ), //CSPI2_MOSI (E19)
	(23 | GPIO_PORTD | GPIO_PF ), //CSPI2_MISO (E17)
	(22 | GPIO_PORTD | GPIO_PF ), //CSPI2_SCLK (D19)
//	(21 | GPIO_PORTD | GPIO_PF ), //CSPI2_SSO (D18)
//	(20 | GPIO_PORTD | GPIO_PF ), //CSPI2_SS1 (C19)
//	(19 | GPIO_PORTD | GPIO_PF ), //CSPI2_SS2 (B19)
#endif

#if defined (CONFIG_FB_IMX)
	(5 | GPIO_PORTA | GPIO_PF), //LSCLK (F4)
	(6 | GPIO_PORTA | GPIO_PF), //LD0 (D2)
	(7 | GPIO_PORTA | GPIO_PF), //LD1 (C1)
	(8 | GPIO_PORTA | GPIO_PF), //LD2 (D1)
	(9 | GPIO_PORTA | GPIO_PF), //LD3 (C2)
	(10 | GPIO_PORTA | GPIO_PF), //LD4 (E2)
	(11 | GPIO_PORTA | GPIO_PF), //LD5 (B2)
	(12 | GPIO_PORTA | GPIO_PF), //LD6 (C3)
	(13 | GPIO_PORTA | GPIO_PF), //LD7 (B1)
	(14 | GPIO_PORTA | GPIO_PF), //LD8 (E1)
	(15 | GPIO_PORTA | GPIO_PF), //LD9 (A1)
	(16 | GPIO_PORTA | GPIO_PF), //LD10 (C4)
	(17 | GPIO_PORTA | GPIO_PF), //LD11 (B3)
	(18 | GPIO_PORTA | GPIO_PF), //LD12 (A2)
	(19 | GPIO_PORTA | GPIO_PF), //LD13 (D3)
	(20 | GPIO_PORTA | GPIO_PF), //LD14 (A3)
	(21 | GPIO_PORTA | GPIO_PF), //LD15 (E3)
	(22 | GPIO_PORTA | GPIO_PF), //LD16 (B4)
	(23 | GPIO_PORTA | GPIO_PF), //LD17 (C5)
	(24 | GPIO_PORTA | GPIO_PF), //REV (A4)
	(25 | GPIO_PORTA | GPIO_PF), //CLS (D4)
	(26 | GPIO_PORTA | GPIO_PF), //PS (B5)
	(27 | GPIO_PORTA | GPIO_PF), //SPL_SPR (E4)
	(28 | GPIO_PORTA | GPIO_PF), //HSYNC (A5)
	(29 | GPIO_PORTA | GPIO_PF), //VSYNC (C6)
	(30 | GPIO_PORTA | GPIO_PF), //CONTRAST (B6)
	(31 | GPIO_PORTA | GPIO_PF), //OE_ACD (A6)
//	(29 | GPIO_OUT | GPIO_PORTA | GPIO_GPIO | GPIO_PUEN), // LCD backlight enable
#endif
#if defined (CONFIG_SMSC911X)
	(0 | GPIO_PORTF | GPIO_GPIO | GPIO_IN),		// etherernet interrupt
#endif

};

#if defined (CONFIG_FB_IMX)
#include <linux/fb.h>
#include <mach/imxfb.h>



static struct imx_fb_videomode vcmx212_fb_modes[] = {
	{
		.mode = {
			.name		= "default",
			.pixclock	= 65536,
			.xres		= 240,
			.yres		= 320,
			.hsync_len	= 0x1e,
			.vsync_len	= 0x3,  
			.left_margin	= 0xa,
			.upper_margin	= 0x6,
			.right_margin	= 0xe,
			.lower_margin	= 0x5,
		},
		//.pcr	 	= 0xcac80083,
		.pcr		= 0xcae80084, //aditya
		.bpp		= 16,
	},
};

static int tllmx21_fb_init(struct platform_device *pdev)
{
	/* todo - power on LCD, and backlight?
	*/
	return 0;
}

static void tllmx21_fb_exit(struct platform_device *pdev)
{
}

static void tllmx21_lcd_power(int on)
{
	volatile u32 tmp;
	
    if(on) {
        printk("%s:Backlight On\n", __FUNCTION__);
		tmp = __raw_readl(VA_GPIO_BASE + MXC_DR(0));
		tmp |= (1 << 29);
		__raw_writel(tmp, VA_GPIO_BASE + MXC_DR(0));
   } else {
      //  printk("%s:Backlight Off XXXXXXXXX\n", __FUNCTION__);
	   /* commented out in 2.6.16 version, why? 
		tmp = __raw_readl(VA_GPIO_BASE + MXC_DR(0));
		tmp &= ~(1 << 29);
		__raw_writel(tmp, VA_GPIO_BASE + MXC_DR(0));
		*/
    }
}

static struct imx_fb_platform_data vcmx212_fb_info = {

	.mode = vcmx212_fb_modes,
	.num_modes = ARRAY_SIZE(vcmx212_fb_modes),

	.pwmr		= 0,
	.lscr1		= 0x400c0373, 
	.dmacr		= 0x00020008, //can be 40008

	.init = tllmx21_fb_init,
	.exit = tllmx21_fb_exit,
	.lcd_power	= tllmx21_lcd_power,
};

#endif /* defined(CONFIG_FB_IMX) */

#if defined (CONFIG_I2C_IMX)

static struct imxi2c_platform_data mx21_i2c_data = {
	.bitrate = 100000,
};

static struct i2c_board_info mx21_i2c_devices[] = {
};

#endif


#if defined (CONFIG_SPI_IMX)
static struct spi_board_info mx21_spi_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.max_speed_hz = 1000000,
		.bus_num = 1,
		.chip_select = 0,
	},
};

static int mx21_spi_cs[] = {GPIO_PORTD + 21, GPIO_PORTD + 20 , GPIO_PORTD + 19};

static struct spi_imx_master mx21_spi_1_data = {
	.chipselect = mx21_spi_cs,
	.num_chipselect = ARRAY_SIZE(mx21_spi_cs),

};
#endif


#ifdef CONFIG_SMSC911X
static struct resource smsc911x_resources[] = {
	[0] = {
		.start	= TLL_MX21_ETH_PHYS,
		.end	= (TLL_MX21_ETH_PHYS + TLL_MX21_ETH_SIZE)-1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= TLL_MX21_ETH_IRQ,
		.end	= TLL_MX21_ETH_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct smsc911x_platform_config tllmx21_smsc911x_config = {
	.irq_polarity	= SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
	.irq_type	= SMSC911X_IRQ_TYPE_OPEN_DRAIN,
	.flags		= SMSC911X_USE_32BIT,
	.phy_interface	= PHY_INTERFACE_MODE_MII,
};

static struct platform_device smsc911x_device = {
	.name		= "smsc911x",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(smsc911x_resources),
	.resource	= smsc911x_resources,
	.dev		= {
		.platform_data = &tllmx21_smsc911x_config,
	},
};
#endif //SMSC911X

#if defined(CONFIG_USB_IMX21_HCD)
static struct imx21_usb_platform_data tllmx21_usb_pdata = {
	.set_mode     = NULL,
	.set_speed    = NULL,
	.set_suspend  = NULL,
	.set_oe       = NULL,
};

static u64 imx21_usb_dmamask = 0xffffffffUL;
static struct platform_device imx_usb_hcd_device = {
	.name = "imx21-hc",
	.id   = 0,
	.dev  = {
		.platform_data = &tllmx21_usb_pdata,
		.dma_mask = &imx21_usb_dmamask,
		.coherent_dma_mask = 0xffffffff,
	}
};
#endif

static struct platform_device *devices[] __initdata = {
#if defined(CONFIG_SMSC911X)
	&smsc911x_device,
#endif
#if defined(CONFIG_USB_IMX21_HCD)
	&imx_usb_hcd_device,
#endif
};

static struct imxuart_platform_data uart_pdata[] = {
	{
		.flags = IMXUART_HAVE_RTSCTS,
	}, {
		.flags = IMXUART_HAVE_RTSCTS,
	}, {
		.flags = IMXUART_HAVE_RTSCTS,
	},
};

static void __init
tll_mx21_init(void)
{
	/* Configure the system clocks */
	//mx21_system_clk_init();	 
	
	/* Setup gpio mappings
	*/
	mxc_gpio_setup_multiple_pins(tll_mx21_pins, ARRAY_SIZE(tll_mx21_pins),"tll_mx21");
	
	mxc_register_device(&mxc_uart_device0, &uart_pdata[0]);
	mxc_register_device(&mxc_uart_device1, &uart_pdata[1]);
	mxc_register_device(&mxc_uart_device2, &uart_pdata[2]);

#if defined (CONFIG_SMSC911X)
	/* Ethernet Interrupt Line */
	set_irq_type(TLL_MX21_ETH_IRQ,  IRQ_TYPE_LEVEL_LOW);	
#endif
	
#if defined (CONFIG_I2C_IMX)
	/*Configure I2C bus*/
	i2c_register_board_info(1,mx21_i2c_devices,
				ARRAY_SIZE(mx21_i2c_devices));
	mxc_register_device(&mxc_i2c_device0, &mx21_i2c_data);
#endif

#if defined (CONFIG_SPI_IMX)
	spi_register_board_info(mx21_spi_board_info,
				ARRAY_SIZE(mx21_spi_board_info));
	mxc_register_device(&mxc_spi_device0, &mx21_spi_1_data);
#endif

#if defined(CONFIG_FB_IMX)
//#if 0 // bdd - need to fix MPR3 setup ??? 
	// Boost DMA priority of LCD over CPU
//	MAX_MPR3 	= 0x543021;  
//#endif	
	printk("%s: LCD\n", __FUNCTION__);
	mxc_register_device(&mxc_fb_device, &vcmx212_fb_info);
	tllmx21_lcd_power(0);
#endif

/*#if defined (CONFIG_SPI_IMX)
	PCCR0 |= PCCR0_CSPI2_EN;	
#endif	*/

#if defined(CONFIG_USB_IMX21_HCD)
	printk("IMX: USB HCD\n");
	/* Enable the clocks to the USB OTG module */
	PCCR0 |= PCCR0_HCLK_USBOTG_EN | PCCR0_USBOTG_EN ;

#if defined (CONFIG_SPI_IMX)
	PCCR0 |= PCCR0_CSPI2_EN;
#endif
	/* Reset the USB OTG module */
	USBOTG_RST_CTRL = (USBOTG_RST_RSTCTRL |
			USBOTG_RST_RSTFC   |
			USBOTG_RST_RSTFSKE |
			USBOTG_RST_RSTRH   |
			USBOTG_RST_RSTHSIE |
			USBOTG_RST_RSTHC);

	/* Wait for reset to finish */
	while (USBOTG_RST_CTRL != 0) {
		schedule_timeout(1);
	}
	
	/* Enable clock to the host controller */
	USBOTG_CLK_CTRL = USBOTG_CLK_CTRL_HST | USBOTG_CLK_CTRL_MAIN;
	
	/*
	* Configure the USBOTG module:
	*   Host xcvr diff TX, diff RX
	*   USBOTG xcvr diff TX, diff RX
	*   USBOTG in host mode
	*/
	USBOTG_HWMODE = (USBOTG_HWMODE_OTGXCVR_TD_RD |
			USBOTG_HWMODE_HOSTXCVR_TD_RD |
			USBOTG_HWMODE_CRECFG_HOST);
				
				
#endif
	platform_add_devices(devices, ARRAY_SIZE(devices));
}

static void __init
tll_mx21_map_io(void) {
	mx21_map_io();
}

static void __init tll_mx21_timer_init(void) {
	mx21_clocks_init(32768, 26000000);
}

static struct sys_timer tll_mx21_timer = {
	.init	= tll_mx21_timer_init,
};

MACHINE_START(TLL_MX21, "TLL MX21 Mezzanine")
	/* MAINTAINER: "Brian Dodge <bdodge@comcast.net>" */
	.phys_io	= 0x00200000,
	.io_pg_offst	= ((0xe0200000) >> 18) & 0xfffc,
	.boot_params	= 0xc0000000,
	.map_io		= tll_mx21_map_io,
	.init_irq	= mx21_init_irq,
	.init_machine	= tll_mx21_init,
	.timer		= &tll_mx21_timer,
MACHINE_END
