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

#include "regulator.h"

#ifndef VENDOR_EDIT
#define VENDOR_EDIT
#endif

#ifdef VENDOR_EDIT
/* Feiping@Cam.Drv, 20190921, add for 19169 AF*/
#include "imgsensor.h"
#endif

#ifdef VENDOR_EDIT
/* Shipei.Chen@Cam.Drv, 20200416,  add the ldo3 for sensor porting! */
#include "soc/oppo/oppo_project.h"
extern int is_fan53870_pmic(void);
extern int fan53870_cam_ldo_set_voltage(int LDO_NUM, int set_mv);
extern int fan53870_cam_ldo_disable(int LDO_NUM);
#define MAIN_AVDD_LDO_NUM (3)
#define SUB_AVDD_LDO_NUM (4)
#define MAIN_AVDD1_LDO_NUM (5)
#define MAIN2_AVDD_LDO_NUM (4)
#define MAIN_AVDD_VOLTAGE (2900)
#define SUB_AVDD_VOLTAGE (2800)
#define MAIN_AVDD1_VOLTAGE (1800)
#define MAIN2_AVDD_VOLTAGE (2800)
#define MAIN2_DVDD_LDO_NUM (2)
#define MAIN2_DVDD_VOLTAGE (1200)
#endif

static const int regulator_voltage[] = {
	REGULATOR_VOLTAGE_0,
	REGULATOR_VOLTAGE_1000,
	REGULATOR_VOLTAGE_1100,
	REGULATOR_VOLTAGE_1200,
	REGULATOR_VOLTAGE_1210,
	REGULATOR_VOLTAGE_1220,
	REGULATOR_VOLTAGE_1500,
	REGULATOR_VOLTAGE_1800,
	REGULATOR_VOLTAGE_2500,
	REGULATOR_VOLTAGE_2800,
	REGULATOR_VOLTAGE_2900,
};

struct REGULATOR_CTRL regulator_control[REGULATOR_TYPE_MAX_NUM] = {
	{"vcama"},
	{"vcamd"},
	{"vcamio"},
	#ifdef VENDOR_EDIT
	//Feiping.Li@Cam.Drv, 20200102, add for porting 19040 sensor
	{"vcama1"},
	{"vcamaf"}
	#endif
};

static struct REGULATOR reg_instance;

#ifdef VENDOR_EDIT
/* Feiping@Cam.Drv, 20190921, add for 19169 AF*/
extern struct IMGSENSOR gimgsensor;
struct regulator *regulator_get_regVCAMAF(void)
{
	struct IMGSENSOR *pimgsensor = &gimgsensor;
	return regulator_get(&((pimgsensor->hw.common.pplatform_device)->dev), "vcamaf");
}
EXPORT_SYMBOL(regulator_get_regVCAMAF);
/* Feiping@Cam.Drv, 20200102, add for 19040 AF*/
struct regulator *regulator_get_regVCAMAF_19040(int sensor_idx)
{
	struct IMGSENSOR *pimgsensor = &gimgsensor;
	struct regulator *regulator_addr = NULL;
	char str_regulator_name[LENGTH_FOR_SNPRINTF];
	memset(str_regulator_name, 0, LENGTH_FOR_SNPRINTF);

	snprintf(str_regulator_name, sizeof(str_regulator_name), "cam%d_%s", sensor_idx, "vcamaf");
	regulator_addr = regulator_get(&((pimgsensor->hw.common.pplatform_device)->dev), str_regulator_name);
	if (IS_ERR(regulator_addr)) {
		printk("regulator address is error");
		regulator_addr = NULL;
	}

	return regulator_addr;
}
EXPORT_SYMBOL(regulator_get_regVCAMAF_19040);
#endif

#ifdef VENDOR_EDIT
/* Shipei.Chen@Cam.Drv, 20200416,  add the ldo3 for sensor porting!*/
int Cam_LDO_PowerOn_Fan53870(int ldo_num, int set_mv_avdd)
{
	if (is_fan53870_pmic())
	{
		fan53870_cam_ldo_set_voltage(ldo_num, set_mv_avdd);
		printk("cam1 set vcama on successed in regulator.c\n");
	} else {
		printk("cam1 set vcama on fail in regulator.c\n");
		return IMGSENSOR_RETURN_ERROR;
	}

	return IMGSENSOR_RETURN_SUCCESS;
}
int Cam_LDO_PowerDown_Fan53870(int ldo_num)
{
	if (is_fan53870_pmic())
	{
		fan53870_cam_ldo_disable(ldo_num);
		printk("set fan53870_cam_ldo off successed\n");
	} else {
		printk("set fan53870_cam_ldo off failed\n");
		return IMGSENSOR_RETURN_ERROR;
	}

	return IMGSENSOR_RETURN_SUCCESS;
}
EXPORT_SYMBOL(Cam_LDO_PowerOn_Fan53870);
EXPORT_SYMBOL(Cam_LDO_PowerDown_Fan53870);
#endif

static enum IMGSENSOR_RETURN regulator_init(
	void *pinstance,
	struct IMGSENSOR_HW_DEVICE_COMMON *pcommon)
{
	struct REGULATOR *preg = (struct REGULATOR *)pinstance;
	int type, idx;
	char str_regulator_name[LENGTH_FOR_SNPRINTF];

	for (idx = IMGSENSOR_SENSOR_IDX_MIN_NUM;
		idx < IMGSENSOR_SENSOR_IDX_MAX_NUM;
		idx++) {
		for (type = 0;
			type < REGULATOR_TYPE_MAX_NUM;
			type++) {
			memset(str_regulator_name, 0,
				sizeof(str_regulator_name));
			snprintf(str_regulator_name,
				sizeof(str_regulator_name),
				"cam%d_%s",
				idx,
				regulator_control[type].pregulator_type);
			preg->pregulator[idx][type] = regulator_get(
					&pcommon->pplatform_device->dev,
					str_regulator_name);
			if (preg->pregulator[idx][type] == NULL)
				PK_INFO("ERROR: regulator[%d][%d]  %s fail!\n",
						idx, type, str_regulator_name);
			atomic_set(&preg->enable_cnt[idx][type], 0);
		}
	}
	return IMGSENSOR_RETURN_SUCCESS;
}

static enum IMGSENSOR_RETURN regulator_release(void *pinstance)
{
	struct REGULATOR *preg = (struct REGULATOR *)pinstance;
	int type, idx;
	struct regulator *pregulator = NULL;
	atomic_t *enable_cnt = NULL;

	for (idx = IMGSENSOR_SENSOR_IDX_MIN_NUM;
		idx < IMGSENSOR_SENSOR_IDX_MAX_NUM;
		idx++) {

		for (type = 0; type < REGULATOR_TYPE_MAX_NUM; type++) {
			pregulator = preg->pregulator[idx][type];
			enable_cnt = &preg->enable_cnt[idx][type];
			if (pregulator != NULL) {
				for (; atomic_read(enable_cnt) > 0; ) {
					regulator_disable(pregulator);
					atomic_dec(enable_cnt);
				}
			}
		}
	}
	return IMGSENSOR_RETURN_SUCCESS;
}

#ifdef VENDOR_EDIT
//Feiping.Li@Cam.Drv, 20191231, add for solve camera can't use pm8008
static struct regulator *regulator_reinit(void *pinstance, int sensor_idx, int type)
{
    struct REGULATOR *preg = (struct REGULATOR *)pinstance;
    char str_regulator_name[LENGTH_FOR_SNPRINTF];
    struct IMGSENSOR *pimgsensor = &gimgsensor;
    snprintf(str_regulator_name,
            sizeof(str_regulator_name),
            "cam%d_%s",
            sensor_idx,
            regulator_control[type].pregulator_type);
    preg->pregulator[sensor_idx][type] =
        regulator_get(&((pimgsensor->hw.common.pplatform_device)->dev), str_regulator_name);
    pr_err("reinit %s regulator[%d][%d] = %pK\n", str_regulator_name,
            sensor_idx, type, preg->pregulator[sensor_idx][type]);
    msleep(10);
    if(IS_ERR(preg->pregulator[sensor_idx][type])){
        pr_err("regulator reinit fail %pK\n",preg->pregulator[sensor_idx][type]);
        return NULL;
    } else {
    	pr_err("regulator reinit success");
    	return preg->pregulator[sensor_idx][type];
    }
}
#endif

static enum IMGSENSOR_RETURN regulator_set(
	void *pinstance,
	enum IMGSENSOR_SENSOR_IDX   sensor_idx,
	enum IMGSENSOR_HW_PIN       pin,
	enum IMGSENSOR_HW_PIN_STATE pin_state)
{
	struct regulator     *pregulator;
	struct REGULATOR     *preg = (struct REGULATOR *)pinstance;
	int reg_type_offset = 0;
	int ret_mask = 0;
	atomic_t             *enable_cnt;

	#ifdef VENDOR_EDIT
	/* Shipei.Chen@Cam.Drv, 20200416,  add the ldo3 for sensor porting! */
	if ((pin == IMGSENSOR_HW_PIN_AVDD)
		&& (is_project(OPPO_20601) || is_project(OPPO_20660) ||is_project(OPPO_20605)))
	{
		if (sensor_idx == IMGSENSOR_SENSOR_IDX_MAIN) {
			if (pin_state == IMGSENSOR_HW_PIN_STATE_LEVEL_0) {
				ret_mask = Cam_LDO_PowerDown_Fan53870(MAIN_AVDD_LDO_NUM);
			} else if ((pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_0)
				&& (pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)) {
				ret_mask = Cam_LDO_PowerOn_Fan53870(MAIN_AVDD_LDO_NUM, MAIN_AVDD_VOLTAGE);
			}
		} else if (sensor_idx == IMGSENSOR_SENSOR_IDX_SUB) {
			if (pin_state == IMGSENSOR_HW_PIN_STATE_LEVEL_0) {
				ret_mask = Cam_LDO_PowerDown_Fan53870(SUB_AVDD_LDO_NUM);
			} else if ((pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_0)
				&& (pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)) {
				ret_mask = Cam_LDO_PowerOn_Fan53870(SUB_AVDD_LDO_NUM, SUB_AVDD_VOLTAGE);
			}
		} else if ((sensor_idx == IMGSENSOR_SENSOR_IDX_MAIN2)|| (sensor_idx == IMGSENSOR_SENSOR_IDX_SUB2) || (sensor_idx == IMGSENSOR_SENSOR_IDX_MAIN3)) {
			if (pin_state == IMGSENSOR_HW_PIN_STATE_LEVEL_0) {
				ret_mask = Cam_LDO_PowerDown_Fan53870(MAIN2_AVDD_LDO_NUM);
			} else if ((pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_0)
				&& (pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)) {
				ret_mask = Cam_LDO_PowerOn_Fan53870(MAIN2_AVDD_LDO_NUM, MAIN2_AVDD_VOLTAGE);
			}
		}

		if (ret_mask) {
			printk("Cam AVDD power on failed ret:%d", ret_mask);
		}

		return ret_mask;
	}

	if (pin == IMGSENSOR_HW_PIN_AVDD_1
		&& (is_project(OPPO_20601) || is_project(OPPO_20660) ||is_project(OPPO_20605)))
	{
		if (pin_state == IMGSENSOR_HW_PIN_STATE_LEVEL_0) {
			ret_mask = Cam_LDO_PowerDown_Fan53870(MAIN_AVDD1_LDO_NUM);
		} else if ((pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_0)
			&& (pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)) {
			ret_mask = Cam_LDO_PowerOn_Fan53870(MAIN_AVDD1_LDO_NUM, MAIN_AVDD1_VOLTAGE);
		}

		if (ret_mask) {
			printk("Cam AVDD_1 power on failed ret:%d", ret_mask);
		}

		return ret_mask;
	}

	if ((pin == IMGSENSOR_HW_PIN_DVDD)
		&& (is_project(OPPO_20601) || is_project(OPPO_20660) ||is_project(OPPO_20605)))
	{
		if (pin_state == IMGSENSOR_HW_PIN_STATE_LEVEL_0) {
			ret_mask = Cam_LDO_PowerDown_Fan53870(MAIN2_DVDD_LDO_NUM);
		} else if ((pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_0)
			&& (pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)) {
			ret_mask = Cam_LDO_PowerOn_Fan53870(MAIN2_DVDD_LDO_NUM, MAIN2_DVDD_VOLTAGE);
		}

		if (ret_mask) {
			printk("MAIN2_DVDD power on failed ret:%d", ret_mask);
		}

		return ret_mask;
	}
	#endif

	#ifndef VENDOR_EDIT
	/* Feiping.Li@Cam.Drv, 20200110, add for ois i2c error */
	if (pin > IMGSENSOR_HW_PIN_DOVDD   ||
	    pin < IMGSENSOR_HW_PIN_AVDD    ||
	    pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_0 ||
	    pin_state >= IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)
		return IMGSENSOR_RETURN_ERROR;
	#else
	if (pin > IMGSENSOR_HW_PIN_AVDD_AF   ||
	    pin < IMGSENSOR_HW_PIN_AVDD    ||
	    pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_0 ||
	    pin_state >= IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)
		return IMGSENSOR_RETURN_ERROR;
	#endif

	reg_type_offset = REGULATOR_TYPE_VCAMA;

	pregulator =
		preg->pregulator[sensor_idx][
			reg_type_offset + pin - IMGSENSOR_HW_PIN_AVDD];

	#ifdef VENDOR_EDIT
	//Feiping.Li@Cam.Drv, 20200102, add for porting 19040 sensor
	if (IS_ERR(pregulator)) {
		pregulator = regulator_reinit(preg, sensor_idx, reg_type_offset + pin - IMGSENSOR_HW_PIN_AVDD);
		if (IS_ERR(pregulator)) {
			return IMGSENSOR_RETURN_ERROR;
		}
	}
	#endif

	enable_cnt =
		&preg->enable_cnt[sensor_idx][
			reg_type_offset + pin - IMGSENSOR_HW_PIN_AVDD];

	if (pregulator) {
		if (pin_state != IMGSENSOR_HW_PIN_STATE_LEVEL_0) {
			if (regulator_set_voltage(pregulator,
				regulator_voltage[
				pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0],
				regulator_voltage[
				pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0])) {

				PK_PR_ERR(
				  "[regulator]fail to regulator_set_voltage, powertype:%d powerId:%d\n",
				  pin,
				  regulator_voltage[
				  pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0]);
			}

			#ifdef VENDOR_EDIT
			//Feiping.Li@Cam.Drv, 20200114, add for set pm8008 work in right mode
			if (!regulator_set_load(pregulator, 10000)) {
				PK_DBG("set load success");
			}
			#endif

			if (regulator_enable(pregulator)) {
				PK_PR_ERR(
				"[regulator]fail to regulator_enable, powertype:%d powerId:%d\n",
				pin,
				regulator_voltage[
				  pin_state - IMGSENSOR_HW_PIN_STATE_LEVEL_0]);
				return IMGSENSOR_RETURN_ERROR;
			}
			atomic_inc(enable_cnt);
		} else {
			if (regulator_is_enabled(pregulator))
				PK_DBG("[regulator]%d is enabled\n", pin);

			if (regulator_disable(pregulator)) {
				PK_PR_ERR(
					"[regulator]fail to regulator_disable, powertype: %d\n",
					pin);
				return IMGSENSOR_RETURN_ERROR;
			}
			atomic_dec(enable_cnt);
		}
	} else {
		PK_PR_ERR("regulator == NULL %d %d %d\n",
				reg_type_offset,
				pin,
				IMGSENSOR_HW_PIN_AVDD);
	}

	return IMGSENSOR_RETURN_SUCCESS;
}

static enum IMGSENSOR_RETURN regulator_dump(void *pinstance)
{
	struct REGULATOR *preg = (struct REGULATOR *)pinstance;
	int i, j;

	for (j = IMGSENSOR_SENSOR_IDX_MIN_NUM;
		j < IMGSENSOR_SENSOR_IDX_MAX_NUM;
		j++) {

		for (i = REGULATOR_TYPE_VCAMA;
		i < REGULATOR_TYPE_MAX_NUM;
		i++) {
			#ifdef VENDOR_EDIT
			//Feiping.Li@Cam.Drv, 20200114, add for fix kernel dump issue
			if (IS_ERR_OR_NULL(preg->pregulator[j][i])) {
				PK_DBG("regulator idx(%d, %d) alread release ", j, i);
				return IMGSENSOR_RETURN_SUCCESS;
			} else {
				if (regulator_is_enabled(preg->pregulator[j][i]) &&
					atomic_read(&preg->enable_cnt[j][i]) != 0)
					PK_DBG("index= %d %s = %d\n",
						j,
						regulator_control[i].pregulator_type,
						regulator_get_voltage(
						preg->pregulator[j][i]));
			}
			#else
			if (regulator_is_enabled(preg->pregulator[j][i]) &&
				atomic_read(&preg->enable_cnt[j][i]) != 0)
				PK_DBG("index= %d %s = %d\n",
					j,
					regulator_control[i].pregulator_type,
					regulator_get_voltage(
					preg->pregulator[j][i]));
			#endif
		}
	}
	return IMGSENSOR_RETURN_SUCCESS;
}

static struct IMGSENSOR_HW_DEVICE device = {
	.id        = IMGSENSOR_HW_ID_REGULATOR,
	.pinstance = (void *)&reg_instance,
	.init      = regulator_init,
	.set       = regulator_set,
	.release   = regulator_release,
	.dump      = regulator_dump
};

enum IMGSENSOR_RETURN imgsensor_hw_regulator_open(
	struct IMGSENSOR_HW_DEVICE **pdevice)
{
	*pdevice = &device;
	return IMGSENSOR_RETURN_SUCCESS;
}

