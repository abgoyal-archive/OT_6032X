#define LOG_TAG "CamCalCamCal"

#include <cutils/xlog.h> //#include <utils/Log.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <math.h>


//seanlin 120921 for 658x #include "MediaHal.h"
//#include "src/lib/inc/MediaLog.h" //#include "src/lib/inc/MediaLog.h"
//#include "camera_custom_nvram.h"
#include "camera_custom_nvram.h"
//cam_cal
#include "cam_cal.h"
#include "cam_cal_define.h"
extern "C"{
//#include "cam_cal_layout.h"
#include "camera_custom_cam_cal.h"
}
#include "camera_calibration_cam_cal.h"

#include <stdio.h> //for rand?
#include <stdlib.h>  //sl121106 for atoi()//for rand?
//#include "src/core/scenario/camera/mhal_cam.h" //for timer

//COMMON


#define DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW

#define CUSTOM_CAM_CAL_ROTATION_00_IMX132MIPIRAW CUSTOM_CAM_CAL_ROTATION_0_DEGREE    
#define CUSTOM_CAM_CAL_ROTATION_01_IMX132MIPIRAW CUSTOM_CAM_CAL_ROTATION_0_DEGREE    
#define CUSTOM_CAM_CAL_COLOR_ORDER_00_IMX132MIPIRAW CUSTOM_CAM_CAL_COLOR_SHIFT_00     //SeanLin@20110629: 
#define CUSTOM_CAM_CAL_COLOR_ORDER_01_IMX132MIPIRAW CUSTOM_CAM_CAL_COLOR_SHIFT_00 

//#define CUSTOM_CAM_CAL_PART_NUMBERS_START_ADD 5
//#define CUSTOM_CAM_CAL_NEW_MODULE_NUMBER_CHECK 1 //

#define CAM_CAL_SHOW_LOG_IMX132MIPIRAW 1
#define CAM_CAL_VER "ver8900~"   //83 : 6583, 00 : draft version 120920

#ifdef CAM_CAL_SHOW_LOG_IMX132MIPIRAW
//#define CAM_CAL_LOG_IMX132MIPIRAW(fmt, arg...)    LOGD(fmt, ##arg)
#define CAM_CAL_LOG_IMX132MIPIRAW(fmt, arg...)    XLOGD(CAM_CAL_VER " "fmt, ##arg)
#define CAM_CAL_ERR_IMX132MIPIRAW(fmt, arg...)    XLOGE(CAM_CAL_VER "Err: %5d: "fmt, __LINE__, ##arg)
#else
#define CAM_CAL_LOG_IMX132MIPIRAW(fmt, arg...)    void(0)
#define CAM_CAL_ERR_IMX132MIPIRAW(fmt, arg...)    void(0)
#endif
#define CAM_CAL_LOG_IF_IMX132MIPIRAW(cond, ...)      do { if ( (cond) ) { CAM_CAL_LOG_IMX132MIPIRAW(__VA_ARGS__); } }while(0)

////<		
UINT32 DoCamCalModuleVersion_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize,UINT32* pGetSensorCalData);
UINT32 DoCamCalPartNumber_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
//UINT32 DoCamCalShadingTable(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalSingleLsc_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalN3dLsc_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCalAWBGain_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal2AGain_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal3AGain_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
UINT32 DoCamCal3DGeo_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);


#if 1
//typedef enum

//}CAM_CAL_MODULE_TYPE;

#else
#define 	CALIBRATION_LAYOUT_SLIM_LSC1  0 //Legnacy module for 657x
#define 	CALIBRATION_LAYOUT_N3D_DATA1 1 //N3D module for 658x
#define 	CALIBRATION_LAYOUT_SUNNY_Q8N03D_LSC1 2  //SL 110317
#define 	MAX_CALIBRATION_LAYOUT_NUM 3
#endif

typedef enum // : MUINT32
{
    CAM_CAL_LAYOUT_RTN_PASS = 0x0,
    CAM_CAL_LAYOUT_RTN_FAILED = 0x1,
    CAM_CAL_LAYOUT_RTN_QUEUE = 0x2
} CAM_CAL_LAYOUT_T;

enum
{
	CALIBRATION_LAYOUT_SLIM_LSC1 = 0, //Legnacy module for 657x
	CALIBRATION_LAYOUT_N3D_DATA1, //N3D module for 658x
	CALIBRATION_LAYOUT_SUNNY_Q8N03D_LSC1,  //SL 110317
    CALIBRATION_LAYOUT_SENSOR_OTP,
	MAX_CALIBRATION_LAYOUT_NUM
};

typedef struct
{
    UINT16 Include; //calibration layout include this item?
    UINT32 StartAddr; // item Start Address
    UINT32 BlockSize;   //BlockSize
    UINT32 (*GetCalDataProcess)(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);//(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData);
} CALIBRATION_ITEM_STRUCT;

typedef struct
{
	UINT32 HeaderAddr; //Header Address
	UINT32 HeaderId;   //Header ID
	UINT32 DataVer;   ////new for 658x CAM_CAL_SINGLE_EEPROM_DATA, CAM_CAL_SINGLE_OTP_DATA,CAM_CAL_N3D_DATA
//seanlin 121016 for 658x 	UINT32 CheckShading; // Do check shading ID?
//seanlin 121016 for 658x 	UINT32 ShadingID;    // Shading ID
	CALIBRATION_ITEM_STRUCT CalItemTbl[CAMERA_CAM_CAL_DATA_LIST];
} CALIBRATION_LAYOUT_STRUCT;

//static UINT8 gIsInitedCamCal = 0;//seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
const MUINT8 CamCalPartNumber_IMX132MIPIRAW[24]={0x57,0x61,0x6E,0x70,0x65,0x69,0x20,0x4C,0x69,0x61,0x6E,0x67,
	                                                       0x20,0x53,0x6F,0x70,0x68,0x69,0x65,0x52,0x79,0x61,0x6E,0x00};

const CALIBRATION_LAYOUT_STRUCT CalLayoutTbl_IMX132MIPIRAW[MAX_CALIBRATION_LAYOUT_NUM]=
{
	{//CALIBRATION_LAYOUT_SLIM_LSC1 without Defect //data sheet of excel : "Slim"
		0x00000000, 0x010200FF, CAM_CAL_SINGLE_EEPROM_DATA,
		{
			{0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
			{0x00000001, 0x00000000, 0x00000000, DoCamCalPartNumber_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
			{0x00000001, 0x0000000C, 0x00000284, DoCamCalSingleLsc_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
			{0x00000001, 0x00000004, 0x00000008, DoCamCalAWBGain_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_3A_GAIN
			{0x00000000, 0x00000000, 0x00000000, DoCamCal3DGeo_IMX132MIPIRAW}  //CAMERA_CAM_CAL_DATA_3D_GEO
		}
	},
	{//CALIBRATION_LAYOUT_N3D //data sheet of excel : "3D_EEPROM 8M+2M _0A_2"
		0x00000000, 0x020A00FF,CAM_CAL_N3D_DATA,
		{
			{0x00000001, 0x00000002, 0x00000000, DoCamCalModuleVersion_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
			{0x00000001, 0x00000000, 0x00000018, DoCamCalPartNumber_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
			{0x00000001, 0x1480009C, 0x00000840, DoCamCalN3dLsc_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
			{0x00000001, 0x1400001C, 0x00000080, DoCamCal3AGain_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_3A_GAIN
			{0x00000001, 0x00000A00, 0x00000898, DoCamCal3DGeo_IMX132MIPIRAW}  //CAMERA_CAM_CAL_DATA_3D_GEO
		}
	},	
	{//CALIBRATION_LAYOUT_SUNY
		0x00000000, 0x796e7573, CAM_CAL_SINGLE_EEPROM_DATA,
		{		
			{0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
			{0x00000001, 0x00000000, 0x00000000, DoCamCalPartNumber_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
			{0x00000001, 0x0000000C, 0x00000284, DoCamCalSingleLsc_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
			{0x00000001, 0x00000004, 0x00000008, DoCamCalAWBGain_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_3A_GAIN
			{0x00000000, 0x00000000, 0x00000000, DoCamCal3DGeo_IMX132MIPIRAW}  //CAMERA_CAM_CAL_DATA_3D_GEO
		}
	},	
	{//CALIBRATION_LAYOUT_SENSOR_OTP
		0x00000001, 0x010b00ff, CAM_CAL_SINGLE_OTP_DATA,
		{		
			{0x00000001, 0x00000000, 0x00000000, DoCamCalModuleVersion_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_MODULE_VERSION
			{0x00000001, 0x00000005, 0x00000002, DoCamCalPartNumber_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_PART_NUMBER
			{0x00000001, 0x00000017, 0x00000001, DoCamCalSingleLsc_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_SHADING_TABLE
			{0x00000001, 0x00000007, 0x0000000E, DoCamCal2AGain_IMX132MIPIRAW}, //CAMERA_CAM_CAL_DATA_3A_GAIN
			{0x00000000, 0x00000000, 0x00000000, DoCamCal3DGeo_IMX132MIPIRAW}  //CAMERA_CAM_CAL_DATA_3D_GEO
		}
	}	
};

//static CAM_CAL_LAYOUT_T gIsInitedCamCal = CAM_CAL_LAYOUT_RTN_QUEUE;//seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
static UINT16 LayoutType = (MAX_CALIBRATION_LAYOUT_NUM+1); //seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
static bool bFirstLoad = TRUE;    
static MINT32 dumpEnable=0;

static CAM_CAL_LAYOUT_T  gIsInitedCamCal = CAM_CAL_LAYOUT_RTN_QUEUE;//(CAM_CAL_LAYOUT_T)CAM_CAL_LAYOUT_RTN_QUEUE;//seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
//MUINT32 gIsInitedCamCal = CAM_CAL_LAYOUT_RTN_QUEUE;//(CAM_CAL_LAYOUT_T)CAM_CAL_LAYOUT_RTN_QUEUE;//seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.

UINT32 ShowCmdErrorLog_IMX132AW(CAMERA_CAM_CAL_TYPE_ENUM cmd)
{
       CAM_CAL_ERR_IMX132MIPIRAW("Return ERROR %s\n",CamCalErrString[cmd]);
       return 0;
}
#if 0 //for test, no use currently
UINT32 DoReadDataByCmd(CAMERA_CAM_CAL_TYPE_ENUM Command, NT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize,UINT8* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    UINT32 ioctlerr, err;
    cam_calCfg.u4Offset = start_addr;
    cam_calCfg.u4Length = BlockSize; //sizeof(ucModuleNumber)
    cam_calCfg.pu1Params= pGetSensorCalData;
    ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
    if(!ioctlerr)
    {
        err = CAM_CAL_ERR_NO_ERR;
    }
    else
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
        ShowCmdErrorLog_IMX132AW(Command);
    }
    return err;
}
#endif //for test, no use currently
UINT32 DoCamCalModuleVersion_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize,UINT32* pGetSensorCalData)
{
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    UINT32 err=  0;
    /*
    if(start_addr<CAM_CAL_TYPE_NUM)
    {
        pCamCalData->DataVer = start_addr;    
    }
    else
    {
       err =  CamCalReturnErr[pCamCalData->Command];       
       ShowCmdErrorLog_IMX132AW(pCamCalData->Command);

    }*/
    #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================Module version==================\n");
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[DataVer] = 0x%x\n", pCamCalData->DataVer);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"RETURN = 0x%x \n", err);            
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================Module version==================\n");
    #endif    
    return err;
}

UINT32 DoCamCalPartNumber_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    MUINT32 idx;    
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];     
    UINT8 ucModuleNumber[CAM_CAL_PART_NUMBERS_COUNT_BYTE]={0,0,0,0,0,0,0,0,0,0,0,0,
    	                                                                                                     0,0,0,0,0,0,0,0,0,0,0,0};
    if(BlockSize==(CAM_CAL_PART_NUMBERS_COUNT_BYTE))
    {
        cam_calCfg.u4Offset = start_addr;
        cam_calCfg.u4Length = CAM_CAL_PART_NUMBERS_COUNT_BYTE; //sizeof(ucModuleNumber)
        cam_calCfg.pu1Params= (u8 *)&ucModuleNumber[0];
        ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
        if(!ioctlerr)
        {
            err = CAM_CAL_ERR_NO_ERR;
        }
        else
        {
            CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
            ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
        }
    }
    else
    {
        CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"use default part number\n");        
        srand(time(NULL));
        for(idx=0;idx<(CAM_CAL_PART_NUMBERS_COUNT*LSC_DATA_BYTES);idx++)
        {   
            ucModuleNumber[idx]=CamCalPartNumber_IMX132MIPIRAW[idx];
            if(ucModuleNumber[idx] ==0x20)
            {            
                //disable random> TBD
                //ucModuleNumber[idx] = (UINT32)rand(); //random
                //disable random< TBD
            }            
        }
        err = CAM_CAL_ERR_NO_ERR;
    }
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"%s\n",ucModuleNumber);       
    memcpy((char*)&pCamCalData->PartNumber[0],ucModuleNumber,sizeof(CAM_CAL_PART_NUMBERS_COUNT_BYTE));
    #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================Part Number==================\n");
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[Part Number] = %s\n", pCamCalData->PartNumber);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"RETURN = 0x%x \n", err);            
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================Part Number==================\n");
    #endif        
    return err;
}

UINT32 DoCamCalAWBGain_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    MUINT32 idx;    
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];     
    UINT32 PregainFactor, PregainOffset;
    UINT32 PregainFactorH, PregainOffsetH;
    UINT32 GainValue;
    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
        ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
    }
    else if(pCamCalData->DataVer < CAM_CAL_N3D_DATA)
    { 
        if(BlockSize!=CAM_CAL_SINGLE_AWB_COUNT_BYTE)
        {
            CAM_CAL_ERR_IMX132MIPIRAW("BlockSize(%d) is not correct (%d)\n",BlockSize,CAM_CAL_SINGLE_AWB_COUNT_BYTE);
            ShowCmdErrorLog_IMX132AW(pCamCalData->Command);            
        }
        else
        {
            ////Only AWB Gain without AF>////
            pCamCalData->Single2A.S2aVer = 0x01;
            pCamCalData->Single2A.S2aBitEn = CAM_CAL_AWB_BITEN;
            pCamCalData->Single2A.S2aAfBitflagEn = 0x0;// //Bit: step 0(inf.), 1(marco), 2, 3, 4,5,6,7
            memset(pCamCalData->Single2A.S2aAf,0x0,sizeof(pCamCalData->Single2A.S2aAf));
            ////Only AWB Gain without AF<////
            ////Only AWB Gain Gathering >////
            cam_calCfg.u4Offset = start_addr|0xFFFF;
            cam_calCfg.u4Length = 4;
            cam_calCfg.pu1Params = (u8 *)&PregainFactor;            
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else        
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }            
            cam_calCfg.u4Offset = start_addr+4;
            cam_calCfg.u4Length = 4;
            cam_calCfg.pu1Params = (u8 *)&PregainOffset; 
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else        
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }            

            PregainFactorH = ((PregainFactor>>16)&0xFFFF);
            PregainOffsetH = ((PregainOffset>>16)&0xFFFF);
            if((PregainOffset==0)||(PregainOffsetH==0))
            {
                //pre gain	
                pCamCalData->Single2A.S2aAwb.rUnitGainu4R = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4G = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4B  = 512;
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"Pegain has no Calinration Data!!!\n");                
            }
            else
            {
                //pre gain	
                pCamCalData->Single2A.S2aAwb.rUnitGainu4R = 
                                                                           (((PregainFactor&0xFF)<<8)|
                                                                       ((PregainFactor&0xFF00)>>8))*512 /
                                                                           (((PregainOffset&0xFF)<<8)|
                                                                       ((PregainOffset&0xFF00)>>8));
                pCamCalData->Single2A.S2aAwb.rUnitGainu4G = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4B  =
                                                                           (((PregainFactorH&0xFF)<<8)|
                	                                                 ((PregainFactorH&0xFF00)>>8))*512/
                	                                                     (((PregainOffsetH&0xFF)<<8)|
                	                                                 ((PregainOffsetH&0xFF00)>>8));
            	err=0;
            }

            if((pCamCalData->Single2A.S2aAwb.rUnitGainu4R==0)||(pCamCalData->Single2A.S2aAwb.rUnitGainu4B==0))
            {
                //pre gain	
                pCamCalData->Single2A.S2aAwb.rUnitGainu4R = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4G = 512;
                pCamCalData->Single2A.S2aAwb.rUnitGainu4B  = 512;
                CAM_CAL_ERR_IMX132MIPIRAW("RGB Gain is not reasonable!!!\n");       
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }    
            ////Only AWB Gain Gathering <////
            #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
            CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================AWB CAM_CAL==================\n");
            CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[CAM_CAL PREGAIN VALUE] = 0x%x\n", PregainFactor);
            CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[CAM_CAL PREGAIN OFFSET] = 0x%x\n", PregainOffset);
            CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
            CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rCalGain.u4G] = %d\n",  pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
            CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);	
            CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================AWB CAM_CAL==================\n");
            #endif
////////////////////////////////////////////////////////////////////////////////        
        }        
    }    
    return err;
}

/********************************************************/
//Please put your AWB+AF data funtion, here.
/********************************************************/
UINT32 DoCamCal2AGain_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
	stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    MUINT32 idx;    
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];     
    UINT32 GainValue;
	
	UINT32 CalGain, FacGain;
	UINT8 AWBAFConfig;
	UINT32 AFInf, AFMacro;
	UINT8 tempMin = 255;
	UINT8 tempGain, CalR, CalGr, CalGb, CalB, FacR, FacGr, FacGb, FacB;
	
	//Structure
	/*
	Byte[0]:Version
	Byte[1]: [x,x,x,x,enbAFMacro, enbAFInf, enbAF, enbWB]
	Byte[9:2]: {GoldenB, GoldenGb, GoldenGr, GoldenR, UnitB, UnitGb, UnitGr, UnitR}
	Byte[11:10]: AF inf.
	Byte[13:12]: AF Macro
	*/
    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
        ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
    }
    else if(pCamCalData->DataVer < CAM_CAL_N3D_DATA)
    { 
        if(BlockSize!=14)
        {
            CAM_CAL_ERR_IMX132MIPIRAW("BlockSize(%d) is not correct (%d)\n",BlockSize,14);
            ShowCmdErrorLog_IMX132AW(pCamCalData->Command);            
        }
        else
        {
			// Check the config. for AWB & AF
			cam_calCfg.u4Offset = (start_addr+1);
            cam_calCfg.u4Length = 1;
            cam_calCfg.pu1Params = (u8 *)&AWBAFConfig;            
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else        
            {
                pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            } 

			pCamCalData->Single2A.S2aVer = 0x01;
            pCamCalData->Single2A.S2aBitEn = (0x03 & AWBAFConfig);
			CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"S2aBitEn=0x%x", pCamCalData->Single2A.S2aBitEn);
            pCamCalData->Single2A.S2aAfBitflagEn = (0x0C & AWBAFConfig);// //Bit: step 0(inf.), 1(marco), 2, 3, 4,5,6,7
            //memset(pCamCalData->Single2A.S2aAf,0x0,sizeof(pCamCalData->Single2A.S2aAf));

            if(0x1&AWBAFConfig){
                ////AWB////
                cam_calCfg.u4Offset = (start_addr+2);
                cam_calCfg.u4Length = 4;
                cam_calCfg.pu1Params = (u8 *)&CalGain;            
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
				CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"Read CalGain OK\n");
                if(!ioctlerr)
                {                
    				// Get min gain
    				CalR  = CalGain&0xFF;
					CalGr = (CalGain>>8)&0xFF;
					CalGb = (CalGain>>16)&0xFF;
					CalB  = (CalGain>>24)&0xFF;
					CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"Extract CalGain OK\n");
    				for (int i=0;i<4;i++){
						tempGain = (CalGain>>(8*i)) & 0xFF;
						CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"item=%d", tempGain);
                        if( tempGain < tempMin){
    					    tempMin = tempGain;
    						CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"New tempMin=%d", tempMin);
                        }
                    }
    				err = CAM_CAL_ERR_NO_ERR;
                }
                else        
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                    ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
                }            
    
    
    			
                cam_calCfg.u4Offset = (start_addr+6);
                cam_calCfg.u4Length = 4;
                cam_calCfg.pu1Params = (u8 *)&FacGain; 
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
				CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"Read FacGain OK\n");
                if(!ioctlerr)
                {                
    				// Get min gain
    				FacR  = FacGain&0xFF;
					FacGr = (FacGain>>8)&0xFF;
					FacGb = (FacGain>>16)&0xFF;
					FacB  = (FacGain>>24)&0xFF;
					CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"Extract CalGain OK\n");
    				for (int i=0;i<4;i++){
    					tempGain = (FacGain>>(8*i)) & 0xFF;
						CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"item=%d", tempGain);
                        if( tempGain < tempMin){
    					    tempMin = tempGain;
    						CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"New tempMin=%d", tempMin);
                        }
                    }
    				err = CAM_CAL_ERR_NO_ERR;
                }
                else        
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                    ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
                }            
    	        CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"Start assign value\n");
                pCamCalData->Single2A.S2aAwb.rUnitGainu4R = (u32)(CalR*512/tempMin);
                pCamCalData->Single2A.S2aAwb.rUnitGainu4G = (u32)( (CalGr+CalGb)*512/2/tempMin );
                pCamCalData->Single2A.S2aAwb.rUnitGainu4B  = (u32)(CalB*512/tempMin);
    
    	        pCamCalData->Single2A.S2aAwb.rGoldGainu4R = (u32)(FacR*512/tempMin);
                pCamCalData->Single2A.S2aAwb.rGoldGainu4G = (u32)( (FacGr+FacGb)*512/2/tempMin );
                pCamCalData->Single2A.S2aAwb.rGoldGainu4B  = (u32)(FacB*512/tempMin);
    			
                ////Only AWB Gain Gathering <////
                #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================AWB CAM_CAL==================\n");
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[CalGain] = 0x%x\n", CalGain);
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[FacGain] = 0x%x\n", FacGain);
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rCalGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4R);
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rCalGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4G);
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rCalGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rUnitGainu4B);	
    			CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rFacGain.u4R] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4R);
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rFacGain.u4G] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4G);
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[rFacGain.u4B] = %d\n", pCamCalData->Single2A.S2aAwb.rGoldGainu4B);	
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================AWB CAM_CAL==================\n");
                #endif
            }
			if(0x2&AWBAFConfig){
                ////AF////
                cam_calCfg.u4Offset = (start_addr+10);
                cam_calCfg.u4Length = 2;
                cam_calCfg.pu1Params = (u8 *)&AFInf;            
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
                if(!ioctlerr)
                {                
    				err = CAM_CAL_ERR_NO_ERR;
                }
                else        
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                    ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
                }            
    
                cam_calCfg.u4Offset = (start_addr+12);
                cam_calCfg.u4Length = 2;
                cam_calCfg.pu1Params = (u8 *)&AFMacro;            
                ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
                if(!ioctlerr)
                {                
    				err = CAM_CAL_ERR_NO_ERR;
                }
                else        
                {
                    pCamCalData->Single2A.S2aBitEn = CAM_CAL_NONE_BITEN;
                    CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                    ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
                }
    	
                pCamCalData->Single2A.S2aAf[0] = AFInf;
                pCamCalData->Single2A.S2aAf[1] = AFMacro;
     			
                ////Only AWB Gain Gathering <////
                #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================AF CAM_CAL==================\n");
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[AFInf] = %d\n", AFInf);
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[AFMacro] = %d\n", AFMacro);
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================AF CAM_CAL==================\n");
                #endif
            }

			
////////////////////////////////////////////////////////////////////////////////        
        }        
    }    
    return err;
}
UINT32 DoCamCalSingleLsc_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    MUINT32 idx;    
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];     
    UINT32 PregainFactor, PregainOffset;
    UINT32 PregainFactorH, PregainOffsetH;
    UINT32 GainValue;
    if(pCamCalData->DataVer >= CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
        ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
    }
    else
    {
        if(BlockSize!=CAM_CAL_SINGLE_LSC_SIZE)
        {
            CAM_CAL_ERR_IMX132MIPIRAW("BlockSize(%d) is not correct (%d)\n",BlockSize,CAM_CAL_SINGLE_LSC_SIZE);
            ShowCmdErrorLog_IMX132AW(pCamCalData->Command);            
        }
        else
        {
            pCamCalData->SingleLsc.TableRotation=CUSTOM_CAM_CAL_ROTATION_00_IMX132MIPIRAW;         
            cam_calCfg.u4Offset = (start_addr|0xFFFF);
            cam_calCfg.u4Length = BlockSize; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->SingleLsc.LscTable.Data[0];
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];  
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }
        }
    }    
    #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================SingleLsc Data==================\n");
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[1st] = %x, %x, %x, %x \n", pCamCalData->SingleLsc.LscTable.Data[0],
    	                                                                         pCamCalData->SingleLsc.LscTable.Data[1],
    	                                                                         pCamCalData->SingleLsc.LscTable.Data[2],
    	                                                                         pCamCalData->SingleLsc.LscTable.Data[3]);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[1st] = SensorLSC(1)?MTKLSC(2)?  %x \n", pCamCalData->SingleLsc.LscTable.MtkLcsData.MtkLscType);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"RETURN = 0x%x \n", err);            
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================SingleLsc Data==================\n");
    #endif    
//    err =  CamCalReturnErr[pCamCalData->Command];  //seanlin121121 wait for OTP put correct sensor LSC data 
    return err;	
}
UINT32 DoCamCalN3dLsc_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    MUINT32 idx;    
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];     
    UINT32 PregainFactor, PregainOffset;
    UINT32 PregainFactorH, PregainOffsetH;
    UINT32 GainValue;
    if(pCamCalData->DataVer != CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
        ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
    }
    else
    {
        if(BlockSize!=CAM_CAL_N3D_LSC_SIZE)
        {
            CAM_CAL_ERR_IMX132MIPIRAW("BlockSize(%d) is not correct (%d)\n",BlockSize,CAM_CAL_N3D_LSC_SIZE);
            ShowCmdErrorLog_IMX132AW(pCamCalData->Command);            
        }
        else
        {
            pCamCalData->N3DLsc.Data[0].TableRotation=CUSTOM_CAM_CAL_ROTATION_00_IMX132MIPIRAW;
            cam_calCfg.u4Offset = (start_addr|0xFFFF);            pCamCalData->N3DLsc.Data[1].TableRotation=CUSTOM_CAM_CAL_ROTATION_01_IMX132MIPIRAW;            

            cam_calCfg.u4Length = BlockSize; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->N3DLsc.Data[0].LscTable.Data[0];
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];  
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }
            cam_calCfg.u4Offset = ((start_addr>>16)|0xFFFF);
            cam_calCfg.u4Length = BlockSize; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->N3DLsc.Data[1].LscTable.Data[0];
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];  
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }
        }
    }    
    #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================3DLsc Data==================\n");
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[1st] = %x, %x, %x, %x \n", pCamCalData->N3DLsc.Data[0].LscTable.Data[0],
    	                                                                         pCamCalData->N3DLsc.Data[0].LscTable.Data[1],
    	                                                                         pCamCalData->N3DLsc.Data[0].LscTable.Data[2],
    	                                                                         pCamCalData->N3DLsc.Data[0].LscTable.Data[3]);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[1st] = SensorLSC(1)?MTKLSC(2)?  %x \n", 
    	                                                  pCamCalData->N3DLsc.Data[0].LscTable.MtkLcsData.MtkLscType);    
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[2nd] = %x, %x, %x, %x \n", pCamCalData->N3DLsc.Data[1].LscTable.Data[0],
    	                                                                         pCamCalData->N3DLsc.Data[1].LscTable.Data[1],
    	                                                                         pCamCalData->N3DLsc.Data[1].LscTable.Data[2],
    	                                                                         pCamCalData->N3DLsc.Data[1].LscTable.Data[3]);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[[2nd]] = SensorLSC(1)?MTKLSC(2)?  %x \n", 
    	                                                  pCamCalData->N3DLsc.Data[1].LscTable.MtkLcsData.MtkLscType);    
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"RETURN = 0x%x \n", err);            
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================3DLsc Data==================\n");
    #endif    
    return err;	
}

UINT32 DoCamCal3AGain_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    MUINT32 idx;    
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];     
    UINT32 PregainFactor, PregainOffset;
    UINT32 PregainFactorH, PregainOffsetH;
    UINT32 GainValue;
    if(pCamCalData->DataVer != CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
        ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
    }
    else
    {
        if(BlockSize!=CAM_CAL_N3D_3A_SIZE)
        {
            CAM_CAL_ERR_IMX132MIPIRAW("BlockSize(%d) is not correct (%d)\n",BlockSize,CAM_CAL_N3D_3A_SIZE);
            ShowCmdErrorLog_IMX132AW(pCamCalData->Command);            
        }
        else
        {
            cam_calCfg.u4Offset = (start_addr|0xFFFF);
            cam_calCfg.u4Length = BlockSize; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->N3D3A.Data[0][0];
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];  
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }
            cam_calCfg.u4Offset = ((start_addr>>16)|0xFFFF);
            cam_calCfg.u4Length = BlockSize; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->N3D3A.Data[1][0];
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];  
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }
        }
    }    
    #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================3A Data==================\n");
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[1st] = %x, %x, %x, %x \n", pCamCalData->N3D3A.Data[0][0],pCamCalData->N3D3A.Data[0][1],
    	                                                                         pCamCalData->N3D3A.Data[0][2],pCamCalData->N3D3A.Data[0][3]);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[2nd] = %x, %x, %x, %x \n", pCamCalData->N3D3A.Data[1][0],pCamCalData->N3D3A.Data[1][1],
    	                                                                         pCamCalData->N3D3A.Data[1][2],pCamCalData->N3D3A.Data[1][3]);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"RETURN = 0x%x \n", err);            
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================3A Data==================\n");
    #endif    
    return err;
}

UINT32 DoCamCal3DGeo_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;
    MUINT32 idx;    
    UINT32 ioctlerr;
    UINT32 err =  CamCalReturnErr[pCamCalData->Command];     
    UINT32 PregainFactor, PregainOffset;
    UINT32 PregainFactorH, PregainOffsetH;
    UINT32 GainValue;
    if(pCamCalData->DataVer != CAM_CAL_N3D_DATA)
    {
        err = CAM_CAL_ERR_NO_DEVICE;
        CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
        ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
    }
    else
    {
        if(BlockSize!=CAM_CAL_N3D_3D_SIZE)
        {
            CAM_CAL_ERR_IMX132MIPIRAW("BlockSize(%d) is not correct (%d)\n",BlockSize,CAM_CAL_N3D_3D_SIZE);
            ShowCmdErrorLog_IMX132AW(pCamCalData->Command);            
        }
        else
        {
            cam_calCfg.u4Offset = (start_addr|0xFFFF);
            cam_calCfg.u4Length = BlockSize; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->N3D3D.Data[0][0];
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];  
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }
            cam_calCfg.u4Offset = ((start_addr>>16)|0xFFFF);
            cam_calCfg.u4Length = BlockSize; //sizeof(ucModuleNumber)
            cam_calCfg.pu1Params= (u8 *)&pCamCalData->N3D3D.Data[1][0];
            ioctlerr= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);	
            if(!ioctlerr)
            {
                err = CAM_CAL_ERR_NO_ERR;
            }
            else
            {
                CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                err =  CamCalReturnErr[pCamCalData->Command];  
                ShowCmdErrorLog_IMX132AW(pCamCalData->Command);
            }
        }
    }    
    #ifdef DEBUG_CALIBRATION_LOAD_IMX132MIPIRAW
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================3D Data==================\n");
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[1st] = %x, %x, %x, %x \n", pCamCalData->N3D3D.Data[0][0],pCamCalData->N3D3D.Data[0][1],
    	                                                                         pCamCalData->N3D3D.Data[0][2],pCamCalData->N3D3D.Data[0][3]);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"[2nd] = %x, %x, %x, %x \n", pCamCalData->N3D3D.Data[1][0],pCamCalData->N3D3D.Data[1][1],
    	                                                                         pCamCalData->N3D3D.Data[1][2],pCamCalData->N3D3D.Data[1][3]);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"RETURN = 0x%x \n", err);            
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"======================3D Data==================\n");
    #endif    
    return err;
}

UINT32 DoCamCalLayoutCheck_IMX132MIPIRAW(void)
{
    MINT32 lCamcamFID = -1;  //seanlin 121017 01 local for layout check

    UCHAR cBuf[128] = "/dev/";	
    UCHAR HeadID[5] = "NONE";	
    UINT32 result = CAM_CAL_ERR_NO_DEVICE;
    //cam_cal
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    UINT32 CheckID,i ;
    INT32 err;
    switch(gIsInitedCamCal)
    {
        case CAM_CAL_LAYOUT_RTN_PASS:
        result =  CAM_CAL_ERR_NO_ERR;
        break;	        
        case CAM_CAL_LAYOUT_RTN_QUEUE:        	
        case CAM_CAL_LAYOUT_RTN_FAILED:
        default:
        result =  CAM_CAL_ERR_NO_DEVICE;        	
        break;	
    }
    if ((gIsInitedCamCal==CAM_CAL_LAYOUT_RTN_QUEUE) && (CAM_CALInit() != CAM_CAL_NONE) && (CAM_CALDeviceName(&cBuf[0]) == 0))
    {
        lCamcamFID = open(cBuf, O_RDWR);
		CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"lCamcamFID= 0x%x", lCamcamFID);
        if(lCamcamFID == -1)
        {            
            CAM_CAL_ERR_IMX132MIPIRAW("----error: can't open CAM_CAL %s----\n",cBuf);
            gIsInitedCamCal=CAM_CAL_LAYOUT_RTN_FAILED;
            result =  CAM_CAL_ERR_NO_DEVICE;      
            return result;//0;
        }	
        //read ID
        cam_calCfg.u4Offset = 0xFFFFFFFF;
        for (i = 0; i< MAX_CALIBRATION_LAYOUT_NUM; i++)
        {
            if (cam_calCfg.u4Offset != CalLayoutTbl_IMX132MIPIRAW[i].HeaderAddr)
            {
                CheckID = 0x00000000;
                cam_calCfg.u4Offset = CalLayoutTbl_IMX132MIPIRAW[i].HeaderAddr;
                cam_calCfg.u4Length = 4;
                cam_calCfg.pu1Params = (u8 *)&CheckID;
                err= ioctl(lCamcamFID, CAM_CALIOC_G_READ , &cam_calCfg);                
                if(err< 0)
                {
                    CAM_CAL_ERR_IMX132MIPIRAW("ioctl err\n");
                    CAM_CAL_ERR_IMX132MIPIRAW("Read header ID fail err = 0x%x \n",err);
                    gIsInitedCamCal=CAM_CAL_LAYOUT_RTN_FAILED;
                    result =  CAM_CAL_ERR_NO_DEVICE; 
                    break;
                }
            }
			CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"Table[%d] ID= 0x%x",i, CheckID);
            if(CheckID == CalLayoutTbl_IMX132MIPIRAW[i].HeaderId)
            {
                LayoutType = i;
                gIsInitedCamCal=CAM_CAL_LAYOUT_RTN_PASS;   	
                result =  CAM_CAL_ERR_NO_ERR;	
                break;
            }			
        }        
        CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"LayoutType= 0x%x",LayoutType);	
        CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"result= 0x%x",result);	 
        ////
        close(lCamcamFID);		
    }	
    else //test
    {
        CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"----gIsInitedCamCal_0x%x!----\n",gIsInitedCamCal);
        CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"----NO CAM_CAL_%s!----\n",cBuf);
		CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"----NO CCAM_CALInit_%d!----\n",CAM_CALInit());
    }
    return  result;
}

UINT32 CAM_CALGetCalData_IMX132MIPIRAW(UINT32* pGetSensorCalData)
{	
    UCHAR cBuf[128] = "/dev/";	
    UCHAR HeadID[5] = "NONE";	
    UINT32 result = CAM_CAL_ERR_NO_DEVICE;
    //cam_cal
    stCAM_CAL_INFO_STRUCT  cam_calCfg;
    UINT32 CheckID,i ;
    INT32 err = CAM_CAL_ERR_NO_DEVICE;
//    static UINT16 LayoutType = (MAX_CALIBRATION_LAYOUT_NUM+1); //seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
    INT32 CamcamFID = 0;  //seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
    UINT16 u2IDMatch = 0;
    UINT32 ulPartNumberCount = 0;
    CAMERA_CAM_CAL_TYPE_ENUM lsCommand;
    
    PCAM_CAL_DATA_STRUCT pCamCalData = (PCAM_CAL_DATA_STRUCT)pGetSensorCalData;    
//    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"CAM_CALGetCalData_IMX132MIPIRAW(0x%8x)----\n",(unsigned int)pGetSensorCalData);
    //====== Get Property ======
    char value[32] = {'\0'};    
    property_get("camcalcamcal.log", value, "0");
    dumpEnable = atoi(value);
    //====== Get Property ======



    lsCommand = pCamCalData->Command;
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"pCamCalData->Command = 0x%x \n",pCamCalData->Command);
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"lsCommand = 0x%x \n",lsCommand);    
    //Make sure Layout is confirmed, first
    if(DoCamCalLayoutCheck_IMX132MIPIRAW()==CAM_CAL_ERR_NO_ERR)
    {  
        pCamCalData->DataVer = (CAM_CAL_DATA_VER_ENUM)CalLayoutTbl_IMX132MIPIRAW[LayoutType].DataVer;   
        if ((CAM_CALInit() != CAM_CAL_NONE) && (CAM_CALDeviceName(&cBuf[0]) == 0))
        {
            CamcamFID = open(cBuf, O_RDWR);	
            if(CamcamFID == -1)
            {
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"----error: can't open CAM_CAL %s----\n",cBuf);
                result =  CamCalReturnErr[lsCommand];       
                ShowCmdErrorLog_IMX132AW(lsCommand);
                return result;
            } 
            /*********************************************/
            if ((CalLayoutTbl_IMX132MIPIRAW[LayoutType].CalItemTbl[lsCommand].Include != 0) 
            	&&(CalLayoutTbl_IMX132MIPIRAW[LayoutType].CalItemTbl[lsCommand].GetCalDataProcess != NULL))
            {		
                result =  CalLayoutTbl_IMX132MIPIRAW[LayoutType].CalItemTbl[lsCommand].GetCalDataProcess(
                	                    CamcamFID, 
                	                    CalLayoutTbl_IMX132MIPIRAW[LayoutType].CalItemTbl[lsCommand].StartAddr, 
                	                    CalLayoutTbl_IMX132MIPIRAW[LayoutType].CalItemTbl[lsCommand].BlockSize, 
                	                    pGetSensorCalData);	
            }
            else
            {
                result =  CamCalReturnErr[lsCommand];       
                ShowCmdErrorLog_IMX132AW(lsCommand);       						
            }
            /*********************************************/   
			close(CamcamFID);
        }
    }
    else
    {

       result =  CamCalReturnErr[lsCommand];       
       ShowCmdErrorLog_IMX132AW(lsCommand);
        return result;
    }         
    CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"result = 0x%x\n",result);	
    return  result;
}


UINT32 DoCamCalDataReset_IMX132MIPIRAW(INT32 CamcamFID, UINT32 start_addr, UINT32 BlockSize, UINT32* pGetSensorCalData)
{
    gIsInitedCamCal = CAM_CAL_LAYOUT_RTN_QUEUE;//seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
    LayoutType = (MAX_CALIBRATION_LAYOUT_NUM+1); //seanlin 121017 why static? Because cam_cal_drv will get data one block by one block instead of overall in one time.
    bFirstLoad = TRUE;
    return 0;
}

#if 0
#if 1
    unsigned int size[MAX_CALIBRATION_LAYOUT_NUM]={ CALIBRATION_DATA_SIZE_SLIM_LSC1,
    												 CALIBRATION_DATA_SIZE_N3D_DATA1,
    												 CALIBRATION_DATA_SIZE_SUNNY_Q8N03D_LSC1};
#endif 
        if(bFirstLoad)
        {
            cam_calCfg.u4Length = CAM_CAL_PART_NUMBERS_COUNT*LSC_DATA_BYTES; //sizeof(ulModuleNumber)
            cam_calCfg.u4Offset = size[LayoutType]+CUSTOM_CAM_CAL_PART_NUMBERS_START_ADD*LSC_DATA_BYTES;
            cam_calCfg.pu1Params= (u8 *)ulModuleNumber;
            err= ioctl(CamcamFID, CAM_CALIOC_S_WRITE, &cam_calCfg);		
            bFirstLoad = FALSE;
            #ifdef CUSTOM_CAM_CAL_NEW_MODULE_NUMBER_CHECK
            cam_calCfg.u4Length = CAM_CAL_PART_NUMBERS_COUNT*LSC_DATA_BYTES; //sizeof(ulModuleNumber)
            cam_calCfg.u4Offset = size[LayoutType]+CUSTOM_CAM_CAL_PART_NUMBERS_START_ADD*LSC_DATA_BYTES;
            cam_calCfg.pu1Params= (u8 *)&ulPartNumbertmp[0];
            err= ioctl(CamcamFID, CAM_CALIOC_G_READ, &cam_calCfg);		
            for(i=0;i<CAM_CAL_PART_NUMBERS_COUNT;i++)
            {    
                CAM_CAL_LOG_IF_IMX132MIPIRAW(dumpEnable,"ulPartNumbertmp[%d]=0x%8x\n",i,ulPartNumbertmp[i]);							
            }    
            #endif			
        }
#endif
