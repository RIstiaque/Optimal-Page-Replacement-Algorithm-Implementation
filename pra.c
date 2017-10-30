/* pra.c */

/* Use this file to implement your page replacement algorithm! */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "machine.h"

// This is all your code with names changed
int hex2int(char c){
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
	return c - '0';
}


unsigned long long hextodec(int input_s, char *input){
	// memory addresses at most 8 bytes and unsigned
	unsigned long long result = 0;
	int exp = 0;
	int i;
	for(i = input_s-1; i >= 0; i--){
		result = result + hex2int(input[i]) * pow(16, exp++);
	}

	return result;
}

unsigned long pgNum(unsigned long in){
  unsigned long OFFS_NUM_BITS = 5;
  unsigned long page_num = in >> OFFS_NUM_BITS;
  return page_num;
}

void skip_lines(char *buff, int buf_size, FILE *fp, int n){
	int count = 0;
	while(count < n){
		buff = fgets(buff, buf_size, fp);
		count++;
	}
}

// Technical detail, see the assignment PDF for more details.
int parse(char *buff){

	char *type = strtok(buff, " "); // Parse / throw away the preceeding I / L / S indicator
	char *addr_str = strtok(NULL, " "); // Parse the actual address (as a string)
	addr_str = strtok(addr_str, ","); // Parse / throw away the trailing ",2\n"
	unsigned long long addr_long = hextodec(strlen(addr_str), addr_str); // Convert to decimal value

	// Move inside smaller (8 bit) addres space.  These numbers are in a 48 bit address space
	int addr = addr_long % 256;
	return addr;
}


// This function is run by the machine before any addresses are accessed.
void initialize_student_code(){

}

unsigned int select_frame()
{  // Selects a frame to use (implements sme page replacement algorithm)
  int pgArray[4] = {-1, -1, -1, -1};
  int safeArray[4] = {0,0,0,0};
  int i;
  int pgTableEntries = 0;
  for (i=0;i<8;i++)
    {
      struct page_table_entry *e = get_page_table_entry(i);
      if (e->present == 1)
	{
	  pgArray[pgTableEntries] = i; // Make sure we can cross ref w/ future page #
	  pgTableEntries++;
	}
    }
  if (pgTableEntries<4)
    {
      return pgTableEntries;
    }
  FILE *fp = fopen("./instructions.txt", "r"); // FILE *fp so I can use fgets
  int buf_size = 80;
  char *buff = malloc(sizeof(char) * buf_size);
  skip_lines(buff, buf_size, fp, 5 + get_c());
  int addr = parse(buff);
  unsigned long pageNum = pgNum(addr);
  int safe = 0;
  while (safe < 3) // Took out a < 100000
    {
      buff = fgets(buff, buf_size, fp);
      addr = parse(buff);
      pageNum = pgNum(addr);
      for (i=0;i<4;i++)
  	{
  	  if ((int)pageNum == pgArray[i]) // Same pg #
  	    {
  	      if (safeArray[i] == 0) // Previous not marked safe.
  		{
  		  safeArray[i] = 1;
  		  safe++;
  		}
  	    }
  	}
    }
  fclose(fp);
  for (i=0;i<4;i++)
    {
      if (safeArray[i] == 0)
  	{
  	  struct page_table_entry *e = get_page_table_entry(pgArray[i]);
	  return e->frame_number;
  	}
    }
}
