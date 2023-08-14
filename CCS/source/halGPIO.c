#include  "../header/halGPIO.h"     // private library - HAL layer
#include  "../header/LCD.h"
#include "stdio.h"
#define delay_count  20
#define MC_0_VAL 0xFFCF



char rcv_data = 0; /// for UART
///////////
int  count = 0;
unsigned int val0 = 0 ,val1 = 0;
int flag = 0;
int mask_dist,deg_telemeter;
int distance_to_send;
char  data_to_send;
unsigned int LDR1_samp, LDR2_samp;
///for script mode
char Script_Num;
unsigned int d = 50; // d of script mode
unsigned int d_count  = 0;
char command;



//--------------------------------------------------------------------
//             System Configuration  
//--------------------------------------------------------------------
void sysConfig(void){ 
	GPIOconfig();
	timer_trigger_and_echo_config();
    UART_CONFIG();
	lcd_init();
	lcd_clear();
	ADC_channels_Config();
}

void sleep_gie(){
    __bis_SR_register(LPM0_bits + GIE);
}

//---------------------------------------------------------------------
//            Polling based Delay function
//---------------------------------------------------------------------
void delay(unsigned int t){  // t[micro sec]
	volatile unsigned int i;
	for(i=t; i>0; i--);
}

void delayInMs(int t){ // t[m sec]
    volatile unsigned int i;
   for(i=10;i>0;i--) delay(t*10);
}

//---------------------------------------------------------------------
//            Enter from LPM0 mode
//---------------------------------------------------------------------
void enterLPM(unsigned char LPM_level){
	if (LPM_level == 0x00) 
	  _BIS_SR(LPM0_bits);     /* Enter Low Power Mode 0 */
        else if(LPM_level == 0x01) 
	  _BIS_SR(LPM1_bits);     /* Enter Low Power Mode 1 */
        else if(LPM_level == 0x02) 
	  _BIS_SR(LPM2_bits);     /* Enter Low Power Mode 2 */
	else if(LPM_level == 0x03) 
	  _BIS_SR(LPM3_bits);     /* Enter Low Power Mode 3 */
        else if(LPM_level == 0x04) 
	  _BIS_SR(LPM4_bits);     /* Enter Low Power Mode 4 */
}
//---------------------------------------------------------------------
//            Enable interrupts
//---------------------------------------------------------------------
void enable_interrupts(){
    __bis_SR_register(GIE);
}
//---------------------------------------------------------------------
//            Disable interrupts
//---------------------------------------------------------------------
void disable_interrupts(){
    __bic_SR_register(GIE);
}


//---------------------------------------------------------------------
//  LCD
//---------------------------------------------------------------------


//******************************************************************
// send a command to the LCD
//******************************************************************
void lcd_cmd(unsigned char c){

    LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h

    if (LCD_MODE == FOURBIT_MODE)
    {
        LCD_DATA_WRITE &= ~OUTPUT_DATA;// clear bits before new write
                LCD_DATA_WRITE |= ((c >> 4) & 0x0F) << LCD_DATA_OFFSET;
        lcd_strobe();
                LCD_DATA_WRITE &= ~OUTPUT_DATA;
            LCD_DATA_WRITE |= (c & (0x0F)) << LCD_DATA_OFFSET;
        lcd_strobe();
    }
    else
    {
        LCD_DATA_WRITE = c;
        lcd_strobe();
    }
}
//******************************************************************
// send data to the LCD
//******************************************************************
void lcd_data(unsigned char c){

    LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h

    LCD_DATA_WRITE &= ~OUTPUT_DATA;
    LCD_RS(1);
    if (LCD_MODE == FOURBIT_MODE)
    {
            LCD_DATA_WRITE &= ~OUTPUT_DATA;
                LCD_DATA_WRITE |= ((c >> 4) & 0x0F) << LCD_DATA_OFFSET;
        lcd_strobe();
                LCD_DATA_WRITE &= (0xF0 << LCD_DATA_OFFSET) | (0xF0 >> 8 - LCD_DATA_OFFSET);
                LCD_DATA_WRITE &= ~OUTPUT_DATA;
        LCD_DATA_WRITE |= (c & 0x0F) << LCD_DATA_OFFSET;
        lcd_strobe();
    }
    else
    {
        LCD_DATA_WRITE = c;
        lcd_strobe();
    }

    LCD_RS(0);
}

//******************************************************************
// initialize the LCD
//******************************************************************
void lcd_init(){

    char init_value;

    if (LCD_MODE == FOURBIT_MODE) init_value = 0x3 << LCD_DATA_OFFSET;
        else init_value = 0x3F;

    LCD_RS_DIR(OUTPUT_PIN);
    LCD_EN_DIR(OUTPUT_PIN);
    LCD_RW_DIR(OUTPUT_PIN);
        LCD_DATA_DIR |= OUTPUT_DATA;
        LCD_RS(0);
    LCD_EN(0);
    LCD_RW(0);

    DelayMs(15);
        LCD_DATA_WRITE &= ~OUTPUT_DATA;
    LCD_DATA_WRITE |= init_value;
    lcd_strobe();
    DelayMs(5);
        LCD_DATA_WRITE &= ~OUTPUT_DATA;
    LCD_DATA_WRITE |= init_value;
    lcd_strobe();
    DelayUs(200);
        LCD_DATA_WRITE &= ~OUTPUT_DATA;
    LCD_DATA_WRITE |= init_value;
    lcd_strobe();

    if (LCD_MODE == FOURBIT_MODE){
        LCD_WAIT; // may check LCD busy flag, or just delay a little, depending on lcd.h
                LCD_DATA_WRITE &= ~OUTPUT_DATA;
        LCD_DATA_WRITE |= 0x2 << LCD_DATA_OFFSET; // Set 4-bit mode
        lcd_strobe();
        lcd_cmd(0x28); // Function Set
    }
        else lcd_cmd(0x3C); // 8bit,two lines,5x10 dots

    lcd_cmd(0x0F); //Display On, Cursor On, Cursor Blink
    lcd_cmd(0x01); //Display Clear
    lcd_cmd(0x6); //Entry Mode
    lcd_cmd(0x80); //Initialize DDRAM address to zero
    lcd_clear();
}
//******************************************************************
// Delay usec functions
//******************************************************************
void DelayUs(unsigned int cnt){

    unsigned char i;
        for(i=cnt ; i>0 ; i--) asm("nop"); // tha command asm("nop") takes raphly 1usec

}
//******************************************************************
// Delay msec functions
//******************************************************************
void DelayMs(unsigned int cnt){

    unsigned char i;
        for(i=cnt ; i>0 ; i--) DelayUs(1000); // tha command asm("nop") takes raphly 1usec

}
//******************************************************************
// lcd strobe functions
//******************************************************************
void lcd_strobe(){
  LCD_EN(1);
  asm("NOP");
  asm("Nope");    // DIDNT COMPILE
  LCD_EN(0);
}



  void start_timer_pwm_engine(){
      TBCCTL1  = OUTMOD_7;
      TBCTL |= MC_1; // START UP MODE
      if(state == state1 || state == state3 || state == state6)
           TBCCTL0 |= CCIE; // FOR DELAY COUNTING
  }


  //--------------------------------------------------------------------
  //             Timers functions
  //--------------------------------------------------------------------


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER1_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) TIMER1_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{


  //  TACCTL2 &= ~COV;
    switch(__even_in_range(TAIV, 0x0A))
  {
      case  TAIV_TACCR1:                   // Vector  2:  TACCR1 CCIFG
          TACCTL1 &= ~CCIE;                //STOP TRIGGER INTERUPT
         //   TACCTL1 &= 0XFF0F;                 // CCIE OF AND OUTMOD_0
          TAR = 0;
          TACCTL2 &= ~CCIFG;
          TACCTL2 |= CCIE;
        break;

      case TAIV_TACCR2:                    // Vector  4:  TACCR2 CCIFG  -- INPUT CAPTURE
          if(flag){
               val1 = TACCR2;
                flag ^= 1;
                TACCTL2 &= ~CCIE;
                //TACCTL2 &= ~CCIFG;
                TACTL &= MC_0_VAL;       //TIMER MC0
                LPM0_EXIT;
                  }

          else
                   {
                    val0 = TACCR2;
                    TAR = 0;
                    val0 = 0;
                   // val0 = TAR;
                   // TACCTL2 &= ~CCIFG;
                    flag ^= 1;
                 }

          break;

      //case  TAIV_TACCR3:                   // Vector  :  TACCR3 CCIFG

        // break;
      default:  break;
  }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector =TIMER0_B0_VECTOR
__interrupt void TIMER_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) TIMER_B0_ISR(void)
#else
#error Compiler not supported!
#endif
{
    if(state == state6){

          if(d_count == d){
               TBCCTL0 &= ~CCIE;
               LPM0_EXIT;
               d_count = 0;
           }
           else
           {
               d_count ++;
           }
    }

else
  {
     if(count == delay_count){
        LPM0_EXIT;
        count = 0;
    }
    else
    {
        count ++;
    }
 }
}


void start_ultra_trigger() {
    TACCR0 = 0XFFFF; // we wanted up mode
    TACCR1 = 0X20;
    TAR = 0;
    TACCTL1 |= OUTMOD_7;
    TACCTL1 &= ~CCIFG;
    TACCTL1 |= CCIE;
    TACTL |= MC_1;   // start trigger upmode: mode 7
}

void stop_ultra_trigger(){
    TBCCTL3 = OUTMOD_0;

}

void start_capture_echo(){
    TACTL &= MC_0_VAL;
    TAR = 0;
    TACTL |= MC_2;  // TIMER A TO MC_2 FOR CAPTURING ECHO
    TACCTL2 |= CCIE;
}


void output_trigger(){
    TAR = 0;
    TACCR1 =  100;
    P1OUT |= 0X04;
    TACCTL1 |= CCIE;
    TACTL |= MC_2;

}


//--------------------------------------------------------------------
//             distance functions
//--------------------------------------------------------------------

int calc_dis(){
    return (val1 - val0);
}

//--------------------------------------------------------------------
//             Uart functions
//--------------------------------------------------------------------
void send_dist_and_angle(int distance,int deg){
    distance_to_send = distance;
    char i;
    for (i = 0; i< 4; i++)
    {
       switch(i) {
       case 0: data_to_send = distance_to_send >> 8; break;
       case 1: data_to_send = distance_to_send; break;
       case 2: data_to_send = deg >> 8; break;
       case 3: data_to_send = deg; break;
         }
       while (!(IFG2 & UTXIFG1));
       TXBUF1 = data_to_send;
     //  __bis_SR_register(LPM0_bits + GIE);
       if(distance_to_send == 0xffff)break;
    }

}


void send_dist(int distance){
    distance_to_send = distance;
    char i;
    for (i = 0; i< 2; i++)
    {
       switch(i) {
       case 0: data_to_send = distance_to_send >> 8; break;
       case 1: data_to_send = distance_to_send; break;
         }
       while (!(IFG2 & UTXIFG1));
       TXBUF1 = data_to_send;
     //  __bis_SR_register(LPM0_bits + GIE);
       if(distance_to_send == 0xffff)break;
    }

}


#pragma vector=USART1RX_VECTOR
__interrupt void USART1_rx (void)
{

  static char state1_LSB_Byte = 0;
  static char state2_LSB_Byte = 0;
  if(rcv_data == 0){
   switch(RXBUF1){
   case '0':
       state = state0;
       LPM0_EXIT;
       break;
   case '1':
       state = state1;  // state1 is object detector
       rcv_data =1;
       LPM0_EXIT;
   break;
   case '2':
       state = state2;  // state2 is telemeter
       rcv_data =1;
       LPM0_EXIT;
   break;
   case '3':
       state = state3;
       rcv_data =0;
       LPM0_EXIT;
   break;
   case '4':
       state = state4;
       rcv_data = 1;
       LPM0_EXIT;

   break;
   case '5':
          state = state5;   // state5 is environment config for the light sources detector
          rcv_data = 0;
          LPM0_EXIT;
    break;

   case '6':
             state = state6;   // state6 is script mode
             rcv_data = 1;
             LPM0_EXIT;
       break;


   }
  }

  else{
         switch(state){
         case state1: // state1 is object detector
             if (state1_LSB_Byte == 0){    // Receiving MSB of mask distance
                 mask_dist = RXBUF1;
                 mask_dist = mask_dist << 8;
                 state1_LSB_Byte = 1;
             }

             else {
                 mask_dist |= RXBUF1; // Receiving LSB of mask distance
                 state1_LSB_Byte = 0;
                 rcv_data = 0;
                 LPM0_EXIT;
             }
             break;

         case state2:  // state2 is telemeter
                     if (state2_LSB_Byte == 0){    // Receiving MSB of mask distance
                         deg_telemeter = RXBUF1;
                         deg_telemeter = deg_telemeter << 8;
                         state2_LSB_Byte = 1;
                     }

                     else {
                         deg_telemeter |= RXBUF1; // Receiving LSB of mask distance
                         state2_LSB_Byte = 0;
                         rcv_data = 0;
                         LPM0_EXIT;
                     }

         break;
         case state4:
             command = RXBUF1;
             LPM0_EXIT;
         break;

         case state6:
             Script_Num = RXBUF1;
             rcv_data = 0;
             LPM0_EXIT;
          break;

      }

  }

}


#pragma vector=USART1TX_VECTOR
__interrupt void USART1_tx (void)
{
   // static char send_selector;
    TXBUF1 = data_to_send;
    IE2 &= ~UTXIE1;
}


void send_config_array(){
    TXBUF1 = 0x1;
    transferBlock(Flash_Address,&TXBUF1,40);
}


void send_ldr(unsigned int angle){
    unsigned int sample;
    sample =  get_LDR1_samp();
    while (!(IFG2 & UTXIFG1));
    TXBUF1 = sample >> 8; // send msb ldr1
     while (!(IFG2 & UTXIFG1));
    TXBUF1 = sample;      // send lsb ldr1
    sample =  get_LDR2_samp();
    while (!(IFG2 & UTXIFG1));
    TXBUF1 = sample >> 8;  // send msb ldr2
    while (!(IFG2 & UTXIFG1));
    TXBUF1 = sample;      // send lsb ldr2
    while (!(IFG2 & UTXIFG1));
    TXBUF1 = angle >> 8;  // send msb angle
    while (!(IFG2 & UTXIFG1));
    TXBUF1 = angle;  // send lsb angle
}

///////////////SET  rcv_data/////////
void Set_rcv_data(char rcv_data_init ){
    rcv_data = rcv_data_init;
}
////////////////////////////////////



//--------------------------------------------------------------------
//             ADC functions
//--------------------------------------------------------------------

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    switch(ADC12IV){
    case 0x06:
        LDR1_samp = ADC12MEM0; break;
    case 0x08:
        LDR2_samp = ADC12MEM1;
        LPM0_EXIT; break;
    }
}

//--------------------------------------------------------------------
//             Port1 functions
//--------------------------------------------------------------------
#pragma vector=PORT1_VECTOR
  __interrupt void PBs_handler(void){
      delay(debounceVal);
      if(state == state5){
          P1IFG &= ~0X01;
          LPM0_EXIT;
      }
      else{
          state = state5;
      }
  }



 //--------------------------------------------------------------------
 //             getters functions
 //--------------------------------------------------------------------


int get_mask_dist(){
    return mask_dist;
}

int get_deg(){
    return deg_telemeter;
}


void start_sampling(){
    ADC12CTL0 |= ENC + ADC12SC;

}

unsigned int get_LDR1_samp(){
    return LDR1_samp;
}

unsigned int get_LDR2_samp(){
    return LDR2_samp;
}

//--------------------------------------------------------------------
//             Flash functions
//--------------------------------------------------------------------


void write_int_flash(int adress, int value)
{
  int *Flash_ptr;                           // Flash pointer
  Flash_ptr = (int *)adress;                // Initialize Flash pointer
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  *Flash_ptr = value;                       // Write value to flash
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

void erase_segment(int adress)
{
  int *Flash_ptr;                           // Flash pointer

  Flash_ptr = (int *)adress;                // Initialize Flash pointer
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY;                            // Clear Lock bit

  *Flash_ptr = 0;                           // Dummy write to erase Flash segment

  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}

//--------------------------------------------------------------------
//             DMA functions
//--------------------------------------------------------------------

void transferBlock(char *  addr_src, char * adrr_dst, int blk_sz){
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    DMACTL0 = DMA0TSEL_10;                     // CCR2 trigger
    DMA0SA = (void (*)())addr_src;                  // Source block address
    DMA0DA = (void (*)())&TXBUF1;                     // Destination single address
    DMA0SZ = blk_sz;                            // Block size
    DMA0CTL = DMADT_4 + DMASRCINCR_3 + DMASBDB + DMAEN+DMAIE; // Rpt, inc src
    __bis_SR_register(LPM0_bits + GIE);
}

void transferBlock_script(char *addr_src, char * adrr_dst, int blk_sz){
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
     DMA1SA = (void (*)( ))addr_src;            // Start block address
     DMA1DA = (void (*)( ))adrr_dst;            // Destination block address
     DMA1SZ = blk_sz;                          // Block size
     DMA1CTL = DMADT_1 + DMASRCINCR_3 + DMADSTINCR_3 + DMASRCBYTE + DMADSTBYTE; // Rpt, inc
     DMA1CTL |= DMAEN+DMAIE;                         // Enable DMA0
     DMA1CTL |= DMAREQ;
     __bis_SR_register(LPM0_bits + GIE);
}




#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=DMA_VECTOR
  __interrupt void DMA0_handler(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(DMA_VECTOR))) DMA0_handler (void)
#else
#error Compiler not supported!
#endif
  {
      switch(DMAIV){
      case 0x02:
          DMA0CTL &= ~DMAEN + ~DMAIE;
          LPM0_EXIT;

      case 0x04:
          DMA0CTL &= ~DMAEN + ~DMAIE;
          LPM0_EXIT;
          break;
      }

  }

//////////////////////SCRIPT MODE FUNCTIONS////////////////////////////

char get_Script_Num(){
    return Script_Num;
}

void set_d(char x)
{
    d = x;
}


char get_command(){
    return command;
}







