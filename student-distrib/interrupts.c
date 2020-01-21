/* This file will implement the interrupts and exceptions in the interrupt table. */
#include "interrupts.h"
#include "lib.h"

/*
 * This section of code implements the 32 intel-defined exceptions at the 
 * beginning of the IDT.
 */
uint32_t exception_handler(){
	while(1){
		puts("Made it to the exception handler\n");
	};
	return 0;
}

uint32_t reserved_error(){
	puts("RESERVED INTERRUPT CALLED!!!\n");
	while(1){}
	return -1;
}

uint32_t divide_error(){
	puts("Division error\n");
	while(1){}
	return -1;
}

uint32_t nonmask_interrupt(){
	puts("Nonmaskable interrupt\n");
	while(1){}
	return -1;
}

uint32_t breakpoint(){
	puts("Breakpoint reached\n");
	while(1){}
	return -1;
}

uint32_t overflow(){
	puts("OVERFLOW error!\n");
	while(1){}
	return -1;
}

uint32_t bound_range_exceeded(){
	puts("Index out of bounds!\n");
	while(1){}
	return -1;
}

uint32_t invalid_opcode(){
	puts("Invalid opcode\n");
	while(1){}
	return -1;
}

uint32_t device_unavailable(){
	puts("Device unavailable\n");
	while(1){}
	return -1;
}

uint32_t double_fault(){
	puts("Double fault\n");
	while(1){}
	return -1;
}

uint32_t coprocessor_segment_overrun(){
	puts("coprocessor segment overrun\n");
	while(1){}
	return -1;
}

uint32_t invalid_tss(){
	puts("Invalid tss\n");
	while(1){}
	return -1;
}

uint32_t segment_not_present(){
	puts("Segment not present\n");
	while(1){}
	return -1;
}

uint32_t stack_segment_fault(){
	puts("Stack segment fault\n");
	while(1){}
	return -1;
}

uint32_t general_protection(){
	puts("General protection exception\n");
	while(1){}
	return -1;
}

uint32_t page_fault(){
	puts("PAGE FAULT\n");
	while(1){}
	return -1;
}

uint32_t fpu_floating_point_error(){
	puts("FPU floating point error\n");
	while(1){}
	return -1;
}

uint32_t alignment_check(){
	puts("Alignment check exception\n");
	while(1){}
	return -1;
}

uint32_t machine_check(){
	puts("Machine check exception");
	while(1){}
	return -1;
}

uint32_t simd_floating_point_exception(){
	puts("SIMD floating point exception");
	while(1){}
	return -1;
}
