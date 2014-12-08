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
 // And reset the timer
