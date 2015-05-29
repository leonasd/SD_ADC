#include <msp430.h>
#include <SD_SPI.h>
#include <BCSplus_init.h>
#include <Device_Init.h>

#define SD_SECTOR_ADDR  251
#define SD_SECTOR_FIRST 0
#define SD_SECTOR_NUM   512
#define NULL            0

unsigned char Is_adr_change = 0;
unsigned int buff1[256] = {0};
unsigned int buff2[256] = {0};
unsigned int *adress = buff1;
unsigned int *tran = NULL;
unsigned long sector = 0;

int main(void)
{
	unsigned char temp = 0;
	BCSplus_graceInit();
	GPIO_Init();
    Device_Init();



	__enable_interrupt();
	while(1)
	{
		if(Is_adr_change)
		{
		    Is_adr_change = 0;
			if(sector < 4)
		    {
		    	continue;
		    }
		    else
			{
				do
				{
					temp = SD_Write_Sector(SD_SECTOR_ADDR+sector, (unsigned char *)tran, SD_SECTOR_FIRST, SD_SECTOR_NUM);
				}while(temp == 0);
			}
		}
	}
}


#pragma vector=DMA_VECTOR
__interrupt void DMA_ISR(void)
{
  switch(__even_in_range(DMAIV,16))
  {
    case 2:                                 // DMA0IFG = DMA Channel 0
    	     adress = ((adress-buff1) == 0) ? buff2 : buff1;
    	     tran   = ((adress-buff1) == 0) ? buff1 : buff2;
    	     __data16_write_addr((unsigned short) &DMA0DA,(unsigned long) adress);
    	     sector++;
    	     Is_adr_change = 1;
    	     P1OUT ^= BIT0;
             break;
    default: break;
  }
}


#pragma vector=PORT2_VECTOR
__interrupt void Port_1(void)
{
	if(TA0CTL & MC_3)
	{
		TA0CTL &= ~MC_3;
		tran = NULL;
		//还要关定时器
	}
	else
	{
		TA0CTL |= MC_1;
	}
	P2IFG &= ~BIT1;
}

//#pragma vector=ADC12_VECTOR
//__interrupt void ADC12ISR (void)
//{
//	switch(__even_in_range(ADC12IV,34))
//	{
//		case  0: break;                           // Vector  0:  No interrupt
//		case  2: break;                           // Vector  2:  ADC overflow
//		case  4: break;                           // Vector  4:  ADC timing overflow
//		case  6:
//			           result = ADC12MEM0;        // Vector  6:  ADC12IFG0
//				       P1OUT ^= 0x01;
//				       write2DAC8411(result);
//		               break;
//		case  8: break;                           // Vector  8:  ADC12IFG1
//		case 10: break;                           // Vector 10:  ADC12IFG2
//		case 12: break;                           // Vector 12:  ADC12IFG3
//		case 14: break;                           // Vector 14:  ADC12IFG4
//		case 16: break;                           // Vector 16:  ADC12IFG5
//		case 18: break;                           // Vector 18:  ADC12IFG6
//		case 20: break;                           // Vector 20:  ADC12IFG7
//		case 22: break;                           // Vector 22:  ADC12IFG8
//		case 24: break;                           // Vector 24:  ADC12IFG9
//		case 26: break;                           // Vector 26:  ADC12IFG10
//		case 28: break;                           // Vector 28:  ADC12IFG11
//		case 30: break;                           // Vector 30:  ADC12IFG12
//		case 32: break;                           // Vector 32:  ADC12IFG13
//		case 34: break;                           // Vector 34:  ADC12IFG14
//		default: break;
//	}
//}



