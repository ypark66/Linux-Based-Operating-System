/* 
 * NOTE: I use the term "Printable Character" throughout this doc. A printable character
 * is any character which will be written to the screen. NOT stuff like CAPSLOCK, CTRL
 * or SHIFT. 
 */

/* This file will hold all functions relating to the keyboard. */
#include "keyboard.h"
#include "terminal.h"
#define BUFSIZE		128		// Maximum size of keyboard buffer.

/* Define special scancodes pertaining to particular keys */
#define SHIFT		128		// Both shifts are assigned this as their "character."
#define CAPSLOCK	129
#define BACKSPACE	'\b'
#define CTRL		130
#define ENTER		'\n'
#define F1			131
#define F2			132
#define F3			133
#define ALT			134

/* Keyboard buffers for each terminal. */
unsigned char term_bufs[3][BUFSIZE];
uint32_t term_cursor_x[3] = {0};		// Holds current cursor coordinates.
uint32_t term_cursor_y[3] = {0};

/* Useful globals for knowing if a key is pressed. */
uint8_t shift_pressed = 0;		// "boolean" for telling if shift is pressed.
uint8_t capslock_pressed = 0;	// "boolean" for telling if capslock is pressed.
uint8_t ctrl_pressed = 0;		// "boolean" for telling if ctrl is pressed.
uint8_t alt_pressed = 0;		// "boolean" for telling us if user is holding halt.



/* 
 * Initialie all value of cursors.
 */
void init_term_cursor(){
	int i;
	for(i = 0; i<3;i++){
		term_cursor_x[i] = 0;
		term_cursor_y[i] = 0;
	}

}

/* 
 * get cursor of current running terminal
 */
uint32_t get_term_cursor_x(uint32_t index){
	return term_cursor_x[index];
}


/* 
 * get cursor of current running terminal
 */
uint32_t get_term_cursor_y(uint32_t index){
	return term_cursor_y[index];
}

/* 
 * set cursor of current running terminal
 */
void set_term_cursor_x(uint32_t index, uint32_t x){
	term_cursor_x[index] = x;	
}

/* 
 * set cursor of current running terminal
 */
void set_term_cursor_y(uint32_t index, uint32_t y){
	term_cursor_y[index] = y;
}

/* 
 * This boolean tells us which characters to push to the buffer.
 * For example, we shouldn't push SHIFT into the buffer because nothing
 * should display on a SHIFT press.
 */
uint8_t print_me = 0;

/* Keyboard buffer variables. */
unsigned char kbd_buf[BUFSIZE];		// Buffer to write to the terminal.
uint8_t kbd_buf_idx = 0;			// Index of our spot in the buffer.

/* 
 * This variable tells us what VGA line and col we started typing on. Used
 * to make sure backspace can jump up lines when it needs to and that it
 * wont delete anything that the terminal prints onto the line.
 */
unsigned int x_start = 0;
unsigned int y_start = 0;

/* RTC debugging vars. Array contains all of the possible frequencies in Hz. */
int freqs[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
int freq_idx = 0;
int num_freqs = 10;

/* FROM OSDEVER.NET */
/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    130,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   128,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   128,				/* Right shift */
  '*',
   134,	/* Alt */
  ' ',	/* Space bar */
  129,	/* Caps lock */
    131,	/* 59 - F1 key ... > */
    132,   133,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

/******************************/
/***** KEYBOARD FUNCTIONS *****/
/******************************/
/* Flush the keyboard buffer so that it won't contain old characters. */
void flush_kbd_buf(){
	memset(kbd_buf, 0, BUFSIZE);
	kbd_buf_idx = 0;				// Reset index so we write from the beginning.
}



/* Initialize the keyboard so we can read from it. */
void keyboard_init(){
	uint32_t idtPort = 0x21;		// This is the location that the keyboard interrupt descriptor occupies in the IDT.
	
	// Set the keyboard interrupt vector.
	idt[idtPort].size = 0x1;			// This is a 32-bit gate.
	idt[idtPort].seg_selector = KERNEL_CS;
	idt[idtPort].reserved1 = 0x1;		// Set these reserved bits to signal to the IDT that this is an interrupt.
	idt[idtPort].reserved2 = 0x1;
	SET_IDT_ENTRY(idt[(int)idtPort], kb_linker);
	idt[idtPort].present = 0x1;		// Mark the interrupt as present.
}

/*
 * This function is the interrupt handler for the keyboard that will determine
 * which character to put into the keyboard buffer according to currently pressed
 * keys. This handler also deals with the CTRL-L command to clear the screen and
 * reset the cursor position.
 */
void keyboard_handler(){
	/* 
	 * Temporarily change the video mapping just for the execution of this interrupt.
	 * We want to make sure that we always print to actual video memory for this.
	 */
	int32_t temp_vidmap = page_table_vid[0];				// Save the current mapping.
	page_table_vid[0] = VIM_MEM_INDEX | RW | PRESENT | USER;	// Change the mapping for this print.
	uint8_t cur_term_num = get_cur_term();
	uint8_t term_save = get_term_in_service();
	set_term_in_service(cur_term_num);

	/* 
	 * Grab the terminal write x_start and y_start values if terminal write
	 * was just called.
	 */
	if(get_override()){
		x_start = get_x_start_tw();
		y_start = get_y_start_tw();
		set_override(0);
	}
	
	uint32_t irq_num = 1;	// Keyboard is IRQ1.
	disable_irq(irq_num);	// Stop nested interrupts from the keyboard from occurring.
	
	uint8_t scancode = inb(0x60);		// 0x60 is the keyboard data port.
	uint8_t scan_char;					// Will hold the character corresponding to this scancode.
	
	/* Check if the key has just been released: */
	if(scancode & 0x80){	// 0x80 means the highest bit is high, which is how the AT keyboard signals a key release.
		uint8_t mask = 0x7F;					// This mask (0111 1111) will remove the most significant bit to retrieve the original scancode.
		scan_char = kbdus[scancode & mask];		// Retrieve the character.
		
		/* Check for a release of the shift, ctrl, or alt keys: */
		if(scan_char == SHIFT){
			shift_pressed = 0;
		}
		else if(scan_char == CTRL){
			ctrl_pressed = 0;
		}
		else if(scan_char == ALT){
			alt_pressed = 0;
		}
	}
	/* Otherwise the key has been pressed for the first time. */
	else{
		scan_char = kbdus[scancode];	// Fetch the current character.
		
		/* Check if the key pressed corresponds to a printable character. */
		if(scan_char >= 0x20 && scan_char < 0x7F && ctrl_pressed == 0){ // Printable characters.
			print_me = 1;
		}
		else{
			print_me = 0;
		}
		
		/* Check to see if the user is currently pressing shift, capslock, or CTRL. */
		if(scan_char == SHIFT){
			shift_pressed = 1;
		}
		else if(scan_char == CAPSLOCK){
			capslock_pressed = !capslock_pressed;	// CAPSLOCK is a special case since it isn't held down by the user.
		}
		else if(scan_char == CTRL){
			ctrl_pressed = 1;
		}
		else if(scan_char == ALT){
			alt_pressed = 1;
		}
		
		/* Test for ALT-Fx, x = 1, 2, 3. */
		if(alt_pressed == 1){
			/* Make sure that the user is pressing a key we care about. */
			if(scan_char == F1 || scan_char == F2 || scan_char == F3){
								// Currently displayed terminal.
				uint8_t new_term = 0;								// Requested terminal.
				
				/*
				 * Save the terminal screen and current cursor position
				 * and update the current terminal number.
				 */
				switch(scan_char){
					case F1:
						new_term = 0;
						break;
					case F2:
						new_term = 1;
						break;
					case F3:
						new_term = 2;
						break;
				}
				
				/* Only switch terminals if there is a match. */
				if(cur_term_num != new_term){
					/* Get memory indices. */
					int32_t* vidmem = (int32_t*)VIM_MEM_INDEX;
					int32_t* cur_term_mem = (int32_t*)(ONE_GIG + FOUR_KB * (cur_term_num + 1));	// Index into the vidmem page table.
					int32_t* new_term_mem = (int32_t*)(ONE_GIG + FOUR_KB * (new_term + 1));
					
					/* Perform the saving and swapping. */
					memcpy(cur_term_mem, vidmem, FOUR_KB);									// Save current display.
					memcpy(vidmem, new_term_mem, FOUR_KB);									// Show new terminal's display.
					memcpy(term_bufs[cur_term_num], kbd_buf, BUFSIZE);						// Save the current buffer.
					memcpy(kbd_buf, term_bufs[new_term], BUFSIZE);							// Load new buffer.
					set_screen_coords(term_cursor_x[new_term], term_cursor_y[new_term]);	// Set new screen coordinates for writing.
					update_cursor(term_cursor_x[new_term], term_cursor_y[new_term]);		// Set new cursor location.
					set_cur_term(new_term);
				}
			}
		}
		
		/* Test for CTRL-L */
		if(ctrl_pressed == 1){
			switch(scan_char){
				case 'l':	// This one is lowercase 'L'
					/* clear the screen and move the cursor to the top. */
					clear();					// Function from lib.h that clears the VGA.
					update_cursor(0, 0);		// Reset the cursor to (0, 0).
					set_screen_coords(0, 0);	// Reset the lib.c variables that control where in VGA memory we write.
					term_cursor_x[cur_term_num] = get_screen_x();							// Save old screen_x.
					term_cursor_y[cur_term_num] = get_screen_y();					// Save old screen_y.
					// x_start = 0;	// Adjust the backspace parameters.
					// y_start = 0;
					break;
				/* CRTL-ONE is used for RTC read/write testing. */
				case '1':	// This one is the number ONE (#1).
					clear();
					update_cursor(0, 0);
					set_screen_coords(0, 0);
					term_cursor_x[cur_term_num] = get_screen_x();							// Save old screen_x.
					term_cursor_y[cur_term_num] = get_screen_y();							// Save old screen_y.
					freq_idx++;
					/* If we've exceeded the dimensions of the array, reset.*/
					if(freq_idx == num_freqs){
						freq_idx = 0;
					}
					printf("Testing freq: %d\n", freqs[freq_idx]);
					void* buf;	// Empty buffer.
					rtc_write(0, buf, freqs[freq_idx]);
					break;
				/* This will close and then open RTC, which should reset the freq to 2 Hz.*/
				case '2':
					puts("\nResetting RTC!\n");
					rtc_close();
					rtc_open();
			}
		}
		
		/* Test for backspace. */
		if(scan_char == BACKSPACE){
			/* Move the text cursor back one place and delete the character there.*/
			int curr_x = get_screen_x();	// Fetch current x position.
			int curr_y = get_screen_y();	// Fetch current y position.
			
			if(curr_x == 0 && curr_y == 0){
				// Do nothing. Special case.
			}
			else if(curr_x != x_start){
				curr_x -= 1;
			}
			else{
				/* 
				 * Backspace may go onto the line above if that's the line
				 * we started typing on.
				 */
				if(curr_y > y_start){
					curr_x = VGA_WIDTH;
					curr_y -= 1;
				}
			}
			
			/* 
			 * Set the cursor and screen coordinates to the same position
			 * and "delete" the character we backspaced.
			 */
			update_cursor(curr_x, curr_y);
			set_screen_coords(curr_x, curr_y);
			term_cursor_x[cur_term_num] = get_screen_x();							// Save old screen_x.
			term_cursor_y[cur_term_num] = get_screen_y();							// Save old screen_y.
			clear_char();
			kbd_buf[kbd_buf_idx--] = 0;			// Remove that character from the buffer as well.
		}
		
		/* Test for enter. */
		if(scan_char == ENTER){
			/* 
			 * Now that the user has pressed enter (finished input), 
			 * read what was written. 
			 */
			send_buffer(kbd_buf);
			flush_kbd_buf();						// Clear the buffer.
			 
			/* 
			 * Fetch the y coordinate and see if we need to
			 * scroll or actually move the cursor down. 
			 */
			int curr_y = term_cursor_y[cur_term_num];
			if(curr_y == VGA_HEIGHT - 1){
				/* Scroll. See function header for better explanation. */
				scroll();
			}
			else{
				curr_y += 1;	// Move y index on screen down.
			}
			
			/* Move the text cursor down one row and reset x. */
			set_screen_coords(0, curr_y);
			update_cursor(0, curr_y);
			term_cursor_x[cur_term_num] = get_screen_x();							// Save old screen_x.
			term_cursor_y[cur_term_num] = get_screen_y();							// Save old screen_y.
			x_start = 0;
			y_start = curr_y;			// Update the new starting line.
		}
		
		/* 
		 * Handle the case when either capslock or shift is pressed.
		 * This means we need to display the alternate character for
		 * this key. 
		 */
		if(shift_pressed ^ capslock_pressed){
			// The alphabetical characters can be adjusted with a simple offset.
			if(scan_char > 0x60 && scan_char < 0x7B){ // Goes from 'a' to 'z'
				scan_char -= 0x20;		// 0x20 is the offset in ASCII between 'a' and 'A'
			}
		}
		
		/* We only care about if shift is pressed for non-alphabetical keys. */
		if(shift_pressed){
			/* A switch statement might not be the cleanest way to handle this,
			 * but the non-alphabetical characters don't have a nice neat offset
			 * between all of the shift and non-shift values. Some do, but since
			 * there are so many exceptions, the code would be just as messy. 
			 */
			switch(scan_char){
				case '`':
					scan_char = '~';
					break;
				case '1':
					scan_char = '!';
					break;
				case '2':
					scan_char = '@';
					break;
				case '3':
					scan_char = '#';
					break;
				case '4':
					scan_char = '$';
					break;
				case '5':
					scan_char = '%';
					break;
				case '6':
					scan_char = '^';
					break;
				case '7':
					scan_char = '&';
					break;
				case '8':
					scan_char = '*';
					break;
				case '9':
					scan_char = '(';
					break;
				case '0':
					scan_char = ')';
					break;
				case '-':
					scan_char = '_';
					break;
				case '=':
					scan_char = '+';
					break;
				case '[':
					scan_char = '{';
					break;
				case ']':
					scan_char = '}';
					break;
				case '\\':
					scan_char = '|';
					break;
				case ';':
					scan_char = ':';
					break;
				case '\'':
					scan_char = '"';
					break;
				case ',':
					scan_char = '<';
					break;
				case '.':
					scan_char = '>';
					break;
				case '/':
					scan_char = '?';
					break;
			}
		}
		
		/* 
		 * If this is a "printable character" then put it into our buffer and
		 * print it on the screen.
		 */
		if(print_me == 1){
			int curr_x = get_screen_x(); 	// Fetch current x position.
			int curr_y = get_screen_y();	// Fetch current y position.
			
			/* If we have reached the x limit of the screen, go to a newline.*/
			if(curr_x == VGA_WIDTH - 1){
				if(curr_y == VGA_HEIGHT - 1){
					scroll();
					
					/* Reset x, but keep y stationary since we scrolled. */
					set_screen_coords(0, curr_y);
					update_cursor(0, curr_y);
					term_cursor_x[cur_term_num] = get_screen_x();							// Save old screen_x.
					term_cursor_y[cur_term_num] = get_screen_y();							// Save old screen_y.
				}
				else{
					/* Move x to the beginning of the line and move down a row. */
					set_screen_coords(0, curr_y + 1);
					update_cursor(0, curr_y + 1);
					term_cursor_x[cur_term_num] = get_screen_x();							// Save old screen_x.
					term_cursor_y[cur_term_num] = get_screen_y();							// Save old screen_y.
				}
			}
			
			/* Make sure that we are pointing to video memory for this print. */
			putc(scan_char);
			
			/* If we have not gone over the 128 character limit... */
			if(kbd_buf_idx > 127){
				flush_kbd_buf();
			}
			kbd_buf[kbd_buf_idx++] = scan_char;
		}
	}
	
	page_table_vid[0] = temp_vidmap;							// Restore the original mapping.
	set_term_in_service(term_save);

	// Send EOI.
	send_eoi(irq_num);
	enable_irq(irq_num);
}
