/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */
#include <linux/kernel.h>
#include "cam_cal_list.h"
#include "eeprom_i2c_common_driver.h"
#include "eeprom_i2c_custom_driver.h"
#include "kd_imgsensor.h"

#define MAX_EEPROM_SIZE_16K 0x4000


#ifndef VENDOR_EDIT
#define VENDOR_EDIT
#endif

struct stCAM_CAL_LIST_STRUCT g_camCalList[] = {
	/*Below is commom sensor */
#ifdef VENDOR_EDIT
	/* Shipei.Chen@Cam.Drv, 20200508,  OTP Porting!*/
	{S5KGM1ST_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{OV16A10_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{S5K3P9SP_SENSOR_ID, 0xA8, Common_read_region},
	{HI846_SENSOR_ID, 0xA2, Common_read_region, MAX_EEPROM_SIZE_16K},
	{GC02K_SENSOR_ID, 0xA4, Common_read_region},
	/* Shipei.Chen@Cam.Drv, 20200525,  otp porting for 20630!*/
	{S5KGW3_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{ATHENSB_OV32A1Q_SENSOR_ID, 0xA8, Common_read_region},
	{ATHENSB_HI846_SENSOR_ID, 0xA2, Common_read_region, MAX_EEPROM_SIZE_16K},
	{ATHENSB_GC02K_SENSOR_ID, 0xA4, Common_read_region},
	/* Zhijian.Wang@Cam.Drv, 20200608,  otp porting for cake!*/
	{CAKE_OV13B10_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{CAKE_GC02K0_SENSOR_ID, 0xA4, Common_read_region},
	/* Shengguang.Zhu@Cam.Drv, 20200730,  otp porting for 20633!*/
	{ATHENSC_S5KGM1ST_SENSOR_ID, 0xA0, Common_read_region, MAX_EEPROM_SIZE_16K},
	{ATHENSC_IMX471_SENSOR_ID, 0xA8, Common_read_region, MAX_EEPROM_SIZE_16K},
#endif
	/*  ADD before this line */
	{0, 0, 0}       /*end of list */
};

unsigned int cam_cal_get_sensor_list(
	struct stCAM_CAL_LIST_STRUCT **ppCamcalList)
{
	if (ppCamcalList == NULL)
		return 1;

	*ppCamcalList = &g_camCalList[0];
	return 0;
}


