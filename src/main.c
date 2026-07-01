#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "process/pr.h"

#define PAGE_SIZE 0x10000
#define ADDRESS_SPACE 0x100000
#define PTE_SIZE 0x4000
#define P_MEMORY 0x1000 /* A PROCESS NEEDS 4KB */
#define PARENT_SUCCESS 0
#define PAGE_NUMBER ADDRESS_SPACE / PAGE_SIZE
#define OFFSET(VIRTUAL_ADDRESS, PAGE_SIZE) (VIRTUAL_ADDRESS % PAGE_SIZE)

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

typedef struct LOGICAL_MEMORY {
	struct LOGICAL_MEMORY *logical_space;
	struct LOGICAL_MEMORY **pte_frame;
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

static size_t return_size(LOGICAL_MEMORY *array_instance){
	size_t array_size;
	/*
	 * Below gets the absolute size of an index
	 */
	array_size = sizeof(array_instance->logical_space[PTE_SIZE]) /
							sizeof(array_instance->logical_space[0]);

	return array_size;
}

static LOGICAL_MEMORY **which_free_frame()
{
	int count = 0;
	LOGICAL_MEMORY *ARRAY_INSTANCE;
    LOGICAL_MEMORY **pointer_to_free_frame = malloc(PAGE_NUMBER * sizeof(LOGICAL_MEMORY *));
	for (int index = 0; index < sizeof(pointer_to_free_frame); index++) {
		if (ARRAY_INSTANCE->pte_frame[index] == NULL) {
			pointer_to_free_frame[count++] = NULL; 
		}
	}
	pointer_to_free_frame[count] = NULL;
	return pointer_to_free_frame;
}

static void allocate_frame(pid_t *process_addr, LOGICAL_MEMORY **FRAME_FREE)
{
        size_t FRAME_SIZE = sizeof(FRAME_FREE);
	// Allocate a frame inside of multi dimensional array of logical memory
        for (int frame_index = 0; frame_index < FRAME_SIZE; frame_index++) {
			FRAME_FREE[frame_index] = (LOGICAL_MEMORY *)process_addr; 
	}	
}

static void allocate_mem_logical(LOGICAL_MEMORY *logical_instance) {
	logical_instance->logical_space = malloc(ADDRESS_SPACE * sizeof(LOGICAL_MEMORY *));
}

static LOGICAL_MEMORY *DEBUG_ADDR_LOGICAL(LOGICAL_MEMORY *lm)
{
	return lm->logical_space;
} 

int main(int argc, char *argv[])
{
    LOGICAL_MEMORY l, *lg;

	/*
	 * ALLOCATE FOR LOGICAL MEMORY
	 */
	allocate_mem_logical(lg);
	LOGICAL_MEMORY **space = &lg->logical_space;

	/*
	 * ALLOCATE A MEMORY FOR EACH FRAME
	 */
    lg->pte_frame = malloc(PAGE_NUMBER * sizeof(LOGICAL_MEMORY *));
	LOGICAL_MEMORY **free_frames = which_free_frame();

	/*
	 * COMMENCE CREATE_PROCESS()
	 */
	pid_t pr = create_process();

	/*
	 * ALLOCATION FOR LOGICAL MEMORY STARTS HERE
	 */
    allocate_frame(&pr, free_frames);

	/*
	 * PUT FRAME TO SPACE
	 */
	lg->logical_space = *lg->pte_frame;

	/*
	 * PAGE NUMBER & OFFSET
	 */
	uint32_t PN = PAGE_NUMBER;	
	uint32_t OFT = OFFSET((uint32_t)(uintptr_t)space, PAGE_SIZE);

	/*
	 * DEBUG OPTION ON 
	 *
	 * A valid page frame should return no trash value
	 */
	if(strcmp(argv[1], "-d") == 0) {
	    if (space != NULL && lg != NULL) {
			printf("logical space: ok\nframe: ok") ; 
		} else {
			errno = 0;
		}
	}
}
