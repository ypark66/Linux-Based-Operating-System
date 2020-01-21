#ifndef _TERMINAL_H
#define _TERMINAL_H

/* This is the header file for terminal-related variables and functions. */
#include "lib.h"

#define VGA_WIDTH 80		// Value taken from lib.c
#define VGA_HEIGHT 25		// Value taken from lib.c
/* Maximum keyboard buffer size. */
#define	MAX_BUF		128

/***** MISC HELPERS *****/
void send_buffer(unsigned char* buf);
unsigned int get_x_start_tw();
unsigned int get_y_start_tw();
unsigned int get_override();
unsigned int set_override(unsigned int ovr);
uint8_t get_cur_term();
uint8_t set_cur_term(uint8_t num);

/* These functions will allow us to fetch and set the terminal in service right now. */
uint8_t get_term_in_service();
void set_term_in_service(uint8_t term_num);
/****************************/
/***** CURSOR FUNCTIONS *****/
/****************************/
/* Enables the text mode cursor. */
void enable_cursor(uint8_t start, uint8_t end);

/* Disables the text-mode cursor. */
void disable_cursor();

/* Sets the position of the text cursor. */
void update_cursor(int x, int y);

/***********************************/
/***** CORE TERMINAL FUNCTIONS *****/
/***********************************/
/* Initializes terminal functionality (text cursor). */
int32_t terminal_open();

/* Closes terminal (may do nothing at all). */
int32_t terminal_close();

/* Reads all characters from the keyboard buffer. */
int32_t terminal_read(int32_t fd, const void* kb_buf, int32_t num_bytes);

/* Writes num_bytes number of bytes from buf to the terminal. */
int32_t terminal_write(int32_t fd, const void* buf, int32_t num_bytes);





#endif
