#ifndef _api_H_
#define _api_H_

#include  "../header/halGPIO.h"     // private library - HAL layer


extern void lcd_puts(const char *);
extern void set_angle(unsigned int , unsigned int , char );
extern void telemeter();
extern void environment_config(void);
extern void light_sources_detector(void);
extern void do_script();
extern void telemter_script(unsigned int);
extern void write_one_file();
extern void rotate_char(char);
extern void store_one_file();
extern void count_from_x(char);
extern void count_to_x(char);

#endif







