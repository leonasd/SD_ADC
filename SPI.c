/*
 * SPI.c
 *
 *  Created on: 2014年11月20日
 *      Author: LFC
 */

#include <msp430.h>

#define SPI_SIMO            BIT3
#define SPI_SOMI            BIT4
#define SPI_CLK             BIT7
#define SPI_CS              BIT3

#define SPI_CLK_SEL         P2SEL
#define SPI_SIMO_SEL        P3SEL
#define SPI_SOMI_SEL        P3SEL
#define SPI_CLK_DIR         P2DIR
#define SPI_SIMO_DIR        P3DIR
#define SPI_SOMI_DIR        P3DIR

#define SPI_CLK_OUT         P2OUT
#define SPI_SIMO_OUT        P3OUT
#define SPI_SOMI_OUT        P3OUT
#define SPI_SOMI_REN        P3REN

#define SPI_CS_SEL          P2SEL
#define SPI_CS_OUT          P2OUT
#define SPI_CS_DIR          P2DIR

unsigned char *SPI_Tx_Buffer;
unsigned char *SPI_Rx_Buffer;

unsigned int SPI_Tx_Size = 0;
unsigned int SPI_Rx_Size = 0;

unsigned char SPI_Rx_Or_Tx = 0;
static void SPI_TxISR();
static void SPI_RxISR();
/**************************************************************************
 *名      称：SPI_init
 *功      能：对硬件SPI进行初始化设置
 *入口参数：无
 *出口参数：无
 *说      明：初始化单片机SPI功能
 *范      例：无
 *************************************************************************/

void SPI_init()
{
    SPI_CLK_SEL |= SPI_CLK;
    SPI_SIMO_SEL |= SPI_SIMO;
    SPI_SOMI_SEL |= SPI_SOMI;

    SPI_CLK_DIR |= SPI_CLK;
    SPI_SIMO_DIR |= SPI_SIMO;
    SPI_SOMI_REN  |= SPI_SOMI;
    SPI_SOMI_OUT |= SPI_SOMI;

    SPI_CS_SEL &= ~SPI_CS;
    SPI_CS_OUT |= SPI_CS;
    SPI_CS_DIR |= SPI_CS;

    UCA0CTL1 |= UCSWRST;

    UCA0CTL0 = UCCKPL + UCMSB + UCMST + UCMODE_0 + UCSYNC;

    UCA0CTL1 = UCSWRST + UCSSEL_2;

    UCA0BR0 = 150;
    UCA0BR1 = 0;
    UCA0MCTL = 0;

    UCA0CTL1 &= ~UCSWRST;
    UCA0IE |= UCRXIE + UCTXIE;
    __enable_interrupt();
}

/**************************************************************************
 *名      称：SPI_CS_High()
 *功      能：3线硬件SPI模式，使能CS为高电平
 *入口参数：无
 *出口参数：无
 *说      明：CS管脚可以任意指定
 *范      例：无
 *************************************************************************/

void SPI_CS_High()
{
    SPI_CS_OUT |= SPI_CS;
}

/**************************************************************************
 *名      称：SPI_CS_Low()
 *功      能：3线硬件SPI模式，控制使能CS为低电平
 *入口参数：无
 *出口参数：无
 *说      明：CS管脚可以任意指定
 *范      例：无
 *************************************************************************/

void SPI_CS_Low()
{
    SPI_CS_OUT &= ~SPI_CS;
}

/**************************************************************************
 *名      称：SPI_Interrupt_Sel(unsigned char onOff)
 *功      能：开启发送或接受中断
 *入口参数：onOff = 0：关闭发送中断，打开接受中断
                  onOff = 1：打开发送中断，关闭接受中断
                  onOff = 2：打开发送中断，打开接受中断
 *出口参数：无
 *说      明：使用此函数来控制选择当前中断模式
 *范      例：SPI_Interrupt_Sel(0);
                  SPI_Interrupt_Sel(1);
 *************************************************************************/

void SPI_Interrupt_Sel(unsigned char onOff)
{
    if(onOff == 0)
    {
    	UCA0IE &= ~UCTXIE;
    	UCA0IE |= UCRXIE;
    }
    else if(onOff == 1)
    {
    	UCA0IE &= ~UCRXIE;
    	UCA0IE |= UCTXIE;
    }
    else
    {
    	UCA0IE |= UCRXIE;
    	UCA0IE |= UCTXIE;
    }
}

/**************************************************************************
 *名      称：SPI_RxFrame(unsigned char *pBuffer, unsigned int size)
 *功      能：3线硬件SPI模式下，接收指定数目的字节
 *入口参数：*pBuffer：指向存放接受数据的数组
                  size：要接收的字节数
 *出口参数：0：当前SPI被占用
                  1：当前SPI空闲
 *说      明：使用此函数接收一帧数据
 *范      例：SPI_RxFrame(CMD, 6);  接受6个字节放入CMD中
 *************************************************************************/

unsigned char SPI_RxFrame(unsigned char *pBuffer, unsigned int size)
{
    if(size ==0 )                        return  (1);
    if(UCA0STAT & UCBUSY)    return  (0);
    _disable_interrupts();
    SPI_Rx_Or_Tx = 0;
    SPI_Rx_Buffer = pBuffer;
    SPI_Rx_Size = size - 1;
    SPI_Interrupt_Sel(SPI_Rx_Or_Tx);
    _enable_interrupts();
    UCA0TXBUF = 0xff;             //在接收模式下，也先发送一次空字节，一遍提供时钟通信

    _bis_SR_register(LPM0_bits);

//	unsigned int gie = _get_SR_register() & GIE;
//	IFG2 &= ~UCA0RXIFG;
//
//	while(size--)
//	{
//		while( !(IFG2 & UCA0TXIFG) );
//		UCA0TXBUF = 0xff;
//		while( !(IFG2 & UCA0RXIFG) );
//		*pBuffer++ = UCA0RXBUF;
//	}
    return 1;
}

/**************************************************************************
 *名      称：SPI_TxFrame(unsigned char *pBuffer, unsigned int size)
 *功      能：3线硬件SPI模式下，发送指定数目的字节
 *入口参数：*pBuffer：指向待发送数据的数组
                  size：要发送的字节数
 *出口参数：0：当前SPI被占用
                  1：当前SPI空闲
 *说      明：使用此函数发送一帧数据
 *范      例：SPI_TxFrame(CMD, 6);  从CMD中取出6个字节发送
 *************************************************************************/

unsigned char SPI_TxFrame(unsigned char *pBuffer, unsigned int size)
{
    if(size ==0 )            return  (1);
    if(UCA0STAT & UCBUSY)    return  (0);
    _disable_interrupts();
    SPI_Rx_Or_Tx = 1;
    SPI_Tx_Buffer = pBuffer;
    SPI_Tx_Size = size - 1;
    SPI_Interrupt_Sel(SPI_Rx_Or_Tx);
    //__enable_interrupt();
    UCA0IFG &= ~(UCTXIFG + UCRXIFG);
    __bis_SR_register(GIE);
    UCA0TXBUF = *SPI_Tx_Buffer;

    _bis_SR_register(LPM0_bits);

//	unsigned int gie = _get_SR_register() & GIE;
//
//	while(size--)
//	{
//		while( !(IFG2 & UCA0TXIFG) );
//		UCA0TXBUF = *pBuffer++;
//	}
//	while(UCA0STAT & UCBUSY);
//	UCA0RXBUF;
//	_bis_SR_register(gie);
    return  (1);
}

/**************************************************************************
 *名      称：USCI0TX_ISR_HOOK()
 *功      能：响应Tx中断服务
 *入口参数：无
 *出口参数：无
 *说      明：包含唤醒CPU的代码
 *范      例：无
 *************************************************************************/

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV,4))
  {
    case 0:   break;                             // Vector 0 - no interrupt
    case 2:                                      // Vector 2 - RXIFG
    	      SPI_RxISR();
    	      if(SPI_Rx_Size == 0)
    	      _bic_SR_register_on_exit(LPM0_bits);
              break;
    case 4:
              SPI_TxISR();
              if(SPI_Tx_Size == 0)
              _bic_SR_register_on_exit(LPM0_bits);
    	      break;                             // Vector 4 - TXIFG
    default:  break;
  }
}

/**************************************************************************
 *名      称：SPI_RxISR()
 *功      能：Rx事件处理函数
 *入口参数：无
 *出口参数：无
 *说      明：对接收到的数据，区别对待进行处理
 *范      例：无
 *************************************************************************/

static void SPI_RxISR()
{
    *SPI_Rx_Buffer = UCA0RXBUF;

    if(SPI_Rx_Size != 0)
    {
    	SPI_Rx_Size--;
    	SPI_Rx_Buffer++;
    	UCA0TXBUF = 0xff;
    }
    UCA0IE &= ~UCTXIFG;
}

/**************************************************************************
 *名      称：SPI_TxISR()
 *功      能：Tx事件处理函数
 *入口参数：无
 *出口参数：无
 *说      明：无
 *范      例：无
 *************************************************************************/

static void SPI_TxISR()
{
    UCA0RXBUF;

    if(SPI_Tx_Size != 0)
    {
    	SPI_Tx_Size--;
    	SPI_Tx_Buffer++;
    	UCA0TXBUF = *SPI_Tx_Buffer;
    }
    else
    UCA0IE &= ~UCTXIFG;
}

/**************************************************************************
 *名      称：SPI_HighSpeed()
 *功      能：设置SPI为高速
 *入口参数：无
 *出口参数：无
 *说      明：有些SPI设备可以工作在高速SPI状态
 *范      例：无
 *************************************************************************/

void SPI_HighSpeed()
{
    UCA0CTL1 |= UCSWRST;
    UCA0BR0 = 15;
    UCA0BR1 = 0;
    UCA0MCTL = 0;
    UCA0CTL1 &= ~UCSWRST;
}

/**************************************************************************
 *名      称：SPI_LowSpeed()
 *功      能：设置SPI为低速
 *入口参数：无
 *出口参数：无
 *说      明：有些SPI设备需要工作在低速SPI状态
 *范      例：无
 *************************************************************************/

void SPI_LowSpeed()
{
    UCA0CTL1 |= UCSWRST;
    UCA0BR0 = 50;
    UCA0BR1 = 0;
    UCA0MCTL = 0;
    UCA0CTL1 &= ~UCSWRST;
    UCA0IFG &= ~UCTXIFG;
}
