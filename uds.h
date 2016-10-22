#ifndef UDS_H
#define UDS_H

#ifdef UDS_GLOBAL
	#define UDS_EXT 
#else
	#define UDS_EXT extern
#endif

#define SAVE_BUFFER_LEN	20
#define ConfigurationSize	8
#define VinCodeSize		17
typedef struct
{	
	u8 CF_Buff[100]; //[0]: frameLen,
	u8 RdmNumberPtr;
	u8 SendDataTempBuff[SAVE_BUFFER_LEN];
	u16 RdmNumberBuff[10];
	u8 ConfigurationBuff[8];//第一个字节位帧长度
	u8 VinCodeBuff[17];
	
	u16 CurrentcalcKey;
	
	u8 ReadSurplusLength;
	u8 ConfigDataLen;
	u8 Read_SF_num;
	
	u8 WriteSurplusLength;
	u8 DtcDataLen;
	u8 Write_SF_num;
	u16 WriteConfigurationID;
	
}_Eol_dtc;
UDS_EXT _Eol_dtc eol_dev;	

UDS_EXT u8 SN_SequenceNumber;

UDS_EXT volatile  union byte_def udsConfig;//不能编辑
	#define Enter_mode				udsConfig.bit.b0
	#define SecurityAccess_ok			udsConfig.bit.b1
	#define WairSecurityAccess			udsConfig.bit.b2
	#define SaveCodeFlag				udsConfig.bit.b3
	#define EnterDataReadSts 			udsConfig.bit.b4
	#define Exit_mode					udsConfig.bit.b7


#define SingleFrame		0x0
#define FirstFrame			0x01
#define ConsecutiveFrame	0x02
#define FlowControl			0x03
/*****************************************************************/
#define SessionControl 		0x10
#define ECUReset 			0x11
#define ClearDTC 			0x14
#define KWPReadDTC 			0x18
#define ReadDTC 			0x19
#define ReadID 			0x22
#define SecurityAccess 		0x27
#define WriteID 			0x2E
#define InputOutputControlID 	0x2F
#define CommunicationControl 	0x28
#define RoutineControl 			0x31
#define TesterPresent 			0x3E
#define ControlDTCSetting 		0x85

#define NRC_				0x7F
#define DtcMask 			0x40
#define udsLen 			8
#define UDS_CF_START_NUM   		0x21

enum can_NRC_sts
{
	ServiceNotSupported=0x11,
	SubFuntionErr = 0x12,
	MessageLength=0x13,
	ConditionsNotCorrect=0x22,
	requestSequenceError=0x24,
	requestOutOfRange=0x31,
    	securityAccessDenied=0x33,
    	invalidKey=0x35,
    	exceededNumberOfAttempts=0x36,
    	requiredTimeDelayNotExpired=0x37,
    	generalProgrammingFailure=0x72,
    	ServiceBusy=0x78,//
};

#define defaultsession 				0x01
#define EOLsession 				0x03
#define StartSecurityAccess			0x03
#define AskSecurityAccess			0x04
#define RRMreset					0x03
#define ReadDTCsts				0x02
	#define DTCstsMask				0x09

#define configurationAddr 			0xD009
/*****************************************************************/
//EOL config info	
typedef struct
{
	u16 SendSFudsAskTimeOut;
	u16 udsAskTimeOut;
}_uds_sts;
UDS_EXT _uds_sts uds_sts;	
/*****************************************************************/
UDS_EXT void ReqArmEolConfig(void);
UDS_EXT void ReqArmEolVinCode(void);

UDS_EXT void TimerUdsSycle(void);
UDS_EXT void  SendCarUdsInfo_CF(u16 addr);
UDS_EXT void ExitEolMode(void);
UDS_EXT u16 GetRdmNumber(void);
void ProcConsecutiveFrameMsgData(u16 RespAddr,u8 *cData);
void SendArmEolConfig(void);
void SendArmEolVinCode(void);
void Get_UDS_INFO(u16 RespAddr,u8 *cData);
void ProcOBDTask(void);

u8 CheckFrameLenErr(u8 OptionType,u8 Len);
void SendFCFrame(u16 addr);
u16 GetSubAddr(u8 *cData,u8 Len);

#endif

