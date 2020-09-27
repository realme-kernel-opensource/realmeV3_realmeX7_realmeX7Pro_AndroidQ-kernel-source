/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#include <odm_mdtbo.h>                  // for customized_get_odm_mdtbo_index()
#include <platform/mt_gpio.h>		// for mt_get_gpio_in()
//#ifdef VENDOR_EDIT
/* Fuchun.Liao@BSP.CHG.Basic 2017/12/25 modify for multi db */
#include <oppo_project.h>
//#endif /* VENDOR_EDIT */

#define GPIO_111	(0x80000000 | 111)
#define GPIO_175	(0x80000000 | 175)
#define GPIO_HIGH	(1)
#define GPIO_LOW	(0)

/******************************************************************************
* NOTE: This function is for the internal projects of MT6768.
*        -----------------------------------------
*       | GPIO_111    GPIO_175    PCB_ID          |
*       |   Low          Low      EVB   (MTK0754) |
*       |   Low          High     Phone (MTK0719) |
*       |   High         Low      Phone (MTK0966) |
*       |   High         High     TBD             |
*        -----------------------------------------
*
*       Customers need to implement their own customized_get_odm_mdtbo_index()
*       function.
******************************************************************************/
static int customized_get_odm_mdtbo_index_oldcdt(void)
{
	int idx = 0;
	unsigned int project = 0, operator = 0;

	project = get_project();
	operator = get_Operator_Version();
	switch(project) {
		case OPPO_18531:
		case OPPO_18161:
		case OPPO_19391:
			idx = 1;
			break;
		case OPPO_18561:
		case OPPO_19531:
			idx = 2;
			break;
		case OPPO_18311:
			if (operator == OPERATOR_18328_ASIA_SIMPLE_NORMALCHG) {
				idx = 2;
			} else {
				idx = 0;
			}
			break;
		case OPPO_18011:
			idx = 1;
			break;
		case OPPO_19165:
		case OPPO_19166:
			idx = 0;
			break;
		case OPPO_19040:
			idx = 0;
			break;
		case OPPO_19131:
		case OPPO_19132:
		case OPPO_19133:
		case OPPO_19420:
		case OPPO_19421:
			idx = 0;
			break;
		default:
			idx = 0;
			break;
	}
	dprintf(CRITICAL, "lk db project:%d, operator:%d, idx:%d\n", project, operator, idx);
	return idx;

}

static int customized_get_odm_mdtbo_index_newcdt(void)
{
	int idx = 0;
	unsigned int dtsiNo = 0;

	dtsiNo = get_dtsiNo();
	switch(dtsiNo) {
		case OPPO_19040:
			idx = 0;
			break;
		case OPPO_19131:
		case OPPO_19132:
		case OPPO_19133:
		case OPPO_19420:
			idx = 0;
			break;
		case OPPO_20001:
		case OPPO_20002:
		case OPPO_20003:
		case OPPO_20200:
		case OPPO_20611:
		case OPPO_20610:
			idx = 0;
			break;
		case OPPO_20630:
		case OPPO_20631:
		case OPPO_20632:
		case OPPO_20633:
		case OPPO_20634:
		case OPPO_20635:
			idx = 0;
			break;
		case OPPO_20625:
		case OPPO_20626:
			idx = 1;
			break;
		default:
			idx = 0;
			break;
	}
	dprintf(CRITICAL, "lk new cdt DtsiNo:%d, idx:%d\n", dtsiNo, idx);
	return idx;
}

int customized_get_odm_mdtbo_index(void)
{
	if (is_new_cdt()) {
		return customized_get_odm_mdtbo_index_newcdt();
	} else {
		return customized_get_odm_mdtbo_index_oldcdt();
	}
}

