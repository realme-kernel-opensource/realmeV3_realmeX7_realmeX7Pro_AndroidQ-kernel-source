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

#include "kd_imgsensor.h"
#include "imgsensor_sensor_list.h"

#ifndef VENDOR_EDIT
#define VENDOR_EDIT
#endif

/* Add Sensor Init function here
 * Note:
 * 1. Add by the resolution from ""large to small"", due to large sensor
 *    will be possible to be main sensor.
 *    This can avoid I2C error during searching sensor.
 * 2. This file should be the same as
 *    mediatek\custom\common\hal\imgsensor\src\sensorlist.cpp
 */
struct IMGSENSOR_SENSOR_LIST
	gimgsensor_sensor_list[MAX_NUM_OF_SUPPORT_SENSOR] = {
	/*IMX*/
#if defined(IMX586_MIPI_RAW)
	{IMX586_SENSOR_ID, SENSOR_DRVNAME_IMX586_MIPI_RAW, IMX586_MIPI_RAW_SensorInit},
#endif
#if defined(IMX616_MIPI_RAW)
	{IMX616_SENSOR_ID, SENSOR_DRVNAME_IMX616_MIPI_RAW, IMX616_MIPI_RAW_SensorInit},
#endif
#if defined(IMX319_MIPI_RAW)
	{IMX319_SENSOR_ID, SENSOR_DRVNAME_IMX319_MIPI_RAW, IMX319_MIPI_RAW_SensorInit},
#endif
#if defined(S5K3M5SX_MIPI_RAW)
	{S5K3M5SX_SENSOR_ID, SENSOR_DRVNAME_S5K3M5SX_MIPI_RAW, S5K3M5SX_MIPI_RAW_SensorInit},
#endif
#if defined(GC02M1B_MIPI_MONO)
	{GC02M1B_SENSOR_ID, SENSOR_DRVNAME_GC02M1B_MIPI_MONO, GC02M1B_MIPI_MONO_SensorInit},
#endif
#if defined(IMX682_MIPI_RAW)
	{IMX682_SENSOR_ID, SENSOR_DRVNAME_IMX682_MIPI_RAW, IMX682_MIPI_RAW_SensorInit},
#endif
#if defined(OV32A1Q_MIPI_RAW)
	{OV32A1Q_SENSOR_ID, SENSOR_DRVNAME_OV32A1Q_MIPI_RAW, OV32A1Q_MIPI_RAW_SensorInit},
#endif
#if defined(IMX471_MIPI_RAW)
	{IMX471_SENSOR_ID, SENSOR_DRVNAME_IMX471_MIPI_RAW, IMX471_MIPI_RAW_SensorInit},
#endif
#if defined(HI846_MIPI_RAW)
	{HI846_SENSOR_ID, SENSOR_DRVNAME_HI846_MIPI_RAW, HI846_MIPI_RAW_SensorInit},
#endif
#if defined(GC2385_MIPI_RAW)
	{GC2385_SENSOR_ID, SENSOR_DRVNAME_GC2385_MIPI_RAW, GC2385_MIPI_RAW_SensorInit},
#endif
#if defined(GC2375H_MIPI_RAW)
	{GC2375H_SENSOR_ID, SENSOR_DRVNAME_GC2375H_MIPI_RAW, GC2375H_MIPI_RAW_SensorInit},
#endif
/* Shipei.Chen@Cam.Drv, 20200413, sensor porting for ov02b1b!*/
#if defined(OV02B1B_MIPI_RAW)
	{OV02B1B_SENSOR_ID, SENSOR_DRVNAME_OV02B1B_MIPI_RAW, OV02B1B_MIPI_RAW_SensorInit},
#endif
#if defined(IMX686_MIPI_RAW)
	{IMX686_SENSOR_ID, SENSOR_DRVNAME_IMX686_MIPI_RAW, IMX686_MIPI_RAW_SensorInit},
#endif
	/*  ADD sensor driver before this line */
	{0, {0}, NULL}, /* end of list */
};

/* e_add new sensor driver here */

