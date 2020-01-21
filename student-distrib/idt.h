#ifndef _IDT_H
#define _IDT_H

#include "types.h"
#include "x86_desc.h"
#include "interrupts.h"
#include "syscalls.h"

void idt_init();

#endif
