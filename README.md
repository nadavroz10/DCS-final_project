# DCS-final_project
This project implements a system which include a client side (PC) and a server (MSP430 microcontroller). The PC side includes a GUI for the menu and for the graphical visualization of the system’s data. The server side includes a Servo motor 5-volt engine, an Ultra-Sonic sensor and two LDR sensors which are connected to the microcontroller.  The communication between both sides is made with UART.
The system supports the following 4 independent operating modes:
-	Objects detector Radar: 
This utility is 180 degrees scan by the Servo motor. During the scan the system builds  a real -time graphical “radar – like” picture that contains all the objects which are in range 2 – MaxDistance [cm]. The user defines the value of MaxDistance before the scan begins, and it’s  no bigger than 450 cm.

-	Telemeter:
Here the user selects a certain degree for the Servo motor. After the Servo turns to the selected degree the system shows a dynamic picture of the location of the closest object to the Ultra-sonic sensor. The user presses ‘e’ button to exit this operating mode and the Servo motor stays on the defined degree.

-	Light sources detector:
This operating mode is like the Objects detector Radar. The difference is that here the system detects light sources instead of dark objects. Here the scan is also 180 degrees but the distances are in range 5 – 50 cm. The distance determination is made according to the light strength detected by the LDR sensors.  For better accuracy, users should calibrate the system when arriving to a new light environment or using a new kind of light sources.
Entering calibration mode is made by pressing Key0 on the MSP430 side. The calibration is made by performing 10 samples. Each sample is made by putting the light source in the next distance and pressing on Key0. The required calibration distances are from 5 cm to 10 cm in steps of 5 cm.

-	Script mode:
The system can handle up to 3 independent scripts. The scripts are saved in the microcontroller’s Flash memory and hence  users don’t need to send the same scripts every time they use the system. Each script defined by a text file and contains a combination of the following commands (up to 10 commands in a script):

    1.Inc_ldc X  - counting from 0 to X with delay D is displayed on the LCD.
 	
    2.dec_lcd X - counting from X to 0 with delay D is displayed on the LCD.
    
    3.Rra_lcd - Rotate right onto LCD from pixel index 0 to pixel index 31 a single char x with delay d.
 	
    4.Set_delay - Set the delay d value ( units of 10ms ).
 	
    5.clear_lcd - clear_lcd.
 	
    6.servo_deg p- Point the Ultrasonic sensor to degree p and show the degree and distance (dynamically) onto PC screen.
 	
    7.servo_scan l,r -  Scan area between left l angle to right r angle (once) and show
 	the degree and distance (dynamically) onto PC screen.
 	
    8.sleep - Set the MCU into sleep mode.


