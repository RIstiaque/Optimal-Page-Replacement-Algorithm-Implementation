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

void skip_lines(char *buff, int buf_size, FILE *fp, int n){
  int count = 0;
  while(count < n){
    buff = fgets(buff, buf_size, fp);
    count++;
  }
}

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

	// Clever trick to convert ascii numbers to int
	// Each number character is offset from the value of the character '0'
	// Their offset is exactly their numeric value
	return get_c() - '0';
}

// Utility method to convert large hex number to decimal;
unsigned long long hextodec(int input_s, char *input){
	// memory addresses at most 8 bytes and unsigned
	unsigned long long result = 0;
	int exp = 0;
	int i;
	for(i = input_s-1; i >= 0; i--){
		//printf("i: %d  hextoint(input[i]): %d  (int)pow(16, i): %f\n", i, hextoint(input[i]), pow(16, exp));
		result = result + hex2int(input[i]) * pow(16, exp++);
	}

	return result >> 5;
}


int parse(char *buff){
  
  char *type = strtok(buff, " "); // Parse / throw away the preceeding I / L / S indicator
  char *addr_str = strtok(NULL, " "); // Parse the actual address (as a string)
  addr_str = strtok(addr_str, ","); // Parse / throw away the trailing ",2\n"
  unsigned long long addr_long = hextodec(strlen(addr_str), addr_str); // Convert to decimal value
  
  // Move inside smaller (8 bit) addres space.  These numbers are in a 48 bit address space
  int addr = addr_long % 256;
  return addr >> 5;
}


// This function is run by the machine before any addresses are accessed.
void initialize_student_code(){

}

unsigned int select_frame()
{  // Selects a frame to use (implements sme page replacement algorithm)
  int pgArray[4] = {-1, -1, -1, -1};
  int safeArray[4] = {0,0,0,0};
  int i;
  int a = 0;
  for (i=0;i<8;i++)
    {
      struct page_table_entry *e = get_page_table_entry(i);
      if (e->present == 1)
	{
	  pgArray[a] = i; // Make sure we can c r w/ future page #
	  a++;
	}
    }

  if (a<4)
    {
      return a;
    }
  FILE *fp = fopen("./instructions.txt", "r"); // FILE *fp so I can use fgets
  int buf_size = 80;
  char *buff = malloc(sizeof(char) * buf_size);
  skip_lines(buff, buf_size, fp, 4 + get_c());
  int pageNum = 0;
  int safe = 0;
  a = 0;
  /* printf("Right before the while loop\n"); */
  while (safe < 2 && a<100001)
    {
      buff = fgets(buff, buf_size, fp);
      // This indicates the end of the instructions.txt file
      if(buff[0] == '=') break;
      pageNum = parse(buff);
      for (i=0;i<4;i++)
  	{
  	  if (pageNum == pgArray[i]) // Same pg #
  	    {
  	      if (safeArray[i] == 0) // Previous not marked safe.
  		{
  		  safeArray[i] = 1;
  		  safe++;
  		}
  	    }
  	}
      /* for (i=0;i<4;i++) */
      /* 	{ */
      /* 	  printf("pg%d = %d,", i, pgArray[i]); */
      /* 	} */
      /* printf("\n%d    %d\n", a, get_c()); */
      /* for (i=0;i<4;i++) */
      /* 	{ */
      /* 	  printf("safe%d = %d,", i, safeArray[i]); */
      /* 	} */
      /* printf("\n"); */
      a++;
    }
  fclose(fp);
  for (i=0;i<4;i++)
    {
      if (safeArray[i] == 0)
  	{
	  /* printf("Got to this part so p1 \n"); */
  	  pageNum = pgArray[i];
  	}
    }
  if (a >= 100000)
    {
      int evict = INT_MAX;
      int evictFr = -1;
      for (i=0;i<4;i++)
	{
	  if (safeArray[i] == 0)
	    {
	      /* printf("Got to this part so p1 \n"); */
	      
	      pageNum = pgArray[i];
	      struct page_table_entry *e = get_page_table_entry(pageNum);
	      int entry = e->C;
	      if (entry < evict)
		{
		  evict = entry;
		  evictFr = i;
		}
	      
	    }
	}
      return evictFr;
    }
  struct page_table_entry *e = get_page_table_entry(pageNum);
  return e->frame_number;
}
