
/* machine.c */

// Simulates memory of a machine
// with small addresses (a small virtual address space)
// and a small amount of physical memory
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "pra.h"
#include "machine.h"


// To easily turn on / off debug printing
#ifdef DEBUG
 #define D if(1) 
#else
 #define D if(0) 
#endif

const unsigned long ADDR_SIZE = 1;	// bytes (size of memory address  1 byte = 8 bit addresses)

// These should all be exactly powers of 2 ALWAYS!!!!
const unsigned long VIRT_ADDR_SIZE = 256;	// bytes (size of process virtual address space)
// This number is derived from the ADDR_SIZE above.  The address space is 8 bits,
// with 8 bits there are 2**8 = 256 slots 

const unsigned long RAM_SIZE = 128;	// bytes (size of physical memory)
// This number is somewhat arbitrary because it is an attribute of the simulated machine.

const unsigned long PAGE_SIZE = 32;		// bytes (size of a page = size of a frame)
// This number is somewhat arbitrary, because it is an attribute of the simulated OS on the simulated machine.
// Linux uses 4096B pages.

unsigned long ADDR_NUM_BITS;
unsigned long PAGE_NUM_BITS;
unsigned long OFFS_NUM_BITS;

// The total number of frames in the system (start counting from 0)
// With pages that are 32B, each page is exactly 1/4 of RAM, 
//	so there are 4 frames
// With pages that are 32B, each page is exactly 1/8 of the Virtual Address Size, 
//	so there are 8 page table entries
unsigned int NUM_FRAMES;
unsigned int NUM_PAGES;

// Number of instructions read
unsigned int C;

// Number of misses globally
unsigned long long misses;

// Counter to keep track of time spent reading disk
// Every disk read increments this using the 
// read_disk() function.
unsigned long long TIME = 0;

// The page table
struct page_table_entry *pagetable;


// Utility method to convert from HEX character (just one) to decimal value
int hextoint(char c){
	switch(c){
		case 'a': case 'A':
			return 10;
		case 'b': case 'B':
			return 11;
		case 'c': case 'C':
			return 12;
		case 'd': case 'D':
			return 13;
		case 'e': case 'E':
			return 14;
		case 'f': case 'F':
			return 15;
	}

	// Clever trick to convert ascii numbers to int
	// Each number character is offset from the value of the character '0'
	// Their offset is exactly their numeric value
	return c - '0';
}

// Utility method to convert large hex number to decimal;
unsigned long long hex2dec(int input_s, char *input){
	// memory addresses at most 8 bytes and unsigned
	unsigned long long result = 0;
	int exp = 0;
	int i;
	for(i = input_s-1; i >= 0; i--){
		//printf("i: %d  hextoint(input[i]): %d  (int)pow(16, i): %f\n", i, hextoint(input[i]), pow(16, exp));
		result = result + hextoint(input[i]) * pow(16, exp++);
	}

	return result;
}


//////////////////////////////////////////////////////////////
// HEADER FILE STUFF
struct page_table_entry *get_page_table_entry(int index){
	if(index < 0 || index >= NUM_PAGES) return NULL;
	return &pagetable[index];
}

// A utility function to print the entire page table
void dump_page_table(int s, int e){
	int i;
	for(i = s; i < e; i++){
		struct page_table_entry *cur = &pagetable[i];
		printf("pagetable[%d] {Fr: %lu  Pe: %d  Hi: %u  R: %u  C: %u}\n", i,  cur->frame_number, cur->present, cur->hits, cur->R, cur->C);
	}
}

unsigned int get_num_pages(){
	return NUM_PAGES;
}

unsigned int get_num_frames(){
	return NUM_FRAMES;
}

unsigned int get_c(){
	return C;
}
// END HEADER FILE STUFF
/////////////////////////////////////////////////////////////


// Simulates reading the disk by just wasting time
// Amount of time is arbitrary
void read_disk(){
	TIME = TIME + 20000; // microseconds to read 1mebibytes from disk (useconds)

	// Can be used to cause the program to run longer (to increase dramatic effect)
	//usleep(20000); // microseconds (us)
}


// Marks all pages associated with given frame as 'not present' 
void set_not_present(unsigned long frame_num){
	int i;
	for(i = 0; i < NUM_PAGES; i++){
		if(pagetable[i].frame_number == frame_num){
			pagetable[i].present = 0;
		}
	}
}


// Resets all the R bits (recently used marking).  This occurs every 25 memory accesses
void reset_R_bits(){
	int i;
	for(i = 0; i < NUM_PAGES; i++){
		struct page_table_entry *cur = &pagetable[i];
		cur->R = 0;
	}
}

// Simulates a page fault by asking (the student code) select_frame()
// to pick a frame to evict
void page_fault(struct page_table_entry *e){
	read_disk();

	unsigned int new_frame = select_frame(); // <-- Student Code!
	set_not_present(new_frame);

	// Set the correct values for this frame
	e->frame_number = new_frame;
	e->present = 1;
	e->hits++;
	e->R = 1;
	e->C = get_c();
}

// Simulates the MMU
void mmu(unsigned long in){

	unsigned long page_num = in >> OFFS_NUM_BITS;

	// Lookup page table entry
	struct page_table_entry *e = &pagetable[page_num];
	if(e->present == 1){
		e->C = get_c();
		D printf("HIT!\n");
	} else {
		D printf("MISS!\n");
		misses++;
		page_fault(e);
	}

	D 
	{
		unsigned long mask = (unsigned long)(pow(2, OFFS_NUM_BITS)-1);
		unsigned long offset = (in & mask);

		printf("Virtual addr: %#12lX\n", in);
		printf("page_num: %#3lX   offset: %#3lX\n", page_num, offset);
		printf("\e[1;34mUsing Frame Number: %lX\e[m\n", (e->frame_number));
		//unsigned long phys_addr =  (((e->frame_number) << OFFS_NUM_BITS) | offset);
		//printf("Physical Address: %lX\n", phys_addr);
	}
}


// Utility function to skip a bunch of lines from the memory trace file
// This method  uses read_file_line() but ignores the result.
void skip_files_lines(char *buff, int buf_size, FILE *fp, int n){
	int count = 0;
	while(count < n){
		buff = fgets(buff, buf_size, fp);
		count++;
	}
}

// Technical detail, see the assignment PDF for more details.
int parse_address(char *buff){

	char *type = strtok(buff, " "); // Parse / throw away the preceeding I / L / S indicator
	char *addr_str = strtok(NULL, " "); // Parse the actual address (as a string)
	addr_str = strtok(addr_str, ","); // Parse / throw away the trailing ",2\n"
	unsigned long long addr_long = hex2dec(strlen(addr_str), addr_str); // Convert to decimal value

	// Move inside smaller (8 bit) addres space.  These numbers are in a 48 bit address space
	int addr = addr_long % 256;
	return addr;
}


// Main program, reads the memory trace file line by line, simulates 
// memory
int main(int argc, char *argv[]){

	if(argc != 2)
	{
		printf("Invalid usage!\nCorrect usage example:\n\tmachine <num_addresses>\n");
		exit(1);
	}

	// The total number of frames in the system (start counting from 0)
	// With pages that are 32B, each page is exactly 1/4 of RAM, 
	//	so there are 4 frames
	// With pages that are 32B, each page is exactly 1/8 of the Virtual Address Size, 
	//	so there are 8 page table entries
	NUM_FRAMES = RAM_SIZE / PAGE_SIZE; // total number of frames
	NUM_PAGES = VIRT_ADDR_SIZE / PAGE_SIZE; // total number of pages (virtual frames)

	ADDR_NUM_BITS = ADDR_SIZE * 8;
	PAGE_NUM_BITS = (int)(log(NUM_PAGES) / log(2));
	OFFS_NUM_BITS = ADDR_NUM_BITS - PAGE_NUM_BITS;

	D
	{
		printf("ADDR_SIZE: %lu\n", ADDR_SIZE);
		printf("VIRT_ADDR_SIZE: %lu\n", VIRT_ADDR_SIZE);
		printf("RAM_SIZE: %lu\n", RAM_SIZE);
		printf("PAGE_SIZE: %lu\n", PAGE_SIZE);
		printf("NUM_FRAMES: %u\n", NUM_FRAMES);
		printf("NUM_PAGES: %u\n", NUM_PAGES);
		printf("ADDR_NUM_BITS: %lu\n", ADDR_NUM_BITS);
		printf("PAGE_NUM_BITS: %lu\n", PAGE_NUM_BITS);
		printf("OFFS_NUM_BITS: %lu\n", OFFS_NUM_BITS);
		printf("\n");
	}


	// Initialize page table
	pagetable = malloc(sizeof(struct page_table_entry) * NUM_PAGES);
	int i;
	for(i = 0; i < NUM_PAGES; i++){
		pagetable[i].frame_number = 0;
		pagetable[i].present = 0;
		pagetable[i].hits = 0;
		pagetable[i].R = 0;
		pagetable[i].C = 0;
	}


	// open / prepare to read instructions file
	FILE *fp = fopen("./instructions.txt", "r"); // FILE *fp so I can use fgets
	int buf_size = 80;
	char *buff = malloc(sizeof(char) * buf_size);
	skip_files_lines(buff, buf_size, fp, 5);

	initialize_student_code(); // <-- Student Code!

	C = 0;
	int addr;
	while(C < atoi(argv[1]))
	{
		// Read address from file
		buff = fgets(buff, buf_size, fp);
		// This indicates the end of the instructions.txt file
		if(buff[0] == '=') break;
		addr = parse_address(buff);

		if(C % 25 == 0) reset_R_bits();

		C++;
		D printf("Address: %d\n", addr);
		D dump_page_table(0, NUM_PAGES);
		mmu(addr);
		D printf("Miss Percentage: %0.2f\n", ((double)misses/(double)C));
		D printf("\n");
	}


	printf("Total time waiting for disk: %llu microseconds (%.3f seconds).\n",  TIME, TIME / 1000000.0);
	free(pagetable);

	return 0;
}