#include "hlSrioConfig.h"

/************************************************************************
*       hlSrioReconfig       	traverse the siro network again to find if there are reset
*                                      switch or srio devices.
�����ͨ��������ϵĽڵ���ö��,������ͨ��������
�ϵĽڵ���ö��,������hlSrioInit����ǰ,����岻������,
�����������hlSrioReconfig�������,�����.
ԭ���Ǵ����������,�������Ϻ͸ô����������1848
�˿ڻᱨһ����.����������и���������������
����ackid��ͬ��,��ά�����ͷ���ȥ�ղ�����.
����취��hlSrioInit������,�жϳ��˿�״̬��,���Ͼͽ�
�����λ���,���������ܻ�Ӱ��ö���ٶ�,����û��
************************************************************************/
HL_STATUS hlSrioReconfig()
{
	hostAllSwTbl *pHostAllSwTbl=NULL;
	int i;
	UINT16 hopcount=1;
	long returnValue;	
	UINT32 hostTargetID;
	PEFCAR_DESC pefcar;
                
	if(globalSrioNetStatus!=1)
	{
		printf("SRIO net not initialized!\n");
		return HL_ERROR;
	}

	hostTargetID = globalSysHostID;
	globalNewSrioDevIndex=0;

	globalSwitchResetNum=0;
	for(i=0;i<MAX_NUMBER_RIO_SW;i++)
	{
		globalSwitchStatus[i]=0;
	}
	returnValue=hlSrioHostConfigLock(hostTargetID);           
	if(returnValue==HL_ALREADY_LOCK)
	{
		/*the switch connect to host 8641d*/
		pefcar.word =SRIO_LOCAL_REG_READ(SRIO_PEFCAR_OFFSET);
		if(pefcar.desc.sw==1)
		{
			/*get the switch virtual id*/
			pHostAllSwTbl=pDrvCtrl->pAllSwTbl;
			if(pHostAllSwTbl==NULL)
			{
				printf("SRIO Switch pHostAllSwTbl == NULL!\n");
				return HL_ERROR;
			}
			returnValue=hlSrioSwitchReconfig(pDrvCtrl,hopcount,0,0);
			if(returnValue!=HL_OK)
			{
				printf("ERROR during srio net reconfig!\n");
				return HL_ERROR;
			}

			DB(printf("There has %d switch been reset,now i need to reconfig it's route table\n",globalSwitchResetNum));
			if( globalSwitchResetNum > 0 )
				hlSrioReConfigRoute(pDrvCtrl,hostTargetID);

			hlMasterResetEndpoint(hostTargetID);

			SRIO_LOCAL_REG_READ(0);
	
			DB(printf("The Srio Reconfig complete.\n"));
		}                                        
	}
	else
	{
		/*the host must have already connect to some other srio device,else something must be wrong  */
		return HL_ERROR;        
	}
	return HL_OK;                                
}

/***********************************************************************************
*       hlSrioSwitchReconfig			check each port of the switch according to the software switch
*									route table
***********************************************************************************/
long hlSrioSwitchReconfig(
	RIO_DRV_CTRL *pDrvCtrl,
	UINT16  hopCount,
	UINT32 localSwitchIndex,
	UINT32 parentSwitchIndex)
{
	hostAllSwTbl *pHostAllSw=NULL;
	SWITCH_ROUTE_TBL *pSw=NULL;
	int parentPortNum,linkStatus;
	int tgtId;	
	int srioDevIndex;
	int status = 0;
	long returnValue;
	UINT32 hostTargetID=globalSysHostID;
	UINT32 childHopCount,switchNum,swVirtID;
	UINT32 port,brotherPort;
	UINT32 newSrioDevNum=0;
	UINT32 tempport = 0xff;
	UINT32 switchId = 0;
	UINT32 maxPortNum = 0;
	int masterEn;
	UINT32 mtRdata=0;
	/*srio devices has been enumed by the host,1 means has been checked,0 means not */
	int srioLockDevStatus[MAX_SRIO_SWITCH_PORT_NUM];
	UINT16 truehopcount=0;

	if(hopCount>=1)
		truehopcount = hopCount-1;

	pHostAllSw=pDrvCtrl->pAllSwTbl;
	memset(srioLockDevStatus,0,sizeof(int)*MAX_SRIO_SWITCH_PORT_NUM);
	childHopCount=hopCount+1;                
	if(localSwitchIndex == 0)
	{
		switchId = SRIO_LOCAL_REG_READ(SRIO_COMP_TAG);
		parentPortNum = HOST_PORT;
	}
	else
	{
		if (HL_OK == hlMaintRead(0xff,truehopcount,SRIO_COMP_TAG,&mtRdata))
			switchId = mtRdata;            
		else         
			hlMtRdFailReport (0xff,truehopcount,SRIO_COMP_TAG,pHostAllSw);

		/*get the mother port,*/
		(srioSwitchMethod.swtichControl)(pDrvCtrl, hostTargetID,0xff,truehopcount,READ_INITIATOR_PORT, &parentPortNum); 
	}
	
	switchId&=0xff;
	switchId-=RIO_SW_FIRST_VIRTUAL_TGTID;                
	globalSwitchStatus[switchId] = globalSwitchStatus[switchId]|0x1;   

	if( (globalSwitchStatus[switchId]&0x2) == 0x2 )
	{
		hlSrioConfigSetDiscover(hostTargetID, 0xff, truehopcount);			
		/*updata the LUT,  */
		(srioSwitchMethod.switchLoadLUTtoAll)(pDrvCtrl,hostTargetID,0xff,truehopcount,parentPortNum, hostTargetID); 					   
		/*set the default port to be the parent port num*/
		(srioSwitchMethod.swtichControl)(pDrvCtrl, hostTargetID,0xff,truehopcount,SET_DEFAULT_PORT,&parentPortNum );
		/*DB(printf("parentport Num=%d\n",parentPortNum));*/
	}

	/*DB(printf("Find switch, switch = %d !\n",switchId));*/
	pSw=pHostAllSw->hostAllSwInfo[switchId];
	
	if(localSwitchIndex == 0) 
		maxPortNum = ((SRIO_LOCAL_REG_READ(SRIO_SW_PORT_INFO))&SRIO_SW_PORT_NUM_MASK)>>8;
	else
	{
		if (HL_OK == hlMaintRead(0xff,truehopcount,SRIO_SW_PORT_INFO,&mtRdata))		
			maxPortNum = ((mtRdata)&SRIO_SW_PORT_NUM_MASK)>>8;
		else        
			hlMtRdFailReport (0xff,truehopcount,SRIO_SW_PORT_INFO,pHostAllSw);                           
	}
                        
	/*check each port of the switch */
	for(port=0;port<maxPortNum;port++)
	{
		if(localSwitchIndex == 0) 
			status = SRIO_LOCAL_REG_READ(SRIO_PORTX_STATUS_OFFSET(port));
		else
		{
			if (HL_OK == hlMaintRead(0xff,truehopcount,SRIO_PORTX_STATUS_OFFSET(port), &mtRdata))
				status = mtRdata;                
			else             
				hlMtRdFailReport (0xff,truehopcount,SRIO_PORTX_STATUS_OFFSET(port),pHostAllSw);                                        
		}
		if((status&PORT_OK)==PORT_OK)
		{
			DB(printf("switch %d port %d status is OK!\n",switchId,port));			
		
			/*use 0xff as default id to visit this srio device*/
			if(localSwitchIndex == 0)
			{				
				SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID, 0xff);
				SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_PORT, port);
			}
			else			
			{				
				(srioSwitchMethod.switchLoadLUTtoAll)(pDrvCtrl,hostTargetID,0xff,truehopcount,port,0xff);
			}
    
			/*whether it's a srio device. deal with the endpoint reset*/
			tgtId=hlSrioGetDeviceId(pSw,port,&srioDevIndex);
			if(tgtId!=0xff)
			{
				if( (globalSwitchStatus[switchId]&0x2) == 0x2 )
				{
					DB(printf("the switch %d has been reset. reconfig the srio device!\n",switchId));
					/*try to lock*/
					returnValue=hlSrioRemoteConfigLock(hostTargetID,hopCount);					
					/*disable the master enable bit*/
					if (HL_OK == hlMaintRead(0xff, hopCount, SRIO_PGCCSR_OFFSET, &mtRdata))			
						masterEn = mtRdata;
					
					if (masterEn & MASTER_ENABLE)
						hlMaintWrite(0xff, hopCount, SRIO_PGCCSR_OFFSET,masterEn&(~MASTER_ENABLE)); 
					
					hlSrioConfigSetDiscover(hostTargetID, 0xff, hopCount);					
					/*set id*/
					hlMaintWrite(0xff, hopCount, SRIO_BDIDCSR_OFFSET,tgtId<<16);
					/*updata switch route table*/								
					if( localSwitchIndex == 0 )
					{
						do{
							SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID, tgtId);									
							SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_PORT, port);
							SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID, tgtId); 
							tempport = SRIO_LOCAL_REG_READ(SRIO_ROUTE_CFG_PORT);
						}while(tempport!=port); 									   
					}
					else
						(srioSwitchMethod.switchLoadLUTtoAll)(pDrvCtrl,hostTargetID,0xff,truehopcount,port,tgtId);					
								
				}
				/*DB(printf("switch = %d, id =%d port=%d \n",switchId,tgtId,port));*/
				continue;
			}

			/*whether it's a switch connect*/
			switchNum=hlSrioGetSwitchNum(pHostAllSw,port,switchId,&brotherPort);
			if(switchNum!=0xff)
			{
				/*another switch connect */
				if(port==parentPortNum)	/*parent switch*/                                                
					continue;
                    
				if((globalSwitchStatus[switchNum]&0x1) == 1 )
				{
					/*there must be a ring exist, this switch has been reconfiged before */
					DB(printf("ring exit, between switch %d and switch %d!\n",switchId,switchNum));
					/*check local switch has been reset*/
					if( (globalSwitchStatus[switchId]&0x2) == 0x2 )
					{
						DB(printf("the switch has been reset,do nothing for the local port %d!\n",port));	
						continue;
					}
					else
					{
						/*check the port status,if the link partner reset then reconfig the link*/
						linkStatus = hlSrioCheckLink(status,port,truehopcount,localSwitchIndex,hostTargetID);
						if( linkStatus == HL_ERROR )
						{
							printf("!!!Link status is error,switch %d port %d!!!\n",switchId,port);
							return HL_ERROR;
						}
						else if( linkStatus == HL_LINK_RESET )		
							hlSrioResetAckID(port,brotherPort,truehopcount,localSwitchIndex);
						
						continue;
					}
				}
				DB(printf("find switch, switch id = %d, port = %d, portStatus = 0x%x, hopcount = %d!\n",switchNum,port,status,hopCount));
				/*check the port status,if the link partner reset then reconfig the link*/
				linkStatus = hlSrioCheckLink(status,port,truehopcount,localSwitchIndex,hostTargetID);
				if( linkStatus == HL_ERROR )
				{
					printf("!!!Link status is error,switch %d port %d!!!\n",switchId,port);
					return HL_ERROR;
				}				
				else if( linkStatus == HL_LINK_RESET )		
					hlSrioResetAckID(port,brotherPort,truehopcount,localSwitchIndex);
				
				/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				������������жϵ�������оƬ������
				��ȥͬ�����ӵĽ���оƬ��ackid��ֱ�ӷ��أ�
				����û�������Ľ���оƬȥ���ackid��ͬ������.
				!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

				/*check local switch has been reset*/
				if( (globalSwitchStatus[switchId]&0x2) == 0x2 )
				{					
					/*switch has been found ,because local switch has been reset, so it can not synchronize the ackid*/
					/*so i don't know whether the find switch has been reset*/
					globalSwitchStatus[switchNum] = 1;	
					printf("because the local switch has been reset, so it cannot synchronize the ackid,just continue!\n");
					continue;
				}
				
				returnValue=hlSrioRemoteConfigLock(hostTargetID,hopCount);				
				if(returnValue==HL_ALREADY_LOCK)
				{
					DB(printf("switch %d not reset!\n",switchNum));							
					/*switch has been found but not reset*/
					globalSwitchStatus[switchNum] = 1;
					hlSrioSwitchReconfig(pDrvCtrl,childHopCount,switchNum,localSwitchIndex);					
				}
				else if(returnValue==HL_LOCK)
				{
					DB(printf("switch %d has been reset!\n",switchNum));
					globalSwitchResetNum++;
					/*switch has been found and has been reset*/
					globalSwitchStatus[switchNum] = 3;
					
					/*this switch must have been reset !*/
					hlSrioConfigSetDiscover(hostTargetID,0xff,hopCount);
					/*set local switch virtual id*/
					swVirtID=switchNum+RIO_SW_FIRST_VIRTUAL_TGTID;
					(srioSwitchMethod.swtichControl)(pDrvCtrl, hostTargetID,0xff,hopCount,SET_SWITCH_ID, &swVirtID);
					hlSrioSwitchReconfig(pDrvCtrl,childHopCount,switchNum,localSwitchIndex);										
				}            
				else
				{
					printf("There is something wrong with the srio topo interconnect map!\n");
					printf("the switch not locked before!\n");
					return HL_ERROR;
				}                                                                        
			}                        
		}
		else
		{
			tgtId=hlSrioGetDeviceId(pSw,port,&srioDevIndex);
			switchNum=hlSrioGetSwitchNum(pHostAllSw, port, switchId,&brotherPort);
			if(tgtId!=0xff)
			{
				memset(&(pSw->hostCfgRouteTbl[srioDevIndex]),0,sizeof(SWITCH_ROUTE_NODE_INFO));
				pSw->srioDeviceNum--;
			}
			if(switchNum!=0xff)
			{
				pHostAllSw->switchInterConnectionInfo[switchId][switchNum].distance=0;
				hlSrioSwitchTblModify(pHostAllSw, switchId);
			}
		}			
	}                                    

	/*if any new srio device found*/
	(pSw->srioDeviceNum)+=newSrioDevNum;
	pHostAllSw->srioDevNum+=newSrioDevNum;

	return HL_OK;                        
}
        
void hlSrioDeviceEnableSet(UINT32 hostTargetId,UINT32 dstId)
{
	UINT32 temp = 0;
	UINT32 defaultHopcount = SRIO_ZYNQ_MAX_HOPCOUNT;				
	UINT32 mtRdata=0;			
	if (HL_OK == hlMaintRead(dstId,defaultHopcount,SRIO_PGCCSR_OFFSET, &mtRdata))
		temp = mtRdata;
	else
		hlMtRdFailReport (dstId, defaultHopcount, SRIO_PGCCSR_OFFSET, NULL);
				
	temp |= MASTER_ENABLE;
	hlMaintWrite(dstId,defaultHopcount,SRIO_PGCCSR_OFFSET,temp);				
}

HL_STATUS hlSrioCheckLink(
	UINT32  status,
	int 	port,
	UINT16  truehopcount,
	UINT32  localSwithIndex,
    UINT32  hostTargetID)
{
	/*��ʾ��ö˿������Ĵ������������Ҫ��ackid��ֵ��0*/
	if(((status&PORT_ERR) == PORT_ERR))
	{			
		if(((status&PORT_ERROR1) == PORT_ERROR1)||((status&PORT_ERROR2) == PORT_ERROR2)||((status&PORT_ERROR3) == PORT_ERROR3))
		{
			printf("578 port %d status is error! The connected board restart! clear the ackid...\n",port);
			if(localSwithIndex == 0)			
				SRIO_LOCAL_REG_WRITE(SRIO_ACKID_STATUS_OFFSET(port),0);			
			else			
				hlMaintWrite(0xff, truehopcount, SRIO_ACKID_STATUS_OFFSET(port),0);
			
			return HL_LINK_RESET;
		}		
		else
		{
			printf("port %d status is error! Skip the port!!!\n",port);
			return HL_ERROR;
		}		
	}
	else if(((status&PORT_ERROR4) == PORT_ERROR4)||((status&PORT_ERROR5) == PORT_ERROR5))
	{
		printf("1848 port %d status is error! The connected board restart! clear the ackid...\n",port);
		if(localSwithIndex == 0)				
			SRIO_LOCAL_REG_WRITE(SRIO_ACKID_STATUS_OFFSET(port),0);		
		else				
			hlMaintWrite(0xff, (UINT16)truehopcount, SRIO_ACKID_STATUS_OFFSET(port),0);
			
		return HL_LINK_RESET;
	}
	return HL_OK;
}


#if 0
/************************************************************
������оƬ��������������Ľ���оƬ
�����Ľ���оƬ��srio�ڵ��ackid���������
����оƬ�϶˿ڵ�ackid�Ͳ�ƥ���ˣ���ʱ��Ҫ
��������ackid��ʹ��������ȣ����ܼ���ö��
************************************************************/
void SetAckID(int hostid,int hostport,int port,int linkport)
{
	UINT32 status,inbound;
	UINT16 hopcount = 1;
	static int cc = 0;
	if( cc == 0 )
	{
		M_Enable_Local_Master(hostid);
		sysAWR32(0x90000e00,0x22800070,hostid);
		sysAWR32(0x90000e00,0x22800074,hostport);
		cc++;
	}
	status = SRIO_LOCAL_REG_READ(SRIO_PORT_SELFLOCK_OFFSET(port));
	status &= ~TRANSMIT_RECEIVE_ENABLE;	
	SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port), status);
	
	/*port self lock*/
	status = SRIO_LOCAL_REG_READ(SRIO_PORT_SELFLOCK_OFFSET(port));		
	SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port),status|2);
	SRIO_LOCAL_REG_WRITE(SRIO_LINK_REQUEST_OFFSET(port),4);
	inbound = SRIO_LOCAL_REG_READ(SRIO_LINK_RESPONSE_OFFSET(port));
	inbound = (inbound >>5 )&(0x1f);
	printf("inbound = 0x%x\n",inbound); 
	SRIO_LOCAL_REG_WRITE(SRIO_ACKID_STATUS_OFFSET(port),(0x10000000|(inbound<<8)|inbound));
		
	SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID,0xff);
	SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_PORT,port);
	/*port unlock*/
	SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port),status);
	printf("write link port,val = 0x%x\n",(0x1010)|(inbound<<24));		
	hlMaintWrite(0xff, hopcount, SRIO_ACKID_STATUS_OFFSET(linkport),((0x1010)|(inbound<<24)));
		
	printf("write local port,val = 0x%x\n",0x11000000|(inbound<<8)|inbound);
	SRIO_LOCAL_REG_WRITE(SRIO_ACKID_STATUS_OFFSET(port),(0x11000000|(inbound<<8)|inbound));
		
	status = SRIO_LOCAL_REG_READ(SRIO_PORT_SELFLOCK_OFFSET(port));
	status |= TRANSMIT_RECEIVE_ENABLE;	
	SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port), status);
	
	printf("finish reset the ack id!\n");	 
}

void test(int port,int linkport)
{
	UINT32 status,inbound;
	UINT16 hopcount = 1;
	/*
	status = SRIO_LOCAL_REG_READ(SRIO_PORT_SELFLOCK_OFFSET(port));
	status &= ~TRANSMIT_RECEIVE_ENABLE;	
	SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port), status);*/
		
	/*port self lock*/
	status = SRIO_LOCAL_REG_READ(SRIO_PORT_SELFLOCK_OFFSET(port));		
	SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port),status|2);
	SRIO_LOCAL_REG_WRITE(SRIO_LINK_REQUEST_OFFSET(port),4);
	inbound = SRIO_LOCAL_REG_READ(SRIO_LINK_RESPONSE_OFFSET(port));
	inbound = (inbound >>5 )&(0x1f);
	printf("inbound = 0x%x\n",inbound); 
	SRIO_LOCAL_REG_WRITE(SRIO_ACKID_STATUS_OFFSET(port),(0x10000000|(inbound<<8)|inbound));
		
	SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID,0xff);
	SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_PORT,port);
	/*port unlock*/
	SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port),status);
	printf("write link port,val = 0x%x\n",(0x1010)|(inbound<<24));		
	hlMaintWrite(0xff, hopcount, SRIO_ACKID_STATUS_OFFSET(linkport),((0x1010)|(inbound<<24)));
		
	printf("write local port,val = 0x%x\n",0x11000000|(inbound<<8)|inbound);
	SRIO_LOCAL_REG_WRITE(SRIO_ACKID_STATUS_OFFSET(port),(0x11000000|(inbound<<8)|inbound));

/*
	status = SRIO_LOCAL_REG_READ(SRIO_PORT_SELFLOCK_OFFSET(port));
	status |= TRANSMIT_RECEIVE_ENABLE;	
	SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port), status);
	*/
	printf("finish reset the ack id!\n");	 
}
#endif

/*************************************************************************************************
*       hlSrioResetAckID			synchronize the ackid
*							ע��:����ͬ��ackid��ʱ��,ԭ��ϣ����ͬ��
							�Ĺ�����,���˿ڵ�״̬���ó�ֻ���շ�
							ά����,�������շ����ݰ�,����578����оƬ
							�²���ͨ��,����1848��ȴ����������.����
							���ܸı�˿ڵ��շ�ʹ��λTRANSMIT_RECEIVE_ENABLE
**************************************************************************************************/
void hlSrioResetAckID(	
	int 	port,
	int 	linkpartnerport,
	UINT16  truehopcount,
	UINT32  localSwithIndex)
{
	UINT32 status,inbound;
	printf("!!!Find the reset board,local switch is %d, port is %d. now start reset the ack id!\n",localSwithIndex,port);
	if(localSwithIndex == 0)
	{
		/*port self lock*/
		status = SRIO_LOCAL_REG_READ(SRIO_PORT_SELFLOCK_OFFSET(port));		
		SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port),status|2);
		
		SRIO_LOCAL_REG_WRITE(SRIO_LINK_REQUEST_OFFSET(port),4);
		inbound = SRIO_LOCAL_REG_READ(SRIO_LINK_RESPONSE_OFFSET(port));
		
		inbound = (inbound >>5 )&(0x1f);	
		SRIO_LOCAL_REG_WRITE(SRIO_ACKID_STATUS_OFFSET(port),(0x10000000|(inbound<<8)|inbound));
		
		SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID,0xff);
		SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_PORT,port);
		/*port unlock*/
		SRIO_LOCAL_REG_WRITE(SRIO_PORT_SELFLOCK_OFFSET(port),status);
		hlMaintWrite(0xff,1,SRIO_ACKID_STATUS_OFFSET(linkpartnerport),((0x1010)|(inbound<<24)));
		SRIO_LOCAL_REG_WRITE(SRIO_ACKID_STATUS_OFFSET(port),(0x11000000|(inbound<<8)|inbound));		
		
		printf("!impossible---finish the ackid config\n");
		/*���˿ڴ���λ���*/
		status = SRIO_LOCAL_REG_READ(SRIO_PORTX_STATUS_OFFSET(port));		
		SRIO_LOCAL_REG_WRITE(SRIO_PORTX_STATUS_OFFSET(port),PORT_ERR_CLR|(status&0x40000000));			
		
	}
	else
	{				
		hlMaintRead(0xff, truehopcount,SRIO_PORT_SELFLOCK_OFFSET(port),&status);	
		hlMaintWrite(0xff, truehopcount, SRIO_PORT_SELFLOCK_OFFSET(port),status|2);		

		hlMaintWrite(0xff, truehopcount, SRIO_LINK_REQUEST_OFFSET(port),4);
		hlMaintRead(0xff, truehopcount,SRIO_LINK_RESPONSE_OFFSET(port),&inbound);

		inbound = (inbound >>5 )&(0x1f);
		hlMaintWrite(0xff, truehopcount, SRIO_ACKID_STATUS_OFFSET(port),(0x10000000|(inbound<<8)|inbound));

		hlMaintWrite(0xff, truehopcount, SRIO_ROUTE_CFG_DESTID,0xff);						
		hlMaintWrite(0xff, truehopcount, SRIO_ROUTE_CFG_PORT,port);		

		/*port unlock*/
		hlMaintWrite(0xff, truehopcount, SRIO_PORT_SELFLOCK_OFFSET(port),status);			

		hlMaintWrite(0xff, truehopcount+1, SRIO_ACKID_STATUS_OFFSET(linkpartnerport),((0x1010)|(inbound<<24)));	
		hlMaintWrite(0xff, truehopcount, SRIO_ACKID_STATUS_OFFSET(port),(0x11000000|(inbound<<8)|inbound));
		
		printf("!!!impossible---finish the ackid config\n");
		/*���˿ڴ���λ���*/
		hlMaintRead(0xff, truehopcount,SRIO_PORTX_STATUS_OFFSET(port),&status);	
		hlMaintWrite(0xff, truehopcount, SRIO_PORTX_STATUS_OFFSET(port),PORT_ERR_CLR|(status&0x40000000));		
	}
}

/********************************************************************************
*       hlSrioReConfigRoute		reconifg the reset switch route table
*							
********************************************************************************/
void hlSrioReConfigRoute(RIO_DRV_CTRL *pDrvCtrl,int hostTargetID)
{        
	int i,j,k;
	UINT16 hopcount=0;
	int tgtId=0xff, srioDevId;
	int port;
	hostAllSwTbl *pHostAllSwTbl=NULL;
	SWITCH_ROUTE_TBL *pSw=NULL;
	int switchCount = 0;
	UINT16 truehopcount = 0;
	UINT32 tempport = 0xff;
	pHostAllSwTbl=pDrvCtrl->pAllSwTbl;		
                        
	while(switchCount<globalSwitchResetNum)
	{
		for(i=0;i<(pHostAllSwTbl->switchNum);i++)
		{
			if( (globalSwitchStatus[i]&0x2) != 0x2 )
				continue;

			DB(printf("reconifg the switch %d route table !\n",i));						
			hopcount=(UINT16)pHostAllSwTbl->switchInterConnectionInfo[0][i].distance;  
			truehopcount=hopcount;							
			tgtId = i+RIO_SW_FIRST_VIRTUAL_TGTID;			
			for(j=0;j<pHostAllSwTbl->switchNum;j++)
			{
				if(i==j)
					continue;
				pSw=pHostAllSwTbl->hostAllSwInfo[j];
				port=hlSrioLoadPortGet(&pHostAllSwTbl->switchInterConnectionInfo[i][j]);
				srioDevId=j+RIO_SW_FIRST_VIRTUAL_TGTID;
				if(hopcount == 0)   
				{
					do{
						SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID, srioDevId);                                    
						SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_PORT, port);
						SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID, srioDevId); 
						tempport = SRIO_LOCAL_REG_READ(SRIO_ROUTE_CFG_PORT);
					}while(tempport!=port);
				}
				else
				{
					(srioSwitchMethod.switchLoadLUTtoAll)(pDrvCtrl,hostTargetID,tgtId,truehopcount,port, srioDevId);
				}					
				DB(printf("local switch %d switch id=%d ,port=%d  hopcount = %d!\n",j,srioDevId,port,hopcount));
				for(k=0;k<pSw->srioDeviceNum;k++)
				{                                
					srioDevId=pSw->hostCfgRouteTbl[k].nodeIndex;
					port=hlSrioLoadPortGet(&pHostAllSwTbl->switchInterConnectionInfo[i][j]);												
					/*update the switch route table*/
					if(hopcount == 0)       
					{
						do{
							SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID, srioDevId);                                    
							SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_PORT, port);
							SRIO_LOCAL_REG_WRITE(SRIO_ROUTE_CFG_DESTID, srioDevId); 
							tempport = SRIO_LOCAL_REG_READ(SRIO_ROUTE_CFG_PORT);
						}while(tempport!=port);
					}
					else
					{
						(srioSwitchMethod.switchLoadLUTtoAll)(pDrvCtrl,hostTargetID,tgtId,truehopcount,port,srioDevId);
						DB(printf("id = %d, port = %d\n",srioDevId,port));
					}						
				}
			}       
			switchCount++;
		}		
	}        
}
 
HL_STATUS hlMasterResetEndpoint(UINT32 hostTargetID)
{	  
	int j,k,srioDevId;
	int i=0;
	SWITCH_ROUTE_TBL *pSw=NULL;
	UINT32 returnValue = 0;
	if( globalSwitchResetNum > 0 )
	{		 
		DB(printf("Master the reset endpoint!\n"));
		if( pDrvCtrl->pAllSwTbl == NULL )
			return HL_ERROR;
					 
		for(j=0;j<pDrvCtrl->pAllSwTbl->switchNum;j++)
		{	 
			if( i == globalSwitchResetNum )
				break;
			if( (globalSwitchStatus[j]&0x2) != 0x2 )
				continue;
			
			pSw = pDrvCtrl->pAllSwTbl->hostAllSwInfo[j];
			if( pSw == NULL )
				continue;

			i++;						 
			for(k=0;k<SWITCH_LOCAL_DEV_NUM;k++)
			{				
				if(pSw->hostCfgRouteTbl[k].deviceStyle==0||pSw->hostCfgRouteTbl[k].deviceStyle==SRIO_HOST)
					continue;
				   
				srioDevId=pSw->hostCfgRouteTbl[k].nodeIndex;

				/*���������ڵ�����*/
				if( srioDevId >=0xc0 && srioDevId <=0xdf)
					continue;
				
				DB(printf("srioDevId id = %d--0x%x\n",srioDevId,srioDevId));
				hlSrioDeviceEnableSet(hostTargetID, srioDevId);
			}
		}
		return HL_OK;
	}
	else
		return returnValue;
}


