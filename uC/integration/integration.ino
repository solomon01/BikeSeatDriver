#define F_CPU 8000000UL
#include <SPI.h>
#include <leddriver.h>
#include "Adafruit_BLE_UART.h"
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2
#define ADAFRUITBLE_RST 9
leddriver leds;


// for directions
/*to create the compass directions
compass
//8 possible directions
		0x00		north north (full north) =0
		0x01		north east		=1
		0x03		north west		=3
		0x22		south south (full south) =34
		0x21		south east               =33
		0x23		south west               =35
		0x11		east east (full east)    =17
		0x33		west west (full west)    =51
*/

#define NORTH_NORTH (0x00)
#define NORTH_EAST (0x01)
#define NORTH_WEST (0x02)
#define SOUTH_SOUTH (0x22)
#define SOUTH_EAST (0x21)
#define SOUTH_WEST (0x23)
#define EAST_EAST (0x11)
#define WEST_WEST (0x33)

//for turns
//possible turns are: forawrd, right(slight,regular and hard), left (slight,regular and hard)and "U" turn
 /*
 //one byte for the direction and turns
directions 
				//four possible direction indicators
		0x00		forward
		0x11		backward (could be usually given as make a U tern in the GPS module)

					right: there are three direction indicators that could be considered as right
		0x20				Sharp right ( greater than 90 degree right)
		0x21				regular right (approximately 90 degree right)
		0x22				slight right (less than 90 degree right)

					left: there are three direction indicators that could be considered as left
		0x30				Sharp left ( greater than 90 degree left)
		0x31				regular left (approximately 90 degree left)
		0x32				slight left (less than 90 degree left)

 */

#define FORWARD (0x00)
#define U_TURN (0x11)
#define HARD_RIGHT (0x20)
#define REGULAR_RIGHT (0x21)
#define SLIGHT_RIGHT (0x22)
#define HARD_LEFT (0x30)
#define REGULAR_LEFT (0x31)
#define SLIGHT_LEFT (0x32)



//timing LUT to be used in imediacy of turns
int timing_LUT[] = { 2500, 5000, 10000};
//represnts the state of LEDs 0 for blinker , 1 otherwise
int cur_LED_timing_state = 0;

int heading_from_bluetooth =0;
int immediacy_from_bluetooth =0;
int direction_from_bluetooth =0;
int speed_from_bluetooth =0;

void toggle_LEDS() 
{   
   if (cur_LED_timing_state) 
   {
     leds.zeroRing();heading_direction(heading_from_bluetooth);
   } 
   else 
   {
      leds.zeroRing(); heading_direction(heading_from_bluetooth);direction_of_the_turns(direction_from_bluetooth);
   }
   // Toggle the state representation of the LEDs
   cur_LED_timing_state = !cur_LED_timing_state;
}

void init_timer() 
{
 // Start timer with a prescale of 1028
TCCR1A = 0;
TCCR1B = 0;
TCCR1B |= (1 << CS02) | (1 << CS00);
 TCNT1 = 0;
}
void check_timer() 
{
 // TCNT1 is the value in the counter −−− // if it is greater than our immediacy value...
 if (TCNT1 >= timing_LUT[immediacy_from_bluetooth]) 
 {
 // Call the toggle function
 toggle_LEDS();
 // And reset the timer xample.cpp Page 2
 TCNT1 = 0;
 }
}



Adafruit_BLE_UART uart = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

// Test value:
// byte 0: 0x03 --> north west
// byte 1: 0x01 --> medium immediacy
// byte 2: 0x21 --> regular right
// byte 3: 0xDF --> MSB indicates whether to light the decimal point, mask off.
//                  the rest is a binary representation of decimal number of just the digits: 
//                  Example: to represent 9.5 mph, multiply by 10 --> 95. In binary --> 0101 1111, 
//                  then toggle MSB to indicate decimal --> 1101 1111 --> 0xDF
// Full message: [ 0x03 | 0x01 | 0x21 | 0xDF ]

#define tHEADING	(0x03)
#define tIMMEDIACY	(0x01)
#define tDIRECTION	(0x21)
#define tSPEED		(0xDF)

// handle BLE events
// Use these events to trigger the Bluetooth Status LED 
// in the middle of the ring on the display board

void aciCallback(aci_evt_opcode_t event)
{
  switch(event)
  {
    case ACI_EVT_DEVICE_STARTED:
      //device is started and is ready to be connected
      //leds.statusLED(0);            
      break;
    case ACI_EVT_CONNECTED:
      // device connected event
      //leds.statusLED(1);      
      break;
    case ACI_EVT_DISCONNECTED:
      // device distonnected event
    //leds.statusLED(0);
      break;
    default:
      break;

  }

}

// data received

// this is where the message 

void rxCallback(uint8_t *buffer, uint8_t len)
{
  // this is how you access each byte in the buffer
  uint8_t heading       = *(buffer + 0);
  uint8_t immediacy     = *(buffer + 1);
  uint8_t thisDirection = *(buffer + 2);    // 'direction' is a reserved word?!?
  uint8_t thisSpeed     = *(buffer + 3);    // 'speed' is a reserved word?!?

  // do acctions with the hex values accepted from the bluetooth
  //for heading
          heading_from_bluetooth=heading;       //this variable will hold the value of the direction from buffer
          immediacy_from_bluetooth =immediacy;
          direction_from_bluetooth = thisDirection;
          speed_from_bluetooth = thisSpeed;

}


//to handle the direction s of the ride. This will call the compas function
void heading_direction(int heading_to)
{
		switch (heading_to)
		{
                       case(255):                  //test LEDs if phone sends 0xFF
                            leds.testSequence(); 
                            break;                         
			case(NORTH_NORTH):
                            leds.ringAdd(0);leds.ringAdd(15);
                            break;					//full north
			case(NORTH_EAST):
                            leds.ringAdd(1);leds.ringAdd(2);
                            break;						//north east 
			case(NORTH_WEST):
                            leds.ringAdd(13);leds.ringAdd(14);
                            break;					//north west
			case(EAST_EAST):
                            leds.ringAdd(3);leds.ringAdd(4);
                            break;						//value for full east
			case(SOUTH_EAST):
                            leds.ringAdd(5);leds.ringAdd(6);						
			    break;							//value for south east		
			case(SOUTH_SOUTH):
                            leds.ringAdd(7);leds.ringAdd(8);
                            break;						//value for full south
			case(SOUTH_WEST):
                            leds.ringAdd(9);leds.ringAdd(10);
                            break;						//value for south west
			case(WEST_WEST):
                            leds.ringAdd(11);leds.ringAdd(12);
			    break;							//value for full west	
			default :
				break;
		}
  }
 //to handle the immediacy of the turn
 
 //for the bytes containing speed
 void speed_of_bike(unsigned int velocity)
 {
	int bike_speed;
	int dec;
	if (velocity & 0b10000000)					//if >128, MSB of the speed in from the IOS would be 1, MSB=1, is a decimal point
	{
		dec=1;
		bike_speed=velocity & 0b01111111;
	}
	else
	{
		dec=0;
		bike_speed=velocity;
	}
	leds.sevenSeg(bike_speed, dec);
 }


 //this function will deal with the direction of the ride
 //it will have right and left and the type of tun (regular, slight or hard turn)

 void direction_of_the_turns(int turn)
 {  
		switch (turn)
		{
			case(FORWARD):
                            leds.ringAdd(15);leds.ringAdd(14);leds.ringAdd(1);leds.ringAdd(0);
			    break;				          //forward
			case(U_TURN):
			    leds.ringAdd(9);leds.ringAdd(8);leds.ringAdd(7);leds.ringAdd(6);
                            break;						//backward
			case(HARD_RIGHT):
                            leds.ringAdd(6);leds.ringAdd(5);leds.ringAdd(4);leds.ringAdd(3);
			    break;						//sharp right
			case(REGULAR_RIGHT):
                            leds.ringAdd(2);leds.ringAdd(3);leds.ringAdd(4);leds.ringAdd(5);
			    break;						//regular right
			case(SLIGHT_RIGHT):	
                            leds.ringAdd(4);leds.ringAdd(3);leds.ringAdd(2);leds.ringAdd(1);					
			    break;						//slight right
			case(HARD_LEFT):
                            leds.ringAdd(12);leds.ringAdd(11);leds.ringAdd(10);leds.ringAdd(9);
			    break;					//sharp left
			case(REGULAR_LEFT):
                            leds.ringAdd(13);leds.ringAdd(12);leds.ringAdd(11);leds.ringAdd(10);
			    break;						//regular left
			case(SLIGHT_LEFT):
                            leds.ringAdd(14);leds.ringAdd(13);leds.ringAdd(12);leds.ringAdd(11);
			    break;							//slight left
			default :
				break;
		}
 
 }
 
 //to test the functions
void tests()
{
    int i;
    int time;
    int heading[]={ 255, NORTH_NORTH,NORTH_EAST, NORTH_WEST,EAST_EAST, SOUTH_EAST, SOUTH_SOUTH, SOUTH_WEST,WEST_WEST};
    int turn[]={ FORWARD,SLIGHT_RIGHT, REGULAR_RIGHT, HARD_RIGHT, SLIGHT_LEFT, REGULAR_LEFT, HARD_LEFT, U_TURN};
    unsigned int miles_hr[]={ 0,5,10,15,25,50,130,135,145};
  
 /* 
   for( i=0; i<9; ++i)
  {
    int time;
    leds.zeroRing();
    heading_direction(heading[i]);
    for(time=0; time<500; ++time)
    {
       leds.tickLEDs();
       delay(2);
    }
  }
 
  //test for turns
  for( i=0; i<8; ++i)
  {leds.zeroRing();
   turns(turn[i]);
   for(time=0; time<500; ++time)
    {
       leds.tickLEDs();
       delay(2);
    }
   
  }*/
  //test for speed
  for( i=0; i<9; ++i)
  {leds.zeroRing();
   speed_of_bike(miles_hr[i]);
   for(time=0; time<500; ++time)
    {
       leds.tickLEDs();
       delay(2);
    }
  }
}
// set callback functions and start BLE service,

// also set up any other I/O

void setup(void)

{ 

  uart.setRXcallback(rxCallback);

 uart.setACIcallback(aciCallback);

  uart.setDeviceName("ECE411"); /* 7 characters max! */

 uart.begin();
  

 

}



// Poll for new data inside main loop

void loop()

{
  

  uart.pollACI();
  //checktimer
   check_timer();
 // Initiate the next scan in the LED display
 // NOTE tickLEDs is a method on the object 'leds'
 leds.tickLEDs();
}
