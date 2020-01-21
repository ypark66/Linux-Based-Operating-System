/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20
#define MASTER_8259_DATA	0x21
#define SLAVE_8259_PORT     0xA0
#define SLAVE_8259_DATA		0xA1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

//int irqCtr[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	// Holds the number of currently running interrupts with a particular IRQ number.
//uint8_t maskSet[8] = {0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00};	// Use the IRQ number as an index to this array to retrieve its mask.

/* Externally-visible functions */
/* Mask all PIC interrupts */
void mask_all(void);

/* Unmask all PIC interrupts */
void unmask_irq_line(uint8_t irq_line);

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
