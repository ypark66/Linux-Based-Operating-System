/*page.h
* .h file for page.c which initializes paging
*/


#ifndef _PCB_H
#define _PCB_H

#include "types.h"

#define EIGHT_KB 8192 //kernel stack size is 8KB, 8192 Bytes
#define EIGHT_MB 0X800000 //bottom of the kernel stack is 8MB

// typedef struct file_descriptor_t{
// 	int32_t * f_ops;
// 	int32_t inode;
// 	int32_t f_position;
// 	int32_t flags;
// }f_descriptor_t;



// typedef struct pcb_t{
// 	uint32_t pid;
// 	f_descriptor_t file_array[8];
	
// 	struct pcb_t* parent_task;
// 	uint32_t parent_esp;
// 	uint32_t parent_ebp;

// 	uint32_t ret_addr;
// 	uint32_t status;

// 	uint8_t* arg;


// } pcb_t;


#endif /* _PCB_H */
