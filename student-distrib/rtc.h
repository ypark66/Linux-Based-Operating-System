#ifndef _RTC_H
#define _RTC_H

#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"

#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT 0x71

/***********************************/
/***** RTC INTERRUPT FUNCTIONS *****/
/***********************************/
/* Assembly linkage function. */
extern void rtc_linker();

/* RTC initialization function. */
void rtc_init();

/* Interrupt handler for the RTC. */
void rtc_handler();

/*******************************/
/****** CORE RTC FUNCTIONS *****/
/*******************************/
/* Initializes the RTC if it is not yet open. */
int32_t rtc_open();

/* Closes the RTC if it has opened. */
int32_t rtc_close();

/* Returns 0 after the next RTC interrupt occurs. */
int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes);

/* Write an interrupt frequency (must be a power of 2 up to 1024) to the RTC. */
int32_t rtc_write(int32_t fd, const void* buf, int32_t freq);

#endif
