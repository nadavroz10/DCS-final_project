#ifndef _bsp_H_4xx_
#define _bsp_H_4xx_

#include  <msp430xG46x.h>  // MSP430x4xx    (MSP430FG4619)



#define   debounceVal      100024

// PushButtons abstraction
#define PBsArrPort     P1IN
#define PBsArrIntPend      P1IFG
#define PBsArrIntEn    P1IE
#define PBsArrIntEdgeSel   P1IES
#define PBsArrPortSel      P1SEL
#define PBsArrPortDir      P1DIR
#define PB0                0x01
#define PB1                0x02
#define PB2                0x04
#define PB3                0x08




//LCD
#define LCD_DATA_WRITE  P1OUT
#define LCD_DATA_DIR    P1DIR
#define LCD_DATA_READ   P1IN
#define LcdCtrl             P2OUT
#define LcdCtrlDir          P2DIR
#define LcdCtrl             P2OUT
#define LCD_CTL_SEL         P2SEL
#define LCD_DATA_SEL      P1SEL


// timers

#define TIMER_A_VEC TIMERA1_VECTOR;

///LED
#define LEDSOUT P9OUT
#define LEDSSEL P9SEL
#define LEDSDIR P9DIR
///



extern void GPIOconfig(void);
extern void TIMERconfig(void);
extern void pwm_engine_init(void);
extern void UART_CONFIG();
extern void timer_trigger_and_echo_config();
extern void trigger_and_echo_legs_config();
extern void PB_config();
extern void ADC_channels_Config();


#endif





