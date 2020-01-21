# x86_desc.S - Set up x86 segment descriptors, descriptor tables
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"
#define KERNEL_STACK 0x0018
#define USER_STACK 0x002B

.text

.globl ldt_size, tss_size
.globl gdt_desc, ldt_desc, tss_desc
.globl tss, tss_desc_ptr, ldt, ldt_desc_ptr
.globl gdt_ptr
.globl idt_desc_ptr, idt
# My globals.
.globl kb_linker, rtc_linker, pit_linker

.align 4


tss_size:
    .long tss_bottom - tss - 1

ldt_size:
    .long ldt_bottom - ldt - 1

    .word 0 # Padding
ldt_desc:
    .word KERNEL_LDT
    .long ldt

    .align 4
		
tss:
_tss:
    .rept 104
    .byte 0
    .endr
tss_bottom:

    .align  16
gdt:
_gdt:

    # First GDT entry cannot be used
    .quad 0

    # NULL entry
    .quad 0

    # Segmentation will not be used
    # CS and DS both are 0-4GB r/w segments
    #
    # The layout is (from Intel IA-32 reference manual):
    #  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
    # |----------------------------------------------------------------------|
    # |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
    # | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
    # |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
    # |----------------------------------------------------------------------|
    #
    # |----------------------------------------------------------------------|
    # |                                    |                                 |
    # | Base 15:0                          | Segment Limit 15:0              |
    # |                                    |                                 |
    # |----------------------------------------------------------------------|

gdt_ptr:
    # Set up an entry for kernel CS
    .quad 0x00CF9A000000FFFF

    # Set up an entry for kernel DS
    .quad 0x00CF92000000FFFF

    # Set up an entry for user CS
    .quad 0x00CFFA000000FFFF

    # Set up an entry for user DS
    .quad 0x00CFF2000000FFFF

    # Set up an entry for TSS
tss_desc_ptr:
    .quad 0

    # Set up one LDT
ldt_desc_ptr:
    .quad 0

gdt_bottom:
	.align 16
ldt:
    .rept 4
    .quad 0
    .endr
ldt_bottom:

.align 4
    .word 0 # Padding
idt_desc_ptr:
    .word idt_bottom - idt - 1
    .long idt


    .align  16
idt:
_idt:
    .rept NUM_VEC
    .quad 0
    .endr
idt_bottom:

gdt_desc:
	.word gdt_bottom - gdt - 1
	.long gdt
	
	.align 16

kb_linker:
	pushal
	call 	keyboard_handler
	popal
	iret
	
rtc_linker:
	pushal
	call 	rtc_handler
	popal
	iret

pit_linker:
	pushal
	call 	pit_handler
	popal
	iret
	
.globl syscall_linker

# This is the assembly linkage for generic system calls.
syscall_linker:
	# Check to see if EAX is between 1 and 10 (bound inclusive).
    pushf
    pushl %esi
    pushl %edi
    pushl %ebp

	cmpl	$0x01, %eax
	jl		syscall_fail
	cmpl	$0x0A, %eax
	jg		syscall_fail
	
    # movw $KERNEL_STACK , %dx
    # movw %dx, %ds
    # movw %dx, %es
    # movw %dx, %fs    
    # movw %dx, %gs
	# If the argument is in range, push args and make the system call.
	pushl	%edx
	pushl	%ecx
	pushl	%ebx    
	call	*syscall_table(, %eax, 4)	# Call the appropriate syscall based on EAX.
    # movw $USER_STACK, %dx
    # movw %dx, %ds
    # movw %dx, %es
    # movw %dx, %fs    
    # movw %dx, %gs


	jmp		syscall_done	# We are done now.
	
syscall_fail:
	movl	$0xFFFFFFFF, %eax			# Return -1
    jmp fail_ret

syscall_done:
	popl %ebx
    popl %ecx
    popl %edx

fail_ret:
    popl %ebp
    popl %edi
    popl %esi
    popf
    iret
	
# These are the function pointers to the system calls.
syscall_table:
	.long 0, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn

# .global page_fault_test
# page_fault_test:
	# addl $4, %esp
	# iret


