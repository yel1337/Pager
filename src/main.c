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
#define RAW_ADDRESS_REFERENCE 4101

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
	uint32_t **logical_space;
	uint32_t *pte_size;
	uint32_t *pte_frame[];
} LOGICAL_MEMORY;

typedef struct{
	int pid;
	uint32_t process_size;
	PT p_table;
	LOGICAL_MEMORY to_log;
} PROCESS; 

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

static pid_t parent_check(PROCESS pr)
{
	if (pr.pid) {
		return PARENT_SUCCESS;
	}
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
	array_size = sizeof((size_t) array_instance->logical_space[PTE_SIZE]) /
							sizeof((size_t) array_instance->logical_space[0][PTE_SIZE]);

	return array_size;
}
static uint32_t **which_free_frame()
{
	LOGICAL_MEMORY *array_instance;
	size_t size = return_size(array_instance);
	uint32_t **pointer_to_free_frame = malloc(sizeof(size));
	for (int index = 0; index < size; index++) {
		if (array_instance->pte_frame[index]!= NULL) {
			pointer_to_free_frame[index] = &array_instance->pte_frame[index];
		} else {
			pointer_to_free_frame[index] = NULL;
		}
	}
	return pointer_to_free_frame;
}

static uint32_t **allocate_frame()
{
	LOGICAL_MEMORY *frame_instance;
	PROCESS pr;
	size_t s_array = return_size(frame_instance);	
	uint32_t ***free_spaces = which_free_frame(frame_instance, s_array);
	pid_t parent_pid = create_process();
	pr.pid = (int) parent_pid;

	// Allocate a frame inside of multi dimensional array of logical memory
	for (int frame_populate = 0; s_array < frame_populate; frame_populate++) {
		if (free_spaces[frame_populate] != NULL) {
			frame_instance->pte_frame[frame_populate] = (uint32_t *)parent_pid;
		}
	}
	return frame_instance->pte_frame;
}

static uint32_t **allocate_mem_logical(LOGICAL_MEMORY *l_space) {
	l_space->logical_space = (uint32_t **) malloc(PAGE_SIZE * sizeof(uint32_t *));
}

static uint32_t **which_free_logical_entry(LOGICAL_MEMORY *entry_instance)
{
	size_t size = return_size(entry_instance);
	uint32_t **pointer_to_free_logical_entry[size]; 
	for (int entry_index = 0; entry_index < size; entry_index++) {
		if (entry_instance->logical_space[entry_index] != NULL) {
			pointer_to_free_logical_entry[entry_index] = malloc(sizeof(PTE_SIZE));
		} else {
			pointer_to_free_logical_entry[entry_index] = NULL; 
		}
	}
	return pointer_to_free_logical_entry;
}

static void *allocate_entry_logical(LOGICAL_MEMORY *lm_instance)
{
	/*
	 * TAKES FRAME POINTER AND ADD IT TO LOGICAL MEMORY
	 */
	uint32_t **to_free_frame = which_free_frame();
	uint32_t **to_free_logical = which_free_logical_entry(lm_instance);
	uint32_t **the_frame = allocate_frame();
	for (int logical_entry_index = 0; lm_instance->logical_space[logical_entry_index] != NULL
						&& to_free_frame[logical_entry_index] != NULL; logical_entry_index++) {
		lm_instance->logical_space[logical_entry_index] = &the_frame[logical_entry_index];						
	}
}
static uint32_t PAGE_NUMBER()
{
	pid_t *address_reference = create_process();
	uint32_t get_pn = (uintptr_t) address_reference / PAGE_SIZE;
	return get_pn;
}

static uint32_t OFFSET(LOGICAL_MEMORY *do_offset_frame, uint32_t pn)
{
	pn = page_number();
	pid_t *address_reference = RAW_ADDRESS_REFERENCE;
	uint32_t get_offset = address_reference - (pn * PAGE_SIZE);
	return get_offset;
}

int main(int argc, char *argv[])
{
	LOGICAL_MEMORY l, *lg, *lg_debug;
	PROCESS process;

	/*
	 * ALLOCATE FOR LOGICAL MEMORY
	 */
	uint32_t *l_allocate = allocate_mem_logical(lg);
	if (l_allocate == NULL) {
		errno = ENOMEM;
		return 0;
	}

	/*
	 * ALLOCATION FOR LOGICAL MEMORY STARTS HERE
	 */
	uint32_t **frame_instance = allocate_frame();
	
	/*
	 * PROCESS CHECK UP
	 */
	if (parent_check(process) == PARENT_SUCCESS) {
		allocate_entry_logical(lg);
	}

	/*
	 * DEBUG OPTION ON 
	 *
	 * A valid page frame should return no trash value
	 */
}
