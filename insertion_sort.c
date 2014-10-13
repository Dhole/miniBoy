/*
 *  Created on: November 12, 2012
 *      Author: Gil Mizrahi
 *
 *  Aesthetics modification by Dhole October 2014
 */

// ------------------ Insersion sort -------------------
#include "insertion_sort.h"

// sorts array using intersion sort algorithm
// gets: array, length of array, compare function and swap function
void insertion_sort(void *array, int length, 
		    int compare(void*, int, int), void swap(void *, int, int)) {
	int i, j; // loop indexes
	
	for (j = 1; j < length; j++) {// index to insert array
		for (i = j-1; 0 <= i; i--) {
			// compare to previous index and swap if needed
			// else if previous index is smaller, this is my place
			if (!compare(array, i, i+1)) {
				swap(array, i, i+1);
			} else { 
				i = 0; 
			}
			
		}
	}
}
