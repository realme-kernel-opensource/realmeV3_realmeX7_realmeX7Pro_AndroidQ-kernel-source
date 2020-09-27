/*
 * Copyright (c) 2015 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/backlight.h>
#include <drm/drmP.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>
#include <linux/of_graph.h>
#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>
#include <video/of_videomode.h>
#include <video/videomode.h>

#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
#include "../mediatek/mtk_corner_pattern/mtk_data_hw_roundedpattern.h"
#endif

#define CONFIG_MTK_PANEL_EXT
#if defined(CONFIG_MTK_PANEL_EXT)
#include "../mediatek/mtk_panel_ext.h"
#include "../mediatek/mtk_log.h"
#include "../mediatek/mtk_drm_graphics_base.h"
#endif
/* enable this to check panel self -bist pattern */
/* #define PANEL_BIST_PATTERN */

/* option function to read data from some panel address */
/* #define PANEL_SUPPORT_READBACK */

#define LCM_DSI_CMD_MODE 1

#define REGFLAG_CMD       0xFFFA
#define REGFLAG_DELAY       0xFFFC
#define REGFLAG_UDELAY  0xFFFB
#define REGFLAG_END_OF_TABLE    0xFFFD

#define BRIGHTNESS_MAX    2047
#define BRIGHTNESS_HALF   1023

struct samsung {
	struct device *dev;
	struct drm_panel panel;
	struct backlight_device *backlight;
	struct gpio_desc *reset_gpio;
	//struct gpio_desc *bias_gpio;

	bool prepared;
	bool enabled;

	int error;
};

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[128];
};

#define samsung_dcs_write_seq(ctx, seq...)                                     \
	({                                                                     \
		const u8 d[] = {seq};                                          \
		BUILD_BUG_ON_MSG(ARRAY_SIZE(d) > 64,                           \
				 "DCS sequence too big for stack");            \
		samsung_dcs_write(ctx, d, ARRAY_SIZE(d));                      \
	})

#define samsung_dcs_write_seq_static(ctx, seq...)                              \
	({                                                                     \
		static const u8 d[] = {seq};                                   \
		samsung_dcs_write(ctx, d, ARRAY_SIZE(d));                      \
	})

static inline struct samsung *panel_to_samsung(struct drm_panel *panel)
{
	return container_of(panel, struct samsung, panel);
}

#ifdef PANEL_SUPPORT_READBACK
static int samsung_dcs_read(struct samsung *ctx, u8 cmd, void *data, size_t len)
{
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	ssize_t ret;

	if (ctx->error < 0)
		return 0;

	ret = mipi_dsi_dcs_read(dsi, cmd, data, len);
	if (ret < 0) {
		dev_err(ctx->dev, "error %d reading dcs seq:(%#x)\n", ret, cmd);
		ctx->error = ret;
	}

	return ret;
}

static void samsung_panel_get_data(struct samsung *ctx)
{
	u8 buffer[3] = {0};
	static int ret;

	if (ret == 0) {
		ret = samsung_dcs_read(ctx, 0x0A, buffer, 1);
		dev_info(ctx->dev, "return %d data(0x%08x) to dsi engine\n",
			 ret, buffer[0] | (buffer[1] << 8));
	}
}
#endif

#if defined(CONFIG_RT5081_PMU_DSV) || defined(CONFIG_MT6370_PMU_DSV)
static struct regulator *disp_bias_pos;
static struct regulator *disp_bias_neg;

static int samsung_panel_bias_regulator_init(void)
{
	static int regulator_inited;
	int ret = 0;

	if (regulator_inited)
		return ret;

	/* please only get regulator once in a driver */
	disp_bias_pos = regulator_get(NULL, "dsv_pos");
	if (IS_ERR(disp_bias_pos)) { /* handle return value */
		ret = PTR_ERR(disp_bias_pos);
		pr_err("get dsv_pos fail, error: %d\n", ret);
		return ret;
	}

	disp_bias_neg = regulator_get(NULL, "dsv_neg");
	if (IS_ERR(disp_bias_neg)) { /* handle return value */
		ret = PTR_ERR(disp_bias_neg);
		pr_err("get dsv_neg fail, error: %d\n", ret);
		return ret;
	}

	regulator_inited = 1;
	return ret; /* must be 0 */
}

static int samsung_panel_bias_enable(void)
{
	int ret = 0;
	int retval = 0;

	samsung_panel_bias_regulator_init();

	/* set voltage with min & max*/
	ret = regulator_set_voltage(disp_bias_pos, 5400000, 5400000);
	if (ret < 0)
		pr_err("set voltage disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_set_voltage(disp_bias_neg, 5400000, 5400000);
	if (ret < 0)
		pr_err("set voltage disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	/* enable regulator */
	ret = regulator_enable(disp_bias_pos);
	if (ret < 0)
		pr_err("enable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_enable(disp_bias_neg);
	if (ret < 0)
		pr_err("enable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	return retval;
}

static int samsung_panel_bias_disable(void)
{
	int ret = 0;
	int retval = 0;

	samsung_panel_bias_regulator_init();

	ret = regulator_disable(disp_bias_neg);
	if (ret < 0)
		pr_err("disable regulator disp_bias_neg fail, ret = %d\n", ret);
	retval |= ret;

	ret = regulator_disable(disp_bias_pos);
	if (ret < 0)
		pr_err("disable regulator disp_bias_pos fail, ret = %d\n", ret);
	retval |= ret;

	return retval;
}
#endif

static struct regulator *disp_ldo3;       //ldo3:VDDI 1.8V
static struct regulator *disp_ldo5;		  //ldo5:VCI 3.0V

static int lcm_panel_ldo3_regulator_init(struct device *dev)
{
	static int regulator_inited;
	int ret = 0;

	if (regulator_inited)
		return ret;
    pr_err("get lcm_panel_ldo3_regulator_init\n");

	/* please only get regulator once in a driver */
	disp_ldo3 = regulator_get(dev, "VMC");
	if (IS_ERR(disp_ldo3)) { /* handle return value */
		ret = PTR_ERR(disp_ldo3);
		pr_err("get ldo3 fail, error: %d\n", ret);
		return ret;
	}
	regulator_inited = 1;
	return ret; /* must be 0 */

}

static int lcm_panel_ldo3_enable(struct device *dev)
{
	int ret = 0;
	int retval = 0;
	
	printk("%s\n",__func__);
	lcm_panel_ldo3_regulator_init(dev);

	/* set voltage with min & max*/
	ret = regulator_set_voltage(disp_ldo3, 1800000, 1800000);
	if (ret < 0)
		pr_err("set voltage disp_ldo3 fail, ret = %d\n", ret);
	retval |= ret;

	/* enable regulator */
	ret = regulator_enable(disp_ldo3);
	if (ret < 0)
		pr_err("enable regulator disp_ldo3 fail, ret = %d\n", ret);
	retval |= ret;
    pr_err("get lcm_panel_ldo3_enable\n");

	return retval;
}

static int lcm_panel_ldo3_disable(struct device *dev)
{
	int ret = 0;
	int retval = 0;
printk("%s\n",__func__);
	lcm_panel_ldo3_regulator_init(dev);

	ret = regulator_disable(disp_ldo3);
	if (ret < 0)
		pr_err("disable regulator disp_ldo3 fail, ret = %d\n", ret);
	retval |= ret;
    pr_err("disable regulator disp_ldo3\n");

	return retval;
}

static int lcm_panel_ldo5_regulator_init(struct device *dev)
{
	static int regulator_inited;
	int ret = 0;

	if (regulator_inited)
		return ret;
    pr_err("get lcm_panel_ldo5_regulator_init\n");

	/* please only get regulator once in a driver */
	disp_ldo5 = regulator_get(dev, "VMCH");
	if (IS_ERR(disp_ldo5)) { /* handle return value */
		ret = PTR_ERR(disp_ldo5);
		pr_err("get ldo5 fail, error: %d\n", ret);
		return ret;
	}
	regulator_inited = 1;
	return ret; /* must be 0 */

}

static int lcm_panel_ldo5_enable(struct device *dev)
{
	int ret = 0;
	int retval = 0;
	printk("%s\n",__func__);
	lcm_panel_ldo5_regulator_init(dev);

	/* set voltage with min & max*/
	ret = regulator_set_voltage(disp_ldo5, 3000000, 3000000);
	if (ret < 0)
		pr_err("set voltage disp_ldo5 fail, ret = %d\n", ret);
	retval |= ret;

	/* enable regulator */
	ret = regulator_enable(disp_ldo5);
	if (ret < 0)
		pr_err("enable regulator disp_ldo5 fail, ret = %d\n", ret);
	retval |= ret;
    pr_err("get lcm_panel_ldo5_enable\n");

	return retval;
}

static int lcm_panel_ldo5_disable(struct device *dev)
{
	int ret = 0;
	int retval = 0;
	printk("%s\n",__func__);
	lcm_panel_ldo5_regulator_init(dev);

	ret = regulator_disable(disp_ldo5);
	if (ret < 0)
		pr_err("disable regulator disp_ldo5 fail, ret = %d\n", ret);
	retval |= ret;
    pr_err("disable regulator disp_ldo5\n");

	return retval;
}

static void samsung_dcs_write(struct samsung *ctx, const void *data, size_t len)
{
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	ssize_t ret;
	char *addr;

	if (ctx->error < 0)
		return;
	printk("%s\n",__func__);
	addr = (char *)data;
	if ((int)*addr < 0xB0)
		ret = mipi_dsi_dcs_write_buffer(dsi, data, len);
	else
		ret = mipi_dsi_generic_write(dsi, data, len);
	if (ret < 0) {
		dev_err(ctx->dev, "error %zd writing seq: %ph\n", ret, data);
		ctx->error = ret;
	}
}

static void samsung_panel_init(struct samsung *ctx)
{
/*	ctx->bias_gpio = devm_gpiod_get(ctx->dev, "bias", GPIOD_OUT_HIGH);
	gpiod_set_value(ctx->bias_gpio, 1);
	devm_gpiod_put(ctx->dev, ctx->bias_gpio);
	msleep(2);
*/	
	printk("samsung enter init");
	ctx->reset_gpio = devm_gpiod_get(ctx->dev, "reset", GPIOD_OUT_HIGH);
	udelay(10 * 1000);
	gpiod_set_value(ctx->reset_gpio, 0);
	udelay(10 * 1000);
	gpiod_set_value(ctx->reset_gpio, 1);
	udelay(10 * 1000);
	devm_gpiod_put(ctx->dev, ctx->reset_gpio);

	samsung_dcs_write_seq_static(ctx, 0xF0, 0x5A, 0x5A);
	samsung_dcs_write_seq_static(ctx, 0xFC, 0x5A, 0x5A);
	samsung_dcs_write_seq_static(ctx, 0xB0, 0x0C);
	samsung_dcs_write_seq_static(ctx, 0xFF, 0x10);
	samsung_dcs_write_seq_static(ctx, 0xB0, 0x22);
	samsung_dcs_write_seq_static(ctx, 0xD1, 0x1E);    //60.5Hz
	samsung_dcs_write_seq_static(ctx, 0xF0, 0xA5, 0xA5);
	samsung_dcs_write_seq_static(ctx, 0xFC, 0xA5, 0xA5);
	samsung_dcs_write_seq_static(ctx, 0x9F, 0xA5, 0xA5);
	msleep(10);
	/* sleep out */
	samsung_dcs_write_seq_static(ctx, 0x11);
	msleep(20);
	samsung_dcs_write_seq_static(ctx, 0x9F, 0x5A, 0x5A);
	/*TE sync on*/
	samsung_dcs_write_seq_static(ctx, 0x9F, 0xA5, 0xA5);
	samsung_dcs_write_seq_static(ctx, 0x35, 0x00);
	samsung_dcs_write_seq_static(ctx, 0x9F, 0x5A, 0x5A);
	/* CASET/PASET Setting */
	samsung_dcs_write_seq_static(ctx, 0x2A, 0x00, 0x00, 0x04, 0x37);
	samsung_dcs_write_seq_static(ctx, 0x2B, 0x00, 0x00, 0x09, 0x5F);
	/* Err_FG Setting */
	samsung_dcs_write_seq_static(ctx, 0xF0, 0x5A, 0x5A);
	samsung_dcs_write_seq_static(ctx, 0xFC, 0x5A, 0x5A);
	samsung_dcs_write_seq_static(ctx, 0xE1, 0x00, 0x00, 0x02, 0x02, 0x42, 0x02);
	samsung_dcs_write_seq_static(ctx, 0xE2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	samsung_dcs_write_seq_static(ctx, 0xB0, 0x0C);
	samsung_dcs_write_seq_static(ctx, 0xE1, 0x19);
	samsung_dcs_write_seq_static(ctx, 0xF0, 0xA5, 0xA5);
	samsung_dcs_write_seq_static(ctx, 0xFC, 0xA5, 0xA5);
	/* FD Setting */
	samsung_dcs_write_seq_static(ctx, 0xF0, 0x5A, 0x5A);
	samsung_dcs_write_seq_static(ctx, 0xBE, 0x02, 0x29); //MODE ON
	samsung_dcs_write_seq_static(ctx, 0xB0, 0x02);
	samsung_dcs_write_seq_static(ctx, 0xC9, 0x10);
	samsung_dcs_write_seq_static(ctx, 0xF0, 0xA5, 0xA5);
	/* FFC Setting (OSC:90.25  MIPI CLK:562MHz) */
	samsung_dcs_write_seq_static(ctx, 0xF0, 0x5A, 0x5A);
	samsung_dcs_write_seq_static(ctx, 0xE9, 0x11, 0x65, 0xA2, 0xE7, 0xBD, 0xA9, 0x12, 0x8C, 0x00, 0x1A, 0xB8);
	samsung_dcs_write_seq_static(ctx, 0xF0, 0xA5, 0xA5);
	/* Dimming Setting */
	samsung_dcs_write_seq_static(ctx, 0xF0, 0x5A, 0x5A);
	samsung_dcs_write_seq_static(ctx, 0xB0, 0x06);
	samsung_dcs_write_seq_static(ctx, 0xB7, 0x01);
	samsung_dcs_write_seq_static(ctx, 0xF0, 0xA5, 0xA5);
	samsung_dcs_write_seq_static(ctx, 0x53, 0x28);
	samsung_dcs_write_seq_static(ctx, 0x51, 0x03, 0xFF);
	/* ACL MODE */
	samsung_dcs_write_seq_static(ctx, 0x55, 0x00); //ACL OFF
	samsung_dcs_write_seq_static(ctx, 0xF0, 0x5A, 0x5A);
	samsung_dcs_write_seq_static(ctx, 0xB0, 0xDD);
	samsung_dcs_write_seq_static(ctx, 0xB9, 0x00);
	samsung_dcs_write_seq_static(ctx, 0xF0, 0xA5, 0xA5);
	/* Display On */
	msleep(110);
	samsung_dcs_write_seq_static(ctx, 0x29);
}

static int samsung_disable(struct drm_panel *panel)
{
	struct samsung *ctx = panel_to_samsung(panel);
	printk("%s\n",__func__);
#ifdef VENDOR_EDIT
/* Hujie@PSW.MM.DisplayDriver.AOD, 2019/12/10, add for keylog*/
	pr_err("debug for samsung %s\n", __func__);
#endif
	if (!ctx->enabled)
		return 0;

	if (ctx->backlight) {
		ctx->backlight->props.power = FB_BLANK_POWERDOWN;
		backlight_update_status(ctx->backlight);
	}

	ctx->enabled = false;

	return 0;
}

static int samsung_unprepare(struct drm_panel *panel)
{
	struct samsung *ctx = panel_to_samsung(panel);
	printk("%s\n",__func__);
#ifdef VENDOR_EDIT
	/* Hujie@PSW.MM.DisplayDriver.AOD, 2019/12/10, add for keylog*/
	pr_err("debug for samsung %s\n", __func__);
#endif
	if (!ctx->prepared)
		return 0;

	samsung_dcs_write_seq_static(ctx, MIPI_DCS_SET_DISPLAY_OFF);
	msleep(20);
	samsung_dcs_write_seq_static(ctx, MIPI_DCS_ENTER_SLEEP_MODE);
	msleep(160);

	ctx->error = 0;
	ctx->prepared = false;
	lcm_panel_ldo3_disable(ctx->dev);
	lcm_panel_ldo5_disable(ctx->dev);
#if defined(CONFIG_RT5081_PMU_DSV) || defined(CONFIG_MT6370_PMU_DSV)
	samsung_panel_bias_disable();
#endif
	ctx->reset_gpio = devm_gpiod_get(ctx->dev, "reset", GPIOD_OUT_HIGH);
	gpiod_set_value(ctx->reset_gpio, 0);
	devm_gpiod_put(ctx->dev, ctx->reset_gpio);
	msleep(5);
/*	
	ctx->bias_gpio = devm_gpiod_get(ctx->dev, "bias", GPIOD_OUT_HIGH);
	gpiod_set_value(ctx->bias_gpio, 0);
	msleep(15);
	devm_gpiod_put(ctx->dev, ctx->bias_gpio);*/
	return 0;
}

static int samsung_prepare(struct drm_panel *panel)
{
	struct samsung *ctx = panel_to_samsung(panel);
	int ret;

	pr_info("%s\n", __func__);
	if (ctx->prepared)
		return 0;

	/* VDDI pull up*/
	lcm_panel_ldo3_enable(ctx->dev);
	/* VCI pull up*/
	lcm_panel_ldo5_enable(ctx->dev);
#if defined(CONFIG_RT5081_PMU_DSV) || defined(CONFIG_MT6370_PMU_DSV)
	samsung_panel_bias_enable();
#endif

	samsung_panel_init(ctx);

	ret = ctx->error;
	if (ret < 0)
		samsung_unprepare(panel);

	ctx->prepared = true;

#ifdef PANEL_SUPPORT_READBACK
	samsung_panel_get_data(ctx);
#endif

	return ret;
}

static int samsung_enable(struct drm_panel *panel)
{
	struct samsung *ctx = panel_to_samsung(panel);
	printk("%s\n",__func__);
	if (ctx->enabled)
		return 0;

	if (ctx->backlight) {
		ctx->backlight->props.power = FB_BLANK_UNBLANK;
		backlight_update_status(ctx->backlight);
	}

	ctx->enabled = true;

	return 0;
}

static const struct drm_display_mode default_mode = {
	.clock = 196247,
	.hdisplay = 1080,
	.hsync_start = 1080 + 128,
	.hsync_end = 1080 + 128 + 20,
	.htotal = 1080 + 128 + 20 + 118,
	.vdisplay = 2400,
	.vsync_start = 2400 + 20,
	.vsync_end = 2400 + 20 + 4,
	.vtotal = 2400 + 20 + 4 + 6,
	.vrefresh = 60,
};

#if defined(CONFIG_MTK_PANEL_EXT)
static struct mtk_panel_params ext_params = {
	.cust_esd_check = 0,
	.esd_check_enable = 1,
	.lcm_esd_check_table[0] = {

			.cmd = 0x53, .count = 1, .para_list[0] = 0x24,
		},
	.lcm_color_mode = MTK_DRM_COLOR_MODE_DISPLAY_P3,
	.physical_width_um = 68256,
	.physical_height_um = 152512,
};

static int samsung_setbacklight_cmdq(void *dsi, dcs_write_gce cb,
	void *handle, unsigned int level)
{
	char bl_tb0[] = {0xf0, 0x5a, 0x5a};
	char bl_tb1[] = {0x51, 0xff, 0xff};
	char bl_tb2[] = {0xf0, 0xa5, 0xa5};

	printk("%s, bl_levl = %d",__func__,level);
	if (level > 255)
		level = 255;

	level = level * 1023 / 255;
	bl_tb1[1] = ((level >> 8) & 0x3);
	bl_tb1[2] = (level & 0xff);

	if (!cb)
		return -1;

	cb(dsi, handle, bl_tb0, ARRAY_SIZE(bl_tb0));
	cb(dsi, handle, bl_tb1, ARRAY_SIZE(bl_tb1));
	cb(dsi, handle, bl_tb2, ARRAY_SIZE(bl_tb2));

	return 0;
}
static struct mtk_panel_funcs ext_funcs = {
	.set_backlight_cmdq = samsung_setbacklight_cmdq,
};
#endif

struct panel_desc {
	const struct drm_display_mode *modes;
	unsigned int num_modes;

	unsigned int bpc;

	struct {
		unsigned int width;
		unsigned int height;
	} size;

	/**
	 * @prepare: the time (in milliseconds) that it takes for the panel to
	 *           become ready and start receiving video data
	 * @enable: the time (in milliseconds) that it takes for the panel to
	 *          display the first valid frame after starting to receive
	 *          video data
	 * @disable: the time (in milliseconds) that it takes for the panel to
	 *           turn the display off (no content is visible)
	 * @unprepare: the time (in milliseconds) that it takes for the panel
	 *             to power itself down completely
	 */
	struct {
		unsigned int prepare;
		unsigned int enable;
		unsigned int disable;
		unsigned int unprepare;
	} delay;
};

static int samsung_get_modes(struct drm_panel *panel)
{
	struct drm_display_mode *mode;
	printk("%s\n",__func__);
	mode = drm_mode_duplicate(panel->drm, &default_mode);
	if (!mode) {
		dev_err(panel->drm->dev, "failed to add mode %ux%ux@%u\n",
			default_mode.hdisplay, default_mode.vdisplay,
			default_mode.vrefresh);
		return -ENOMEM;
	}

	drm_mode_set_name(mode);
	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(panel->connector, mode);

	panel->connector->display_info.width_mm = 68;
	panel->connector->display_info.height_mm = 152;

	return 1;
}

static const struct drm_panel_funcs samsung_drm_funcs = {
	.disable = samsung_disable,
	.unprepare = samsung_unprepare,
	.prepare = samsung_prepare,
	.enable = samsung_enable,
	.get_modes = samsung_get_modes,
};

static int samsung_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct samsung *ctx;
	struct device_node *backlight;
	int ret;
	struct device_node *dsi_node, *remote_node = NULL, *endpoint = NULL;

        dsi_node = of_get_parent(dev->of_node);
        if (dsi_node) {
                endpoint = of_graph_get_next_endpoint(dsi_node, NULL);
                if (endpoint) {
                        remote_node = of_graph_get_remote_port_parent(endpoint);
                        pr_info("device_node name:%s\n", remote_node->name);
                   }
        }
        if (remote_node != dev->of_node) {
                pr_info("%s+ skip probe due to not current lcm.\n", __func__);
		//lcm_panel_ldo3_enable(ctx->dev);
        	//lcm_panel_ldo5_enable(ctx->dev);
                return 0;
        }
	pr_info("%s+\n", __func__);
	ctx = devm_kzalloc(dev, sizeof(struct samsung), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;
	
	mipi_dsi_set_drvdata(dsi, ctx);
	ctx->dev = dev;
	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
#if (LCM_DSI_CMD_MODE)
	dsi->mode_flags = MIPI_DSI_MODE_LPM | MIPI_DSI_MODE_EOT_PACKET
			 | MIPI_DSI_CLOCK_NON_CONTINUOUS;
#else
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST
			 | MIPI_DSI_MODE_LPM | MIPI_DSI_MODE_EOT_PACKET
			 | MIPI_DSI_CLOCK_NON_CONTINUOUS;

#endif
	backlight = of_parse_phandle(dev->of_node, "backlight", 0);
	if (backlight) {
		ctx->backlight = of_find_backlight_by_node(backlight);
		of_node_put(backlight);

		if (!ctx->backlight)
			return -EPROBE_DEFER;
	}
	
/*	
	ctx->bias_gpio = devm_gpiod_get(ctx->dev, "bias", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->bias_gpio)) {
		dev_err(dev, "cannot get bias-gpios %ld\n",
			PTR_ERR(ctx->bias_gpio));
		return PTR_ERR(ctx->bias_gpio);
	}
	gpiod_set_value(ctx->bias_gpio, 1);
	devm_gpiod_put(ctx->dev, ctx->bias_gpio);
	msleep(2);
*/	
	lcm_panel_ldo3_enable(ctx->dev);
	lcm_panel_ldo5_enable(ctx->dev);
	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
		dev_err(dev, "cannot get reset-gpios %ld\n",
			PTR_ERR(ctx->reset_gpio));
		return PTR_ERR(ctx->reset_gpio);
	}
	devm_gpiod_put(dev, ctx->reset_gpio);
	ctx->prepared = true;
	ctx->enabled = true;

	drm_panel_init(&ctx->panel);
	ctx->panel.dev = dev;
	ctx->panel.funcs = &samsung_drm_funcs;

	ret = drm_panel_add(&ctx->panel);
	if (ret < 0)
		return ret;
	ret = mipi_dsi_attach(dsi);
	if (ret < 0)
		drm_panel_remove(&ctx->panel);
#if defined(CONFIG_MTK_PANEL_EXT)
	ret = mtk_panel_ext_create(dev, &ext_params, &ext_funcs, &ctx->panel);
	if (ret < 0)
		return ret;
#endif

	pr_info("%s-\n", __func__);

	return ret;
}

static int samsung_remove(struct mipi_dsi_device *dsi)
{
	struct samsung *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);

	return 0;
}

static const struct of_device_id samsung_of_match[] = {
	{
		.compatible = "tianma,hx83112f,dphy,vdo",
	},
	{} };

MODULE_DEVICE_TABLE(of, samsung_of_match);

static struct mipi_dsi_driver samsung_driver = {
	.probe = samsung_probe,
	.remove = samsung_remove,
	.driver = {

			.name = "hx83112f_fhdp_dsi_vdo_dphy_tianma_lcm_drv",
			.owner = THIS_MODULE,
			.of_match_table = samsung_of_match,
		},
};

module_mipi_dsi_driver(samsung_driver);

MODULE_AUTHOR("Freya Li <freya.li@mediatek.com>");
MODULE_DESCRIPTION("samsung ams601nn04 CMD LCD Panel Driver");
MODULE_LICENSE("GPL v2");
