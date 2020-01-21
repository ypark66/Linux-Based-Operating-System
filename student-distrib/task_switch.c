#include "task_switch.h"

/* Globals for shell setup. */
uint8_t shells_running[NUM_TERMS] = {1, 0, 0};
uint8_t sched_arr_init = 0;

/* Array of scheduler structs for keeping track of important data. */
sched_t sched_arr[NUM_TERMS];
sched_t temp_sched;

/*
 * This function will initialize the PIT to perform an interrupt every 10-50 ms.
 */
void init_pit(){
	outb(0x00, PIT_DATA);	// 0x00 sets the default values.
	
	/* Create the PIT's entry in the IDT. */
	uint8_t idtPort = 0x20;						//This is the location that the PIT interrupt descriptor occupies in the IDT.
	idt[idtPort].size = 0x1;					// This is a 32-bit gate.
	idt[idtPort].seg_selector = KERNEL_CS;
	idt[idtPort].reserved1 = 0x1;				// Set these reserved bits to signal to the IDT that this is an interrupt.
	idt[idtPort].reserved2 = 0x1;
	SET_IDT_ENTRY(idt[idtPort], pit_linker);	
	idt[idtPort].present = 0x1;					//Mark the interrupt as present.
	
	/* Tell the user it worked. */
	puts("PIT initialized.\n");
}

/*
 * This is a helper function that will generate the next process number to be
 * scheduled.
 */
int32_t get_next_proc(int32_t cur_term, int32_t* proc_arr){
<<<<<<< HEAD
	int32_t next_proc_num = (cur_term + 1) % MAX_PROCESSES;
=======
	int32_t next_proc_num = (cur_term + 1) % NUM_TERMS;
>>>>>>> f21547c05921ca98dc9045a0d0dceb879c0241b3

	/* 
	 * Check to see if the current terminal/shell has a child process. 
	 */
	int proc_it; // Generic iterator.
	pcb_t* child_pcb = NULL;
<<<<<<< HEAD
	for(proc_it = next_proc_num; proc_it < MAX_PROCESSES; proc_it++){
=======
	for(proc_it = NUM_TERMS; proc_it < MAX_PROCESSES; proc_it++){ // Start at 3 since there are 3 shells.
>>>>>>> f21547c05921ca98dc9045a0d0dceb879c0241b3
		if(proc_arr[proc_it] != FREE){
			child_pcb = get_pcb_loc(proc_it);
			if(child_pcb -> term_number == next_proc_num){
				next_proc_num = proc_it;
			}
		}
	}
	return next_proc_num;
}
/*
 * This will set the PCB data of each shell to be similar to shell 0's 
 * PCB to give the illusion of independence.
 */
void init_shell_pcbs(){
	/* Edit the PCB data. */
	sched_arr_init = 1;
	pcb_t* base_pcb = get_pcb_loc(0);			// Get shell 0's PCB.
	int32_t shell_num;
	
	for(shell_num = 1; shell_num < NUM_TERMS; shell_num++){ // We only need to init shell 1 and 2.
		pcb_t* new_pcb = get_pcb_loc(shell_num);	// Get the new shell's PCB.
		
		/* Make changes. */
		new_pcb -> parent_pid = base_pcb -> parent_pid;
		new_pcb -> parent_phys_addr = base_pcb -> parent_phys_addr;
		new_pcb -> parent_esp = base_pcb -> parent_esp;
		new_pcb -> parent_ebp = base_pcb -> parent_ebp;
		new_pcb -> term_number = shell_num;
	}
}

/* 
 * This is a small helper function that will run a shell on a terminal matching
 * its shell number (shell 0 runs on terminal 0, etc.).  
 */
void run_shell(int32_t shell_num, uint8_t active_term){	
	send_eoi(0);
	
	/* Make sure this shell is not running. */
	if(shells_running[shell_num]){
		return;
	}
	
	/* Execute the shell. */
	sti();								// Make sure interrupts are enabled.
	shells_running[shell_num] = 1; 		// Mark current shell as running.
	sys_execute((uint8_t*)"shell");	
}

/* 
 * The PIT handler will take care of task switching and all associated manipulations
 * of VGA mappings and stack pointers.
 */
void pit_handler(){
	/***** SAVE PROCESS STATE *****/
	asm volatile(
		"movl	%%ebp, %0;"
		"movl	%%esp, %1;"
		:"=r"(temp_sched.ebp),
		"=r"(temp_sched.esp)
		:
	);
	temp_sched.ote_mb = page_directory[OTE_MB/FOUR_MB];		// Save the paging data for this program.

	/***** CHECK WHICH TASK TO SWITCH TO *****/
	int32_t proc_num = get_process_number();
	int32_t* proc_arr = get_proc_arr();
	int32_t curr_term = get_pcb_loc(proc_num) -> term_number;
	int32_t next_proc_num = get_next_proc(curr_term, proc_arr);
	// int32_t next_proc_num = proc_num;
	
	/* Go through all of the processes and see which one should be serviced next. */
	// int it;	// Generic iterator.
	// for(it = 0; it < MAX_PROCESSES; it++){
		// proc_num = (proc_num + 1) % MAX_PROCESSES;
		// /* Break on the first process number we find that is not free and is not in the same terminal. */
		// if(proc_arr[proc_num] != FREE && proc_num != curr_term){
			// next_proc_num = proc_num;
			// break;
		// }
	// }
	
	// /* 
	 // * If we are attempting to service a shell, we know that
	 // * its terminal number is the same as its index in the process
	 // * array. Check process array indices 3-5 to see if the shell has a
	 // * child process running via checking for a matching terminal number. 
	 // */
	// if(next_proc_num < 3){	// Indices 0, 1, and 2 of the proc_arr are for shells.
		// int proc_it; // Generic iterator.
		// pcb_t* child_pcb = NULL;
		// for(proc_it = 3; proc_it < MAX_PROCESSES; proc_it++){
			// if(proc_arr[proc_it] != FREE){
				// child_pcb = get_pcb_loc(proc_it);
				// if(child_pcb -> term_number == next_proc_num){
					// next_proc_num = proc_it;
					// break;
				// }
			// }
		// }
	// }
	
	/***** CHECK IF WE NEED TO REMAP VGA. *****/
	//uint8_t active_term = get_cur_term();
	uint8_t new_term = 0;
	if(next_proc_num < NUM_TERMS){
		new_term = next_proc_num;
	}
	else{
		pcb_t* next_pcb = get_pcb_loc(next_proc_num);
		new_term = next_pcb -> term_number;
	}
	set_term_in_service(new_term);					// Mark the next terminal as "in service."
	
	/* Remap the user pointer. */
	if(get_cur_term() != new_term){
		page_table_vid[0] = (VIM_MEM_INDEX + (new_term + 1) * FOUR_KB) | RW | PRESENT | USER;
	}
	else{
		page_table_vid[0] = VIM_MEM_INDEX | RW | PRESENT | USER;
	}
	
	/***** CHANGE THE TSS, EPB, AND ESP ******/	
	if(shells_running[1] == 0){ 		// If shell 1 is not running...
		sched_arr[0] = temp_sched;		// Then the data we have is for shell 0.
		page_table_vid[0] = (VIM_MEM_INDEX + (1 + 1) * FOUR_KB) | RW | PRESENT | USER;  //TERMINAL ONE + INDEX(1)
		set_screen_coords(0,0);
		set_term_in_service(1);
		run_shell(1, get_cur_term());
	}
	else if(shells_running[2] == 0){ 	// If shell 2 is not running...
		sched_arr[1] = temp_sched;		// Then the data we have is for shell 1.
		page_table_vid[0] = (VIM_MEM_INDEX + (2 + 1) * FOUR_KB) | RW | PRESENT | USER; //TERMINAL TWO + INDEX(1)
		set_screen_coords(0,0);
		set_term_in_service(2);
		run_shell(2, get_cur_term());
	}
	else{
		if(sched_arr_init == 0){
			init_shell_pcbs();			// Set the PCBs of each shell to seem independent.
			sched_arr[2] = temp_sched;	// If we have not yet initialized the shell PCBs, we need to manually put this data into the scheduling structure.
		}
		else{
			sched_arr[curr_term] = temp_sched;	// This statement only works AFTER initialization of all shells and PCBs.
		}
	}
	
	/* 
	 * The following stack address corresponds to the BOTTOM of the 8 kB kernel
	 * stack allocated for this process.
	 */
	tss.esp0 = KERNEL_LOC + FOUR_MB - (EIGHT_KB * next_proc_num) - 1;
	tss.ss0 = KERNEL_DS;	
	page_directory[OTE_MB/FOUR_MB] = sched_arr[new_term].ote_mb;
	set_term_in_service(new_term);
	send_eoi(0);				// Send an EOI for interrupt 0.
	asm volatile(
		"movl	%%cr3, %%eax;"
		"movl	%%eax, %%cr3;"
		"movl	%0, %%ebp;"
		"movl	%1, %%esp;"
		:
		:"r"(sched_arr[new_term].ebp),
		"r"(sched_arr[new_term].esp)
		:"eax"
	);
}
