/*file.h
* .h file for file.c which implements file system
*/


#ifndef _FILE_H
#define _FILE_H

#include "types.h"

#define MAX_NAME_LEN 32    //maximum file name length
#define FOUR_KB 4096


typedef struct dentry{
	int8_t fname[MAX_NAME_LEN];
	int32_t file_type;
	int32_t inode_num;
	int8_t reserved[24];	//reserve remaining 24 bytes
} dentry_t;

typedef struct boot_block{
	int32_t dir_count;
	int32_t inode_count;
	int32_t data_count;
	int8_t reserved[52];	//reserve remaing 52 bytes
	dentry_t d_entries[63]; //max of 63 entries available
}boot_block_t;

typedef struct inode{
	int32_t length;
	int32_t data_block_num[1023];	//max of 1023 data block
}inode_t;


int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
extern void get_block_address(unsigned int address);
int32_t fopen();
int32_t fclose();
int32_t fwrite(int32_t fd, const void* buf, int32_t nbytes);
int32_t fread(int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_open();
int32_t dir_close();
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_read(int32_t fd, const void* buf, int32_t nbytes);
int32_t get_file_length(dentry_t* dentry);
int32_t get_file_length_by_name(uint8_t* fname);

#endif /* _FILE_H */
