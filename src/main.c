/*****************************************************************************
 * Brief Project Info:
 * 
 * This C8051F340 firmware interfaces with an EM-18 RFID reader and a 16x2 LCD
 * to read RFID card numbers and display them on the LCD screen. 
 * The project configures the microcontroller, handles communication with the RFID reader,
 * and controls the LCD display.
 * 
 * User Defined Functions Used:
 *  - delayMS(unsigned int MS): Delay routine in milliseconds.
 *  - writeCommand(unsigned char Command): Sends commands to the LCD.
 *  - writeData(unsigned char Character): Sends data to the LCD.
 *  - readTX(void): Reads a single character from the EM-18 RFID reader.
 * 
 * Original Date of Creation: 9 September 2021
 * 
 *****************************************************************************/

// Include C8051F340 library
#include "c8051F340.h"

//Set SYSCLK value to 12MHz		
#define SYSCLK 12000000	

//Set baud rate as 9600 as EM-18 uses same baud rate
#define BR_UART0 9600	

//Function Prototypes/Signatures 

//Delay Routine
void delayMS(unsigned int MS);			

//Function to send commands to LCD
void writeCommand(unsigned char Command);	

//Function to send data to LCD	
void writeData(unsigned char Character);		

//Function to read character from EM-18
char readTX(void);					

//Register Select of LCD is assigned to P1.5
sbit LCD_RS = P1^5;	

//Read/Write of LCD is assigned to P1.6
sbit LCD_RW = P1^6;		

//Enable pin is assigned to P1.7
sbit LCD_EN = P1^7;		

int i=0;

//Variable to keep count of characters read by readTX()
char count = 0;		

//Array to hold 12-character number of RFID card
char card_number[12];	

/*Array to store list of LCD commands
    0x38 -> 2 lines and 5x7 matrix
    0x01 -> Clear display screen
    0x0C -> Display ON, Cursor OFF
    0x80 -> Force Cursor to beginning of 1st line.
*/
unsigned char commands[4]={0x38,0x01,0x0C,0x08};	

// Main() Function
void main(){
    //Configure internal oscillator for max freq. (12 MHz)
	OSCICN = 0x83;	

    //Enable UART
	XBR0 = 0x01;		

    //Enable Cross-bar
	XBR1 = 0x40;		

    //Set pins 5,6,7 as output pins
	P1MDOUT = 0xE0;	

    //Set P2 as output port
	P2MDOUT = 0xFF;		

    //Set Pin 3.0 as input
	P3MDIN =  0xFE;		

    P0SKIP = 0xFF;
    P1SKIP = 0xFF;
    P2SKIP = 0xFF;	

	//Serial Control register is given command to enable UART receiver
    SCON0 = 0x08;		         
                                                    
    //(REN0 bit is set to  '1')
	CKCON = 0x01;

    //Timer value for 9600 bps
	TH1 = -(SYSCLK/BR_UART0/2/4);

    //Init Timer 1
	TL1 = TH1;		

    //Timer 1 in 8-bit auto-reload		
	TMOD = 0x20;	

    //Start Timer 1			
	TR1 = 1;				


	for(i=0; i<4; i++){
        //Call function to write commands to LCD
		writeCommand(commands[i]);		

        //Call delay routine
		delayMS(50);				
	}

	//As code is running continuously, while loop is used
	while(1){
		count = 0;

        //Force cursor to beginning of first line
		writeCommand(0x80);		

		while(count < 12){
            //Receive data from EM-18 and store it in array
			card_number[count]=readTX();	 

            //Increment counter variable by one.
			count++;			 
		}

		for(i=0; i<12 ;i++){
            //Send data to LCD one character at a time.
			writeData(card_number[i]);	

			delayMS(50);
		}

		delayMS(50);
	}
}

//Delay routine function which takes milliseconds as input.
void delayMS(unsigned int MS){
	unsigned int n;
	unsigned int i;

	for(n=0;n<MS;n++){
		for(i=0;i<65;i++);
	}
}

//Function to write commands to LCD
void writeCommand(unsigned char Command){
    //RS value is set to 0 to denote command mode
	LCD_RS = 0;		

    //RW value is 0 as write mode is selected
	LCD_RW = 0;		
	P2 = Command;

    //EN pin HIGH -> LOW
	LCD_EN = 1;		

	delayMS(15);
	LCD_EN = 0;
}

//Function to write data to LCD
void writeData(unsigned char Character){
    //RS value is set to 1 as actual data is being sent
	LCD_RS = 1;		

    //RW value is 0 as we are writing to LCD
	LCD_RW = 0;		
	P2 = Character;
    //EN pin HIGH -> LOW
	LCD_EN = 1;		

	delayMS(15);
	LCD_EN = 0;
}

//Function to read single character from EM-18
char readTX(void){
	char ch;

    //Wait till TI0 becomes HIGH
	while(TI0==0);		
	TI0 = 0;

    //Assign value stored in SBU0 register to ch
	ch = SBUF0;		

    //Return received character
	return ch; 		
}