/*
 * This file will contain all functions relating to the IDT.
 */
#include "idt.h"

/* void page_fault_test(); */

/* This function will initialize the first intel-defined 32 exceptions in the IDT with the
 * appropriate interrupt vectors for the system calls we must implement.
 */
void idt_init(){
	//SET_IDT_ENTRY(str, handler)
	// Array of function pointers for 20 (really 18) non-reserved exceptions.
	void* intFuncArr[20] = {divide_error,
							reserved_error,
							nonmask_interrupt, 
							breakpoint, 
							overflow,
							bound_range_exceeded,
							invalid_opcode,
							device_unavailable,
							double_fault,
							coprocessor_segment_overrun,
							invalid_tss,
							segment_not_present,
							stack_segment_fault,
							general_protection,
							page_fault,
							reserved_error,
							fpu_floating_point_error,
							alignment_check,
							machine_check,
							simd_floating_point_exception};
	
	/* 
	 * Set all of the interrupt vectors for the intel-defined exceptions to
	 * point to our exception handler. 
	 */
	int idtIt;				// Iterator over the interrupt vectors.
	int numExcept = 32;		// Number of intel-defined exceptions.
	for(idtIt = 0; idtIt < numExcept; idtIt++){
		idt[idtIt].size = 0x1;			// This is a 32-bit gate.
		idt[idtIt].seg_selector = KERNEL_CS;
		idt[idtIt].reserved1 = 0x1;		// Set these reserved bits to signal to the IDT that this is an interrupt.
		idt[idtIt].reserved2 = 0x1;
		
		if(idtIt < 20){ //20 (really 18) non-reserved exceptions.
			SET_IDT_ENTRY(idt[idtIt], intFuncArr[idtIt]);
		}
		else{
			SET_IDT_ENTRY(idt[idtIt], &reserved_error);
		}
		idt[idtIt].present = 0x1;		// Mark the interrupt as present.		
	}
	
	/* Set INT 0x80 to handle system calls. */
	idt[0x80].dpl = 3;					// Make the system calls user-accessible.
	idt[0x80].size = 0x1;				// This is a 32-bit gate.
	idt[0x80].seg_selector = KERNEL_CS;
	idt[0x80].reserved1 = 0x1;
	idt[0x80].reserved2 = 0x1;
	idt[0x80].reserved3 = 0x1;			// Set up a TRAP gate.
	SET_IDT_ENTRY(idt[0x80], &syscall_linker);
	idt[0x80].present = 0x1;		// Mark the interrupt as present.		
}
