#ifndef _HLSRIO_DEBUG_H
#define  _HLSRIO_DEBUG_H

#ifdef HLSRIO_DEBUG_INTERFACE

#include "hlType.h"

/*
 * ��1848�˿ڵ�ͳ�ƹ��ܴ�,��������ͳһ�˿��յ���ת���İ���.
 * ʹ��hlSetPortCounter(1)
 * ��ʹ��hlSetPortCounter(0)
 */
void hlSetPortCounter(char bEnable);

/*
 * ��ʾָ������оƬ�����мĴ�����ֵ�������hlSrioInit�󣬿���ͨ���ú�����ָ����
 * ����оƬ�����мĴ���ֵ������ʾ��swIndexΪָ������оƬ�������š�
 */
void hlReadSwitchAllReg(UINT8 swIndex);

/*
 * ��ɶ�ָ������оƬָ��offset�ļĴ�������д�����������hlSrioInit�󣬿���ͨ���ú�
 * ����ָ���Ľ���оƬ�е�ָ���Ĵ���ֵ����д������
 * swIndexΪָ���Ľ���оƬ������,offsetΪ����оƬ�ڲ��ļĴ���ƫ����,dataΪд���ֵ��
 */
void hlWriteReg(UINT8 swIndex,UINT32 offset,UINT32 data);


/*
 * ��ɶ�ָ������оƬָ��offset�ļĴ������ж������������hlSrioInit�󣬿���ͨ���ú�
 * ����ָ���Ľ���оƬ�е�ָ���Ĵ���ֵ���ж�������
 * swIndexΪָ���Ľ���оƬ������,offsetΪ����оƬ�ڲ��ļĴ���ƫ������
 */
UINT32 hlReadReg(UINT8 swIndex,UINT32 offset);

/*
 * ��ɶ�ָ��EndPointָ��offset�ļĴ������ж������������hlSrioInit�󣬿���ͨ���ú�
 * ����ָ����EndPoint�е�ָ���Ĵ���ֵ���ж�������
 * dstIdΪָ����EndPoint Id,offsetΪEndPoint�ڲ��ļĴ���ƫ������
 */
UINT32 hlReadEP(UINT32 dstId,UINT32 offset);

/*
 * ��ɶ�ָ��EPָ��offset�ļĴ�������д�����������hlSrioInit�󣬿���ͨ���ú�
 * ����ָ����EP�е�ָ���Ĵ���ֵ����д������
 * dstIdΪָ����EP������,offsetΪEP�ڲ��ļĴ���ƫ����,writedataΪд���ֵ��
 */
void hlWriteEP(UINT32 dstId,UINT32 offset, UINT32 writedata);

/*
 * ��ɽ������Ͻ���оƬ�˿ڵ�ͳ�ƹ�����ʾ
 */
void hlSrioPortStaticShow();

#endif
#endif

