/*
 * This file contains the 10 interrupts that we must implement.
 */
#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "file.h"
#include "page.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"
#include "rtc.h"

#define MAX_PROCESSES 6		// Total number of processes allowed.

#define MAX_TASK 8 //maximum number of tasks allowed
#define MIN_TASK 2

#define TYPE_RTC 0
#define TYPE_DIR 1
#define TYPE_FILE 2
#define MAX_FN 8
#define MIN_FN 2
#define FILE_BUF_SIZE 40
#define ENTRY_PT 24

/* Useful constants relating to memory offsets. */
#define FOUR_MB 	0x00400000
#define EIGHT_MB 	0x00800000
#define OTE_MB 		0x08000000		// OTE stands for One Twenty-Eight.
#define EIGHT_KB 	0x00002000
#define KERNEL_LOC 	0x00400000		// This is the virtual address of the kernel space. 
#define USER_IDX 0x00048000

/* Other useful constants. */
#define MAX_FILENAME_LENGTH  	32		// This is the longest that a filename can be.
#define KB_BUF_SIZE_MAX			128

enum process_flags{
	FREE = 0,
	RUNNING = 1,
	SUSPENDED = 2,
};

//file operations jump table 
typedef struct fot_t{
	int32_t (*open) (void);
	int32_t (*close) (void);
	int32_t (*read) (int32_t fd, const void* buf, int32_t nbytes);
	int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
} fot_t;

/* 
 * This struct represents a single entry in the
 * file descriptor table. 
 */
typedef struct file_descriptor{
	fot_t* fot_ptr;
	uint32_t inode;
	uint32_t file_position;
	uint32_t flags;
} file_desc_t;

/*
 * This struct represents a single PCB.
 */
typedef struct pcb{
	file_desc_t file_desc[8];		// The file descriptor table only has 8 entries.
	int32_t pid;					//process id in execute might not be incrementing correctly; if there's a bug related to process number, that's probably it.
	int32_t parent_pid;
	uint32_t parent_phys_addr;		//physical memory address of parent paging
	uint32_t parent_esp;			// Parent ESP
	uint32_t parent_ebp;
	uint32_t file_position;
	uint32_t status;
	uint8_t arg[KB_BUF_SIZE_MAX];
	uint32_t arg_size;				// Holds the size of the arg including the NULL char.
	uint8_t term_number;			// Holds the terminal number of the given process.
} pcb_t;

#ifndef ASM
/* This is the getter function for the current process number. */
int32_t get_process_number();

/* Getter function for PCB location. */
pcb_t* get_pcb_loc(int32_t process_number);

/* Getter for proc_arr. */
int32_t* get_proc_arr();

/* This is the assembly linkage for system calls from INT 0x80. */
extern void syscall_linker();

int32_t sys_halt(uint8_t status);

int32_t sys_execute(const uint8_t* command);

int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t sys_open(const uint8_t* filename);

int32_t sys_close(int32_t fd);

int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

int32_t sys_vidmap(uint8_t** screen_start);

int32_t sys_set_handler(int32_t signum, void* handler_address);

int32_t sys_sigreturn(void);
#endif		// ASM
#endif		// SYSCALLS_H
