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

UINT32 IMX586_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 IMX616_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 IMX319_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 S5K3M5SX_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 GC02M1B_MIPI_MONO_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 IMX682_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 OV32A1Q_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 IMX471_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 HI846_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 GC2385_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 GC2375H_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
/* Shipei.Chen@Cam.Drv, 20200413, sensor porting for ov02b1b!*/
UINT32 OV02B1B_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
UINT32 IMX686_MIPI_RAW_SensorInit(struct SENSOR_FUNCTION_STRUCT **pfFunc);
extern struct IMGSENSOR_SENSOR_LIST gimgsensor_sensor_list[];

#endif

