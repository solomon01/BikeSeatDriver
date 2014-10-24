#pragma once

// ===== DRIVER FUNCTIONS =====
// Initialize Driver
void ledInit();

// This should be called on an interrupt timer whose period
//      is at most 1/(60*4) seconds (the number of rows times
//      POV time)
void tickLEDs();

// ===== DISPLAY FUNCTIONS =====
// Clears the ring display
inline void zeroRing();

// Lights only the left half of the ring
inline void leftRing();

// Lights only the right half of the ring
inline void rightRing();

// Lights a single led on the ring, given the heading
// from 0 (12 o'clock) to 15
void compass(int heading);

// Pass a 1 to turn on status LED, 0 to turn off
inline void statusLED(int state);

// Given a number, display it on the 7 segment
//      Second number = 1 if the number is a 1 digit + decimal
//      Otherwise, two digit number
void sevenSeg(int num, int dec);
