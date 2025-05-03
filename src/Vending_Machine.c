#include <unistd.h>
#include "address_map.h"
#include "UTILS.h"
#include "Timer_Driver.h"
#include "Product.h"

/*
 * Products to be vended
 */
struct product product1;
struct product product2;
struct product product3;

/*
 * Drivers for timer
 */
alt_alarm Coin_Timeout;

/*
 * The following functions will be used to switch states
 * With the main function being used to implement the vending state
 * Setup and delivery functions will be called in the ISR as the switches are pressed
 */
void setup();
void delivery();
int deliveryer();
void vend();
int vender();

/*
 * This function will be used to handle interrupts for the timer alarms
 * For the coin timeout
 */
alt_u32 Coin_Timeout_Handler(void * context);

/*
 * Volatile pointers for IO Devices
 */
volatile int* SW_ptr = (int*) SLIDER_SWITCH_BASE;
volatile int* green_LED_ptr = (int*) GREEN_LED_BASE;
volatile int* red_LED_ptr = (int*) RED_LED_BASE;
volatile int* KEY_ptr = (int*) PUSHBUTTON_BASE;
volatile int* HEX3toHEX0_ptr = (int*) HEX3_HEX0_BASE;

/*
 * Setup for temporary variables
 */
int looper = 0;
int timerFlag = 0;
int depositsum = 0;
int quantity = 0;
int i = 0;

/*
 * Sets up Pulling of our Switches and Keys
 */
int SW0, SW8, SW9 = 0; // Switches
int KEY1, KEY2, KEY3 = 0; // Keys
int SW_VAL, KEY_VAL = 0;

int main(void) {
	while (1) {
		// Switches pulling
		SW_VAL = *(SW_ptr);
		SW8 = SW_VAL & 0x100;
		SW9 = SW_VAL & 0x200;
		// Keys pulling
		KEY_VAL = *(KEY_ptr);
		KEY1 = 0x00000002 & KEY_VAL;
		KEY2 = 0x00000004 & KEY_VAL;
		KEY3 = 0x00000008 & KEY_VAL;

		// Jumps to Initial State if Switch 9 is on
		if (SW9 > 1) {
			setup();
		}
		// Jumps to Preparation State if Switch 8 is on
		else if (SW8 > 1) {
			delivery();
		}
		// If both switch 8 and 9 are off launch Vending State
		else {
			vend();
		}
	}
}

/*
 * Initial State
 */
void setup() {
	// Displays Init to tell the user that the state has been launched successfully
	*(HEX3toHEX0_ptr) = 0x30541078;
	usleep(1000000);
	*(HEX3toHEX0_ptr) = 0x0;
	// While loop to that keeps the device in the Initial State until switch 9 is off
	while (SW9 > 1) {
		//IO Pulling
		SW_VAL = *(SW_ptr);
		SW9 = SW_VAL & 0x200;
		KEY_VAL = *(KEY_ptr);
		KEY1 = 0x00000002 & KEY_VAL;
		KEY2 = 0x00000004 & KEY_VAL;
		KEY3 = 0x00000008 & KEY_VAL;
		// Displays dashes to tell operator we our in Initial State
		*(HEX3toHEX0_ptr) = 0x80808;
		// Sets up a price variable based on Sw7-Sw1 positions (assumes switch 8 is off)
		int price = *(SW_ptr) - 512;
		// This logic checks for SW8 and subtracts it from the value
		if (price > 255) {
			price = price - 256;
		}
		// If KEY1 is pressed, set the price of product1 to price variable
		if (KEY1 > 1) {
			product1.unit_price = price;
			*(HEX3toHEX0_ptr) = HEX3to0_display(product1.unit_price, table); //Display Price
			usleep(1000000);
		}
		// If KEY2 is pressed, set the price of product2 to price variable
		if (KEY2 > 1) {
			product2.unit_price = price;
			*(HEX3toHEX0_ptr) = HEX3to0_display(product2.unit_price, table); //Display Price
			usleep(1000000);
		}
		// If KEY3 is pressed, set the price of product3 to price variable
		if (KEY3 > 1) {
			product3.unit_price = price;
			*(HEX3toHEX0_ptr) = HEX3to0_display(product3.unit_price, table); //Display Price
			usleep(1000000);
		}

	}

	// Displays underscores telling operator that initial state is exiting, sleeps, and then clears display.
	*(HEX3toHEX0_ptr) = 0x404040;
	usleep(1000000);
	*(HEX3toHEX0_ptr) = 0x0;
}

/*
 * Preparation State
 */
void delivery() {
	// While loop to that keeps the device in the Prep State until switch 8 is off
	// Could we display the word 'Prep' here for a second?
	*(HEX3toHEX0_ptr) = 0x73507973;
	usleep(1000000);
	*(HEX3toHEX0_ptr) = 0x0;
	while (SW8 > 1) {
		//IO Pulling
		SW_VAL = *(SW_ptr);
		SW8 = SW_VAL & 0x100;
		KEY_VAL = *(KEY_ptr);
		KEY1 = 0x00000002 & KEY_VAL;
		KEY2 = 0x00000004 & KEY_VAL;
		KEY3 = 0x00000008 & KEY_VAL;
		// Displays dashes to tell operator we our in Initial State
		*(HEX3toHEX0_ptr) = 0x80808;
		if (KEY1 > 1) {
			product1.units_left = deliveryer(product1.unit_price); // If KEY1 is pressed, set the quantity of product1 to quantity variable
		}
		if (KEY2 > 1) {
			product2.units_left = deliveryer(product2.unit_price); // If KEY1 is pressed, set the quantity of product1 to quantity variable
		}
		if (KEY3 > 1) {
			product3.units_left = deliveryer(product3.unit_price); // If KEY1 is pressed, set the quantity of product1 to quantity variable
		}
	}
	// Displays underscores telling operator that initial state is exiting, sleeps, and then clears display.
	*(HEX3toHEX0_ptr) = 0x404040;
	usleep(1000000);
	*(HEX3toHEX0_ptr) = 0x0;
}

/*
 * This function actually sets the product based on the key that is pressed.
 */
int deliveryer(productPrice) {
	// Sets up a quantity variable based on Sw7-Sw1 positions (assumes switch 9 is off)
	int quantity = *(SW_ptr) - 256;
	if (productPrice) {
		*(HEX3toHEX0_ptr) = HEX3to0_display(quantity, table); // Display Quantity
		usleep(1000000);
		return quantity;
	}
	else{
		*(HEX3toHEX0_ptr) = 0x795050;       // Displays Err if no price has been set
		usleep(1000000);
	}
}

/*
 * Vending State
 */
void vend() {
	product1.units_left = vender(KEY1, product1.unit_price, product1.units_left); // Starts Vending item1
	product2.units_left = vender(KEY2, product2.unit_price, product2.units_left); // Starts Vending item2
	product3.units_left = vender(KEY3, product3.unit_price, product3.units_left); // Starts Vending item3
}

/*
 * This function vends the products and is called by the vend function. It is mostly used for saving space
 */
int vender(KEY, productPrice, productQuantity) {
	// Checks for key press to start vending
	if (KEY > 1) {
		// Checks that our product has a price and is stocked
		if (productPrice && productQuantity) {
			timerFlag = 1; // Starts our timer
			*(HEX3toHEX0_ptr) = HEX3to0_display(productQuantity, table); // Displays the current Units of product
			usleep(1000000);
			*(HEX3toHEX0_ptr) = HEX3to0_display(depositsum, table);
			looper = 1;
			alt_alarm_stop(&Coin_Timeout);
			alt_alarm_start(&Coin_Timeout, alt_ticks_per_second() * 15, Coin_Timeout_Handler, NULL); // Starts Timer
			// Loops until our timer is turned off
			while (looper) {
				// IO pulling
				SW_VAL = *(SW_ptr);
				SW0 = SW_VAL & 0x1;
				KEY_VAL = *(KEY_ptr);
				KEY1 = 0x00000002 & KEY_VAL;
				KEY2 = 0x00000004 & KEY_VAL;
				KEY3 = 0x00000008 & KEY_VAL;
				// Nickels
				if (KEY1 > 1) {
					*(HEX3toHEX0_ptr) = HEX3to0_display(05, table);
					depositsum += 5;
					timerFlag = 1;
					alt_alarm_stop(&Coin_Timeout); // Stop Timer
					alt_alarm_start(&Coin_Timeout, alt_ticks_per_second() * 15, Coin_Timeout_Handler, NULL); // Starts Timer
					usleep(500000);
					*(HEX3toHEX0_ptr) = HEX3to0_display(depositsum, table);
				}
				// Dimes
				else if (KEY2 > 1) {
					*(HEX3toHEX0_ptr) = HEX3to0_display(10, table);
					depositsum += 10;
					timerFlag = 1;
					alt_alarm_stop(&Coin_Timeout); // Stop Timer
					alt_alarm_start(&Coin_Timeout, alt_ticks_per_second() * 15, Coin_Timeout_Handler, NULL); // Starts Timer
					usleep(500000);
					*(HEX3toHEX0_ptr) = HEX3to0_display(depositsum, table);
				}
				// Quarters
				else if (KEY3 > 1) {
					*(HEX3toHEX0_ptr) = HEX3to0_display(25, table);
					depositsum += 25;
					timerFlag = 1;
					alt_alarm_stop(&Coin_Timeout); // Stop Timer
					alt_alarm_start(&Coin_Timeout, alt_ticks_per_second() * 15, Coin_Timeout_Handler, NULL); // Starts Timer
					usleep(500000);
					*(HEX3toHEX0_ptr) = HEX3to0_display(depositsum, table);
				}
				// Vend Item
				else if (SW0 > 0 && depositsum >= productPrice) {
					*(HEX3toHEX0_ptr) = HEX3to0_display((depositsum
							- productPrice), table);
					*(green_LED_ptr) = 0x80;
					productQuantity = productQuantity - 1;
					depositsum = 0;
					usleep(1000000);
					timerFlag = 0;
					looper = 0;
					*(HEX3toHEX0_ptr) = 0x0;
					while(SW0){
						SW_VAL = *(SW_ptr);
						SW0 = SW_VAL & 0x1;
					}
					*(green_LED_ptr) = 0x0;
				}
				// Return Change
				else if (SW0 > 0 && depositsum < productPrice) {
					*(red_LED_ptr) = 0x1;
					depositsum = 0;
					usleep(1000000);
					timerFlag = 0;
					looper = 0;
					*(HEX3toHEX0_ptr) = 0x0;
					while(SW0){
						SW_VAL = *(SW_ptr);
						SW0 = SW_VAL & 0x1;
					}
					*(red_LED_ptr) = 0x0;
				}
			}
			*(HEX3toHEX0_ptr) = 0x0;
		}
		// Display Out if there is not product
		else {
			*(HEX3toHEX0_ptr) = 0x3F3E78;
			usleep(1000000);
			*(HEX3toHEX0_ptr) = 0x0;
		}
	}
	return (productQuantity);
}
;

/*
 * Timer Handler
 */
alt_u32 Coin_Timeout_Handler(void * context) {
	if (timerFlag == 0) {
		// Checks if depositsum is not 0 and if so returns the coins after timeout.
		if (depositsum) {
			*(red_LED_ptr) = 0x1;
			depositsum = 0;
			usleep(500000);
			*(red_LED_ptr) = 0x0;
		}
		looper = 0; // Exits while loop in vend state
	}
	timerFlag = 0;
	return alt_ticks_per_second() * 15;
}

/*
 * Function to display our values
 */
int HEX3to0_display(int value, unsigned char * table) {
	int num = value;
	int digitCount = countDigits(value); // Number of digits in the number to be displayed on the Hex Displays
	int digitArray[digitCount]; // Array for split digits
	int digitHexArray[digitCount]; // Array for the HEX representation of digits

	// Setup
	int intermediateHEX = 0;
	int intermediateHEX1 = 0;
	int finalNum = 0;

	// This loop will split the number into a seperate array
	for (i = 0; i < digitCount; i++) {
		digitArray[i] = num % 10;
		num /= 10;
	}

	// This will create an array of hex numbers from the table casted into ints that correspond to the array of numbers from the input above
	for (i = 0; i < digitCount; i++) {
		digitHexArray[i] = (int) table[digitArray[i]];
	}

	// Various logic to handle multiple digits
	if (digitCount == 1) {
		finalNum = ((0 << 24) | digitHexArray[0]);
	} else if (digitCount == 2) {
		intermediateHEX = (digitHexArray[1] << 8 | digitHexArray[0]);
		finalNum = ((0 << 16 | intermediateHEX));
	} else if (digitCount == 3) {
		intermediateHEX = (digitHexArray[2] << 8 | digitHexArray[1]);
		intermediateHEX1 = (intermediateHEX << 8 | digitHexArray[0]);
		finalNum = ((0 << 24 | intermediateHEX1));
	} else {
	}

	// Return the value that is to be written to the Hex Displays in order the show the needed number
	return finalNum;
}

/*
 * This counts the number of digits in the value parameter.
 * This is to facilitate easier parsing of the number written to the Hex Displays
 */
int countDigits(int value) {
	int count = 1;
	while (value /= 10) {
		count++;
	}
	return count;
}
