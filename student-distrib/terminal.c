/* This file contains functions pertaining to the terminal driver. */
#include "terminal.h"

/* 
 * This boolean is a testing variable that tells the read function to call the
 * write function to echo the read characters back to the screen.
 */
#define ECHO_ON		0		// 1 means echo, 0 means don't.

/* Tells us if the terminal is open or not. */
uint8_t terminal_opened = 0;
//uint8_t enter_flag = 0;
unsigned char terminal_buf[MAX_BUF] = {0};
volatile uint8_t buf_sent = 0;

/* Keep track of the amount of characters put onto the sceen by terminal write. */
unsigned int x_start_tw = 0;
unsigned int y_start_tw = 0;
unsigned int override_start = 0;

/* Holds current terminal being used. */
volatile uint8_t cur_term_num = 0;

/* Holds the current terminal being serviced by the scheduler. */
volatile uint8_t term_in_service = 0;

/*******************************************/
/***** MISCELLANEOUS HELPER FUNCTIONS. *****/
/*******************************************/
/* 
 * This function will be called by the keyboard handler
 * and will populate the global terminal buffer with the 
 * keyboard buffer's contents when a user presses ENTER.
 */
void send_buffer(unsigned char* buf){
	int i;
	for(i = 0; i < MAX_BUF; i++){
		terminal_buf[i] = buf[i];
	}
	buf_sent = 1;
}

/* These two functions grab the x_start_tw and y_start_tw variables. */
unsigned int get_x_start_tw(){
	return x_start_tw;
}

unsigned int get_y_start_tw(){
	return y_start_tw;
}

/* 
 * The override functions let us know if the values for x_start and y_start should
 * be over-written
 */
unsigned int get_override(){
	return override_start;
}

unsigned int set_override(unsigned int ovr){
	override_start = ovr;
	return 0;
}

/* These functions are the getters and setters for the current displayed terminal. */
uint8_t get_cur_term(){
	return cur_term_num;
}

uint8_t set_cur_term(uint8_t num){
	cur_term_num = num;
	return 0;
}

/* These functions will allow us to fetch and set the terminal in service right now. */
uint8_t get_term_in_service(){
	return term_in_service;
}

void set_term_in_service(uint8_t term_num){
	term_in_service = term_num;
}

/****************************/
/***** CURSOR FUNCTIONS *****/
/****************************/
/* 
 * This function will set the cursor to be a rectangular block that
 * takes up a full character block. It will set the text mode cursor on
 * if it is off.
 *
 * INPUTS:
 *		start 	-- The scanline of the cursor to start on.
 *		end 	-- The scanline of the cursor to end on. 
 */
void enable_cursor(uint8_t start, uint8_t end){
	outb(0x0A, 0x3D4);							// Select the Cursor Start Register.
	outb((inb(0x3D5) & 0xC0) | start, 0x3D5);	// Enable the cursor.
	
	outb(0x0B, 0x3D4);							// Select the Cursor End Register.
	outb((inb(0x3D5) & 0x60) | end, 0x3D5);		// Select the maximun scan line for the cursor.
}

/* 
 * This function will enable the "Cursor Disable" bit, which disables the
 * textmode cursor.
 */
void disable_cursor(){
	outb(0x0A, 0x3D4);
	outb(0x20, 0x3D5);
}

/* 
 * This function updates the position of the cursor on the screen.
 * It does NOT update the location where the next character will print
 * though, that is a different function.
 *
 * INPUTS:
 *		x -- x coordinate of cursor position.
 *		y -- y coordinate of cursor position.
 */
void update_cursor(int x, int y){
	int pos = y * VGA_WIDTH + x;	// Calculate the proper offset.
	
	outb(0x0F, 0x3D4);					// Select the Cursor Location Low Register.
	outb(pos & 0x00FF, 0x3D5);			// Write only the lowest byte to this register.
	outb(0x0E, 0x3D4);					// Select the Cursor Location High Register.
	outb((pos >> 8) & 0x00FF, 0x3D5);	// Write the highest byte to this register.
}

/***********************************/
/***** CORE TERMINAL FUNCTIONS *****/
/***********************************/
int32_t terminal_open(){
	/* Make sure terminal is not already open. */
	if(terminal_opened){
		puts("Terminal already open.\n");
		return -1;						// Failure.
	}
	terminal_opened = 1;				// Signal that the terminal is now open.
	
	/* Enable the text mode cursor on screen. */
	uint8_t cursor_start = 0x00;		// Make the cursor stretch from the top of a character to the bottom.
	
	/* Read the maximum scan line value from the Maximum Scan Line Register. */
	outb(0x09, 0x3D4);
	uint8_t msl = inb(0x3D5);
	uint8_t cursor_end = msl & 0x1F;		// Mask the upper 3 bits of the register since they do not pertain to the maximum scan line value.
	enable_cursor(cursor_start, cursor_end);
	
	/* Clear the screen and set the cursor to the top. */
	clear();
	update_cursor(0, 0);		// Reset the cursor position to (0, 0).
	set_screen_coords(0, 0);			// Reset the lib.c variables that control where in VGA memory we write.
	return 0;
}


/* Currently, this function does not need to do anything. It is sort of meaningless. */
int32_t terminal_close(){
	if(!terminal_opened){
		puts("Terminal already closed.\n");
		return -1;				// Failure.
	}
	terminal_opened = 0;		// Signal that the terminal is now closed.
	return 0;
}


int32_t terminal_read(int32_t fd, const void* buf, int32_t num_bytes){		
	/* Wait until the keyboard handler sends us a buffer. */
	while(1){
		if(buf_sent && (cur_term_num == term_in_service)){
			break;
		}
	}	

	/* 	
	 * Make sure that the user will not try to write MORE characters than are in
	 * the keyboard buffer. 
	 */	
	if(num_bytes > MAX_BUF){
		num_bytes = MAX_BUF;
	}	

	/* Go through the keyboard buffer and copy over the characters (bytes) to be read. */
	int i = 0;					// Iterator.
	for(i = 0; i < num_bytes; i++){
		((unsigned char*)buf)[i] = terminal_buf[i];
	}	
		

	/* Debugging call that will echo the read buffer to the screen. */
	if(ECHO_ON){
		puts("\nEchoing...\n");
		terminal_write(0, buf, num_bytes);
	}

	/* Reset the buf_sent flag so that we can wait for another. */
	buf_sent = 0;
	return num_bytes;			// Return the number of bytes read.
}

/*
 * This function will write the requested number of bytes to the terminal
 * from the given character buffer. They will be displayed on the screen on
 * their own line.
 *
 * INPUTS:
 *		buf			-- The buffer of characters to write from.
 *		num_bytes	-- The number of bytes (characters) to write from buf to the screen.
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t num_bytes){
	/* Go through number of requested bytes and print. */
	int i = 0;		// Iterator over bytes.
	for(i = 0; i < num_bytes; i++){
		putc(((unsigned char*)buf)[i]);
	}
	x_start_tw = get_screen_x();
	y_start_tw = get_screen_y();
	override_start = 1;
	return 0;
}

