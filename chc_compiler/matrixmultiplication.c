#include <stdio.h>
#include <stdlib.h>

#define N 4

int main(){

	//A data parallel loop; no data is being reused and the loop is just for traversing the array
  //A processing loop. it is doing the same memory to do some computation

	return 0;
}




int find_sum(int *mtrx){

			int acc = 0;

			//This is processing loop. it is doing the same memory to do some computation
			for(int i = 0; i<N; i++){
				acc += mtrx[i];
			}

			printf("%d\n", acc);
			return acc;
}


int recursive_find_sum(int *mtrx, int num){

		if (num <= 0)
				return 0;
		return recursive_find_sum(mtrx, num-1) + mtrx[num-1];

}


/*
		C code to AGP STEPS:
		1.convert to single assignment
		2.convert if/else to merge
		3.convert loops to recursion
		4.convert recursion to expansion

		if having a for loop -> make the function recursive. (should be the first step)
*/


void for_loop(){
		int i;
		int result;
		for(i = 0; i<N; i++){
			result = result + i;
		}

		printf("%d\n",result);
}

int loop_recursive(int x){
	if(x==0)
		return 0;
	else
		return x + loop_recursive(x-1);
}

/*
	factorial example

	1.for each input, Datum(x) input(x)
	2.for each output, Datum(y) output(y)
	3.for each if condition result, Datum(resulttrue)
	4.for each else condition result, Datum(resultfalse)
	5.for each operation result, Datum(xis0) Datum(xminus1)
	6.for each expansion check (expansion control), Datum(xminus1cond)
	7.for each expansion result, Datum(iter)

*/

int fact(int x){
	if(x==0)
		return 1;
	else
		return x*fact(x-1);
}
