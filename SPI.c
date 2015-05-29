/*
 * SPI.c
 *
 *  Created on: 2014��11��20��
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
 *��      �ƣ�SPI_init
 *��      �ܣ���Ӳ��SPI���г�ʼ������
 *��ڲ�������
 *���ڲ�������
 *˵      ������ʼ����Ƭ��SPI����
 *��      ������
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
 *��      �ƣ�SPI_CS_High()
 *��      �ܣ�3��Ӳ��SPIģʽ��ʹ��CSΪ�ߵ�ƽ
 *��ڲ�������
 *���ڲ�������
 *˵      ����CS�ܽſ�������ָ��
 *��      ������
 *************************************************************************/

void SPI_CS_High()
{
    SPI_CS_OUT |= SPI_CS;
}

/**************************************************************************
 *��      �ƣ�SPI_CS_Low()
 *��      �ܣ�3��Ӳ��SPIģʽ������ʹ��CSΪ�͵�ƽ
 *��ڲ�������
 *���ڲ�������
 *˵      ����CS�ܽſ�������ָ��
 *��      ������
 *************************************************************************/

void SPI_CS_Low()
{
    SPI_CS_OUT &= ~SPI_CS;
}

/**************************************************************************
 *��      �ƣ�SPI_Interrupt_Sel(unsigned char onOff)
 *��      �ܣ��������ͻ�����ж�
 *��ڲ�����onOff = 0���رշ����жϣ��򿪽����ж�
                  onOff = 1���򿪷����жϣ��رս����ж�
                  onOff = 2���򿪷����жϣ��򿪽����ж�
 *���ڲ�������
 *˵      ����ʹ�ô˺���������ѡ��ǰ�ж�ģʽ
 *��      ����SPI_Interrupt_Sel(0);
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
 *��      �ƣ�SPI_RxFrame(unsigned char *pBuffer, unsigned int size)
 *��      �ܣ�3��Ӳ��SPIģʽ�£�����ָ����Ŀ���ֽ�
 *��ڲ�����*pBuffer��ָ���Ž������ݵ�����
                  size��Ҫ���յ��ֽ���
 *���ڲ�����0����ǰSPI��ռ��
                  1����ǰSPI����
 *˵      ����ʹ�ô˺�������һ֡����
 *��      ����SPI_RxFrame(CMD, 6);  ����6���ֽڷ���CMD��
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
    UCA0TXBUF = 0xff;             //�ڽ���ģʽ�£�Ҳ�ȷ���һ�ο��ֽڣ�һ���ṩʱ��ͨ��

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
 *��      �ƣ�SPI_TxFrame(unsigned char *pBuffer, unsigned int size)
 *��      �ܣ�3��Ӳ��SPIģʽ�£�����ָ����Ŀ���ֽ�
 *��ڲ�����*pBuffer��ָ����������ݵ�����
                  size��Ҫ���͵��ֽ���
 *���ڲ�����0����ǰSPI��ռ��
                  1����ǰSPI����
 *˵      ����ʹ�ô˺�������һ֡����
 *��      ����SPI_TxFrame(CMD, 6);  ��CMD��ȡ��6���ֽڷ���
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
 *��      �ƣ�USCI0TX_ISR_HOOK()
 *��      �ܣ���ӦTx�жϷ���
 *��ڲ�������
 *���ڲ�������
 *˵      ������������CPU�Ĵ���
 *��      ������
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
 *��      �ƣ�SPI_RxISR()
 *��      �ܣ�Rx�¼�������
 *��ڲ�������
 *���ڲ�������
 *˵      �����Խ��յ������ݣ�����Դ����д���
 *��      ������
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
 *��      �ƣ�SPI_TxISR()
 *��      �ܣ�Tx�¼�������
 *��ڲ�������
 *���ڲ�������
 *˵      ������
 *��      ������
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
 *��      �ƣ�SPI_HighSpeed()
 *��      �ܣ�����SPIΪ����
 *��ڲ�������
 *���ڲ�������
 *˵      ������ЩSPI�豸���Թ����ڸ���SPI״̬
 *��      ������
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
 *��      �ƣ�SPI_LowSpeed()
 *��      �ܣ�����SPIΪ����
 *��ڲ�������
 *���ڲ�������
 *˵      ������ЩSPI�豸��Ҫ�����ڵ���SPI״̬
 *��      ������
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
