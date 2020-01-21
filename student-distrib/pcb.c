/*pcb.c
* file code for pcb
*/

#include "pcb.h"
#include "types.h"
#include "x86_desc.h"
#include "lib.h"


/* 
 * get_addr_pcb(uint32_t pid)
 * DESCRIPTION: finds the address of PCB
 * INPUT: pid
 * OUTPUT: NONE
 * RETURN: returns a pointer to PCB
 * SIDE EFFECT: NONE
 */
// pcb_t* get_addr_pcb(uint32_t pid){
// 	return (pcb_t*) (EIGHT_MB - EIGHT_KB*(pid+1));
// }

// void init_file_array(pcb_t* pcb){
// 	f_descriptor_t fd;
// 	fd.inode = 0;
// 	fd.f_position = 0;
// 	fd.flags = 0;
// }
