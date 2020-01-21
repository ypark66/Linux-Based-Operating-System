/*
 * syscalls.c
 * This file will implement the system calls.
 */
#include "syscalls.h"

int32_t proc_arr[MAX_PROCESSES] = {FREE};		// Contains a list of ENUM types that tell us if a certain stack space is free or not.

int32_t empty_function(){
	return -1;
}

//jump tables for keyboard, a file, device (RTC), and directory

fot_t stdin_fot = {&empty_function, &empty_function, &terminal_read, &empty_function};
fot_t stdout_fot = {&empty_function, &empty_function, &empty_function, &terminal_write};
fot_t file_fot = {&fopen, &fclose, &fread, &fwrite};
fot_t rtc_fot = {&rtc_open, &rtc_close, &rtc_read, &rtc_write};
fot_t dir_fot = {&dir_open, &dir_close, &dir_read, &dir_write};

/*
 * This function will return the current process number by using ESP.
 */
int32_t get_process_number(){
	uint32_t older_esp0;
	asm volatile(
		"movl	%%esp, %0;"
		:"=r"(older_esp0)
		:
	);
	
	/* This will automatically "floor" by virtue of how integer division works. */
	int32_t process_number;
	process_number = ((EIGHT_MB - older_esp0) + 1) / EIGHT_KB;	
	return process_number;
}

/* 
 * This function will take the given process number and return the location of
 * that process' PCB.
 */
pcb_t* get_pcb_loc(int32_t process_number){
	uint32_t pcb_loc = KERNEL_LOC + FOUR_MB - (EIGHT_KB * (process_number + 1));	// Calculate the location of this PCB.
	return (pcb_t*)pcb_loc;
}

/* Getter function for proc_arr -- used in scheduler. */
int32_t* get_proc_arr(){
	return proc_arr;
}

/*
 * Halts the process that calls this function.
 */
int32_t sys_halt(uint8_t status){
	int32_t process_number = get_process_number();
	/* Do absolutely nothing if the process number is 0 (the base shell). */
	if(process_number == 0){
		return 0;
	}
	pcb_t* cur_pcb_loc = get_pcb_loc(process_number);
	
	/* Just close everything. */
	int32_t loopCount;
	for(loopCount = 2; loopCount < 8; loopCount++){	//indices 2-7 are the dynamically assigned indices in the file descriptor array
		sys_close(loopCount);
	}
	
	tss.esp0 = KERNEL_LOC + FOUR_MB - (EIGHT_KB * (cur_pcb_loc -> parent_pid)) - 1;
	
	//restore parent paging(the same paging set up in sys_execute)
	uint32_t virt_addr_128mb_idx = OTE_MB / FOUR_MB;
	page_directory[virt_addr_128mb_idx] = cur_pcb_loc -> parent_phys_addr;
	
	/* Flush the TLB by reloading CR3. */
	asm volatile(
		"movl	%%cr3, %%eax;"
		"movl	%%eax, %%cr3;"
		:
		:
		:"eax"
	);
	
	/* Zero extend status. */
	uint32_t zext_status = 0;
	zext_status += status;
	
	/* Denote the current process slot as free. */
	proc_arr[process_number] = FREE;
	
	//jump to execute return(jump to IRET in execute's(or parent's) stack by restoring parent's esp and ebp registers)
	asm volatile(
		"cli;"
		"movl	%0, %%esp;"	// Parent ESP
		"movl	%1, %%ebp;"	// Parent EBP
		
		"movl	%2, %%eax;"
		"leave;"
		"ret;"
		
		// Just for safety. 
		:
		:"r"(cur_pcb_loc -> parent_esp),
		"r"(cur_pcb_loc -> parent_ebp),
		"r"(zext_status)
		:"esp","ebp"
	);
	
	return 0;
}

/*
 * Execute will perform all of the necessary preparations for executing a user-
 * generated system call, including setting up the kernel stack for the process,
 * setting up the process' 4 MB page, parsing the arguments given by the user,
 * and setting up the context switch properly.
 *
 * INPUTS:
 *		command -- 	This is a space-delimited list of arguments (filename comes first)
 *					that gives all of the information necessary to execute the user's
 *					code.
 */ 
int32_t sys_execute(const uint8_t* command){
	int32_t parent_pid = get_process_number();
	
	/* Figure out our current process. */
	uint32_t older_esp0;
	uint32_t older_ss0;
	asm volatile(
		"movl	%%esp, %0;"
		"movl	%%ebp, %1;"
		:"=r"(older_esp0),
		"=r"(older_ss0)
		:
	);
	
	/* Find the next free process stack slot, if one exists. */
	uint32_t proc_it;
	uint32_t process_number = -1;
	for(proc_it = 0; proc_it < MAX_PROCESSES; proc_it++){
		if(proc_arr[proc_it] == FREE){
			process_number = proc_it;
			break;
		}
	}
	
	/* If the process number is still -1, there are not any available slots. */
	if(process_number == -1){
		return -1;
	}
	else{
		proc_arr[process_number] = RUNNING;
	}
	
	/***** PARSE ARGUMENTS *****/
	uint8_t filename[MAX_FILENAME_LENGTH] = {0};	// Filename.
	uint8_t arg[KB_BUF_SIZE_MAX] = {0};						// This is the argument for getargs, it should not be larger than the keyboard buffer (128 bytes).

	/* Parse out the filename we need. */
	uint32_t arg_it = 0;									// Iterator for command.
	while(command[arg_it] != ' ' && command[arg_it] != 0){ // Until we see a space, keep appending characters.
		/* Make sure we don't run into NULL or exceed the max name length. */
		if(arg_it > MAX_FILENAME_LENGTH){
			proc_arr[process_number] = FREE;
			return -1;
		}
		filename[arg_it] = command[arg_it];
		arg_it++;
	}
	
	arg_it++;		// Skip the leading space.
	uint32_t getargs_it = 0;
	while(command[arg_it] != '\0'){
		/* Don't exceed the max length */
		if(getargs_it > KB_BUF_SIZE_MAX - 1){
			proc_arr[process_number] = FREE;
			return -1;
		}
		arg[getargs_it++] = command[arg_it++];
	}
	
	/***** CHECK EXECUTABLE *****/
	uint8_t magic_nums[4] = {0x7F,0x45,0x4C,0x46}; 

	dentry_t dentry;
	if(read_dentry_by_name(filename, &dentry) != 0){
		proc_arr[process_number] = FREE;
		return -1;
	}

	int32_t buf_size = get_file_length_by_name(filename);
	uint8_t file_buf[FILE_BUF_SIZE];			
	
	/* Check to see if the file's header are the magic numbers above. */
	read_data(dentry.inode_num, 0, file_buf, FILE_BUF_SIZE);
	uint32_t exeIt;
	for(exeIt = 0; exeIt < 3; exeIt++){	// Magic number is only 4 bytes long.
		/* We fail if the first byte (the header) is not the magic number array above. */
		if(file_buf[exeIt] != magic_nums[exeIt]){
			proc_arr[process_number] = FREE;
			return -1;
		}
	}
	
	/* Fetch the entry point. */
	uint32_t entry_point = NULL;
	for(exeIt = 0; exeIt < 4; exeIt++){
		entry_point += file_buf[exeIt + ENTRY_PT] << (MAX_FN * exeIt);
	}
	
	/***** SET UP PROGRAM PAGING *****/
	/* 
	 * Link 128 MB virtual address (virt_addr_128mb) to the physical address
	 * given by phys_addr. 
	 */
	uint32_t phys_addr = EIGHT_MB + (process_number * FOUR_MB);		// This variable holds the physical location that we will be writing to.
	uint32_t virt_addr_128mb_idx = OTE_MB / FOUR_MB;				// This is the virtual address corresponding to 128 MB which we have to map to 8 or 12 MB in phys mem.
	uint32_t par_phys_addr = page_directory[virt_addr_128mb_idx];
	page_directory[virt_addr_128mb_idx] = phys_addr | RW | SET_4MB | USER | PRESENT;
	
	/* Flush the TLB by reloading CR3. */
	asm volatile(
		"movl	%%cr3, %%eax;"
		"movl	%%eax, %%cr3;"
		:
		:
		:"eax"
	);
	
	/***** USER-LEVEL PROGRAM LOADER *****/
	/* Copy file contents to correct location. */
	// THIS FILE SIZE NEEDS TO BE ABLE TO CHANGE DYNAMICALLY!!! FIX.
	read_data(dentry.inode_num, 0,(uint8_t*)(OTE_MB + USER_IDX), buf_size);
	
	/***** CREATE PCB *****/
	/* Create the file descriptor array and put it into memory. */
	pcb_t* pcb_loc = get_pcb_loc(process_number);
	
	/* Create and initialize the PCB. */
	pcb_t pcb;
	uint32_t pcb_it;
	for(pcb_it = 0; pcb_it < MAX_FN; pcb_it++){	// The file descriptor has 8 entries.
		pcb.file_desc[pcb_it].fot_ptr = NULL;
		pcb.file_desc[pcb_it].inode = 0;
		pcb.file_desc[pcb_it].file_position = 0;
		pcb.file_desc[pcb_it].flags = 0;
	}

	/* Grab the parent ESP and EBP and store them. */
	uint32_t pesp = 0;
	uint32_t pebp = 0;
	asm volatile(
		"cli;"
		"movl	%%esp, %0;	"	// Parent ESP
		"movl	%%ebp, %1;	"	// Parent EBP
		// Just for safety.
		"sti;"
		:"=r"(pesp),
		"=r"(pebp)
		:
		//no third colon set, because no registers are modified
	);

	/* Setup the STDIN and STDOUT file descriptors. */
	pcb.file_desc[0].fot_ptr = &stdin_fot;
	pcb.file_desc[0].flags = 1;
	pcb.file_desc[1].fot_ptr = &stdout_fot;
	pcb.file_desc[1].flags = 1;
	
	//copy parent esp/ebp into pcb
	pcb.parent_esp = pesp;
	pcb.parent_ebp = pebp;

	/* Copy arg into the PCB. */
	memcpy(pcb.arg, arg, KB_BUF_SIZE_MAX);
	pcb.arg_size = getargs_it;
	
	/* Save old parent address. */	
	pcb.parent_phys_addr = par_phys_addr;

	/* Save current terminal for this process. */
	pcb.term_number = get_cur_term();
	pcb.parent_pid = parent_pid;
	
	/* Copy our PCB into the proper memory location. */
	memcpy((uint32_t*)pcb_loc, &pcb, sizeof(pcb));
	
	// /***** CONTEXT SWITCH *****/
	 
	//  * The following stack address corresponds to the BOTTOM of the 8 kB kernel
	//  * stack allocated for this process. The -5 comes from -1 to account for 0 indexing
	//  * and -4 so that there is 4 bytes of buffer space between the ESP and the bottom
	//  * of the stack in case a 4-byte long variable is allocated (int or something).
	 
	uint32_t user_esp = OTE_MB + FOUR_MB - 5;
	tss.esp0 = KERNEL_LOC + FOUR_MB - (EIGHT_KB * process_number) - 1;
	tss.ss0 = KERNEL_DS;
	
	uint32_t int_set = 0x200;					// This will set the INTR flag in EFLAGS.
	
	/* Perform an IRET with proper context. */
	asm volatile(
		"cli;"
		"pushl	%0;"	// USER DS	
		"pushl	%1;"	// User ESP
		
		/* Grab and set EFLAGS for the user process. */
		"pushfl;"		// EFLAGS
		"popl	%%eax;"
		"orl	%2, %%eax;" 	
		"pushl	%%eax;"
		
		"pushl	%3;"	// USER CS
		"pushl	%4;"	// ENTRY POINT
		"iret;"
		
		/* Just for safety. */
		"sti;"
		:
		:"r"(USER_DS),
		"r"(user_esp),
		"r"(int_set),
		"r"(USER_CS),
		"r"(entry_point)
		:"eax", "cc"
	);
	return 0;
}

/* sys_read
 * Description : reads file
 	input: fd - file descriptor index
 			buf - buffer to read
 			nbytes - number of bytes to write
 	output: 0 if sucess, -1 error
 	effect: read
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes){
	/* Get the current process number and PCB location. */
	int32_t process_number = get_process_number();
	pcb_t* cur_pcb_loc = get_pcb_loc(process_number);
	
	//check if fd goes out of index
	if(fd < 0 || fd >= MAX_TASK){
		return -1;
	}

	//check if the file exists
 	if((cur_pcb_loc->file_desc[fd].flags & 1) ==0 ){
		return -1;
	} 

	//get the file operation pointer and read
	fot_t* ret = cur_pcb_loc->file_desc[fd].fot_ptr;
	int32_t* fd__ = (int32_t*)&(cur_pcb_loc->file_desc[fd]);
	int32_t ret_val = ret->read((int32_t)fd__, buf, nbytes);
	return ret_val;
}

/* sys_write
 * Description : writes file
 	input: fd - file descriptor index
 			buf - buffer to write
 			nbytes - number of bytes to write
 	output: 0 if sucess, -1 error
 	effect: write
 */
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes){
	/* Get the current process number and PCB location. */
	int32_t process_number = get_process_number();
	pcb_t* cur_pcb_loc = get_pcb_loc(process_number);
	
	//check if fd goes out of index
	if(fd < 0 || fd >= MAX_TASK){
			return -1;
		}

	//check if file exists
 	if((cur_pcb_loc->file_desc[fd].flags & 1) == 0){
		return -1;
	} 

	//get the file operation pointer and write
	fot_t* ret = cur_pcb_loc->file_desc[fd].fot_ptr;
	uint32_t ret_val = ret->write(fd, buf, nbytes);
	return ret_val;
}

/* sys_open
 * Description : opens file
 	input: filename = file name string
 	output: 0 if sucess, -1 error
 	effect: opens file
 */
int32_t sys_open(const uint8_t* filename){
	/* Get the current process number and PCB location. */
	int32_t process_number = get_process_number();
	pcb_t* cur_pcb_loc = get_pcb_loc(process_number);
	
	int i;
	dentry_t dentry;

	//read the file and get dentry
	if(read_dentry_by_name(filename, &dentry) == -1){
		return -1;
	}

	//check which file description it is
	for(i=MIN_TASK; i<MAX_TASK; i++){
		if((cur_pcb_loc->file_desc[i].flags) == 0){
			cur_pcb_loc->file_desc[i].flags = 1;
			cur_pcb_loc->file_desc[i].file_position = 0;
			break;
		}
	}

	if (i == MAX_TASK){
		return -1;
	}

	//check the file type
	switch (dentry.file_type){
		case TYPE_RTC:
			if(rtc_open() == -1){
				return -1;
			}
			cur_pcb_loc->file_desc[i].inode = -1;
			cur_pcb_loc->file_desc[i].fot_ptr = (fot_t *)&rtc_fot;
			break;

		case TYPE_DIR:
			if(dir_open() != 0){
				return -1;
			}
			cur_pcb_loc->file_desc[i].inode = -1;
			cur_pcb_loc->file_desc[i].fot_ptr = (fot_t *)&dir_fot;
			break;

		case TYPE_FILE:
			if(fopen() != 0){
				return -1;
			}

			cur_pcb_loc->file_desc[i].inode = dentry.inode_num;
			cur_pcb_loc->file_desc[i].fot_ptr = (fot_t *)&file_fot;
			break;
	}

	return i;
}

/* sys_close
 * Description : closes file
 	input: file descriptor index
 	output: 0 if sucess, -1 error
 	effect: closes file
 */
int32_t sys_close(int32_t fd){
	/* Get the current process number and PCB location. */
	int32_t process_number = get_process_number();
	pcb_t* cur_pcb_loc = get_pcb_loc(process_number);
	
	fot_t* fot;

	//check for right index
	if(fd < 0 || fd >= MAX_TASK){
			return -1;
		}

		//check if file exists
	if((cur_pcb_loc->file_desc[fd].flags & 1) == 0 ){
		return -1;
	}

	//get file operations pointer


	fot = cur_pcb_loc->file_desc[fd].fot_ptr;
	if(fot->close() == -1){
		return -1;
	}

	//set flag
	cur_pcb_loc->file_desc[fd].flags = 0;




	return 0;
}

/*
 * This function will just take the arguments passed to execute and stored in the
 * PCB and copy them into user space. The argument will be NULL terminated.
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes){
	/* Get the current process number and PCB location. */
	int32_t process_number = get_process_number();
	pcb_t* cur_pcb_loc = get_pcb_loc(process_number);
	
	if(buf == NULL){
		return -1;
	}
	uint8_t* arg_ptr = (*cur_pcb_loc).arg;
	uint32_t arg_size = (*cur_pcb_loc).arg_size;
	/* 
	 * Make sure there actually IS an argument and that the number of
	 * bytes to be written is large enough to include a NULL character too.
	 */
	if((arg_ptr[0] == '\0') || (nbytes < arg_size)){
		return -1;
	}
	if(nbytes > KB_BUF_SIZE_MAX){
		nbytes = KB_BUF_SIZE_MAX;
	}
	memset(buf, 0, nbytes + 1);			// Set the whole thing to NULL to begin with.
	memcpy(buf, arg_ptr, nbytes);		// It is good practice to use memcpy instead of '='.
	return 0;
}

/*
 * Gives the user access to video memory in a safe way.
 */
int32_t sys_vidmap(uint8_t** screen_start){	
	/* Check that the physical address provided is in a valid location. */
	if(screen_start < (uint8_t**)OTE_MB || screen_start > (uint8_t**)(OTE_MB + FOUR_MB)){
		return -1;
	}
	
	/* Fetch the PCB location for this process. */
	uint32_t process_number = get_process_number();
	pcb_t* pcb_loc = get_pcb_loc(process_number);
	uint8_t proc_term = pcb_loc -> term_number;	// Grab the terminal this process is on.
	uint8_t cur_term = get_cur_term();			// Grab the currently active terminal.
	int32_t user_screen = ONE_GIG;
	
	/* See if we should be writing to the VGA memory or the backing buffer. */
	if(cur_term != proc_term){
		page_table_vid[0] = (VIM_MEM_INDEX + FOUR_KB * (proc_term + 1)) | RW | PRESENT | USER;
	}
	else{ // Point to VGA's memory page.
		page_table_vid[0] = VIM_MEM_INDEX | RW | PRESENT | USER;
	}
	
	/* Give the user the mapping. */
	*screen_start = (uint8_t*)user_screen;
	return user_screen;
}

/*
 * Boi.
 */
int32_t sys_set_handler(int32_t signum, void* handler_address){
	return 0;
}

/*
 * 
 */
int32_t sys_sigreturn(void){
	return 0;
}
