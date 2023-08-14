#include  "../header/api.h"    		// private library - API layer
#include  "../header/app.h"    		// private library - APP layer
//#include  <string.h>
#define ascii_offset 48



enum FSMstate state;
enum SYSmode lpm_mode;


void main(void){
  state = state0;  // start in idle state on RESET
  lpm_mode = mode0;     // start in idle state on RESET
  sysConfig();
  
  while(1){
	switch(state){
	  case state0:
	     // erase_segment(0x1000);
	      sleep_gie();
		break;
		 
	  case state1:
	     set_angle(0x275, 0x8c9,0);
	     state = state0;
		break;
		 
	    case state2:
	    telemeter();
		break;
		
	  case state3:
	   light_sources_detector();
	   state = state0;
	    break;

	  case state4:
	      write_one_file();
	      state = state0;
	    break;

	  case state5:
	      environment_config();
	      state = state0;
	  break;

	  case state6:
	      do_script();
	      state = state0;
	      break;

	}
  }
}
  
  
  
  
  
  
