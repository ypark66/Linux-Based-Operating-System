/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
uint8_t master_connected, slave_connected;		// High means unconnected, low means pin is connected.

/* Mask all interrupts. */
void mask_all(void){
	// Mask interrupts on both PICs.
	outb(0xFF, MASTER_8259_DATA);
	outb(0xFF, SLAVE_8259_DATA);
}

/* 
 * Unmask the specific interrupt line that is requested.
 * We should only be unmasking lines that are actually connected.
 */
void unmask_irq_line(uint8_t irq_line){
	// Clear the global masks.
	master_mask = 0x00;
	slave_mask = 0x00;
	
	// Determine which PIC is requesting an opening.
	if(irq_line > 7){
		irq_line -= 8;
		slave_mask = 1 << irq_line;
		slave_connected = slave_connected & ~slave_mask;		// This will register this irq_line as connected.
		outb(slave_connected, SLAVE_8259_DATA);
	}
	else{
		master_mask = 1 << irq_line;
		master_connected = master_connected & ~master_mask;
		outb(master_connected, MASTER_8259_DATA);
	}
}

/* Initialize the 8259 PIC */
void i8259_init(void) {
	// Initialize all pins to be unconnected.
	master_connected = 0xFF;
	slave_connected = 0xFF;
	
	//Use the four CWs to initialize the PICs.
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_DATA);
	outb(ICW3_MASTER, MASTER_8259_DATA);
	outb(ICW4, MASTER_8259_DATA);
	
	outb(ICW1, SLAVE_8259_PORT);	
	outb(ICW2_SLAVE, SLAVE_8259_DATA);
	outb(ICW3_SLAVE, SLAVE_8259_DATA);
	outb(ICW4, SLAVE_8259_DATA);

	/***** UNCOMMENT THIS TO SHOW GPE *****/
	mask_all();			// Masks all PIC interrupts (since nothing is connected).
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	uint8_t orig_mask;
	
	// Determine which PIC we are looking at.
	if(irq_num > 7){
		// Fetch the original mask and set up the new slave mask.
		irq_num -= 8;
		slave_mask = 0x00;
		orig_mask = inb(SLAVE_8259_DATA);
		
		// Put our IRQ line into the slave mask and unmask it.
		slave_mask = ~(1 << irq_num);
		outb(orig_mask & slave_mask, SLAVE_8259_DATA);
	}
	else{
		master_mask = 0x00;
		orig_mask = inb(MASTER_8259_DATA);
		
		// Pur our IRQ line into the master mask and unmask it.
		master_mask = ~(1 << irq_num);
		outb(orig_mask & master_mask, MASTER_8259_DATA);
	}
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
	uint8_t orig_mask;
	
	// Determine which PIC we are looking at.
	if(irq_num > 7){
		// Fetch the original mask and set up the new slave mask.
		irq_num -= 8;
		slave_mask = 0x00;
		orig_mask = inb(SLAVE_8259_DATA);
		
		// Put our IRQ line into the slave mask and mask it.
		slave_mask = 1 << irq_num;
		outb(orig_mask | slave_mask, SLAVE_8259_DATA);
	}
	else{
		master_mask = 0x00;
		orig_mask = inb(MASTER_8259_DATA);
		
		// Pur our IRQ line into the master mask and mask it.
		master_mask = 1 << irq_num;
		outb(orig_mask | master_mask, MASTER_8259_DATA);
	}
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	// If the irq number is from the slave...
	if(irq_num > 7){
		outb((irq_num - 8) | EOI, SLAVE_8259_PORT);	// Send EOI to slave.
		outb(2 | EOI, MASTER_8259_PORT);			// Send EOI to the master. Remember that PIC slave is on IRQ2.
	}
	else{
		outb(irq_num | EOI, MASTER_8259_PORT);
	}
}
