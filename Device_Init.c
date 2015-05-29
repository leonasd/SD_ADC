#include <msp430f5529.h>
#include <Device_Init.h>
#include <SD_SPI.h>
//Init ADC12--TimerA--DMA--
void Device_Init()
{
	ADC12CTL0 = ADC12ON+ADC12SHT0_4 + ADC12REFON + ADC12REF2_5V;
	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_2 + ADC12SHS_1;       // Use sampling timer, set mode
	//ADC12IE = 0x01;                           // Enable ADC12IFG.0
	ADC12CTL0 |= ADC12ENC;

	TA0CCR0 = 3050;
	TA0CCR1 = 20;
	TA0CCTL1 |= OUTMOD_3;                       // CCR1 set/reset mode
	TA0CTL = TASSEL_2 + TACLR;         // SMCLK, upmode, clear TAR

	DMACTL0 = DMA0TSEL_24;                    // ADC12IFGx triggered
	DMACTL4 = DMARMWDIS;                      // Read-modify-write disable
	DMA0CTL &= ~DMAIFG;
	DMA0CTL = DMADT_4+DMAEN+DMADSTINCR_3+DMAIE; // Rpt single tranfer, inc dst, Int
	DMA0SZ = 256;                               // DMA0 size = 512
	__data16_write_addr((unsigned short) &DMA0SA,(unsigned long) &ADC12MEM0);
	__data16_write_addr((unsigned short) &DMA0DA,(unsigned long) buff1);

    SPI_init();
    while(!SD_Init());
}

void GPIO_Init()
{
    //P3DIR |= BIT3 + BIT7;
    //P4DIR |= BIT0 + BIT3;
    //P8DIR |= BIT1 + BIT2;
    P1DIR |= BIT0 + BIT2;
    P1SEL |= BIT2;
    P6SEL |= BIT0;                            // Enable A/D channel A0
    P2REN |= BIT1;                            // Enable P1.4 internal resistance
    P2OUT |= BIT1;                            // Set P1.4 as pull-Up resistance
    P2IES |= BIT1;                            // P1.4 Hi/Lo edge
    P2IFG &= ~BIT1;                           // P1.4 IFG cleared
    P2IE |= BIT1;                             // P1.4 interrupt enabled
}
