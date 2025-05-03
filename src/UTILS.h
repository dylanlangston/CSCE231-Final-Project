#ifndef UTILS_H_
#define UTILS_H_

/* 
 * Provides a set of utilities for operation of the vending machine
 */
unsigned char table[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x7, 0x7f, 0x6f}; // Lookup table for displaying characters on the hex displays

int HEX3to0_display (int value, unsigned char * table); // Function to display a value on the 7-segment hex displays
int countDigits(int value);                             // Counts individual digits of its parameter, used as a helper function to HEX3t0_Display

#endif /* UTILS_H_ */
