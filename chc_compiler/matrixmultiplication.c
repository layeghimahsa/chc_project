#include <stdio.h>
#include <stdlib.h>

#define N 4

int main(){

	int find_sum(int *mtrx);
	int recursive_find_sum(int *mtrx, int num);
	int* return_sum_array(int *mtrx);
	//int result;
	//result = loop_recursive(4);
	//printf("%d\n", result );
	//for_loop();
	/*array_operation(mtrx);
	printf("\n\n");

	for (int i = 0; i < N; i++)
      mtrx[i] = 0;

	recursive_array_operation(mtrx,0);
	for (int i = 0; i < N; i++)
      printf("%d  ", mtrx[i]);
	printf("\n\n");*/

	int mtrx[N] = {1,2,3,4};
	int *res= (int *) malloc(N*sizeof(int));
	//find_sum(mtrx);
	int result = recursive_find_sum(mtrx,4);
	//printf("%d\n", result );


	res = return_sum_array(mtrx);
	for(int k=0; k < N; k++){
		printf("%d\n", res[k] );

	}
	return 0;
}

//non recursive
void matrixMultiply(int mtrx1[][N], int mtrx2[][N], int result[][N]){
	int i,j,k;
	for(i = 0; i< N; i++){
		for(j=0; j < N; j++){
			result[i][j] = 0;
			for(k=0; k < N; k++){
				result[i][j] += mtrx1[i][k]*mtrx2[k][j];

			}
		}
	}
}


//recursive
void multiplyMatrix(int A[][N],
                    int B[][N],
                  	int C[][N])
{
    static int i = 0, j = 0, k = 0;

    // If all rows traversed.
    if (i >= N)
        return;

    // If i < N
    if (j < N)
    {
      if (k < N)
      {
         C[i][j] += A[i][k] * B[k][j];
         k++;

         multiplyMatrix(A, B, C);
      }

      k = 0;
      j++;
      multiplyMatrix(A, B, C);
    }

    j = 0;
    i++;
    multiplyMatrix(A, B, C);
}

// Function to multiply two matrices A[][] and B[][]
void multiplyMatrix(int row1, int col1, int A[][MAX],
                    int row2, int col2, int B[][MAX])
{
    if (row2 != col1)
    {
        printf("Not Possible\n");
        return;
    }

    int C[MAX][MAX] = {0};

    multiplyMatrixRec(row1, col1, A, row2, col2, B, C);

    // Print the result
    for (int i = 0; i < row1; i++)
    {
        for (int j = 0; j < col2; j++)
            printf("%d  ", C[i][j]);

        printf("\n");
    }
}

// Driven Program
/*int main()
{
    int A[][MAX] = { {1, 2, 3},
                    {4, 5, 6},
                    {7, 8, 9}};

    int B[][MAX] = { {1, 2, 3},
                    {4, 5, 6},
                    {7, 8, 9} };

    int row1 = 3, col1 = 3, row2 = 3, col2 = 3;
    multiplyMatrix(row1, col1, A, row2, col2, B);

    return 0;
}
*/


void array_operation(int *mtrx){
			//takes a N*N matrix and adds one to each element
			//int i;

			//this is a data parallel loop; no data is being reused and the loop is just for traversing the array

			/*for(i = 0; i<N; i++){
				mtrx[i] += 1;
			}*/

			mtrx[0] += 1;
			mtrx[1] += 1;
			mtrx[2] += 1;
			mtrx[3] += 1;


			for (int i = 0; i < N; i++)
		      printf("%d  ", mtrx[i]);

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
