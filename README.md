# finalProjectDigitalC
This project implements a system that includes a client-side (PC) and a server (MSP430 microcontroller). The PC side includes a GUI for the menu and for the graphical visualization of the system’s data. The server side includes a Servo motor 5-volt engine, an Ultra-Sonic sensor, and two LDR sensors which are connected to the microcontroller.  The communication between both sides is made with UART.
The system supports the following 4 independent operating modes:
## Objects detector Radar:
This utility is 180 degrees scanned by the Servo motor. During the scan, the system builds  a real-time graphical “radar-like” picture that contains all the objects which
are in range 2 – MaxDistance [cm]. The user defines the value of MaxDistance before the scan begins, and it’s  no bigger than 450 cm.

## Telemeter: 
 The user selects a certain degree for the Servo motor. After the Servo turns to the selected degree the system shows a dynamic picture of the location of the
closest object to the Ultra-sonic sensor. The user presses the ‘e’ button to exit this operating mode and the Servo motor stays on the defined degree.

## Light sources detector:  This operating mode is like the Objects detector Radar.
 The difference is that here the system detects light sources instead of dark objects.
Here the scan is also 180 degrees but the distances are in the range 5 – 50 cm. The distance determination is made according to the light strength detected by the LDR sensors.
 For better accuracy, users should calibrate the system when arriving at a new light environment or using a new kind of light source.
Entering calibration mode is made by pressing Key0 on the MSP430 side. The calibration is made by performing 10 samples. Each sample is made by putting the light source in the next distance and pressing on Key0. The required calibration distances are from 5 cm to 10 cm in steps of 5 cm.

## Script mode:
The system can handle up to 3 independent scripts.
The scripts are saved in the microcontroller’s Flash memory; hence,  users don’t need to send the same scripts every time they use the system.
Each script is defined by a text file and contains a combination of the following commands (up to 10 commands in a script):

    1. Inc_ldc (x)       -  counting from 0 to X with delay d is displayed on the LCD.
 	
    2. dec_lcd (x)       -  counting from X to 0 with delay d is displayed on the LCD.
 	
    3. rra_lcd           -  rotate right onto LCD from pixel index 0 to pixel index 31 a single char x with delay d.
 	
    4. set_delay         -  set the delay d value (units of 10ms).
 	
    5. clear_lcd         -  clear the LCD.
 	
    6. servo_deg (p)     -  point the Ultrasonic sensor to degree p and show the degree and distance (dynamically) on the PC screen.
 	
    7. servo_scan (l,r)  -  scan the area between the left l angle to the right r angle (once) and
                            show the degree and distance (dynamically) on the PC screen.
 	
    8. sleep             -  set the MCU into sleep mode.

> [!IMPORTANT]
> The default value of d is 50, changed only in state 4 (set_delay).

 ## FSM: 
   <img width="489" alt="image" src="https://github.com/Orisadek/finalProjectDigitalC/assets/43981934/9dbdd6c8-5969-46f6-8257-1af329d5b360">



