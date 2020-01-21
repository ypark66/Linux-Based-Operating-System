#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "lib.h"
#include "i8259.h"
#include "idt.h"
#include "x86_desc.h"
#include "terminal.h"
#include "rtc.h"
#include "page.h"

#ifndef ASM
extern void kb_linker();

/* Initialize the IDT entry for the keyboard. */
void keyboard_init();
void init_term_cursor();
uint32_t get_term_cursor_x(uint32_t index);
uint32_t get_term_cursor_y(uint32_t index);
void set_term_cursor_x(uint32_t index, uint32_t x);
void set_term_cursor_y(uint32_t index, uint32_t y);

/* Handler for keyboard interrupt. */
extern void keyboard_handler();
#endif	/* ASM */

#endif	/* KEYBOARD_H */
