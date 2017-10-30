/* machine.h */

// This file provides information to pra.c

// The entires in the page table
struct page_table_entry {
	unsigned long frame_number;
	_Bool present;
	unsigned int hits;
	unsigned int R;
	unsigned int C;
};

// Prints the page table from index START to index END
void dump_page_table(int start, int end);

// Returns a struct containing a single row from the page table
struct page_table_entry *get_page_table_entry (int page_num);


// With pages that are 32B, each page is exactly 1/8 of the Virtual Address Size, 
//	so there are 8 page table entries / pages.
// Returns the number of pages in the address space
unsigned int get_num_pages();

// With pages that are 32B, each page is exactly 1/4 of RAM, 
//	so there are 4 frames
// Returns the number of frames in the system
unsigned int get_num_frames();

unsigned int get_c();