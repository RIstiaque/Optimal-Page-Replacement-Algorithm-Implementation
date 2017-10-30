/* sort.c */

// This program generates sorts the following list of random numbers;

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// Utility / debug function only used to check array is sorted
void print_arr(int *arr, int size)
{
	printf("[");
	int i;
	for(i = 0; i < size; i++){
		printf("%d, ", arr[i]);
	}
	printf("]\n");
}

void swap(int *xp, int *yp)
{
  int temp = *xp;
  *xp = *yp;
  *yp = temp;
}

void bubbleSort(int *arr, int size){
	int i;
	int j;
	for(j = 0; j < (size-1); j++){
		for(i = 0; i < (size-1); i++){
			if(arr[i] > arr[i+1]){
				swap(&arr[i], &arr[i+1]);
			}
		}
	}
}

int main(void)
{
	int n = 100;
	int *arr = malloc(sizeof(int) * n);

	srand(time(NULL));
	int i;
	for(i = 0; i < n; i++){
		arr[i] = rand() % 200;
	}

	printf("Before\n");
	print_arr(arr, n);
	printf("\n");

	bubbleSort(arr, n);
	printf("After\n");
	print_arr(arr, n);
	printf("\n");

	free(arr);

	return 0;
}
