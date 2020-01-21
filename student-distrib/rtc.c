/*
 * This file will contain all functions relating to the RTC.
 */
#include "rtc.h"

volatile uint8_t read_wait = 0;		// Signals if the user is waiting the read the RTC. 1 means waiting.
uint8_t rtc_opened = 0;		// Signals that the RTC is open if 1. 
/***********************************/
/***** RTC INTERRUPT FUNCTIONS *****/
/***********************************/
/* 
 * Here, we initialize the RTC. We set the default interrupt frequency
 * to 2 Hz and enable periodic interrupts. This function is called from
 * the OS intialization (boot) sequence, so interrupts are already disabled.
 * Additionally, we set up the IDT entry for the RTC so that the handler can 
 * be called.
 */
void rtc_init(){
	/* Flag the RTC as open so it will not initialize again. */
	rtc_opened = 1;
	
	/* Initialize the RTC chip itself. */
	outb(0x8A, RTC_INDEX_PORT);				// Select register A on the RTC.
	outb(0x2F, RTC_DATA_PORT);				// Enable the oscillator circuit and set the rate.
	outb(0x8B, RTC_INDEX_PORT);				// Select register B.
	uint8_t b_orig = inb(RTC_DATA_PORT);	// Read the current value of B.
	outb(0x8B, RTC_INDEX_PORT);				// Reset index to B.		
	outb(0x40 | b_orig, RTC_DATA_PORT);		// Enable periodic interrupts.
	
	/* 
	 * We need to throw away the contents of register C 
	 * so that the RTC will continue giving interrupts.
	 */
	outb(0x8C, RTC_INDEX_PORT);
	inb(RTC_DATA_PORT);
	
	/* Create the RTCs entry in the IDT. */
	uint8_t idtPort = 0x28;	//This is the location that the RTC interrupt descriptor occupies in the IDT.
	idt[idtPort].size = 0x1;			// This is a 32-bit gate.
	idt[idtPort].seg_selector = KERNEL_CS;
	idt[idtPort].reserved1 = 0x1;		// Set these reserved bits to signal to the IDT that this is an interrupt.
	idt[idtPort].reserved2 = 0x1;
	SET_IDT_ENTRY(idt[idtPort], rtc_linker);	
	idt[idtPort].present = 0x1;			//Mark the interrupt as present.
	
	/* Tell the user it worked. */
	puts("RTC initialized.\n");
}

/* 
 * This function indicates that an RTC interrupt has occurred. In the future
 * it may be associated with other functions and/or system calls. 
 */
void rtc_handler(){
	uint32_t irq_num = 8;		// The RTC is on IRQ8 (Slave IRQ0)
		
	/* 
	 * We need to throw away the contents of register C 
	 * so that the RTC will continue giving interrupts.
	 */
	outb(0x8C, RTC_INDEX_PORT);
	inb(RTC_DATA_PORT);
	
	/* Set the read_wait to 0. This way read knows to return 0. */
	read_wait = 0;
	
	/* Send the EOI and enable this interrupt pin again. */
	send_eoi(irq_num);
}

/*******************************/
/****** CORE RTC FUNCTIONS *****/
/*******************************/
/* 
 * This function will check to see if the RTC is already opened,
 * and will open it if not. By "opening," I mean that the rate of 
 * periodic interrupts will be reset to 2 Hz if the rtc is not open.
 *
 * RETURN: Returns 0 on success, -1 on failure.
 */
int32_t rtc_open(){
	/* Make sure that the RTC is not already open. */
	// if(rtc_opened){
	// 	puts("The RTC is already open.\n");
	// 	return -1;
	// }
	rtc_opened = 1;								// Mark RTC as opened. 
	
	/* If the RTC is closed (interrupts disabled), then reset the interrupt rate. */
	cli();										// We don't want interrupts.
	outb(0x8A, RTC_INDEX_PORT);					// Select register A on the RTC.
	uint8_t a_orig = inb(RTC_DATA_PORT);		// Get the current values of register A.
	a_orig &= 0xF0;								// Preserve only the upper bits of A.
	outb(0x8A, RTC_INDEX_PORT);					// Reset index.		
	outb(a_orig | 0x0F, RTC_DATA_PORT);			// Write to register A and reset the interrupt frequency to 2 Hz.
	sti();
	return 0;									// Success.
}

/*
 * This function will check to see if the RTC is already closed, and
 * will close it if not. Currently "closing" the RTC has no meaning.
 *
 * RETURN: Returns 0 on success, -1 on failure.
 */
int32_t rtc_close(){
	/* Make sure that the RTC is already open. */
	if(!rtc_opened){
		puts("The RTC is already closed.\n");
		return -1;
	}
	rtc_opened = 0;								// Mark RTC as closed.
	return 0;									// Success.
}

/* 
 * This function will wait for the next RTC interrupt and then returns 0.
 * It uses the read_wait flag to "synchronize" this in a simple way.
 */
int32_t rtc_read(int32_t fd, const void* buf, int32_t nbytes){
	read_wait = 1;		// Signal that we are waiting for an interrupt.
	while(read_wait){
		// Wait for read_wait to be brought low again by an RTC interrupt.
	}
	//puts("Read the RTC.\n");
	// putc('1');		// For debugging just print this random char to screen.
	return 0;
}

/* 
 * rtc_write will set the RTC interrupt frequency to the desired rate
 * specified by the caller. In reality, each frequency corresponds to a
 * 4-bit code in register A that needs to be changed.
 *
 * INPUTS:
 *		freq -- 	The desired frequency to set the RTC interrupts to. Must be a
 *					power of 2 below 1024 Hz.
 *
 * RETURN: Returns 0 on success, -1 on failure.
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
	/* Make sure freq is between 2 Hz and 1024 Hz. */
	if(buf == 0)
		return -1;
	if(nbytes != 4)
		return -1;

	int freq = *(int*)buf;

	if(freq < 2 || freq > 1024){ //power of 2 below 1024 Hz.
		puts("This frequency is out of range.\n");
		return -1;				// Failure.
	}
	
	/* 
	 * If it is in range, let's just do a switch over the 10 possible frequencies.
	 * Since select_bits is only 4 bits, the upper 4 bits will always be 0x0. The
	 * codes for the lower 4 bits come from the RTC data sheet.
	 */
	uint8_t select_bits;		// This is the value we will write into the last 4 bits of register A.
	switch(freq){				// Recall that freq is in Hz.
		case 2:
			select_bits = 0x0F;
			break;
		case 4:
			select_bits = 0x0E;
			break;
		case 8:
			select_bits = 0x0D;
			break;
		case 16:
			select_bits = 0x0C;
			break;
		case 32:
			select_bits = 0x0B;
			break;
		case 64:
			select_bits = 0x0A;
			break;
		case 128:
			select_bits = 0x09;
			break;
		case 256:
			select_bits = 0x08;
			break;
		case 512:
			select_bits = 0x07;
			break;
		case 1024:
			select_bits = 0x06;
			break;
		default:
			puts("Frequency given is invalid.\n");
			return -1;						// Failure.
	}
	
	/* 
	 * If we made it past the switch statement, we have a valid code.
	 * Now we need to append these 4 bits onto the end of register A.
	 */
	cli();									// We shouldn't be interrupted during this process.
	outb(0x8A, RTC_INDEX_PORT);				// Select register A.
	uint8_t a_orig = inb(RTC_DATA_PORT);	// Grab A's original value.
	a_orig &= 0xF0;							// Save only the upper 4-bits (nibble?).
	select_bits |= a_orig;					// Combine values.
	outb(0x8A, RTC_INDEX_PORT);				// Reset index.		
	outb(select_bits, RTC_DATA_PORT);		// Write the data to register A.
	sti();
	return 0;								// Success.
}
