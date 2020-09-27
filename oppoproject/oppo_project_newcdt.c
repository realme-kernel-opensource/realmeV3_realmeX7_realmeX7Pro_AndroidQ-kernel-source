/******************************************************************
** Copyright (C), 2004-2017, OPPO Mobile Comm Corp., Ltd.
** VENDOR_EDIT
** File: - oppo_project.c
** Description: Source file for oppo project.
** Version: 1.0
** Date : 2017/05/16
** Author: Bin.Li@EXP.BSP.bootloader.bootflow
**
** ------------------------------- Revision History:---------------
** libin 2017/05/16 1.0 build this module
** libin 2017/07/07 1.0 add foy project 17321
** libin 2019/03/02 1.0 add foy project 18593 ET\18073 MOBILE(DVT)\18073 All BAND(DVT)
*******************************************************************/
#include <stdlib.h>
#include <string.h>
#include <platform/boot_mode.h>
#include <libfdt.h>
#include <stdio.h>
#include <platform/mt_gpt.h>
#include <platform/mtk_auxadc_sw.h>
#include <platform/mtk_auxadc_hw.h>
#include <printf.h>
#include <platform/errno.h>
#include <platform/mt_gpio.h>
#include <oppo_project_newcdt.h>

static ProjectInfoCDTType local_ProjectInfoCDT;
static ProjectInfoCDTType *g_project = NULL;
static EngInfoType g_enginfo = {0};

static void oppo_reset_gpio_state(GPIO_PIN gpio, int stat) {
	if (stat) {
		MT_SET_GPIO_AS_INPUT_UP_NEWCDT(gpio|0x80000000);
	} else {
		MT_SET_GPIO_AS_INPUT_DOWN_NEWCDT(gpio|0x80000000);
	}
}

static uint8_t oppo_get_hw_id(GPIO_PIN *gpios, int size, bool up, bool reset) {
	GPIO_PIN gpio;
	int i = 0;
	uint8_t hw_mask = 0x00;
	int value;
	if (size > 7) {
		printf("hardware lenth id must less than 8, return\n");
		return -EINVAL;
	}

	for(i = 0; i < size; i++) {
		gpio = gpios[i];
		if (up) {
			MT_SET_GPIO_AS_INPUT_UP_NEWCDT(gpio|0x80000000);
		} else {
			MT_SET_GPIO_AS_INPUT_DOWN_NEWCDT(gpio|0x80000000);
		}
	}
	mdelay(15);
	for(i = 0; i < size; i++) {
		gpio = gpios[i];
		value = mt_get_gpio_in(gpio|0x80000000);
		if (reset) {
			oppo_reset_gpio_state(gpio, value);
		}
		hw_mask |= (((uint8_t)value & 0x01) << (size - i -1));
	}

	return hw_mask;
}

#include <platform/mtk_charger_intf.h>
static struct mtk_charger_info *primary_mchr;
static int oppo_get_adc_value(int channel)
{
	int ret = 0, data[4], i, ret_value = 0, ret_temp = 0;
	int times = 5;

	dprintf(CRITICAL, "%s: channel = %d\n", __func__,channel);
	i = times;
	while (i--) {
		 //ret_value = iio_read_channel_processed(channel, &ret_temp);
		 //ret_value = IMM_GetOneChannelValue(channel, data, &ret_temp);
    	 primary_mchr = mtk_charger_get_by_name("primary_charger");
    	 if (!primary_mchr) {
    		dprintf(CRITICAL, "%s: get primary charger failed\n", __func__);
    		return 0;
    	 }
		 ret_value = mtk_charger_get_adc(primary_mchr,channel,&ret_temp);
         if (ret_value) {
    		dprintf(CRITICAL, "%s: mtk_charger_get_adc failed = %d\n", __func__,ret_value);
    		return 0;
    	 }
		 ret = ret+(ret_temp/1000);
		 dprintf(CRITICAL, "[get_hw_version_voltage(channel%d)]: ret_temp=%d, ret = %d\n",channel,ret_temp,ret);
	}

	//ret = ret * 1500 / 4096;
	ret = ret / times;

	return ret;
}

static int init_newcdt_info(void)
{
	int i = 0;
	
	if (!g_boot_arg) {
		return -1;
	}
	local_ProjectInfoCDT.nVersion = g_boot_arg->nVersion;
	local_ProjectInfoCDT.nProject = g_boot_arg->nProject_newcdt;
	local_ProjectInfoCDT.nDtsi = g_boot_arg->nDtsi;
	local_ProjectInfoCDT.nAudio = g_boot_arg->nAudio;
	local_ProjectInfoCDT.newcdt = g_boot_arg->newcdt;

	for (i = 0; i < FEATURE_COUNT; i++) {
		local_ProjectInfoCDT.nFeature[i] = g_boot_arg->nFeature[i];
	}

	g_enginfo.eng_version = g_boot_arg->eng_version;
	g_enginfo.is_confidential = g_boot_arg->is_confidential;
	
	printf("lk use new cdt local nVersion:%d nProject:%d nDtsi:%d nAudio:%d newcdt:%d Feature0-9:%d %d %d %d %d %d %d %d %d %d eng_version:%d is_confidential:%d \n",
		local_ProjectInfoCDT.nVersion, local_ProjectInfoCDT.nProject, local_ProjectInfoCDT.nDtsi, local_ProjectInfoCDT.nAudio, local_ProjectInfoCDT.newcdt,
		local_ProjectInfoCDT.nFeature[0], local_ProjectInfoCDT.nFeature[1], local_ProjectInfoCDT.nFeature[2], local_ProjectInfoCDT.nFeature[3],
		local_ProjectInfoCDT.nFeature[4], local_ProjectInfoCDT.nFeature[5], local_ProjectInfoCDT.nFeature[6], local_ProjectInfoCDT.nFeature[7],
		local_ProjectInfoCDT.nFeature[8], local_ProjectInfoCDT.nFeature[9],
		g_enginfo.eng_version, g_enginfo.is_confidential);
	return 0;
}

static void ConfigTlmmGpio(u16 GpioNumber, u8  GpioPull)
{
	if (GpioPull == GPIO_PULL_UP) {
		MT_SET_GPIO_AS_INPUT_UP(GpioNumber |0x80000000);
	} else if(GpioPull == GPIO_PULL_DOWN) {
		MT_SET_GPIO_AS_INPUT_DOWN(GpioNumber |0x80000000);
	} else if (GpioPull == GPIO_NO_PULL) {
		MT_SET_GPIO_AS_INPUT_NOPULL(GpioNumber |0x80000000);
	}
	mdelay(5);
}

static u32 ReadTlmmGPIO(UINT16 GpioNumber, UINT8  GpioPull)
{
	u32  value = 0;

	value = mt_get_gpio_in(GpioNumber |0x80000000);
	return value;
}

static u32 ReadGPIOValue(u16 GpioNumber, INT8 PmicIdx, u32 *Value)
{
	u32  Value_Up = 0, Value_Down = 0;
	u32 Status = 0;

	/*TLMM GPIO*/
	if (PmicIdx == PM_DEVICE_ERR) {
		/*pull down first*/
		ConfigTlmmGpio(GpioNumber, GPIO_PULL_DOWN);
		Value_Down = ReadTlmmGPIO(GpioNumber, GPIO_PULL_DOWN);

		/*pull up then*/
		ConfigTlmmGpio(GpioNumber, GPIO_PULL_UP);
		Value_Up = ReadTlmmGPIO(GpioNumber, GPIO_PULL_UP);

		/*reset tlmm gpio*/
		ConfigTlmmGpio(GpioNumber, GPIO_NO_PULL);

	} else { /*PMIC GPIO*/

	}
	printf("gpio %d read out down:%d up:%d\r\n", GpioNumber, Value_Down, Value_Up);

	if (Value_Down == 0 && Value_Up == 1)
		*Value = 0;
	if (Value_Down == 0 && Value_Up == 0)
		*Value = 1;
	if (Value_Down == 1 && Value_Up == 1)
		*Value = 2;

	return Status;

}

static u32 Result_From_GPIOs(GPIO_Comb *GPIOs, u32 *Result, u8 Size)
{
	u32 Status = 0;
	u32 Index = 0;
	u32 Value = 0;
	u32 GPIO = 0;
	INT8 PMIC_Idx = 0;
	u32 Exp = 1;

	for (Index = 0; Index < Size; Index++) {
		GPIO = (GPIOs + Index)->GPIO;
		PMIC_Idx = (GPIOs + Index)->Idx;

		if (GPIO == NOT_SUPPORTED) {
			break;
		}

		Status = ReadGPIOValue(GPIO, PMIC_Idx, &Value);
		if (Status) {
			printf( "failed to read gpio %d value\r\n", GPIO);
			goto exit;
		}

		printf("read gpio%d value:%d\r\n", GPIO, Value);

		*Result += (Value*Exp);
		Exp *= 3;
	}

exit:
	printf("Result_From_GPIOs Result: %d \r\n", *Result);
	return Status;

}

static u32 Result_From_GPIOs_2_Status(GPIO_Comb *GPIOs, u32 *Result, u8 Size, bool up)
{
	u32 Index = 0;
	u32 Value = 0;
	u32 GPIO = 0;
	INT8 PMIC_Idx = 0;
	u32 Exp = 1;


	for (Index = 0; Index < Size; Index++) {
		GPIO = (GPIOs + Index)->GPIO;
		PMIC_Idx = (GPIOs + Index)->Idx;

		if (GPIO == NOT_SUPPORTED) {
			break;
		}

		if (up) {
			MT_SET_GPIO_AS_INPUT_UP(GPIO|0x80000000);
		} else {
			MT_SET_GPIO_AS_INPUT_DOWN(GPIO|0x80000000);
		}
		mdelay(15);
		Value = mt_get_gpio_in(GPIO|0x80000000);
		MT_SET_GPIO_AS_INPUT_NOPULL(GPIO |0x80000000);

		printf("read gpio%d value:%d\r\n", GPIO, Value);

		*Result += (Value*Exp);
		Exp *= 2;
	}

exit:
	printf("Result_From_GPIOs_2_Status Result: %d \r\n", *Result);
	return 0;

}


static u32 Result_From_ADC(u8 Channel, u32 *Ranges, u32 *pcb_idx)
{
	int Index = 0, Value = 0, High = 0, Low = 0;;
	bool FOUND = FALSE;

	Value = oppo_get_adc_value(Channel);
	dprintf(CRITICAL, "oppo_get_pcb_version adc_value:%d, Channel:%d\n", Value, Channel);

	for (Index = 0; Index < 16; Index++)
	{
		Low = Ranges[2*Index];
		if (Low == 0 && Index != 0)
			break;

		if (Low < High) {
			FOUND = FALSE;
			break;
		}

		High = Ranges[2*Index + 1];
		if (Low > High) {
			FOUND = FALSE;
			break;
		}

		if (Low < Value && Value < High) {
			FOUND = TRUE;
			break;
		}
	}

	dprintf(CRITICAL, "FOUND is %d, Index is %d\n", FOUND, Index);
	if (!FOUND) {
		dprintf(CRITICAL, "Failed match ADC value \r\n");
		return 1;
	}

	*pcb_idx = Index;
	return 0;
}

static void Init_PCB_Info(struct OPPO_PCB_Info *Pcb_Info)
{
	u32 Status = 0;
	u32 Result = 0;
	u32 pcb_len = 0;
	u32 pcb_idx = 0;

	if (Pcb_Info->Type == GPIO_DETECT) {
		Status = Result_From_GPIOs(Pcb_Info->PCB_GPIOs, &pcb_idx, 4);
	} else {
		printf("Init_PCB_Info channel is %d\n", Pcb_Info->Channel);
		Status = Result_From_ADC(Pcb_Info->Channel, Pcb_Info->Ranges, &pcb_idx);
	}

	if (Status) {
		printf("Failed to read gpio/adc value\n");
		return;
	}

	if (pcb_idx > 15) {
		printf("result should not over 16\n");
		return;
	}
	if (Pcb_Info->Type == GPIO_DETECT) {
		switch(pcb_idx) {
		case PCB_UNKNOWN:
               Result = PCB_VERSION_PVT1;
		break;
		case PCB_VERSION_EVB1:
			Result = PCB_VERSION_MP1;
			break;
		case PCB_VERSION_T0:
			Result = PCB_VERSION_MP2;
			break;
		case PCB_VERSION_T1:
			Result = PCB_VERSION_MP3;
			break;
		case PCB_VERSION_EVT1:
			Result = PCB_VERSION_DVT3;
			break;
		case PCB_VERSION_EVT2:
			Result = PCB_VERSION_DVT2;
			break;
		case PCB_VERSION_EVT3:
			Result = PCB_VERSION_DVT1;
			break;
		case PCB_VERSION_DVT1:
			Result = PCB_VERSION_EVT3;
			break;
		case PCB_VERSION_DVT2:
			Result = PCB_VERSION_EVT2;
			break;
		case PCB_VERSION_DVT3:
			Result = PCB_VERSION_EVT1;
			break;
		case PCB_VERSION_PVT1:
			Result = PCB_VERSION_T1;
			break;
		case PCB_VERSION_MP1:
			Result = PCB_VERSION_T0;
			break;
		case PCB_VERSION_MP2:
			Result = PCB_VERSION_EVB1;
			break;
		default:
			Result = PCB_UNKNOWN;
		break;
        }
	} else {
		switch(pcb_idx+1) {
		case PCB_VERSION_EVB1:
			Result = PCB_VERSION_EVB1;
			break;
		case PCB_VERSION_T0:
			Result = PCB_VERSION_T0;
			break;
		case PCB_VERSION_T1:
			Result = PCB_VERSION_T1;
			break;
		case PCB_VERSION_EVT1:
			Result = PCB_VERSION_EVT1;
			break;
		case PCB_VERSION_EVT2:
			Result = PCB_VERSION_EVT2;
			break;
		case PCB_VERSION_EVT3:
			Result = PCB_VERSION_EVT3;
			break;
		case PCB_VERSION_DVT1:
			Result = PCB_VERSION_DVT1;
			break;
		case PCB_VERSION_DVT2:
			Result = PCB_VERSION_DVT2;
			break;
		case PCB_VERSION_DVT3:
			Result = PCB_VERSION_DVT3;
			break;
		case PCB_VERSION_PVT1:
			Result = PCB_VERSION_PVT1;
			break;
		case PCB_VERSION_MP1:
			Result = PCB_VERSION_MP1;
			break;
		case PCB_VERSION_MP2:
			Result = PCB_VERSION_MP2;
			break;
		case PCB_VERSION_MP3:
			Result = PCB_VERSION_MP3;
			break;
		default:
			Result = PCB_UNKNOWN;
			break;
        }
	}
	/*g_project should not be null*/
	if (Pcb_Info->Name[Result] != NULL) {
		pcb_len = sizeof(Pcb_Info->Name[Result]);
		if (pcb_len > PCB_MAX_LEN) {
			printf("PCB len overflow size %d > max_len %d\n", pcb_len, PCB_MAX_LEN);
			pcb_len = PCB_MAX_LEN;
		}
#if 0
		strncpy(g_project->nPCB,
			Pcb_Info->Name[Result],
			pcb_len); /*Name Len must lower than 8*/
		printf("PCB Result:%d len:%d, g_project->nPCB:%s\n", Result, pcb_len, g_project->nPCB);
#endif
		local_ProjectInfoCDT.nPCB = Result;
		printf("PCB Name found, nPCB:%d\n", local_ProjectInfoCDT.nPCB);
	} else {
		printf("PCB Name not specified, Result:%d\n", Result);
	}
}

static int Init_RF_Info(struct OPPO_RF_Info *RF_Info)
{
	u32 Result = 0, Status = 0;

	//Status = Result_From_GPIOs(RF_Info->RF_GPIOs, &Result, 4);//this is 3 status gpio: z=0 low=1 hight=2 
	Status = Result_From_GPIOs_2_Status(RF_Info->RF_GPIOs, &Result, 4, false);//this is 2 status gpio:low=0 hight=1 
	if (Status) {
		printf("Init_RF_Info failed Status:%d\n", Status);
		return Result;
	}
	local_ProjectInfoCDT.nRF = Result;
	rf_set_value = local_ProjectInfoCDT.nRF;
	rf_model_id = local_ProjectInfoCDT.nProject;
	switch(local_ProjectInfoCDT.nProject) {
		case OPPO_19550:
	               local_ProjectInfoCDT.nRF = 2;
	               rf_set_value = 2;
			break;
		case OPPO_19551:
	               local_ProjectInfoCDT.nRF = 5;
	               rf_set_value = 5;
			break;
		case OPPO_19597:
	               local_ProjectInfoCDT.nRF = 6;
	               rf_set_value = 6;
			break;
		case OPPO_19357:
	               local_ProjectInfoCDT.nRF = 12;
	               rf_set_value = 12;
			break;
		case OPPO_19131:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_19132:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_19133:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_19420:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_19421:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_20001:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_20002:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_20003:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_20200:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;
		case OPPO_20613:
		case OPPO_20611:
		case OPPO_20610:
		case OPPO_20630:
		case OPPO_20631:
		case OPPO_20632:
		case OPPO_20633:
		case OPPO_20634:
		case OPPO_20635:
		case OPPO_20637:
		case OPPO_20638:
		case OPPO_20639:
		case OPPO_206B4:
		case OPPO_206B7:
		case OPPO_20625:
		case OPPO_20626:
	               local_ProjectInfoCDT.nRF = rf_set_value;
			break;	
		default:
			rf_set_value = 4;
			break;
        }
	return Status;
}

int update_oppo_project_newcdt(void *fdt)
{
	int ret = 0, i = 0;
	int oppoproject_offset = 0;
	char feature_node[64] = {0};

	if (!g_project) {
		Init_Project_Info_newcdt();
	}

	if (!g_project) {
		printf("update_oppo_project_new_cdt g_project NULL\n");
		return -1;
	}
	oppoproject_offset = fdt_path_offset(fdt, "/odm/oppo_project");

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "newcdt", (unsigned int)g_project->newcdt);
	if (ret){
		printf("update_oppo_project set newcdt fail\n");
		return -1;
	}

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "nVersion", (unsigned int)g_project->nVersion);
	if (ret){
		printf("update_oppo_project set nVersion fail\n");
		return -1;
	}
	
	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "nProject", (unsigned int)g_project->nProject);
	if (ret){
		printf("update_oppo_project set nProject fail\n");
		return -1;
	}

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "nDtsi", (unsigned int)g_project->nDtsi);
	if (ret){
		printf("update_oppo_project set nDtsi fail\n");
		return -1;
	}

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "nAudio", (unsigned int)g_project->nAudio);
	if (ret){
		printf("update_oppo_project set nAudio fail\n");
		return -1;
	}

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "nRF", (unsigned int)g_project->nRF);
	if (ret){
		printf("update_oppo_project set nRF fail\n");
		return -1;
	}

	for (i = 0; i < FEATURE_COUNT; i++) {
		sprintf(feature_node, "nFeature%d", i);
		ret = fdt_appendprop_cell(fdt, oppoproject_offset, feature_node, (unsigned int)g_project->nFeature[i]);
		if (ret){
			printf("update_oppo_project set nFeature[%d]: %s %d fail\n", i, feature_node, g_project->nFeature[i]);
			return -1;
		}
	}

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "nOppoBootMode", (unsigned int)g_project->nOppoBootMode);
	if (ret){
		printf("update_oppo_project set nOppoBootMode fail\n");
		return -1;
	}

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "nPCB", (unsigned int)g_project->nPCB);
	if (ret){
		printf("update_oppo_project set nPCB fail\n");
		return -1;
	}

	ret = fdt_appendprop_string(fdt, oppoproject_offset, "nPmicOcp", (u8)g_project->nPmicOcp);
	if (ret){
		printf("update_oppo_project set nPCB fail\n");
		return -1;
	}

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "eng_version", (unsigned int)g_enginfo.eng_version);
	if (ret){
		printf("update_oppo_project set eng_version fail\n");
		return -1;
	}

	ret = fdt_appendprop_cell(fdt, oppoproject_offset, "is_confidential", (unsigned int)g_enginfo.is_confidential);
	if (ret){
		printf("update_oppo_project set is_confidential fail\n");
		return -1;
	}

	printf("update_oppo_project_newcdt newcdt:%d nVersion:%d nProject:%d nDtsi:%d nAudio:%d nPCB:%d nPmicOcp:%s eng_version:%d confidential:%d\n",
		g_project->newcdt, g_project->nVersion, g_project->nProject, g_project->nDtsi, g_project->nAudio, g_project->nPCB, g_project->nPmicOcp,
		g_enginfo.eng_version, g_enginfo.is_confidential);
	return 0;
}

int reflash_PCB_status(void)
{
	u32 Index = 0, Start = 0;
	bool Found = FALSE;
	struct OPPO_Board_Info *Param;

	for (Index = 0;; Index++) {
		Param = OPPO_Board_Params[Index];
		if (!Param) {
			break;
		}
		for (Start = 0; Start < MAX_PROJECTS_SUPPORT; Start++) {
			if (!Param->SupportProjects[Start]) {
				break;
			}
			if (Param->SupportProjects[Start] == local_ProjectInfoCDT.nDtsi) {
				Found = TRUE;
				break;
			}
		}
		if (Found)
			break;
	}

	printf("reflash_PCB_status: Found is %d\n", Found);

	if (!Found) {
		printf("ERROR: reflash_PCB_status Project Not Found \r\n");
		return -1;
	}

	Init_PCB_Info(&Param->PCB_Info);

	return g_project->nPCB;
}

int Init_Project_Info_newcdt(void)
{
	int ret = 0;
	u32 Index = 0, Start = 0;
	bool Found = FALSE;
	//struct OPPO_Board_Info *Params = OPPO_Board_Params;
	struct OPPO_Board_Info *Param;

	if (g_project) {
		printf("new cdt g_project has inited\n");
		return 0;
	}
	
	ret = init_newcdt_info();
	if (ret) {
		printf("Init_Project_Info_newcdt init_newcdt fail\n");
		return -1;
	}
	
	for (Index = 0;; Index++) {
		Param = OPPO_Board_Params[Index];
		if (!Param) {
			break;
		}
		for (Start = 0; Start < MAX_PROJECTS_SUPPORT; Start++) {
			if (!Param->SupportProjects[Start]) {
				break;
			}
			if (Param->SupportProjects[Start] == local_ProjectInfoCDT.nDtsi) {
				Found = TRUE;
				break;
			}
		}
		if (Found)
			break;
	}

	if (!Found) {
		printf("ERROR: Project Not Found \r\n");
		return -1;
	}

	Init_PCB_Info(&Param->PCB_Info);
	Init_RF_Info(&Param->RF_Info);
	g_project = &local_ProjectInfoCDT;
	
	printf("Project:%d Dsti:%d Audio:%d PCB:%d RF:%d BootMode:%x newcdt:%d Feature0-9:%d %d %d %d %d %d %d %d %d %d\n",
		g_project->nProject,
		g_project->nDtsi,
		g_project->nAudio,
		g_project->nPCB,
		g_project->nRF,
		g_project->nOppoBootMode,
		g_project->newcdt,
		g_project->nFeature[0],
		g_project->nFeature[1],
		g_project->nFeature[2],
		g_project->nFeature[3],
		g_project->nFeature[4],
		g_project->nFeature[5],
		g_project->nFeature[6],
		g_project->nFeature[7],
		g_project->nFeature[8],
		g_project->nFeature[9]
		);
	return 0;
}

unsigned int get_cdt_version_newcdt(void)
{
	return g_project ? g_project->nVersion:0;
}

unsigned int get_project_newcdt(void)
{
	return g_project ? g_project->nProject:0;
}

unsigned int is_project_newcdt(int project)
{
	return (get_project_newcdt() == project ? 1 : 0);
}

unsigned int get_dtsiNo_newcdt(void)
{
	return g_project?g_project->nDtsi:0;
}

unsigned int get_audio_project_newcdt(void)
{
	return g_project?g_project->nAudio:0;
}

unsigned int get_Modem_Version_newcdt(void)
{
	return g_project?g_project->nRF:0;
}

unsigned int get_PCB_Version_newcdt(void)
{
	return g_project ? g_project->nPCB : PCB_UNKNOWN;
}

unsigned int get_oppo_feature_newcdt(unsigned int feature_num)
{
	if (feature_num > (FEATURE_COUNT - 1))
		return 0;
	
	return g_project ? g_project->nFeature[feature_num] : 0;
}

unsigned int get_eng_version_newcdt(void)
{
	return g_enginfo.eng_version;
}

unsigned int is_confidential_newcdt(void)
{
	return g_enginfo.is_confidential;
}

unsigned int oppo_daily_build_newcdt(void)
{
	return 1;
}

