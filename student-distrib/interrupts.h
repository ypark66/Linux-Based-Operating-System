#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

/* This file will handle instantiation of the intel exceptions */
#include "types.h"

/* Keyboard interrupt handler. */
void keyboard_handler();

/* The exception handler will perform the necessary assembly linkage before
 * continuting on ot the called exception. */
uint32_t exception_handler();

uint32_t reserved_error();

uint32_t divide_error();

uint32_t nonmask_interrupt();

uint32_t breakpoint();

uint32_t overflow();

uint32_t bound_range_exceeded();

uint32_t invalid_opcode();

uint32_t device_unavailable();

uint32_t double_fault();

uint32_t coprocessor_segment_overrun();

uint32_t invalid_tss();

uint32_t segment_not_present();

uint32_t stack_segment_fault();

uint32_t general_protection();

uint32_t page_fault();

uint32_t fpu_floating_point_error();

uint32_t alignment_check();

uint32_t machine_check();

uint32_t simd_floating_point_exception();

// Array of function pointers.
/*void* intFuncArr[20] = {&divide_error,
						&reserved_error,
						&nonmask_interrupt, 
						&breakpoint, 
						&overflow,
						&bound_range_exceeded,
						&invalid_opcode,
						&device_unavailable,
						&double_fault,
						&coprocessor_segment_overrun,
						&invalid_tss,
						&segment_not_present,
						&stack_segment_fault,
						&general_protection,
						&page_fault,
						&reserved_error,
						&fpu_floating_point_error,
						&alignment_check,
						&machine_check,
						&simd_floating_point_exception};*/
						
#endif
