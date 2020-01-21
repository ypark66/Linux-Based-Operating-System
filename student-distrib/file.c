/*file.c
* implements file system
*/

#include "file.h"
#include "types.h"
#include "lib.h"
#include "syscalls.h"

static boot_block_t* boot_block;



/* 
 * get the block address 
 */
void get_block_address(unsigned int address){
	boot_block = (boot_block_t*) address;
}


/* uint32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
 * Inputs:      uint32_t index = index of inode
 *              dentry_t* dentry = copying destination
 * Return Value: 0 if read else -1
 * Function: read directory entry by inode index */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	if(index >= boot_block->dir_count){
		return -1;
	}

	dentry_t entry = boot_block->d_entries[index];
	int8_t a = entry.fname[0];
	dentry->fname[0] = a ;
	strcpy((int8_t*)dentry->fname, (int8_t*)entry.fname);			//fill file name
	dentry->file_type = entry.file_type;		//fill file type
	dentry->inode_num = entry.inode_num;		//fill inode number
	return 0;
}

/* uint32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 * Inputs:      const uint8_t* fname = file name
 *              dentry_t* dentry = copying destination
 * Return Value: 0 if read else -1
 * Function: read directory entry by file name */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){

	int32_t d_count = boot_block->dir_count;		//number of existing dentries
	int i = 0;
	uint32_t str_len = strlen((int8_t*)fname);				//file name length
	uint32_t entry_fname_length;
	if(str_len > MAX_NAME_LEN){
		return -1;
	}

	for(i = 0; i < d_count; i++){
		dentry_t entry = boot_block->d_entries[i];
		entry_fname_length = strlen((int8_t*)entry.fname);
		if(entry_fname_length>MAX_NAME_LEN)
			entry_fname_length = MAX_NAME_LEN;

		if(entry_fname_length > str_len){	
			if(strncmp((int8_t*)entry.fname, (int8_t*)fname, entry_fname_length) == 0){
				read_dentry_by_index(i, dentry);		//fname same, copy dentry
				return 0;
			}
		}
		else{
			if(strncmp((int8_t*)entry.fname, (int8_t*)fname, str_len) == 0){
				read_dentry_by_index(i, dentry);		//fname same, copy dentry
				return 0;
			}

		}
	}
	return -1;
}



/* uint32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * Inputs:      uint32_t inode
 *              int32_t offset
 								uint8_t* buf
								uint32_t length
 * Return Value: -1 if failed, bytes copied if success
 * Function: read data */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	uint32_t f_addr = (uint32_t)boot_block;									//starting address of boot block
	uint8_t* dest;
	// uint32_t temp_length;

	uint32_t first_inode = FOUR_KB + (uint32_t)boot_block;
	uint32_t inode_addr = FOUR_KB*inode + first_inode;
	inode_t* inode_ptr = (inode_t*) inode_addr;

	if(offset >= inode_ptr->length){			//if out of range, return 0
		return 0;
	}

	int blk_idx = offset / FOUR_KB;		 	//block index
	uint32_t b_offset = offset % FOUR_KB;											//block offset
	uint32_t save_length = length;		 												//bytes to be copied
	while(1){
		//case of copying the entire block
		if((FOUR_KB - b_offset) >= length){
			dest = (uint8_t*)(FOUR_KB * (inode_ptr->data_block_num[blk_idx] + boot_block->inode_count + 1) + f_addr + b_offset);
			memcpy(buf, dest, length);
			return save_length;
		}
		//case of copying more than one block
		// temp_length = length - FOUR_KB + b_offset+1;
		dest = (uint8_t*)(FOUR_KB * (inode_ptr->data_block_num[blk_idx] + boot_block->inode_count + 1) + f_addr + b_offset);
		memcpy(buf, dest, FOUR_KB-b_offset);
		// printf("%s   buf test\n", buf);
		buf = buf + FOUR_KB - b_offset;															//re-initialize parameters
		length = length - FOUR_KB + b_offset;												//for the next loop
		blk_idx++;
		b_offset = 0;
	}
	return 0;

}


/* int32_t get_file_length(dentry_t* dentry)
 * Inputs:      dentry_t* dentry
 * Return Value: 0 if success else -1
 * Function: gets file length */
int32_t get_file_length(dentry_t* dentry){
	if(dentry->file_type != 2){
		return 0;
	}
	uint32_t inode_num = dentry->inode_num;				//get the inode number for this directory entry

	uint32_t first_inode = FOUR_KB + (uint32_t)boot_block;		//initialize the first inode
	uint32_t inode_addr = FOUR_KB*inode_num + first_inode;		//initialize the inode addr
	inode_t* inode = (inode_t*) inode_addr;										//set inode
	return (uint32_t)inode->length;														//get it length
}

/* int32_t get_file_length_by_name(uint8_t* fname)
 * Inputs:      uint8_t* fname
 * Return Value: 0 if success else -1
 * Function: gets file length */
int32_t get_file_length_by_name(uint8_t* fname){
	dentry_t dentry;
	read_dentry_by_name(fname, &dentry);
	uint32_t inode_num = dentry.inode_num;				//get the inode number for this directory entry

	uint32_t first_inode = FOUR_KB + (uint32_t)boot_block;		//initialize the first inode
	uint32_t inode_addr = FOUR_KB*inode_num + first_inode;		//initialize the inode addr
	inode_t* inode = (inode_t*) inode_addr;										//set inode
	return (uint32_t)inode->length;														//get it length
}


/* uint32_t fopen()
 * Inputs: NONE
 * Return Value: 0
 * Function: opens a file */
int32_t fopen(){
	return 0;
}

/* uint32_t fclose()
 * Inputs: NONE
 * Return Value: 0
 * Function: closes a file */
int32_t fclose(){
	return 0;
}

/* uint32_t fwrite()
 * Inputs: NONE
 * Return Value: -1
 * Function: do nothing */
int32_t fwrite(int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

/* uint32_t fread(uint8_t* buf, uint32_t count, const uint8_t* fname)
 * Inputs: uint8_t* buf
 					 uint32_t count
					 const uint8_t* fname
 * Return Value: 0
 * Function: reads a file */
// int32_t fread(uint8_t* buf, uint32_t count, const uint8_t* fname)
int32_t fread(int32_t fd, const void* buf, int32_t nbytes)
{
	int32_t val = read_data(((file_desc_t*)fd)->inode, ((file_desc_t*)fd)->file_position, (uint8_t*)buf, nbytes);
	((file_desc_t*)fd)->file_position += val;
	return val;
}

/* uint32_t dir_open()
 * Inputs: NONE
 * Return Value: 0
 * Function: opens a dir */
int32_t dir_open(){
	return 0;
}

/* uint32_t dir_close()
 * Inputs: NONE
 * Return Value: 0
 * Function: closes a dir */
int32_t dir_close(){
	return 0;
}

/* uint32_t dir_write()
 * Inputs: NONE
 * Return Value: -1
 * Function: do nothing */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

/* uint32_t dir_read(uint32_t idx, uint8_t* buf, uint32_t length, dentry_t** dentry)
 * Inputs: uint32_t idx
 					 uint8_t* buf
					 uint32_t length
					 dentry_t** dentry
 * Return Value: bytes copied
 * Function: reads directory */
// int32_t dir_read(uint32_t idx, uint8_t* buf, uint32_t length, dentry_t** dentry)


int dir_location = 0;
int32_t dir_read(int32_t fd, const void* buf, int32_t nbytes){

    dentry_t dentry;
    int i;
	
    if (read_dentry_by_index(dir_location, &dentry) == 0)
    {
        //clear buffer
        for (i = 0; i < 33; i++)
            ((int8_t*)(buf))[i] = '\0';


        //copy file name into buffer if directory entry can be read
        int32_t len = strlen((int8_t*)dentry.fname);
        if(len > 32)
        	len =32;
        strncpy((int8_t*)buf, (int8_t*)dentry.fname, len);
        dir_location++;
        return len;
    }
    else
    {
		dir_location = 0;
        return 0;
    }




}
