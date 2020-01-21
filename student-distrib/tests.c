#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "page.h"
#include "file.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

// My DEFINES.
//#define FOUR_KB		4096	
#define NUM_ENTRIES 1024    //1KB entires 
#define VID_INDEX   184		
#define MB_4	0x400000			
#define SET_4MB 0x80 

#define PRESENT 1		
#define RW      2
#define USER    4


/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Paging Test
 * 
 * Checks if the page table is initialized correctly by checking its page base address,
 * read/write, user/supervisor, and present bits.
 * Also checks if the page table entry at video memory has present bit = 1
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Files: page.h/page.c
 */
int paging_test()
{
	TEST_HEADER;
	
	int i;

 	int result = PASS;
	for(i = 1; i < NUM_ENTRIES; ++i){
     if( i != VID_INDEX){
        if(!(page_table[i] & RW) || (page_table[i] & PRESENT) ||
        	!(page_table[i] & USER) || !(page_table[i] & i*FOUR_KB)){
           result = FAIL;
        }
     }
     else{										//i = vid index
        if(!(page_table[i] & PRESENT) || !(page_table[i] & USER) ||
        	!(page_table[i] & RW) || !(page_table[i] & i*FOUR_KB)){
          result = FAIL;
          printf("\n index %d of %d                   ", i , page_table[i]);
        }
     }


	}    

	for(i=0; i< NUM_ENTRIES; i++){
		if(!(page_directory[i] & RW)){
			result = FAIL;
		}
		if(i == 0){
			if(!(page_directory[i] & ((unsigned int)page_table) || 
				!(page_directory[i] & RW) || !(page_directory[i] & PRESENT))){
				result = FAIL;
			}
		}
		if(i == 1){
			if(!(page_directory[i] & SET_4MB) || !(page_directory[i] & MB_4)){
				return FAIL;
			}
		}
	}
  return result; 
}


/* Checkpoint 2 tests */
void rtc_write_test(){
	puts("Launching RTC test\n");
	/* Just keep reading the RTC. */
	while(1){
		void* deadbuf;
		rtc_read(0, deadbuf, 0);	// Send a bunch of dummy args.
	}
}


void test_display_files()
{

	// dentry_t* dir_dentry;

	// int i = 0;

	// //read directory
	// uint8_t name_buf[MAX_NAME_LEN];
	// for(i=0; i < 17; i++){
	// 	dir_read(i, name_buf, MAX_NAME_LEN, &dir_dentry);
	// 	printf("FILE_NAME: ");
	// 	terminal_write(0, name_buf , MAX_NAME_LEN);
	// 	printf(",  TYPE: %u,  length: %u \n", dir_dentry->file_type, get_file_length(dir_dentry));

	// }


	// uint8_t fish[FOUR_KB];

	//read non text file
	// fread(fish, FOUR_KB, "fish");
	// terminal_write(fish, FOUR_KB);
	// printf("\n");
	// fread(fish, FOUR_KB, "rtc");
	// terminal_write(fish, FOUR_KB);
	// printf("\n");
	// fread(fish, FOUR_KB, "cat");
	// terminal_write(fish, FOUR_KB);
	// printf("\n");
	// fread(fish, FOUR_KB, "testprint");
	// terminal_write(fish, FOUR_KB);
	// printf("\n");

	// //read text file
	// fread(fish, FOUR_KB, "frame0.txt");
	// terminal_write(fish, FOUR_KB);
	// printf("\n");

	//read large file
	// uint8_t fish[5277];
	// fread(fish,5277,"verylargetextwithverylongname.tx");
	// terminal_write(fish, 5277);
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	/****************************/
	/***** TESTS FOR PART 2 *****/
	/****************************/
/* 	uint32_t BUFSIZE = 1024;
	uint8_t buf[BUFSIZE];
	memset(buf, 0, BUFSIZE);
	while(1){
		terminal_read(0, buf, BUFSIZE-1);
		memset(buf, 0, BUFSIZE);
	} */
	
	
	// rtc_write_test();
	//test_display_files();
	/****************************/
	/***** TESTS FOR PART 1 *****/
	/****************************/
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	
	/* Paging Tests */
	//TEST_OUTPUT("paging_test", paging_test());
	
	/***** TESTS FOR EXCEPTIONS *****/
	/* Divide by 0 test: */
	//int wow = 10 / 0;
	
	/* PAGE FAULT test. */
	/* Make sure that this location (video memory) does not give a page fault! */
	/*int* ptr =0xB8000;
	int newptr = *ptr;*/
	
	/*int *ptr = NULL;
	int *newPtr = *ptr;*/
	
	/* Dereferencing this far away should cause a PAGE FAULT */
	/*int *ptr = 0x2000;			// 8 MB.
	int *newPtr = *ptr;*/
}
