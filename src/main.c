#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "process/pr.h"

#define PAGE_SIZE 0x10000
#define PTE_SIZE 0x4000
#define P_MEMORY 0x1000 /* A PROCESS NEEDS 4KB */
#define PARENT_SUCCESS 0

/*
 * Supposed this is the page table intermediary between
 * logical memory and the physical memory
 *
 * This is where pt data struct starts followed by
 * virtual memory space think of it as it is 
 * located inside of logical memory
 */
typedef struct {
	uint32_t *f_num; // defines the frame number to physical mem
	int writable;    // if set to 0 read-only, when 1 writable True
	int us;          // if set to 0 only kernel, when 1 user-space True
	int size;
	int page_count;
} PT;

typedef struct{
	uint32_t *logical_space;
	uint32_t **pte_frame;
} LOGICAL_MEMORY;

/*
 * Prints out va's index in hex
 */
void _out(uint32_t *frame_mem_addr, int index,
					int debug_flag, size_t n)
{
	if (debug_flag == 0) {
		printf("%p\n", &frame_mem_addr[index]);
	} else if (debug_flag == 1 && n != 0) {
		printf("%zu\n", n);
	}
}

static size_t count_page_indexes(LOGICAL_MEMORY lga)
{
	return ((size_t) sizeof(lga.pte_frame[PTE_SIZE]));
}

static int parent_check(pid_t *pr)
{
	if (pr) {
		return PARENT_SUCCESS;
	}
	return 0;
}

static uint32_t *debug_out_frame(const LOGICAL_MEMORY *logical_mem,
								int frame_index, int debug_flag)
{
	/*
	 * This function can print out page entry's mem address or
	 * number of page indexes
	 * 			0 = PE's MEMORY ADDRESS
	 * 			1 = n OF INDEXES OF A PAGE
	 */
	LOGICAL_MEMORY l_mem;

	if (debug_flag == 0) {
		uint32_t *mem = logical_mem->pte_frame[frame_index];
		return mem;
	} else if (debug_flag == 1) {
		return (uint32_t *) count_page_indexes(l_mem);
	}

	return NULL;
}

static size_t return_size(LOGICAL_MEMORY *array_instance){
	size_t array_size;
	/*
	 * Below gets the absolute size of an index
	 */
	array_size = sizeof((size_t) array_instance->logical_space[PTE_SIZE]) /
							sizeof((size_t) array_instance->logical_space[0]);

	return array_size;
}
static uint32_t **which_free_frame(LOGICAL_MEMORY *ARRAY_INSTANCE)
{
        size_t size = 100;
	uint32_t **pointer_to_free_frame = malloc(size * sizeof(uint32_t *));
	for (int index = 0; index < size; index++) {
		if (ARRAY_INSTANCE->pte_frame[index]!= NULL) {
		  pointer_to_free_frame[index] = ARRAY_INSTANCE->pte_frame[index];
		} else {
			pointer_to_free_frame[index] = NULL;
		}
	}
	return pointer_to_free_frame;
}

static uint32_t **allocate_frame(pid_t *process, uint32_t *FRAME_INSTANCE)
{
        size_t s_array = return_size((LOGICAL_MEMORY *)FRAME_INSTANCE);	
	uint32_t **free_spaces = which_free_frame((LOGICAL_MEMORY *)FRAME_INSTANCE);

	// Allocate a frame inside of multi dimensional array of logical memory
	for (int frame_populate = 0; s_array < frame_populate; frame_populate++) {
		if (free_spaces[frame_populate] != NULL) {
		  ((LOGICAL_MEMORY *)FRAME_INSTANCE)->pte_frame[frame_populate] = (uint32_t *)process;
		}
	}
	return ((LOGICAL_MEMORY *)FRAME_INSTANCE)->pte_frame;
}

static LOGICAL_MEMORY **allocate_mem_logical(LOGICAL_MEMORY *l_instance) {
	return (LOGICAL_MEMORY**)l_instance->logical_space;
}

static uint32_t **which_free_logical_entry(LOGICAL_MEMORY **entry_instance)
{
	size_t size = return_size(*entry_instance);
	uint32_t **pointer_to_free_logical_entry = malloc(size * sizeof(uint32_t *)); 
	for (int entry_index = 0; entry_index < size; entry_index++) {
	  if ((*entry_instance)->logical_space[entry_index] != 0) {
		  pointer_to_free_logical_entry[entry_index] = malloc(PTE_SIZE);
		} else {
			pointer_to_free_logical_entry[entry_index] = NULL; 
		}
	}
	return pointer_to_free_logical_entry;
}

static uint32_t *DEBUG_ADDR_LOGICAL(LOGICAL_MEMORY *lm)
{
	return lm->logical_space;
} 

static void *allocate_entry_logical(LOGICAL_MEMORY **lm_instance, LOGICAL_MEMORY *f_instance, pid_t *process)
{
	/*
	 * TAKES FRAME POINTER AND ADD IT TO LOGICAL MEMORY
	 */
	uint32_t **to_free_frame = which_free_frame(f_instance);
	uint32_t **to_free_logical = which_free_logical_entry(lm_instance);

	if (to_free_logical == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	uint32_t **the_frame = allocate_frame(process, (uint32_t *)f_instance);
	for (int logical_entry_index = 0; (*lm_instance)->logical_space[logical_entry_index] != 0
						&& to_free_frame[logical_entry_index] != 0; logical_entry_index++) {
	  (*lm_instance)->logical_space[logical_entry_index] = *(the_frame[logical_entry_index]);			     	
	}
	return NULL;
}

static uint32_t GET_LOGICAL_ADDRESS(uint32_t PAGE_NUMBER, uint32_t OFFSET)
{
	uint32_t logical_addr = (PAGE_NUMBER * PAGE_SIZE) + OFFSET;
	return logical_addr;
}

static uint32_t PAGE_NUMBER(pid_t *process)
{
  uint32_t get_pn = (*process) / PAGE_SIZE;
  
	return get_pn;
}

static uint32_t OFFSET(uint32_t pn, pid_t *process)
{
  uint32_t get_offset = (*process) - (pn * PAGE_SIZE);
	return get_offset;
}

int main(int argc, char *argv[])
{
	LOGICAL_MEMORY l, *lg, *to_struct;

	/*
	 * ALLOCATE FOR LOGICAL MEMORY
	 */
	LOGICAL_MEMORY *pointer_logical = malloc(sizeof(LOGICAL_MEMORY));;
	allocate_mem_logical(pointer_logical); // THIS WILL HAVE THE POINTER

	/*
	 * ALLOCATE A MEMORY FOR EACH FRAME
	 */
	to_struct = malloc(sizeof(LOGICAL_MEMORY));
        to_struct->pte_frame = malloc(10 * sizeof(uint32_t *));
	uint32_t **free_frames = which_free_frame(pointer_logical);

	/*
	 * COMMENCE CREATE_PROCESS()
	 */
	pid_t pr = create_process();

	/*
	 * ALLOCATION FOR LOGICAL MEMORY STARTS HERE
	 */
	uint32_t **frame_instance = allocate_frame(&pr, (uint32_t *)free_frames);
	
	/*
	 * PROCESS CHECK UP
	 */
	if (parent_check(&pr) == PARENT_SUCCESS) {
	   allocate_entry_logical(&pointer_logical, (LOGICAL_MEMORY *)free_frames, &pr);
	}

	/*
	 * PAGE NUMBER & OFFSET
	 */
	uint32_t PN = PAGE_NUMBER(&pr);	
	uint32_t OFT = OFFSET(PN, &pr);

	/*
	 * DEBUG OPTION ON 
	 *
	 * A valid page frame should return no trash value
	 */
	if(strcmp(argv[1], "-d") == 0) {
	    if (pointer_logical != NULL && lg != NULL) {
			printf("logical space: ok\nframe: ok") ; 
		} else {
			errno = 0;
		}
	}
}
