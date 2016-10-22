#define UDS_GLOBAL
#include <config.h>

volatile  union byte_def udsConfig;
volatile  union byte_def EOLReadSts;
_uds_sts uds_sts={0};
u8 SN_SequenceNumber=0;

_Eol_dtc eol_dev=
{
	0,
	0,
	0,
	0,
	0,
	0,

	0,
	0,
	0,
	0,

	0,	
	0,
	0,
	0,
};		

/*****************************************************/
#if 1
static u16 calcKey(u16 seed)
{
	#define TOPBIT 	0x8000
	#define POLYNOM_1 0x8408
	#define POLYNOM_2 0x8025
	#define BITMASK 	0x0080
	#define INITIAL_REMINDER 0xFFFE
	#define MSG_LEN 2 /* seed length in bytes */
	u8 bSeed[2];
	u16 remainder;
	u8 n;
	u8 i;
	bSeed[0] = (u8)(seed >> 8); /* MSB */
	bSeed[1] = (u8)seed&0xff; /* LSB */
	remainder = INITIAL_REMINDER;
	for (n = 0; n < MSG_LEN; n++)
	{
		remainder ^= ((u16)(bSeed[n]) << 8);
		for (i = 0; i < 8; i++)
		{
			if (remainder & TOPBIT)
			{
				if(remainder & BITMASK)
				{
					remainder = (remainder << 1) ^ POLYNOM_1;
				}
				else
				{
					remainder = (remainder << 1) ^ POLYNOM_2;
				}
			}
			else
			{
				remainder = (remainder << 1);
			}
		}
	}
	return remainder;
}

u16 GetRdmNumber(void)
{
	int i,number;
	srand(1); 
//	printf_os("GetRdmNumber ");
	for (i=0; i<10; i++)
	{
		eol_dev.RdmNumberBuff[i] = rand() % 65535;
//		printf_os("%x,", eol_dev.RdmNumberBuff[i]);
	} 
//	printf_os("\r\n ");
	return(number);
}

void SendFCFrame(u16 addr)
{
	u8 tempTxData[8]={0};
	
	tempTxData[0] = 0x30;
	tempTxData[1] = 0x0;
	tempTxData[2] = 0x0a;
	SendCanBusData(addr,0x0,CAN_ID_STD,CAN_RTR_DATA,tempTxData,8);
}
#endif

void udsNegativeResponse(u16 addr,u8 ID,u8 Parameter)//负响应
{
	u8 tempTxData[8]={0};
	
	tempTxData[0] = 3;
	tempTxData[1] = NRC_;
	tempTxData[2] = ID;
	tempTxData[3] = Parameter;
	SendCanBusData(addr,0x0,CAN_ID_STD,CAN_RTR_DATA,tempTxData,8);
}
void udsPositiveResponse(u16 addr,u8 ID,u8 SubId,u16 Parameter,u8 Len)//正响应
{
	u8 tempTxData[8]={0};
	
	tempTxData[0] = Len;
	tempTxData[1] = ID + DtcMask;
	tempTxData[2] = SubId;
	tempTxData[3] = Parameter>>8;
	tempTxData[4] = Parameter&0xff;
	SendCanBusData(addr,0x0,CAN_ID_STD,CAN_RTR_DATA,tempTxData,8);
}

void SetUdsAskTimeOut(void)
{
	uds_sts.udsAskTimeOut = 200;// 2000ms time out
}

void SetSendSFUdsAskTimeOut(void)
{
	uds_sts.SendSFudsAskTimeOut = 3;// 50MS time out
}

void ExitEolMode(void)
{
	udsConfig.byte = 0x0;
	eol_dev.ReadSurplusLength=0;
	eol_dev.ConfigDataLen=0;
	eol_dev.Read_SF_num = 0;
	
	eol_dev.WriteSurplusLength=0;
	eol_dev.DtcDataLen = 0;
	eol_dev.Write_SF_num = 0;
//	printf_os("Exit Eol mode\r\n");
}
#ifdef ENABLE_UDS
u8 CheckFrameLenErr(u8 OptionType,u8 Len)
{
	u8 Response=0;
	switch(OptionType)
	{
		case SessionControl:
		case ECUReset:
		case TesterPresent:
			if(Len==2)
			{
				Response = 1;
			}
			break;
		case ClearDTC:
			if(Len==4||Len==1 || Len == 3)
			{
				Response = 1;
			}
			break;
		case ReadDTC:
			if(Len==3 || Len==5 || Len==6 || Len==7)
			{
				Response = 1;
			}
			break;
		case ReadID:
			if(Len==3)
			{
				Response = 1;
			}
			break;
		case SecurityAccess:
			if(Len== 2 || Len== 4)
			{
				Response = 1;
			}
			break;
		case NRC_:
			Response = 1;
			break;
	}
	return(Response);
}

u16 GetSubAddr(u8 *cData,u8 Len)
{
	u16 SubAddr=0;
	u8 DataH=0;
	u8 DataL=0;
	switch(Len)
	{
		case 2:
			SubAddr = *(cData+3);
			break;
		case 3:
			DataH = *(cData+3);
			DataL = *(cData+4);
			SubAddr = (u16)(((u16)DataH<<8)|DataL);
			break;
		case 4:
			DataH = *(cData+4);
			DataL = *(cData+5);
			SubAddr = (u16)(((u16)DataH<<8)|DataL);
			break;
	}
	return(SubAddr);
}

u8 CheckTimerOut(u16 RespAddr,u8 OptionType)
{
	if(uds_sts.udsAskTimeOut>0&&uds_sts.udsAskTimeOut<50)
	{
		uds_sts.udsAskTimeOut = 0;
		udsNegativeResponse(RespAddr,OptionType,requiredTimeDelayNotExpired);
		Enter_mode = 0;Exit_mode = 1;
		return(1);
	}
	return(0);
}
void ReadCarUdsCode_Start(u16 addr,u16 DataType)//EOL 0X75A
{
	u8 tempTxData[8]={0};
	u8 i;
	eol_dev.ConfigDataLen = 0;
	if(DataType==configurationAddr)
	{
		eol_dev.ConfigDataLen = ConfigurationSize;
		for (i=0; i<8; i++)
		{
			eol_dev.SendDataTempBuff[i] = eol_dev.ConfigurationBuff[i];
		} 
	}
	else
	{
		udsNegativeResponse(addr,ReadID,SubFuntionErr);
		return;
	}
	eol_dev.ReadSurplusLength = eol_dev.ConfigDataLen;
	if(eol_dev.ConfigDataLen>0)
	{
		EnterDataReadSts = 1;
		tempTxData[0] = ID3StartCmd;
		tempTxData[1] = eol_dev.ConfigDataLen+3;
		tempTxData[2] = ReadID+DtcMask;
		tempTxData[3] = DataType>>8;
		tempTxData[4] = DataType&0xff;
		if(eol_dev.ConfigDataLen<=3)
		{
			for (i=0; i<eol_dev.ConfigDataLen; i++)
			{
				tempTxData[5+i] = eol_dev.SendDataTempBuff[i];
			} 
			EnterDataReadSts = 0;Enter_mode = 1;
		}
		else
		{
			for (i=0; i<3; i++)
			{
				tempTxData[5+i] = eol_dev.SendDataTempBuff[i];
			} 
			eol_dev.ReadSurplusLength-=3;
			eol_dev.Read_SF_num = 0;
			Enter_mode = 1;SetUdsAskTimeOut();
//			printf_os("Send First 3byte\r\n");
		}
		SendCanBusData(addr,0x0,CAN_ID_STD,CAN_RTR_DATA,tempTxData,8);
	}
	else
	{
		udsNegativeResponse(addr,ReadID,generalProgrammingFailure);
	}
}

void ProcSingleMsgData(u16 RespAddr,u8 *cData)
{
	u8 udsFrameLen,DataLen,OptionType,SAtype;
	u16 MaskData=0;
	u16 CalcSecurityKey= 0xffff;
	
	udsFrameLen = *(cData+0);
	DataLen = *(cData+1);
	OptionType = *(cData+2);
	SAtype = *(cData+3);
	if(udsFrameLen!=udsLen)
	{
		udsNegativeResponse(RespAddr,OptionType,ConditionsNotCorrect);
		return;
	}
	if(!CheckFrameLenErr(OptionType,DataLen))
	{
		udsNegativeResponse(RespAddr,OptionType,MessageLength);
		return;
	}
	MaskData = GetSubAddr(cData,DataLen);
	switch(OptionType)
	{
		case SessionControl:
			switch(MaskData)
			{
				case defaultsession:
					udsPositiveResponse(RespAddr,OptionType,defaultsession,0,2);
					if(Enter_mode)
					{
						Enter_mode = 0;Exit_mode = 1;
					}
//					printf_os("Enter defaultsession\r\n");
					break;
				case EOLsession:
					udsPositiveResponse(RespAddr,OptionType,EOLsession,0,2);
					Enter_mode = 1;SetUdsAskTimeOut();
//					printf_os("Enter EOLsession\r\n");
					break;
				case 0x02:
					udsNegativeResponse(RespAddr,OptionType,ServiceNotSupported);
					break;
				default:
					udsNegativeResponse(RespAddr,OptionType,SubFuntionErr);
					break;
			}
			break;
		case ECUReset:
			Enter_mode = 0;Exit_mode = 1;
			if(MaskData==RRMreset)
			{
				udsPositiveResponse(RespAddr,OptionType,RRMreset,0,2);
			}
			else
			{
				udsNegativeResponse(RespAddr,OptionType,SubFuntionErr);
			}
			break;
		case ClearDTC:
			if(SAtype == 0xff && MaskData==0xffff)
			{
				udsPositiveResponse(RespAddr,OptionType,0,0,1);
			}
			else
			{
				udsNegativeResponse(RespAddr,OptionType,ServiceNotSupported);
			}
			break;
		case ReadDTC:
			if(MaskData==0x0209)
			{
				udsPositiveResponse(RespAddr,OptionType,MaskData>>8,(MaskData&0xff)<<8,3);
			}
			else
			{
				udsNegativeResponse(RespAddr,OptionType,ServiceNotSupported);
			}
			break;
		case ReadID:
			if(CheckTimerOut(RespAddr,OptionType))return;
			ReadCarUdsCode_Start(RespAddr,MaskData);
			break;
		case SecurityAccess:
			if(!Enter_mode)
			{
//				printf_os("No Enter EOL Mode ,Start SecurityAccess\r\n");
				udsNegativeResponse(RespAddr,OptionType,requestSequenceError);
				return;
			}
			if(CheckTimerOut(RespAddr,OptionType))return;
			switch(SAtype)
			{
				case StartSecurityAccess:
					eol_dev.CurrentcalcKey = eol_dev.RdmNumberBuff[eol_dev.RdmNumberPtr];
					eol_dev.RdmNumberPtr++;
					if(eol_dev.RdmNumberPtr>=10)
					{
						eol_dev.RdmNumberPtr = 0;
					}
					udsPositiveResponse(RespAddr,OptionType,StartSecurityAccess,eol_dev.CurrentcalcKey,4);
					WairSecurityAccess = 1;SetUdsAskTimeOut();
//					printf_os("StartSecurityAccess :%x\r\n",eol_dev.CurrentcalcKey);
					break;
				case AskSecurityAccess:
					CalcSecurityKey = calcKey(eol_dev.CurrentcalcKey);
//					printf_os("RxSecurityKey:%x,%x\r\n",MaskData,CalcSecurityKey);
					if(MaskData == CalcSecurityKey)
					{
						udsPositiveResponse(RespAddr,OptionType,AskSecurityAccess,0,2);
						SecurityAccess_ok = 1;WairSecurityAccess = 0;SetUdsAskTimeOut();
//						printf_os("SA_ PASS\r\n");
					}
					else
					{
						udsNegativeResponse(RespAddr,OptionType,invalidKey);
//						printf_os("SA_ FAILE\r\n");
					}
					break;
				default:
					udsNegativeResponse(RespAddr,OptionType,SubFuntionErr);
					break;
			}
			break;
		case TesterPresent:
			if(MaskData==0x0)
			{
				if(SecurityAccess_ok||SaveCodeFlag||WairSecurityAccess||EnterDataReadSts||Enter_mode)
				{
					SetUdsAskTimeOut();
				}
				udsPositiveResponse(RespAddr,OptionType,0,0,2);
			}
			else
			{
				udsNegativeResponse(RespAddr,OptionType,SubFuntionErr);
			}
			break;
		default:
			udsNegativeResponse(RespAddr,OptionType,ServiceNotSupported);
			break;
	}
}
void ProcFirstFrameMsgData(u16 RespAddr,u8 *cData)
{
	u8 i;
	u8 udsFrameLen,DataLenH,DataLenL,SAtype,ID_H,ID_L;
	u16 DtcLen;
	udsFrameLen = *(cData+0);
	DataLenH = (*(cData+1))&0x0f;
	DataLenL = *(cData+2);
	DtcLen = (u16)(((u16)DataLenH<<8)|DataLenL);
	SAtype = *(cData+3);
	ID_H = *(cData+4);	
	ID_L = *(cData+5);	
	if(udsFrameLen!=udsLen)
	{
		udsNegativeResponse(RespAddr,SAtype,ConditionsNotCorrect);
		return;
	}
	if(!Enter_mode || !SecurityAccess_ok)
	{
		udsNegativeResponse(RespAddr,SAtype,securityAccessDenied);
//		printf_os("No SecurityAccess Enter Write Data\r\n");
		return;
	}
	if(CheckTimerOut(RespAddr,SAtype))return;
	if(SAtype!=WriteID)
	{
		udsNegativeResponse(RespAddr,WriteID,ServiceNotSupported);
//		printf_os("No WriteID Comm\r\n");
		return;
	}
	/************************************/
	eol_dev.WriteConfigurationID = (u16)(((u16)ID_H<<8)|ID_L);
	if(eol_dev.WriteConfigurationID==configurationAddr || eol_dev.WriteConfigurationID==VinCodeAddr)
	{
		if(eol_dev.WriteConfigurationID==configurationAddr && DtcLen != (ConfigurationSize+3))
		{
			udsNegativeResponse(RespAddr,SAtype,MessageLength);
			return;
		}
		if(eol_dev.WriteConfigurationID==VinCodeAddr && DtcLen != (VinCodeSize+3))
		{
			udsNegativeResponse(RespAddr,SAtype,MessageLength);
			return;
		}
		eol_dev.DtcDataLen = DtcLen-3;
		eol_dev.WriteSurplusLength = eol_dev.DtcDataLen;
		if(eol_dev.WriteSurplusLength<=3 && eol_dev.WriteSurplusLength>0)
		{
			udsPositiveResponse(RespAddr,SAtype,eol_dev.WriteConfigurationID>>8,(eol_dev.WriteConfigurationID&0xff)<<8,3);
			for (i=0; i<eol_dev.WriteSurplusLength; i++)
			{
				eol_dev.CF_Buff[i+1] = (u8)(*(cData+6+i));
			} 
			SaveCodeFlag = 0;
		}
		else
		{
			for (i=0; i<3; i++)
			{
				eol_dev.CF_Buff[i+1] = (u8)(*(cData+6+i));
			} 
			eol_dev.WriteSurplusLength-=3;
			SaveCodeFlag = 1;
			SendFCFrame(RespAddr);
//			printf_os("SendFCFrame\r\n");
			SetUdsAskTimeOut();
		}
	}
	else
	{
		udsNegativeResponse(RespAddr,WriteID,SubFuntionErr);
	}

}
void ProcConsecutiveFrameMsgData(u16 RespAddr,u8 *cData)//20-2f
{
	u8 udsFrameLen;
	u8 tempTextlength=0;
	udsFrameLen = *(cData+0);
	if(udsFrameLen!=udsLen)
	{
		udsNegativeResponse(RespAddr,WriteID,ConditionsNotCorrect);
		return;
	}
	if(!Enter_mode || !SecurityAccess_ok)
	{
		udsNegativeResponse(RespAddr,WriteID,securityAccessDenied);
		return;
	}
	if(CheckTimerOut(RespAddr,WriteID))return;
	if(!SaveCodeFlag)
	{
		udsNegativeResponse(RespAddr,WriteID,requestSequenceError);
		return;
	}
	eol_dev.Write_SF_num = SN_SequenceNumber;
	tempTextlength = eol_dev.DtcDataLen-3;
	if(eol_dev.WriteSurplusLength<=FRAME_DATA_CF && eol_dev.WriteSurplusLength>0)
	{
		udsPositiveResponse(RespAddr,WriteID,eol_dev.WriteConfigurationID>>8,(eol_dev.WriteConfigurationID&0xff)<<8,3);
		if(eol_dev.WriteConfigurationID==configurationAddr)
		{
			CreateMessage(SYS_INFO,eol_config_TOarm,0);
		}
		if(eol_dev.WriteConfigurationID==VinCodeAddr)
		{
			CreateMessage(SYS_INFO,eol_vincode_TOarm,0);
		}
		SaveCodeFlag = 0;
//		printf_os("WriteSurplusLength < 8\r\n");
	}
	else
	{
		eol_dev.WriteSurplusLength =tempTextlength-(FRAME_DATA_CF*(eol_dev.Write_SF_num+1));
		SetUdsAskTimeOut();
	}
}
void  SendCarUdsInfo_CF(u16 addr)
{
	u8 tempTxData[8]={0};
	u8 i=0;
	u8 tempTextlength=0;

	uds_sts.SendSFudsAskTimeOut = 0;
	uds_sts.udsAskTimeOut = 0;
	
	tempTextlength = eol_dev.ConfigDataLen-3;
	
	tempTxData[0] = UDS_CF_START_NUM+eol_dev.Read_SF_num;
//	printf_os("CF (ReadSurplusLength: %d,SF_num:%d)\r\n",eol_dev.ReadSurplusLength,eol_dev.Read_SF_num);
	if(eol_dev.ReadSurplusLength<=7 && eol_dev.ReadSurplusLength>0)
	{
		for(i=0;i<eol_dev.ReadSurplusLength;i++)
		{
			tempTxData[1+i]=eol_dev.SendDataTempBuff[3+i+(FRAME_DATA_CF*eol_dev.Read_SF_num)];
		}
		SendCanBusData(addr,0x0,CAN_ID_STD,CAN_RTR_DATA,tempTxData,8);
		EnterDataReadSts = 0;Enter_mode = 1;
//		printf_os("SendSFUds LastFrame\r\n");
	}
	else
	{
		for(i=0;i<STD_LEN-1;i++)
		{
			tempTxData[1+i]=eol_dev.SendDataTempBuff[3+i+(FRAME_DATA_CF*eol_dev.Read_SF_num)];
		}
		eol_dev.ReadSurplusLength = (tempTextlength-(FRAME_DATA_CF*(eol_dev.Read_SF_num+1)));
		eol_dev.Read_SF_num++;
		if(eol_dev.Read_SF_num>0x0f)
		{
			eol_dev.Read_SF_num = 0;
		}
		SendCanBusData(addr,0x0,CAN_ID_STD,CAN_RTR_DATA,tempTxData,8);
//		printf_os("SF (ReadSurplusLength: %d,SF_num:%d)\r\n",eol_dev.ReadSurplusLength,eol_dev.Read_SF_num);
		if(eol_dev.ReadSurplusLength==0)
		{
			EnterDataReadSts = 0;Enter_mode = 1;
//			printf_os("SendSFUds End\r\n");
		}
		else
		{
			SetSendSFUdsAskTimeOut();
		}
	}
}

void ProcFlowControlMsgData(u16 RespAddr,u8 *cData)
{
	u8 udsFrameLen;
	
	udsFrameLen = *(cData+0);
	if(udsFrameLen!=udsLen)
	{
		udsNegativeResponse(RespAddr,ReadID,ConditionsNotCorrect);
		return;
	}
	if(!EnterDataReadSts)
	{
		udsNegativeResponse(RespAddr,ReadID,requestSequenceError);
		return;
	}
	if(CheckTimerOut(RespAddr,ReadID))return;
	SendCarUdsInfo_CF(RespAddr);
}


void Get_UDS_INFO(u16 RespAddr,u8 *cData)//rx
{
	u8 i;
	u8 MessageType;

	MessageType = (*(cData+1))>>4;
	switch(MessageType)
	{
		case SingleFrame:
			ProcSingleMsgData(RespAddr,cData);
			break;
		case FirstFrame:
			ProcFirstFrameMsgData(RespAddr,cData);
			break;
		case ConsecutiveFrame:
			SN_SequenceNumber = (((cData[1])&0x0f)-1);
			eol_dev.CF_Buff[0]=cData[0];
			for(i=0;i<FRAME_DATA_CF;i++)
			{
				eol_dev.CF_Buff[(SN_SequenceNumber*FRAME_DATA_CF)+4+i] = cData[i+2]; 
			}
			ProcConsecutiveFrameMsgData(EOL_75A,eol_dev.CF_Buff);
			break;
		case FlowControl:
			ProcFlowControlMsgData(RespAddr,cData);
			break;
	}
	
}
#endif
void TimerUdsSycle(void)
{
	if(Enter_mode)
	{
		if(uds_sts.udsAskTimeOut>1)
		{
			uds_sts.udsAskTimeOut--;
		}
	}
	/*********************************/
	if(Enter_mode)
	{
		if(uds_sts.SendSFudsAskTimeOut>0)
		{
			uds_sts.SendSFudsAskTimeOut--;
			if(uds_sts.SendSFudsAskTimeOut==0)
			{
				SendCarUdsInfo_CF(EOL_75A);
			}
		}
	}
	/*********************************/
}

void ProcOBDTask(void)
{
	if(Exit_mode)
	{
		Exit_mode = 0;
		ExitEolMode();
	}
}
