/*
 * Copyright (C) 2017 MediaTek Inc.
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

#include "kd_camera_typedef.h"
#include "imgsensor_i2c.h"

#ifdef VENDOR_EDIT
#include "soc/oppo/oppo_project.h"
#define IMGSENSOR_I2C_SPEED_1000              1000
#endif

#ifdef IMGSENSOR_LEGACY_COMPAT
void kdSetI2CSpeed(u16 i2cSpeed)
{

}

int iReadRegI2C(u8 *a_pSendData, u16 a_sizeSendData,
		u8 *a_pRecvData, u16 a_sizeRecvData,
		u16 i2cId)
{
	return imgsensor_i2c_read(
			imgsensor_i2c_get_device(),
			a_pSendData,
			a_sizeSendData,
			a_pRecvData,
			a_sizeRecvData,
			i2cId,
			IMGSENSOR_I2C_SPEED);
}

int iReadRegI2CTiming(u8 *a_pSendData, u16 a_sizeSendData, u8 *a_pRecvData,
			u16 a_sizeRecvData, u16 i2cId, u16 timing)
{
	return imgsensor_i2c_read(
			imgsensor_i2c_get_device(),
			a_pSendData,
			a_sizeSendData,
			a_pRecvData,
			a_sizeRecvData,
			i2cId,
			timing);
}

int iWriteRegI2C(u8 *a_pSendData, u16 a_sizeSendData, u16 i2cId)
{
	#ifdef VENDOR_EDIT
	/* Shengguang.Zhu@Rm.Cam.Drv, 20200816,  add for the project 20630 sensor s5kgw3 I2C speed is 1000  */
	if (is_project(OPPO_20630) || is_project(OPPO_20631) || is_project(OPPO_20632) || is_project(OPPO_206B4)) {
		printk("the project 20630 choose IMGSENSOR_I2C_SPEED_1000");
		return imgsensor_i2c_write(
			imgsensor_i2c_get_device(),
			a_pSendData,
			a_sizeSendData,
			a_sizeSendData,
			i2cId,
			IMGSENSOR_I2C_SPEED_1000);
	}
	#endif

	return imgsensor_i2c_write(
			imgsensor_i2c_get_device(),
			a_pSendData,
			a_sizeSendData,
			a_sizeSendData,
			i2cId,
			IMGSENSOR_I2C_SPEED);
}

int iWriteRegI2CTiming(u8 *a_pSendData, u16 a_sizeSendData,
			u16 i2cId, u16 timing)
{
	return imgsensor_i2c_write(
			imgsensor_i2c_get_device(),
			a_pSendData,
			a_sizeSendData,
			a_sizeSendData,
			i2cId,
			timing);
}

int iBurstWriteReg(u8 *pData, u32 bytes, u16 i2cId)
{
	return imgsensor_i2c_write(
			imgsensor_i2c_get_device(),
			pData,
			bytes,
			bytes,
			i2cId,
			IMGSENSOR_I2C_SPEED);
}

int iBurstWriteReg_multi(u8 *pData, u32 bytes, u16 i2cId,
				u16 transfer_length, u16 timing)
{
	return imgsensor_i2c_write(
			imgsensor_i2c_get_device(),
			pData,
			bytes,
			transfer_length,
			i2cId,
			timing);
}

#endif
