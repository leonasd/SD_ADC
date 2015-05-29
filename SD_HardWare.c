/*
 * SD_HardWare.c
 *
 *  Created on: 2014年11月21日
 *      Author: LFC
 */
#include <msp430.h>
#include "SPI.h"

/**************************************************************************
 *名      称：SD_Low_Speed()
 *功      能：SD卡在SPI模式下，使能SPI的时钟为低速(300k左右)
 *入口参数：无
 *出口参数：无
 *说      明：读取速度设置为300K左右
 *范      例：SD_Low_Speed();
 *************************************************************************/

void SD_Low_Speed()
{
    SPI_LowSpeed();
}

/**************************************************************************
 *名      称：SD_High_Speed()
 *功      能：SD卡在SPI模式下，使能SPI的时钟为高速
 *入口参数：无
 *出口参数：无
 *说      明：读取速度设置为高速
 *范      例：SD_High_Speed();
 *************************************************************************/

void SD_High_Speed()
{
    SPI_HighSpeed();
}

/**************************************************************************
 *名      称：SD_CS_High()
 *功      能：SD卡在SPI模式下，使能CS管脚为高电平
 *入口参数：无
 *出口参数：无
 *说      明：CS管脚可以自己指定
 *范      例：SD_CS_High();
 *************************************************************************/

void SD_CS_High()
{
    SPI_CS_High();
}

/**************************************************************************
 *名      称：SD_CS_Low()
 *功      能：SD卡在SPI模式下，使能CS管脚为低电平
 *入口参数：无
 *出口参数：无
 *说      明：CS管脚可以自己指定
 *范      例：SD_CS_Low();
 *************************************************************************/

void SD_CS_Low()
{
    SPI_CS_Low();
}

/**************************************************************************
 *名      称：SD_Write_Byte(unsigned char value)
 *功      能：SD卡在SPI模式下，向SD卡中写入一个字节数据
 *入口参数：value：写入数据
 *出口参数：无
 *说      明：可以写入一个字节
 *范      例：SD_Write_Byte(value);
 *************************************************************************/

void SD_Write_Byte(unsigned char value)
{
    unsigned char temp = 0;

    do
    {
    	temp = SPI_TxFrame(&value, 1);
    }while(temp == 0);
}

/**************************************************************************
 *名      称：SD_Write_Frame(unsigned char *pBuffer, unsigned int size)
 *功      能：SD卡在SPI模式下，向SD卡中写入size个字节数据
 *入口参数：*pBuffer：要写入数据的头指针
                  size：要写入的数据个数
 *出口参数：无
 *说      明：可以写入size个字节
 *范      例：无
 *************************************************************************/

void SD_Write_Frame(unsigned char *pBuffer, unsigned int size)
{
    unsigned char temp = 0;

    do
    {
    	temp = SPI_TxFrame(pBuffer, size);
    }while(temp == 0);
}

/**************************************************************************
 *名      称：SD_Read_Byte()
 *功      能：SD卡在SPI模式下，从SD卡中读取一个字节数据
 *入口参数：无
 *出口参数：value：读出的数据
 *说      明：可以读出一个字节
 *范      例：temp = SD_Read_Byte();
 *************************************************************************/

unsigned char SD_Read_Byte()
{
    unsigned char temp = 0;
    unsigned char value = 0;
    do
    {
    	temp = SPI_RxFrame(&value, 1);
    }while(temp == 0);
    return value;
}

/**************************************************************************
 *名      称：SD_Read_Frame(unsigned char *pBuffer, unsigned int size)
 *功      能：SD卡在SPI模式下，从SD卡中读取size个字节数据
 *入口参数：*pBuffer：存储读出数据的头指针
                  size：要读出的数据个数
 *出口参数：无
 *说      明：可以读出size个字节
 *范      例：无
 *************************************************************************/

void SD_Read_Frame(unsigned char *pBuffer, unsigned int size)
{
    unsigned char temp = 0;

    do
    {
    	temp = SPI_RxFrame(pBuffer, size);
    }while(temp == 0);
}
