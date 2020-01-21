#ifndef _TASK_SWITCH_H
#define _TASK_SWITCH_H

#include "x86_desc.h"
#include "lib.h"
#include "idt.h"
#include "i8259.h"
#include "syscalls.h"
#include "terminal.h"
#include "page.h"

#define PIT_DATA	0x43
#define NUM_TERMS	3

typedef struct{
	int32_t esp;		// ESP for this process.
	int32_t ebp;		// EBP for this process.
	int32_t ote_mb;		// Program data mapping for this process.
} sched_t;

extern void pit_linker();

/* Runs an "independent" shell on the terminal "shell_num." */
void run_shell(int32_t shell_num, uint8_t active_term);

/* Performs PIT initialization. */
void init_pit();

/* Interrupt handler. */
void pit_handler();
#endif
