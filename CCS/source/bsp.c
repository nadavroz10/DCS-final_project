  // private library - BSP layer
#ifdef __MSP430FG4619__
#include "../header/bsp_msp430x4xx.h"
#define LabKit
#else
#include "../header/bsp_msp430x2xx.h"
#define PersonalEvalKit
#endif
//-----------------------------------------------------------------------------  
//           GPIO congiguration
//-----------------------------------------------------------------------------



void GPIOconfig(void){
 // volatile unsigned int i; // in case of while loop usage
  
  WDTCTL = WDTHOLD | WDTPW;		// Stop WDT
  pwm_engine_init();
  trigger_and_echo_legs_config();
  PB_config();

  ///leds//
   P1OUT &= ~0xF0;
   P1DIR |= 0xF0;    // P1.4-P1.7 To Output('1')
   P1SEL &= ~0xF0;   // Bit clear P2.4-P2.7
   P2SEL  &= ~0xE0;   // Bit clear P2.5-P2.7
   P1IE |= 0X01;     // button P0
   /////

}



void timer_trigger_and_echo_config(void) //  4 ms timer
{
   //TACCTL1 |= OUTMOD_7;   // FOR TRIGGER ---> now we changed to 1.2
   TACCTL2 |= CAP  + CM_3 + SCS; // FOR ECHO CAPTURE
   TACTL = TASSEL_2 + MC_0;
}

void trigger_and_echo_legs_config(){
    P1SEL |= 0X04;  //now we changed to 1.2 FOR TRIGGER
    P1DIR |= 0X04;
    P2SEL |= 0X01; // P2.0 FOR ECHO
    P2DIR &= ~0X01;

}

void pwm_engine_init(void){
    P2DIR |= 0X04;  // P2.2 CONFIG TO  output PWM
    P2SEL |= 0X04;
    TBCCTL1 = OUTMOD_7; // MODE 7
    TBCTL |= TBSSEL_2;
    TBCTL  &= 0Xffcf;
}

void UART_CONFIG(){
      volatile unsigned int i;

      WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
      FLL_CTL0 |= XCAP14PF;                     // Configure load caps
      do
      {
      IFG1 &= ~OFIFG;                           // Clear OSCFault flag
      for (i = 0x47FF; i > 0; i--);             // Time for flag to set
      }
      while ((IFG1 & OFIFG));                   // OSCFault flag still set?

      P4SEL |= 0x03;                            // P4.1,0 = USART1 TXD/RXD
      ME2 |= UTXE1 + URXE1;                     // Enable USART1 TXD/RXD
      U1CTL |= CHAR;                            // 8-bit character
      U1TCTL |= SSEL0;                          // UCLK = ACLK
      U1BR0 = 0x03;                             // 32k/9600 - 3.41
      U1BR1 = 0x00;                             //
      U1MCTL = 0x4A;                            // Modulation
      U1CTL &= ~SWRST;                          // Initialize USART state machine
      IE2 |= URXIE1;                            // Enable USART1 RX interrupt
}



void ADC_channels_Config(){
    P6SEL |= 0x03;                            // P6.0 for channel A0 and 6.1 for channel A1
    ADC12CTL0 &= ~ENC;
    ADC12CTL0 |= MSC + ADC12ON;
    ADC12CTL1 = SHP + CONSEQ_1;
    ADC12MCTL0 = 0;
    ADC12MCTL1 = EOS + INCH_1;
    ADC12CTL0 |= SHT0_10;
    ADC12IE |= 0X3;
}

void PB_config(){
    P1SEL &= 0XF7;
    P1DIR &= ~0X01;
    P1IES |= 0X01;
}



