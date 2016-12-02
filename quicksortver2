#include <stdio.h>
#include <stdlib.h>
#include "simulator.h"

//swaps two pointer values
void swap(int A, int B){
	int temp = get(A);
	put(A, get(B));
	put(B,temp);
}

int partition(int begin, int end){
	unsigned int pivotIndex = begin + ((end-begin)/2);
	int pivot = get(pivotIndex);
	swap(pivotIndex,end);
	//swap pivot to end
	unsigned int i = begin;
	unsigned int j = end;
	--end;
	while(begin <= end){
		if(get(begin) >= pivot && get(end) < pivot){
			swap(begin, end);
		}
		if(get(begin) < pivot) ++begin;
		if(get(end) >= pivot) --end;
	}
	//swap pivot back in place
	for(i; i<j; ++i){
		if(get(i)>get(j)){
			swap(i,j);
			break;
		}
	}
	return i;
}

/*quicksort(): in-place implementation of quick sort
Partitioning and finding a pivot point is done with partition()
While running in an infinite while loop, use a small array of indexes to keep track of partitions.
In array, numbers are placed in this order as number ofpartitions grow:{begin1, end1, begin2, end2,...,begin(n), end(n)}
The index for this array is incremented/decremented by 2 when going up/down (simulating returning from recursion)
*/
void quicksort(unsigned int begin, unsigned int end){
	//int partitionIndex[1000]; //Note: Real simulator will have pretty much infinite storage, so remember to 'replace' the 1000.
	int pivot;
	unsigned int indexPos = end+1;
	//Initialize partitionIndex + pivot to begin loop iterations
	put(indexPos, begin);
	put(indexPos+1, end);
	while(indexPos > end){
		//read next partition to sort
		begin = get(indexPos);
		end = get(indexPos+1);
		pivot = partition(begin, end);
		//Base case: Partition has only one number
		if(end-begin==0){
			indexPos -= 2;
			//~ printf("\n");
			continue;
		}
		//Base case: Partition has two numbers
		if(end-begin==1){
			if(get(begin)>get(end)){
				//Swap the two
				//~ printf("Swapped %d and %d\n", A[begin], A[end]);
				swap(begin, end);
			}
			indexPos -= 2;
			continue;
		}
		//Else, find pivot and partition
		pivot = partition(begin, end);
		//place partitions in a stack to be sorted later; smaller one to reduce stack size
		if(pivot-begin>=end-pivot){
			put(indexPos, begin);
			put(indexPos+1, pivot-1);
			put(indexPos+2, pivot);
			put(indexPos+3, end);
		}else{
			put(indexPos, pivot);
			put(indexPos+1, end);
			put(indexPos+2, begin);
			put(indexPos+3, pivot-1);
		}
		indexPos+=2;
	}
}

int main(){
	init(128,1000);
	int N = 100;
	int i;
	for(i = 0; i < N; ++i){
		put(i,rand()%1000);
	}
	for(i = 0; i < N; ++i){
		printf("%d ",get(i));
	}
	quicksort(0,N-1);
	done();
	for(i = 1; i < N; ++i){
		if(get(i-1)>get(i)){
			printf("incorrect!\n");
			break;
		}		
	}
}
