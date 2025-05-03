#ifndef TIMER_DRIVER_H_
#define TIMER_DRIVER_H_
#include <sys/alt_alarm.h>
#include <alt_types.h>

alt_alarm Coin_Timeout;    // Struct to handle alarm interrupts for the timer

alt_u32 Coin_Timeout_Handler(void * context);    // Alarm interrupt handler

#endif /* TIMER_DRIVER_H_ */
