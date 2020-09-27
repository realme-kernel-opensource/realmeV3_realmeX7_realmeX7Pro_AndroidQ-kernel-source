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

#ifndef __KD_SENSORLIST_H__
#define __KD_SENSORLIST_H__

#include "kd_camera_typedef.h"
#include "imgsensor_sensor.h"

#ifndef VENDOR_EDIT
#define VENDOR_EDIT
#endif

struct IMGSENSOR_INIT_FUNC_LIST {
	MUINT32   id;
	MUINT8    name[32];
	MUINT32 (*init)(struct SENSOR_FUNCTION_STRUCT **pfFunc);
};

/*IMX*/

#ifdef VENDOR_EDIT
/* Shipei.Chen@Cam.Drv, 20200502,  sensor porting!*/
UINT32 OV16A10_MAIN_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 S5KGM1ST_MAIN_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 S5K3P9SP_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 HI846_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 GC02K_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 OV02B1B_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);

/* Shengguang.Zhu@Cam.Drv, 20200518,  sensor porting!*/
UINT32 S5KGW3_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);

/* Shipei.Chen@Cam.Drv, 20200521,  sensor porting for 20630!*/
UINT32 ATHENSB_OV32A1Q_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 ATHENSB_HI846_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 ATHENSB_GC02K_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 ATHENSB_GC02M1B_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);

/* Zhijian.Wang@Cam.Drv, 20200601,  sensor porting for Cake!*/
UINT32 CAKE_OV13B10_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 CAKE_S5K4H7_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 CAKE_GC02M1B_MIPI_MONO_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 CAKE_GC02K0_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);

/* Shengguang.Zhu@Cam.Drv, 20200722,  sensor porting for athensc!*/
UINT32 ATHENSC_S5KGM1ST_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 ATHENSC_IMX471_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);

#endif
extern struct IMGSENSOR_SENSOR_LIST gimgsensor_sensor_list[];

#endif

