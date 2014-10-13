/*
 *  Created on: November 12, 2012
 *      Author: Gil Mizrahi
 *
 *  Aesthetics modification by Dhole October 2014
 */

// ------------------ Insertion sort -------------------
#ifndef _INSERTION_SORT
#define _INSERTION_SORT

// sorts array using insertion sort algorithm
// gets: array, length of array, compare function and swap function
void insertion_sort(void *array, int length, 
		    int compare(void*, int, int), void swap(void *, int, int));

#endif
