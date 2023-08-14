#ifndef _halGPIO_H_
#define _halGPIO_H_

#include  "../header/app.h"    		// private library - APP layer
#ifdef __MSP430FG4619__
#include "bsp_msp430x4xx.h"
#else
#include "bsp_msp430x2xx.h"
#endif
//#include "bsp_msp430x4xx.h"    		// private library - BSP layer
#define Flash_Address   0x1080          // for ldr config

#define Flash_files_address 0x1000


extern enum FSMstate state;   // global variable
extern enum SYSmode lpm_mode; // global variable
extern char return_char();
extern void sysConfig(void);
extern void delay(unsigned int);
extern void delayHalfSecond();
extern void enterLPM(unsigned char);
extern void enable_interrupts();
extern void disable_interrupts();
extern void delayInMs(int); //delay in ms
extern void exitLpm();
extern void sleep_gie();
extern void start_timer_pwm_engine(void);
extern int get_mask_dist(void);
extern int get_deg();
extern int calc_dis();
extern void start_capture_echo();
extern void send_dist_and_angle(int ,int );
extern void send_dist(int );
extern void start_ultra_trigger();
extern void stop_ultra_trigger(void);
extern void output_trigger();
extern unsigned int get_LDR1_samp();
extern unsigned int get_LDR2_samp();
extern void start_sampling();
extern void write_int_flash(int adress, int value);
extern void send_config_array();
extern void transferBlock(char *  , char * , int );
extern void erase_segment(int);
extern char get_command();
extern char get_Script_Num();
extern void send_ldr(unsigned int );
extern void set_d(char);
extern void Set_rcv_data(char);
extern void transferBlock_script(char * , char *, int );

extern __interrupt void TIMER_B_ISR(void);
extern __interrupt void TIMER_A_ISR(void);
extern __interrupt void USART1_rx (void);
extern __interrupt void DMA0_handler(void);
#endif







