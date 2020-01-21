/*page.h
* .h file for page.c which initializes paging
*/


#ifndef _PAGE_H
#define _PAGE_H

#include "types.h"

#define NUM_ENTRIES 1024    	//1KB entires 
#define FOUR_KB		4096		//4KB
#define ONE_GIG		0x40000000	// One gigabyte.

#define PRESENT 1		
#define RW      2
#define USER    4
#define MB_4	0x400000			
#define SET_4MB 0x80 				//PS=1 indicates 4MBytes
#define VIM_MEM_INDEX 0xB8000

uint32_t page_directory[NUM_ENTRIES] __attribute__((aligned(FOUR_KB)));			//align on 4KB page boundaries
uint32_t page_table[NUM_ENTRIES] __attribute__((aligned(FOUR_KB)));
uint32_t page_table_vid[NUM_ENTRIES] __attribute__((aligned(FOUR_KB)));

extern void initialize_page();			//initialize page directory, page table
extern void init_page_table();
// void flush();

#endif /* _PAGE_H */
