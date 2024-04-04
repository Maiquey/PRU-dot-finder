// sigDisplay.h
//
// Module to drive the 14-sig display 

#ifndef _SEG_DISPLAY_H_
#define _SEG_DISPLAY_H_

#include <stdbool.h>

// Begin/end the background thread which drives the 14-sig display
// Also sets necessary config pins
void SegDisplay_init(void);
void SegDisplay_cleanup(void);

// Setter function for the number displayed by the 14-sig display
void SegDisplay_setNumber(int newValue);

#endif