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
#ifdef VENDOR_EDIT
/* Shipei.Chen@Cam.Drv, 20200502,  sensor porting!*/
#if defined(S5KGM1ST_MIPI_RAW)
{S5KGM1ST_SENSOR_ID, SENSOR_DRVNAME_S5KGM1ST_MIPI_RAW, S5KGM1ST_MAIN_MIPI_RAW_SensorInit},
#endif
#if defined(OV16A10_MIPI_RAW)
{OV16A10_SENSOR_ID, SENSOR_DRVNAME_OV16A10_MIPI_RAW, OV16A10_MAIN_MIPI_RAW_SensorInit},
#endif
#if defined(S5K3P9SP_MIPI_RAW)
	{S5K3P9SP_SENSOR_ID, SENSOR_DRVNAME_S5K3P9SP_MIPI_RAW, S5K3P9SP_MIPI_RAW_SensorInit},
#endif
#if defined(HI846_MIPI_RAW)
	{HI846_SENSOR_ID, SENSOR_DRVNAME_HI846_MIPI_RAW, HI846_MIPI_RAW_SensorInit},
#endif
#if defined(GC02K_MIPI_RAW)
	{GC02K_SENSOR_ID, SENSOR_DRVNAME_GC02K_MIPI_RAW, GC02K_MIPI_RAW_SensorInit},
#endif
#if defined(OV02B1B_MIPI_RAW)
	{OV02B1B_SENSOR_ID, SENSOR_DRVNAME_OV02B1B_MIPI_RAW, OV02B1B_MIPI_RAW_SensorInit},
#endif

/* Shengguang.Zhu@Cam.Drv, 20200518,  sensor porting!*/
#if defined(S5KGW3_MIPI_RAW)
	{S5KGW3_SENSOR_ID, SENSOR_DRVNAME_S5KGW3_MIPI_RAW, S5KGW3_MIPI_RAW_SensorInit},
#endif

/* Shipei.Chen@Cam.Drv, 20200521,  sensor porting for 20630!*/
#if defined(ATHENSB_OV32A1Q_MIPI_RAW)
	{ATHENSB_OV32A1Q_SENSOR_ID, SENSOR_DRVNAME_ATHENSB_OV32A1Q_MIPI_RAW, ATHENSB_OV32A1Q_MIPI_RAW_SensorInit},
#endif
#if defined(ATHENSB_HI846_MIPI_RAW)
	{ATHENSB_HI846_SENSOR_ID, SENSOR_DRVNAME_ATHENSB_HI846_MIPI_RAW, ATHENSB_HI846_MIPI_RAW_SensorInit},
#endif
#if defined(ATHENSB_GC02K_MIPI_RAW)
	{ATHENSB_GC02K_SENSOR_ID, SENSOR_DRVNAME_ATHENSB_GC02K_MIPI_RAW, ATHENSB_GC02K_MIPI_RAW_SensorInit},
#endif
#if defined(ATHENSB_GC02M1B_MIPI_RAW)
	{ATHENSB_GC02M1B_SENSOR_ID, SENSOR_DRVNAME_ATHENSB_GC02M1B_MIPI_RAW, ATHENSB_GC02M1B_MIPI_RAW_SensorInit},
#endif

/* Zhijian.Wang@Cam.Drv, 20200601,  sensor porting for cake!*/
#if defined(CAKE_OV13B10_MIPI_RAW)
	{CAKE_OV13B10_SENSOR_ID, SENSOR_DRVNAME_CAKE_OV13B10_MIPI_RAW, CAKE_OV13B10_MIPI_RAW_SensorInit},
#endif
#if defined(CAKE_S5K4H7_MIPI_RAW)
	{CAKE_S5K4H7_SENSOR_ID, SENSOR_DRVNAME_CAKE_S5K4H7_MIPI_RAW, CAKE_S5K4H7_MIPI_RAW_SensorInit},
#endif
#if defined(CAKE_GC02M1B_MIPI_MONO)
	{CAKE_GC02M1B_SENSOR_ID, SENSOR_DRVNAME_CAKE_GC02M1B_MIPI_MONO, CAKE_GC02M1B_MIPI_MONO_SensorInit},
#endif
#if defined(CAKE_GC02K0_MIPI_RAW)
	{CAKE_GC02K0_SENSOR_ID, SENSOR_DRVNAME_CAKE_GC02K0_MIPI_RAW, CAKE_GC02K0_MIPI_RAW_SensorInit},
#endif

/* Shengguang.Zhu@Cam.Drv, 20200722,  sensor porting for athensc!*/
#if defined(ATHENSC_S5KGM1ST_MIPI_RAW)
    {ATHENSC_S5KGM1ST_SENSOR_ID, SENSOR_DRVNAME_ATHENSC_S5KGM1ST_MIPI_RAW, ATHENSC_S5KGM1ST_MIPI_RAW_SensorInit},
#endif
#if defined(ATHENSC_IMX471_MIPI_RAW)
    {ATHENSC_IMX471_SENSOR_ID, SENSOR_DRVNAME_ATHENSC_IMX471_MIPI_RAW, ATHENSC_IMX471_MIPI_RAW_SensorInit},
#endif

#endif
	/*  ADD sensor driver before this line */
	{0, {0}, NULL}, /* end of list */
};

/* e_add new sensor driver here */

