/*
 * SD_SPI.c
 *
 *  Created on: 2014��11��20��
 *      Author: LFC
 */

#include "SD_HardWare.h"
#include "SPI.h"

#define    SD_WRITE_DELAY               100
#define    SD_EMPTY_CLK                  0xff
#define    SD_EMPTY_DATA               0x00
#define    SD_SPI_CRC                       0xff
#define    SD_CMD0_CRC                  0x95
#define    SD_CMD0                          0+0x40
#define    SD_CMD1                          1+0x40
#define    SD_CMD17                        17+0x40
#define    SD_CMD24                        24+0x40
#define    SD_CMD41                        41+0x40
#define    SD_CMD55                        55+0x40
#define    SD_COMMAND_ACK          0x00
#define    SD_RESET_ACK                   0x01
#define    SD_DATA_ACK                    0xfe
#define    SD_WRITE_ACK                   0x05

#define    SD_TIMEOUT                      100
#define    TIMEOUT                            200

unsigned char Write_Command_SD(unsigned char *CMD);
unsigned char SD_Write_Sector(unsigned long Addr, unsigned char *Ptr, unsigned int FirstNum, unsigned int Num);
unsigned char SD_Read_Sector(unsigned long Addr, unsigned char *Ptr, unsigned int FirstNum, unsigned int Num);
unsigned char SD_Init();
unsigned char SD_Reset();
unsigned char SD_Set_SPI();

/**************************************************************************
 *��      �ƣ�Write_Command_SD(unsigned char *CMD);
 *��      �ܣ�SD����SPIģʽ��,��SD����д��6���ֽڵ�����
 *��ڲ�����CMD��ָ����6���ֽ����������
 *���ڲ�������
 *˵      ������SD����һ��д��6���ֽڵ�����
 *��      ����Write_Command_SD(CMD);
 *************************************************************************/

unsigned char Write_Command_SD(unsigned char *CMD)
{
    unsigned char tmp = 0;
    unsigned char i = 0;
    SD_CS_High();
    SD_Write_Byte(0xff);
    SD_CS_Low();
    SD_Write_Frame(CMD, 6);
    SD_Read_Byte();

    i = SD_TIMEOUT;

    do
    {
    	tmp = SD_Read_Byte();
    	i--;
    }while((tmp == 0xff) && i);
    return (tmp);
}

/**************************************************************************
 *��      �ƣ�SD_Reset()
 *��      �ܣ���λSD��
 *��ڲ�������
 *���ڲ�����0����λʧ��
                  1����λ�ɹ�
 *˵      �����ϵ��SD��λSDģʽ�����ȸ�λ��������ΪSPIģʽ
 *��      ����Write_Command_SD(CMD);
 *************************************************************************/

unsigned char SD_Reset()
{
    unsigned char i = 0, temp = 0;
    unsigned char CMD[6] = {SD_CMD0, 0x00, 0x00, 0x00, 0x00, SD_CMD0_CRC};
    SD_CS_High();

    for(i = 0; i < 0x0f; i++)
    {
    	SD_Write_Byte(SD_EMPTY_CLK);
    }
    i = TIMEOUT;

    do
    {
    	temp = Write_Command_SD(CMD);
    	i--;
    }while((temp != SD_RESET_ACK) && i);

    if( i == 0 )    return (0);
    else             return (1);
}

/**************************************************************************
 *��      �ƣ�SD_Set_SPI()
 *��      �ܣ���SD����ΪSPIģʽ
 *��ڲ�������
 *���ڲ�����0����λʧ��
                  1����λ�ɹ�
 *˵      �����ϵ��SD��λSDģʽ�����ȸ�λ��������ΪSPIģʽ
 *��      ����Write_Command_SD(CMD);
 *************************************************************************/

unsigned char SD_Set_SPI()
{
    unsigned char i = 0, temp = 0xff;
    //unsigned char CMD[6] = {SD_CMD1, 0x00, 0x00, 0x00, 0x00, SD_SPI_CRC};
    unsigned char CMD[6] = {0};

    CMD[0] = SD_CMD1;
    CMD[1] = ( (0x00ffc000 & 0xff000000) >> 24 );
    CMD[2] = ( (0x00ffc000 & 0x00ff0000) >> 16 );
    CMD[3] = ( (0x00ffc000 & 0x0000ff00) >> 8 );
    CMD[4] =    0x00ffc000 & 0x000000ff;
    CMD[5] = SD_SPI_CRC;
    i = TIMEOUT;
    do
    {
    	CMD[0] = SD_CMD1;
    	temp = Write_Command_SD(CMD);
    	if(temp == 0x00)
    	{
    		SD_CS_High();
    	}
    	i--;
    }while((temp != SD_COMMAND_ACK) && i);

    if(i == 0)
        return  (0);
    else
        return  (1);
}

/**************************************************************************
 *��      �ƣ�SD_Init()
 *��      �ܣ���ʼ��SD��ΪSPIģʽ
 *��ڲ�������
 *���ڲ�����0����ʼ��ʧ��
                  1����ʼ���ɹ�
 *˵      ������
 *��      ����SD_Init();
 *************************************************************************/

unsigned char SD_Init()
{
    unsigned char temp = 0;

    SD_Low_Speed();

    temp = SD_Reset();

    if(temp == 0)    return   (0);
    temp = SD_Set_SPI();
    if(temp == 0)    return   (0);
    SD_High_Speed();
    return      (1);
}

/**************************************************************************
 *��      �ƣ�SD_Read_Sector()
 *��      �ܣ�����SD����ָ��λ�õ�ָ���������ݵ�����
 *��ڲ�����Addr������������ַ
                  *Ptr��ָ���Ŷ�ȡ���ݵ����ݻ���
                  FirstNum���׸��ֽڵ�ƫ�Ƶ�ַ
                  Num����ȡ���ݸ���
 *���ڲ�����0����ȡʧ��
                  1����ȡ�ɹ�
 *˵      ������
 *��      ����SD_Read_Sector(83241, buffer, 0, 100);
 *************************************************************************/

unsigned char SD_Read_Sector(unsigned long Addr, unsigned char *Ptr, unsigned int FirstNum, unsigned int Num)
{
    unsigned char temp = 0;
    unsigned int i = 0;
    unsigned int EndNum = 0;
    unsigned char CMD[6] = {0};

    CMD[0] = SD_CMD17;
    Addr = Addr << 9;
    CMD[1] = ( (Addr & 0xff000000) >> 24 );
    CMD[2] = ( (Addr & 0x00ff0000) >> 16 );
    CMD[3] = ( (Addr & 0x0000ff00) >> 8 );
    CMD[4] =    Addr & 0x000000ff;
    CMD[5] = SD_SPI_CRC;
    i = TIMEOUT;
    do
    {
    	temp = Write_Command_SD(CMD);
    	i--;
    }while( (temp != SD_COMMAND_ACK) && i);

    if(i == 0)
    {
    	SD_CS_High();
    	return  (0);
    }

    i = TIMEOUT;

    do
    {
    	temp = SD_Read_Byte();
    	i--;
    }while((temp != SD_DATA_ACK) && i);

    if(i == 0)
    {
    	SD_CS_High();
    	return  (0);
    }

    EndNum = FirstNum + Num;

    for(i = 0; i < FirstNum; i++)
    {
    	SD_Read_Byte();
    }
    SD_Read_Frame(Ptr, Num);
    for(i = 0; i < 512 - EndNum; i++)
    {
    	SD_Read_Byte();
    }

    SD_Read_Byte();
    SD_Read_Byte();

    SD_CS_High();
    return   (1);
}

/**************************************************************************
 *��      �ƣ�SD_Write_Sector()
 *��      �ܣ���SD����ָ��λ��д��ָ������������
 *��ڲ�����Addr������������ַ
                  *Ptr��ָ���д���ݵ����ݻ���
                  FirstNum��д��������׸��ֽڵ�ƫ�Ƶ�ַ
                  Num��д�����ݸ���
 *���ڲ�����0��д��ʧ��
                  1��д��ɹ�
 *˵      ������
 *��      ����SD_Write_Sector(83241, buffer, 0, 100);
 *************************************************************************/

unsigned char SD_Write_Sector(unsigned long Addr, unsigned char *Ptr, unsigned int FirstNum, unsigned int Num)
{
    unsigned char temp = 0;
    unsigned int i = 0;
    unsigned int EndNum = 0;
    unsigned char CMD[6] = {0};

    CMD[0] = SD_CMD24;
    Addr = Addr << 9;
    CMD[1] = ( (Addr & 0xff000000) >> 24 );
    CMD[2] = ( (Addr & 0x00ff0000) >> 16 );
    CMD[3] = ( (Addr & 0x0000ff00) >> 8 );
    CMD[4] =    Addr & 0x000000ff;
    CMD[5] = SD_SPI_CRC;
    i = TIMEOUT;
    do
    {
    	temp = Write_Command_SD(CMD);
    	i--;
    }while( (temp != SD_COMMAND_ACK) && i);

    if(i == 0)    return  (0);

    for(i = 0; i < SD_WRITE_DELAY; i++)
    {
    	SD_Write_Byte(SD_EMPTY_CLK);
    }

    SD_Write_Byte(SD_DATA_ACK);

    EndNum = FirstNum + Num;

    for(i = 0; i < FirstNum; i++)
    {
    	SD_Write_Byte(SD_EMPTY_DATA);
    }

    SD_Write_Frame(Ptr, Num);

    for(i = 0; i < 512 - EndNum; i++)
    {
    	SD_Write_Byte(SD_EMPTY_DATA);
    }

    SD_Write_Byte(0xff);
    SD_Write_Byte(0xff);

    temp = SD_Read_Byte();
    temp = temp & 0x01f;

    if(temp != SD_WRITE_ACK)
    {
    	SD_CS_High();
    	return  (0);
    }

    i = TIMEOUT;

    do
    {
    	i--;
    }while(SD_Read_Byte() != 0xff && i);

    if(i == 0)
    {
    	SD_CS_High();
    	return  (0);
    }

    SD_CS_High();
    return   (1);
}
