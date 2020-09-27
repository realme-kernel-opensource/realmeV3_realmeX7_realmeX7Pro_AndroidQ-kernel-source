/******************************************************************
** Copyright (C), 2004-2017, OPPO Mobile Comm Corp., Ltd.
** VENDOR_EDIT
** File: - oppo_project.h
** Description: Source file for oppo project.
** Version: 1.0
** Date : 2017/05/16
** Author: Bin.Li@EXP.BSP.bootloader.bootflow
**
** ------------------------------- Revision History:---------------
** libin 2017/05/16 1.0 build this module
*******************************************************************/
#ifndef _OPPO_PROJECT_NEWCDT_H_
#define _OPPO_PROJECT_NEWCDT_H_

#define OCP	6
#define PCB_MAX_LEN 8
#define MAX_RF_GPIOS	8
#define NOT_SUPPORTED 0
#define MAX_PROJECTS_SUPPORT 32
#define FEATURE_COUNT		10

#define PM_DEVICE_ERR (-1)
#define PM_DEVICE_0 0
#define PM_DEVICE_1 2

#define GPIO_DETECT	1
#define ADC_DETECT	2

enum PCB_VERSION_NEWCDT{
	PCB_UNKNOWN,
	PCB_VERSION_EVB1,
	PCB_VERSION_T0,
	PCB_VERSION_T1,
	PCB_VERSION_EVT1,
	PCB_VERSION_EVT2,
	PCB_VERSION_EVT3,
	PCB_VERSION_DVT1,
	PCB_VERSION_DVT2,
	PCB_VERSION_DVT3,
	PCB_VERSION_PVT1,
	PCB_VERSION_MP1,
	PCB_VERSION_MP2,
	PCB_VERSION_MP3,
	PCB_VERSION_MAX,
};

enum OPPO_PROJECT_NEWCDT {
    OPPO_UNKOWN_NEWCDT = 0,
    OPPO_19550 = 19550,
    OPPO_19551 = 19551,
    OPPO_19597 = 19597,
    OPPO_19357 = 19357,
    OPPO_19040 = 19040,
    OPPO_19131 = 19131,
    OPPO_19132 = 19132,
    OPPO_19133 = 19133,
    OPPO_19420 = 19420,
    OPPO_20601 = 20601,
    OPPO_20660 = 20660,
    OPPO_20605 = 20605,
    OPPO_19421 = 19421,
    OPPO_20611 = 20611,
    OPPO_20610 = 20610,
    OPPO_20613 = 20613,
    OPPO_20680 = 20680,
    OPPO_20686 = 20686,
    OPPO_20630 = 20630,
    OPPO_20631 = 20631,
    OPPO_20632 = 20632,
    OPPO_20633 = 20633,
    OPPO_20634 = 20634,
    OPPO_20635 = 20635,
    OPPO_20637 = 20637,
    OPPO_20638 = 20638,
    OPPO_20639 = 20639,
    OPPO_206B4 = 0x206B4,
    OPPO_206B7 = 0x206B7,
    OPPO_20625 = 20625,
    OPPO_20626 = 20626,
    OPPO_20001 = 20001,
    OPPO_20002 = 20002,
    OPPO_20003 = 20003,
    OPPO_20200 = 20200,
    OPPO_20075 = 20075,
    OPPO_20076 = 20076,
	
};

enum RF_SET_PROJECT_NEWCDT {
	RF_MODEL_UNKNOWN_NEWCDT = 0,
	RF_MODEL_19550 = 19550,
	RF_MODEL_19551 = 19551,
	RF_MODEL_19597 = 19597,
	RF_MODEL_19357 = 19357,
	RF_MODEL_19040 = 19040,
    RF_MODEL_19131 = 19131,
    RF_MODEL_19132 = 19132,
    RF_MODEL_19133 = 19133,
    RF_MODEL_19420 = 19420,
    RF_MODEL_20601 = 20601,
    RF_MODEL_20660 = 20660,
    RF_MODEL_20605 = 20605,
    RF_MODEL_19421 = 19421,
    RF_MODEL_20611 = 20611,
    RF_MODEL_20610 = 20610,
    RF_MODEL_20613 = 20613,
    RF_MODEL_20680 = 20680,
    RF_MODEL_20686 = 20686,
    RF_MODEL_20630 = 20630,
    RF_MODEL_20631 = 20631,
    RF_MODEL_20632 = 20632,
    RF_MODEL_20633 = 20633,
    RF_MODEL_20634 = 20634,
    RF_MODEL_20635 = 20635,
    RF_MODEL_20637 = 20637,
    RF_MODEL_20638 = 20638,
    RF_MODEL_20639 = 20639,
    RF_MODEL_206B4 = 0x206B4,
    RF_MODEL_206B7 = 0x206B7,
    RF_MODEL_20625 = 20625,
    RF_MODEL_20626 = 20626,
    RF_MODEL_20001 = 20001,
    RF_MODEL_20002 = 20002,
    RF_MODEL_20003 = 20003,
    RF_MODEL_20075 = 20075,
    RF_MODEL_20200 = 20200,
};

enum OPPO_OPERATOR_NEWCDT {
    OPERATOR_UNKOWN_NEWCDT                   	= 0,
};

#define MT_SET_GPIO_AS_INPUT_UP_NEWCDT(pin) \
    do{\
        mt_set_gpio_mode(pin, GPIO_MODE_00);\
        mt_set_gpio_dir(pin, GPIO_DIR_IN);\
        mt_set_gpio_pull_select(pin, GPIO_PULL_UP);\
        mt_set_gpio_pull_enable(pin, GPIO_PULL_ENABLE);\
    }while(0)

#define MT_SET_GPIO_AS_INPUT_DOWN_NEWCDT(pin) \
    do{\
        mt_set_gpio_mode(pin, GPIO_MODE_00);\
        mt_set_gpio_dir(pin, GPIO_DIR_IN);\
        mt_set_gpio_pull_select(pin, GPIO_PULL_DOWN);\
        mt_set_gpio_pull_enable(pin, GPIO_PULL_ENABLE);\
    }while(0)

#define MT_SET_GPIO_AS_INPUT_NOPULL_NEWCDT(pin) \
	do{\
		mt_set_gpio_mode(pin, GPIO_MODE_00);\
		mt_set_gpio_dir(pin, GPIO_DIR_IN);\
		mt_set_gpio_pull_select(pin, GPIO_PULL_DOWN);\
		mt_set_gpio_pull_enable(pin, GPIO_PULL_DISABLE);\
	}while(0)

typedef struct
{
	unsigned int		nVersion;
	unsigned int		nProject;
	unsigned int		nDtsi;
	unsigned int		nAudio;
	unsigned int		nRF;
	unsigned int		nFeature[FEATURE_COUNT];
	unsigned int		nOppoBootMode;
	unsigned int		nPCB;
	unsigned int		newcdt;
	u8				nPmicOcp[OCP];
	u8				reserved[16];
} ProjectInfoCDTType;

typedef struct
{
	unsigned int		eng_version;
	unsigned int		is_confidential;
}EngInfoType;

typedef struct {
	u32 GPIO;
	u8 Idx;
} GPIO_Comb;

struct OPPO_PCB_Info {
	u8 Type;
	u8 Channel; /*if Type is PMIC_DETECT, read from ADC*/
	u32 Ranges[32];
	GPIO_Comb PCB_GPIOs[4]; /*if Type is GPIO_DETECT, read from gpio*/
	u32 Values[16];
	u8 *Name[16];
};

struct OPPO_RF_Info {
	GPIO_Comb RF_GPIOs[MAX_RF_GPIOS];
};

struct OPPO_Board_Info {
	u32 SupportProjects[MAX_PROJECTS_SUPPORT];
	struct OPPO_PCB_Info PCB_Info;
	struct OPPO_RF_Info  RF_Info;
};

static struct OPPO_Board_Info OPPO19551_Param = {
	/*project*/
	.SupportProjects = {19551},
	/*pcb information*/
	.PCB_Info = {
		.Type = GPIO_DETECT,
		/*
		.Channel = ADC_INPUT_PCB_GPIO,
		.Ranges = {0, 80,
				81, 180,
				181, 300,
				301, 420,
				421, 550,
				551, 650,
				651, 760,
				761, 850,
				851, 950,
				951, 1050,
				1051, 1150,
				1151, 1300},
		*/

		.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 37,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 36,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO19597_Param = {
	/*project*/
	.SupportProjects = {19597},
	/*pcb information*/
	.PCB_Info = {
		.Type = GPIO_DETECT,
		/*
		.Channel = ADC_INPUT_PCB_GPIO,
		.Ranges = {0, 80,
				81, 180,
				181, 300,
				301, 420,
				421, 550,
				551, 650,
				651, 760,
				761, 850,
				851, 950,
				951, 1050,
				1051, 1150,
				1151, 1300},
		*/

		.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 37,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 36,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO19040_Param = {
	/*project*/
	.SupportProjects = {19040},
	/*pcb information*/
	.PCB_Info = {
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0, 80,
				81, 180,
				181, 300,
				301, 420,
				421, 550,
				551, 650,
				651, 760,
				761, 850,
				851, 950,
				951, 1050,
				1051, 1150,
				1151, 1300},
		

		/*.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},*/

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 37,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 36,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20601_Param = {
	/*project*/
	.SupportProjects = {20601},
	/*pcb information*/
	.PCB_Info = {
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0, 800, //EVB1
				801, 950, //T0
				951, 1150,//T0-1
				1151, 1230, //EVT1
				1231, 1320, //EVT2
				1321, 1450, //DVT1
				1451, 1550, //DVT2
				1551, 1700, //PVT
				1701, 1800, //MP
				1801, 1900,
				1901, 2000,
				2001, 2100},
		

		/*.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},*/

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 37,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 36,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20660_Param = {
	/*project*/
	.SupportProjects = {20660},
	/*pcb information*/
	.PCB_Info = {
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0, 80,
				81, 180,
				181, 300,
				301, 420,
				421, 550,
				551, 650,
				651, 760,
				761, 850,
				851, 950,
				951, 1050,
				1051, 1150,
				1151, 1300},
		

		/*.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},*/

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 37,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 36,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO19131_Param = {
	/*project*/
	.SupportProjects = {19131},
	/*pcb information*/
	.PCB_Info = {
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},

		/*.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},*/

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 63,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 64,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 65,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 66,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO19132_Param = {
	/*project*/
	.SupportProjects = {19132},
	/*pcb information*/
	.PCB_Info = {
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},

		/*.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},*/

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 63,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 64,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 65,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 66,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO19133_Param = {
	/*project*/
	.SupportProjects = {19133},
	/*pcb information*/
	.PCB_Info = {
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},

		/*.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},*/

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 63,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 64,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 65,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 66,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO19420_Param = {
	/*project*/
	.SupportProjects = {19420},
	/*pcb information*/
	.PCB_Info = {
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},

		/*.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},*/

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 63,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 64,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 65,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 66,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO19421_Param = {
	/*project*/
	.SupportProjects = {19421},
	/*pcb information*/
	.PCB_Info = {
		.Type = ADC_DETECT,

		.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},

		/*.PCB_GPIOs = {
			{
			.GPIO = 115,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 122,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 125,	//PCB_ID0
			.Idx = PM_DEVICE_1,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},*/

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 63,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 64,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 65,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 66,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20001_Param = {
	/*project*/
	.SupportProjects = {20001},
	/*pcb information*/
	.PCB_Info = {
		.Type = GPIO_DETECT,
		
		/*.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},*/

		.PCB_GPIOs = {
			{
			.GPIO = 90,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 89,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 88,	//PCB_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 166,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 167,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 168,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 169,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20002_Param = {
	/*project*/
	.SupportProjects = {20002},
	/*pcb information*/
	.PCB_Info = {
		.Type = GPIO_DETECT,
		
		/*.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},*/

		.PCB_GPIOs = {
			{
			.GPIO = 90,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 89,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 88,	//PCB_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 166,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 167,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 168,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 169,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20003_Param = {
	/*project*/
	.SupportProjects = {20003},
	/*pcb information*/
	.PCB_Info = {
		.Type = GPIO_DETECT,
		
		/*.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},*/

		.PCB_GPIOs = {
			{
			.GPIO = 90,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 89,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 88,	//PCB_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 166,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 167,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 168,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 169,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20200_Param = {
	/*project*/
	.SupportProjects = {20200},
	/*pcb information*/
	.PCB_Info = {
		.Type = GPIO_DETECT,
		
		/*.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},*/

		.PCB_GPIOs = {
			{
			.GPIO = 90,	//PCB_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 89,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 88,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 166,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 167,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 168,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 169,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20611_Param = {
	/*project*/
	.SupportProjects = {20611, 20610, 20613},
	/*pcb information*/
	.PCB_Info = {
	//	.Type = GPIO_DETECT,
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0,732, //evb1
				733,905,//t0
				906,1086, //evt1
				1087,1200,//evt2
				1201,1392, //dvt1
				1393,1517, //dvt2
				1518,1639,//pvt1
				1640,1748,//mp1
				1749,1850,
				2181,2290},
              /*
		.PCB_GPIOs = {
			{
			.GPIO = 90,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 89,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 88,	//PCB_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},
            */
		.Name = {"EVB1", "T0", "EVT1", "EVT2", "DVT1", "DVT2", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 166,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 167,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 168,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 169,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20630_Param = {
	/*project*/
	.SupportProjects = {20630, 20631, 20632, 20633, 20634, 20635},
	/*pcb information*/
	.PCB_Info = {
	//	.Type = GPIO_DETECT,
		.Type = ADC_DETECT,
		
		.Channel = 10,
		.Ranges = {0,732, //evb1
				733,905,//t0
				906,1086, //evt1
				1087,1200,//evt2
				1201,1392, //dvt1
				1393,1517, //dvt2
				1518,1639,//pvt1
				1640,1748,//mp1
				1749,1850,
				2181,2290},
              /*
		.PCB_GPIOs = {
			{
			.GPIO = 90,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 89,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 88,	//PCB_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},
            */
		.Name = {"EVB1", "T0", "EVT1", "EVT2", "DVT1", "DVT2", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 166,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 167,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 168,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 169,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20680_Param = {
	/*project*/
	.SupportProjects = {20680},
	/*pcb information*/
	.PCB_Info = {
		.Type = GPIO_DETECT,
		
		/*.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},*/

		.PCB_GPIOs = {
			{
			.GPIO = 90,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 89,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 88,	//PCB_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 166,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 167,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 168,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 169,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info OPPO20686_Param = {
	/*project*/
	.SupportProjects = {20686},
	/*pcb information*/
	.PCB_Info = {
		.Type = GPIO_DETECT,
		
		/*.Channel = 10,
		.Ranges = {0,700,
				724,732,//t0
				742,752,
				894,905,//evt
				915,925,
				935,945,
				1072,1087,//dvt
				1097,1107,
				1117,1127,
				1369,1392,//pvt
				1491,1518,//mp
				1528,1538,
				1548,1558},*/

		.PCB_GPIOs = {
			{
			.GPIO = 90,	//PCB_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 89,	//PCB_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 88,	//PCB_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = NOT_SUPPORTED,
			},
		},

		.Name = {"EVB1", "T0", "T1", "EVT1", "EVT2", "EVT3", "DVT1", "DVT2", "DVT3", "PVT1", "MP1", "MP2", "MP3"}, // for ADC detect
	},
	/*rf information*/
	.RF_Info = {
		.RF_GPIOs = {
			{
			.GPIO = 166,  //RF_ID0
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 167,  //RF_ID1
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 168,  //RF_ID2
			.Idx = PM_DEVICE_ERR,
			},
			{
			.GPIO = 169,  //RF_ID3
			.Idx = PM_DEVICE_ERR,
			},
		},
	},

};

static struct OPPO_Board_Info *OPPO_Board_Params[] = {
	&OPPO19551_Param,
	&OPPO19597_Param,
	&OPPO19040_Param,
	&OPPO19131_Param,
	&OPPO19132_Param,
	&OPPO19133_Param,
	&OPPO19420_Param,
	&OPPO20601_Param,
	&OPPO20660_Param,
	&OPPO19421_Param,
	&OPPO20001_Param,
	&OPPO20002_Param,
	&OPPO20003_Param,
	&OPPO20200_Param,
	&OPPO20611_Param,
	&OPPO20680_Param,
	&OPPO20686_Param,
	&OPPO20630_Param,
	NULL,
};

extern unsigned rf_set_value;
extern unsigned rf_model_id;

int Init_Project_Info_newcdt(void);

unsigned int get_cdt_version_newcdt(void);

unsigned int get_project_newcdt(void);

unsigned int is_project_newcdt(int project);

unsigned int get_dtsiNo_newcdt(void);

unsigned int get_audio_project_newcdt(void);

unsigned int get_Modem_Version_newcdt(void);

unsigned int get_PCB_Version_newcdt(void);

unsigned int get_oppo_feature_newcdt(unsigned int feature_num);

int update_oppo_project_newcdt(void *fdt);

unsigned int get_eng_version_newcdt(void);

unsigned int is_confidential_newcdt(void);

unsigned int oppo_daily_build_newcdt(void);

int reflash_PCB_status(void);

#endif

