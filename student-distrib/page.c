/*page.c
* initializes page directory and page table
*/

#include "page.h"
#include "types.h"
#include "x86_desc.h"
#include "lib.h"


/* 
 * initialize_page()
 * DESCRIPTION: Initialize page directory and page table
 				Example from the wiki.osdev.org/Paging
 * INPUT: NONE
 * OUTPUT: NONE
 * RETURN: NONE
 * SIDE EFFECT: NONE
 */

void initialize_page(){

	int i;
	for(i = 0; i < NUM_ENTRIES; i++){
	    page_directory[i] = RW;				//initialize pde with only RW =1 
	}


	init_page_table();						//initialize page table for 4KB page

	page_directory[0] = ((unsigned int)page_table | RW | PRESENT);		//first pde entry. for 4KB page, video mem
	page_directory[1] = MB_4 | RW | PRESENT | SET_4MB;			//second pde entry. for 4MB page, kernel
	////////////////////////////////////////////////////^NO USER???????
	page_directory[ONE_GIG / MB_4] = (unsigned int)page_table_vid | RW | PRESENT | USER;

	asm volatile(
		"movl %0, %%cr3;"		//load cr3 the starting address of page directory

		"movl %%cr4, %%eax;"
		"orl $0x10, %%eax;"
		"movl %%eax, %%cr4;"		//enable the page size extension for 4MB pages

		"movl %%cr0, %%eax;"
		"orl $0x80000000, %%eax;"
		"movl %%eax, %%cr0;"        //enable paging

		:
		:"r"(page_directory)
		:"%eax"
	);
}



/* 
 * init_page_table()
 * DESCRIPTION: Initialize page table
 				Example from the wiki.osdev.org/Paging
 * INPUT: NONE
 * OUTPUT: NONE
 * RETURN: NONE
 * SIDE EFFECT: NONE
 */

void init_page_table(){

	unsigned int i;
	for(i = 0; i < NUM_ENTRIES; i++)				//fill the 1024 (1KB) entries of the page table
	{
	    page_table[i] = (i * FOUR_KB) | RW | USER; 		//for each pte, set read/write and user bit to 1
	    if(i == (VIM_MEM_INDEX/FOUR_KB)){				//for video memory entry, also set the present bit to 1
	    	page_table[i] = (i * FOUR_KB) | RW | PRESENT | USER;	
	    }
		
		/* Initialize vidmap paging. */
		page_table_vid[i] = (i * FOUR_KB) | RW | USER;
	}
	/* 
	 * Set up default entries for vidmap. Indices 1, 2, and 3 will hold the backing store
	 * for all of the terminals.
	 */
	page_table_vid[0] = VIM_MEM_INDEX | RW | PRESENT | USER;
	page_table_vid[1] = (VIM_MEM_INDEX + 1 * FOUR_KB) | RW | PRESENT | USER;
	page_table_vid[2] = (VIM_MEM_INDEX + 2 * FOUR_KB) | RW | PRESENT | USER;
	page_table_vid[3] = (VIM_MEM_INDEX + 3 * FOUR_KB) | RW | PRESENT | USER;
	return;
}

