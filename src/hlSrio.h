#ifndef _HLSRIO_H
#define _HLSRIO_H

#include "hlType.h"

/*
 * A--578(port 14)
 * B--578(port 6)
 * C--578(port 0)
 * D--578(port 12)
 * O--578(port 4)
 */


/*
 * 578״̬��ʾ��������ʾ578��5���˿ڵ�״̬���ú��������ڽ�����ö����ɺ�ʹ��
 * ���û�н����壬�򽻻���û��ö�ٵ��ð壬��Ҫ������hlSetLoaclRouteTable����
 * ��ʹ��
 */
HL_STATUS hlShow578Port();

/*
 * �ֶ�·�����ú������Զ���AƬid����Ϊ1��BƬid����Ϊ2��CƬid���ó�3��DƬid����Ϊ4
 * �ú������ڽ�����ö�ٲ��ɹ���״̬�¿�578�˿�״̬ʱ��Ҫ�����еĺ���
 */
void hlSetLoaclRouteTable();

/*
 * ��ȡ578�ļĴ������ú��������ڽ�����ö����ɺ�ʹ��
 * ���û�н����壬�򽻻���û��ö�ٵ��ð壬��Ҫ������hlSetLoaclRouteTable����
 * ��ʹ��
 */
void hlRead578(UINT32 offset);

/*
 * д578�ļĴ������ú��������ڽ�����ö����ɺ�ʹ��
 * ���û�н����壬�򽻻���û��ö�ٵ��ð壬��Ҫ������hlSetLoaclRouteTable����
 * ��ʹ��
 */
void hlWrite578(UINT32 offset,UINT32 data);

/*
 * ��ȡ����A/B/C/D�ڵ�Ĵ������ú��������ڽ�����ö����ɺ�ʹ��
 * ���û�н����壬�򽻻���û��ö�ٵ��ð壬��Ҫ������hlSetLoaclRouteTable����
 * ��ʹ��,dstid��Ҫ��ȡ�ڵ��id��offset��ƫ����
 */
void hlReadLocal(UINT16 dstid,UINT32 offset);

/*
 * д����A/B/C/D�ڵ�Ĵ������ú��������ڽ�����ö����ɺ�ʹ��
 * ���û�н����壬�򽻻���û��ö�ٵ��ð壬��Ҫ������hlSetLoaclRouteTable����
 * ��ʹ��,dstid��Ҫ��ȡ�ڵ��id��offset��ƫ����,data��д��ֵ
 */
void hlWriteLocal(UINT16 dstid,UINT32 offset,UINT32 data);

/*
 * 578��ÿ��port�ṩ��6�ּ������������������Կ����ã������������Ͱ����������ǽ��հ������������İ����ͣ����ȼ�
 * ����������������Ժ�ͿɶԶ˿ڽ��м���.
 * portSel:0-16
 * counterSel:0-5
 * portDirect:0-���գ�1-����
 * packetType:0-7
 * 000 = Count all unicast request packets only. The response packet
	maintenance packets, and maintenance packets with hop 
	count of 0 are excluded from this counter. 
	001 = Count all unicast packet types. This counter includes all 
	request, response, maintenance packets (including the 
	maintenance packets with hop count 0). 
	010 = Count all retry control symbols only.
	011 = Count all control symbols (excluding retry control symbols). 
	100 = Count every 32-bits of unicast data. This counter counts all 
	accepted unicast packets data (including header). 
	101 = Count all multicast packets only.
	110 = Count all multicast control symbols.
	111 = Count every 32-bits of multicast data. This counter includes 
	counting the entire multicast packet (including header). 
 */
void hl578PortStatisticSet(int portSel,int counterSel,int portDirect,int packetType);


/*
 * 578��ÿ��port�ṩ��6�ּ����������ڼ�¼578�˿ڵİ�ͳ�Ƹ��� 
 * portSel:0-16
 * counterSel:0-5  
 */
void hl578PortStatisticShow(int portSel,int counterSel);


/*
 * ö�ٺ������Զ����ö�ټ�·�����ù���
 */
HL_STATUS hlSrioInit();

/*
 * ��ö�ٺ�������ĳ�鴦�����ö��ʱ��δ���ҵ�������ʹ�øú����Դ���������ö�١��ú�����Ҫ��hlSrioInit��ɺ���ʹ�ã�������ܻ������⡣
 */
HL_STATUS hlSrioReInit();

/*
 * �ع���������ĳ�鴦��������󣬿���ʹ�øú����������Ĵ�������id��·�ɵ��������á�
 * �ú�����Ҫ��hlSrioInit��ɺ���ʹ�ã�������Ҫ�ڴ����������ɺ���ʹ��
 * ʹ��ǰ,�����е������ڵ㲻������ô�����ϵĽڵ㷢������,���������⡣
 */
HL_STATUS hlSrioReconfig();

/*
 * ��ʾSrio��������˽ṹ�������hlSrioInit�󣬿���ͨ���ú�����ʾ����ɨ�赽�Ľڵ㼰����оƬ������ʾ����֮������ӹ�ϵ��
 */
void hlSrioTopoShow();

/*
 * ��ʾSrio���������е��豸�ڵ㣬�����hlSrioInit�󣬿���ͨ���ú�����Srio���������е��豸�ڵ������ʾ��
 */
void hlSrioDevIDShow();

/*
 * ��ʾSrio������ÿ������оƬ�����õ�·�ɱ������hlSrioInit�󣬿���ͨ���ú�����ʾÿ�齻��оƬ�����õ�·�ɣ�����ͨ�����鿴���ؾ����״̬��
 */
void hlSrioLUTShow();

/*
 * ����Srio�����е��鲥����ģʽ�������hlSrioInit�󣬿���ͨ���ú��������鲥id��
 * �ú������ֻ�ܱ�����7��,Ҳ����˵���ֻ������7����,�鲥��id��Χ��f1-f7
 * ���е�ֵΪ�豸�ڵ�id,��һ���ڵ�idΪ���鲥id,��f1-f7�е�����һ��;
 * ���нڵ�id��0xffff ��ʾ����.Ŀǰ������ͬʱ����19���ڵ�,��ֵ���Ը��������޸�
 * ��Ҫע�����,������еĽڵ��Ƿ��Ͷ�,���Լ����ղ������ݵ�!���ڹ㲥Ҳͬ��.
 * example:
 * unsigned short group0[10]={0xf1,10,11,12,0xffff};
 * hlSrioSetGroup(group0);
 * ��ʾ�ڵ�10,11,12����f1�������,������idΪ0xf1ʱ,����ͬʱ�ᷢ�͸��ڵ�10,11,12
 */
void hlSrioSetGroup(unsigned short group[]);


/*
 * ����Srio����Ĺ㲥ģʽ�������hlSrioInit�󣬿���ͨ���ú������ù㲥ģʽ
 * �����е�����һ���ڵ���0xf8��id��������,���еĽڵ㶼���յ�
 * ��Ҫע����Ƿ��ͷ��Լ��ղ�������,���鲥ģʽһ��
 */
void hlSrioSetBroadCast();
/*--------------------------------------------------------------------------------------------------*/
	
/*
 * �Լ�����·�ɱ�����ݽṹ 
 * dstID:Ŀ��id
 * portNo:Ŀ��id��ȥ�Ķ˿�
 * hopCount:�����Ǳ���id,�������id
 */
typedef struct _ROUTE_TABLE_
{
        unsigned int dstID;
        unsigned int portNo;
        unsigned int hopCount;
}ROUTE_TABLE;

/*
 *����Լ����� �����·�ɱ�,���ñ�����豸id(deviceID)�ӿں���
 *�ú������ǽ�AƬ��Ϊ���ڵ�,�˿ں�Ϊ14
 *����hostIDΪ���ڵ���豸ID
 *����entryΪ��Ҫ���õ�·�ɱ�,��ʽ�ο�����,������0xFFFFΪ���
 *
						dstID		portNo	hopCount
 ROUTE_TABLE table[30] = {{ 1,       	       0,            0     },									   	
                          { 2,       		6,            0     },
                          { 3,       		12,           0     },
                          { 4,       		0,             1     },
                          { 23,       		6,             1     },
                          { 0xFFFF,   0xFFFF,      0xFFFF}};
 *hopCountΪ0����Ŀ�Ǳ����·��;hopCount��0����Ŀ�Ǳ��巢�͸���Ӧ��deviceID
 *��Ҫ���ó�ȥ�Ķ˿�
 *
 */
void SetRouteTable(int hostID,ROUTE_TABLE entry[]);


#endif 
