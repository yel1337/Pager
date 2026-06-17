#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#define PAGE_SIZE 0x10000
#define PTE_SIZE 0x4000
#define P_MEMORY 0x1000 /* A PROCESS NEEDS 4KB*/
#define BIT_VPN 2

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
	uint32_t *va_space;
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
static uint32_t ***which_free_frame(LOGICAL_MEMORY *array_instance, size_t size)
{
	uint32_t ***pointer_to_free_frame = malloc((size_t)sizeof(size));
	for (int index = 0; index < size; index++) {
		if (array_instance->pte_frame[index]!= NULL) {
			pointer_to_free_frame[index] = &array_instance->pte_frame[index];
		} else {
			pointer_to_free_frame[index] = NULL;
		}
	}
	return pointer_to_free_frame;
}

static uint32_t **allocate_frame(size_t size)
{
	LOGICAL_MEMORY *frame_instance;
	uint32_t ***free_spaces = which_free_frame(frame_instance, size);

	// Allocate a frame inside of multi dimensional array of logical memory
	for (int frame_populate = 0; size < frame_populate; frame_populate++) {
		if (free_spaces[frame_populate] != NULL) {
			frame_instance->pte_frame[frame_populate] = pid_mem;
		}
	}
	return frame_instance->pte_frame;
}

static uint32_t ***which_free_logical_entry(LOGICAL_MEMORY *entry_instance, size_t size)
{
	uint32_t ***pointer_to_free_logical_entry = malloc(sizeof((uint32_t**)PAGE_SIZE));
	for (int entry_index = 0; entry_index < size; entry_index++) {
		if (entry_instance->logical_space[entry_index] != NULL) {
			pointer_to_free_logical_entry[entry_index] = (uint32_t **)entry_instance->logical_space[entry_index];
		} else {
			pointer_to_free_logical_entry[entry_index] = NULL; 
		}
	}
	return pointer_to_free_logical_entry;
}

static void *allocate_entry_logical(LOGICAL_MEMORY *lm_instance, uint32_t **frame)
{
	size_t size_of_logical = return_size(lm_instance);
	frame = allocate_frame(size_of_logical);
	while (which_free_logical_entry(lm_instance, size_of_logical) != NULL)  {
		lm_instance->logical_space = (uint32_t **(*)[PTE_SIZE])frame;
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	LOGICAL_MEMORY lg, *lg_debug;
	PROCESS pr;
	lg.va_space = (uint32_t *) malloc(P_MEMORY * sizeof(PAGE_SIZE));
	if (lg.va_space == NULL) {
		fprintf(stderr, "process ran out of memory\n");
		return EXIT_FAILURE;
	}

	/*
	 * Initialize size, parameters for page table this include
	 * an identifier if a page table is writeable or not
	 * 1 = YES & 0 as NO // DEFAULT 0
	 */
	pr.pid = (unsigned int) pid_rand();
	pr.p_table.page_count = 0;
	int argv_flag = atoi(argv[2]);
	if (pr.pid) {
		printf("pid: %d\n", pr.pid);

		/*
		 * This where Page Table starts allocating a size
		 */
		pr.p_table.writable = (argv_flag < 2) ? argv_flag: 0;
		pr.p_table.us = (argv_flag < 2) ? argv_flag: 0;
		pr.p_table.size = P_MEMORY;
		pr.p_table.page_count++;
	}

	/*
	 * Initialize a context for frame in logical memory
	 */
	pr.process_size = P_MEMORY;
	lg.va_space[pr.p_table.page_count] = (uint32_t *) malloc(PTE_SIZE);

	int x = 13;


	/*
	 * DEBUG OPTION ON 
	 *
	 * A valid page frame should return no trash value
	 */
	lg_debug = &pr.to_log;
	size_t n = 0;
	if (strcmp(argv[1], "-fma") == 0) {
		_out(debug_out_frame(lg_debug, pr.p_table.page_count, 1), 1, 0, n);
	} else if (strcmp(argv[1], "-ni") == 0) {
		size_t page_n = count_page_indexes(lg);
		_out(debug_out_frame(lg_debug, pr.p_table.page_count, 1), 1, 1, n);
	}
}
