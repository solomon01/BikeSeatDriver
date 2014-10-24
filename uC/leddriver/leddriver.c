// NOTE: Assertions are for checking constants passed to functions and should be removed from
//      the final code. If the argument is not a constant (data from a sensor, for example),
//      it should be handled in the calling routine

#include <avr/io.h>
#include "leddriver.h"

// ========== CONSTANTS ==========
// The first row is the right side of the right, such that LSB=top + MSB=bottom
// The first row is the right side of the right, such that LSB=bottom + MSB=top
//      The are arranged this way to make them go clockwise as the 16 bit word increases
// The third row represents the 7 segment MSB, from a-g, then the decimal
// The last row represents the 7 segment LSB, then the status LED
static int ledMatrix[] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

static int sevenSegLUT[] = {
    0b11111100,
    0b01100000,
    0b11011010,
    0b11110010,
    0b01100110,
    0b10110110,
    0b10111110,
    0b11100000,
    0b11111110,
    0b11100110
};

// The state of the status LED
static int statusLEDstate = 0;

// Create power of 2 LUT to increase performance
static int pow2[] = {1, 2, 4, 8, 16, 32, 64, 128};

// The current row being mux'ed
static int curLEDrow;

// Define GPIO Pin Macros
// A,B,D,C correspond to the top bits of port D
// SHIFT,LATCH,DATA correspond to the bottom bits of port B
static rowSelectLUT[] = {
    0b01110000,
    0b10110000,
    0b11010000,
    0b11100000,
};

// ========== TEST MAIN =========================
// This is just for testing purposes. Mostly for syntax checking
//      Delete this. Soon.
void main() {
    int x;
    ledInit();
    zeroRing();
    leftRing();
    rightRing();
    statusLED(0);
    for (x = 0; x < 16; x++) {
        compass(x);
    }
    for (x = 0; x < 100; x++) {
        sevenSeg(x, 0);
    }
    for (x = 0; x < 100; x++) {
        sevenSeg(x, 1);
    }
}

// ========== UPDATE DISPLAY FUNCTIONS ==========
// Clears the ring display
inline void zeroRing() {
    ledMatrix[0] = 0;
    ledMatrix[1] = 0;
}

// Lights only the left half of the ring
inline void leftRing() {
    ledMatrix[0] = 0;
    ledMatrix[1] = 0b11111111;
}

// Lights only the right half of the ring
inline void rightRing() {
    ledMatrix[0] = 0;
    ledMatrix[1] = 0b11111111;
}

// Lights a single led on the ring, given the heading
// from 0 (12 o'clock) to 15
void compass(int heading) {
    int index;

    assert((heading <= 15) || (heading >= 0));

    // Check for carry to second row
    index = heading / 2;

    // Get bit of heading
    heading = pow2[heading % 8];
    zeroRing();

    // Actually set LED on
    ledMatrix[index] = heading;
}

// Pass a 1 to turn on status LED, 0 to turn off
inline void statusLED(int state) {
    assert((state == 0) || (state == 1));

    // Mask the LSB to 0 and add state to it
    ledMatrix[3] = (ledMatrix[3] & 0b11111110) + state;

    // Store this in case sevenSeg gets called
    statusLEDstate = state;
}

// Given a number, display it on the 7 segment
//      Second number = 1 if the number is a 1 digit + decimal
//      Otherwise, two digit number
void sevenSeg(int num, int dec) {
    assert((num / 100) == 0);
    assert((dec == 0) || (dec == 1));

    // Use LUT to set matrix
    ledMatrix[2] = sevenSegLUT[num] + !!(dec);
    ledMatrix[3] = sevenSegLUT[(num % 10)];

    // Repopulate LED state
    statusLED(statusLEDstate);
}


// ========== LED DRIVER FUNCTIONS ==========
void ledInit() {
    // Turn Status LED on
    statusLED(1);

    // Clear direction for A,B,C,D
    DDRD &= 0b00001111;
    // Set intial direction for A,B,C,D
    DDRD |= rowSelectLUT[0];
    // Set data on A,B,C,D to zero = high impedance on input
    PORTD &= 0b00001111;

    // Assure shift register pins are set to output
    DDRB |= 0b00000111;
}

// This should be called on an interrupt timer whose period
//      is at most 1/(60*4) seconds (the number of rows times
//      POV time)
void tickLEDs() {
    int x = 0; // For loop

    // Turn of LEDS by setting current row to high impedance (aka input mode)
    DDRD &= 0b00001111;
    DDRD |= rowSelectLUT[curLEDrow];

    // Increment row being displayed
    curLEDrow = (curLEDrow == 4) ? 0 : curLEDrow++;

    // Push data out shift register
    // Hopefully gcc unrolls this for loop
    for(x = 0; x < 8; x++) {
        // Zero out data bit
        PORTB &= 0b11111110;
        // Add 1 to register if led is active (mod 2 to convert nonzero to 1)
        PORTB |= (ledMatrix[curLEDrow] & x) % 2;

        // Latch Data In
        PORTB &= 0b11111101;
        PORTB |= 0b00000010;

        // Shift Data
        PORTB &= 0b11111011;
        PORTB |= 0b00000100;
    }

    // Reenable output on new row
    DDRD &= 0b00001111;
    DDRD |= rowSelectLUT[curLEDrow];
}
