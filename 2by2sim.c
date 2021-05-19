#include <stdio.h>
#include <stdlib.h>

const mem_size = 1024;
const data_size = 64; //64-bit instructions 
int main_mem [mem_size];
//extern void execute(memory &);


void populate(){


    int data;
    int index = 0;
    
    FILE *fptr;
  
    fptr = fopen("code.txt", "r") ;

    if ( fptr == NULL )
    {
        printf( "the file failed to open." ) ;
    }
  

    while ( fgets (data, data_size, fptr) != NULL ) {
    	main_mem [index] = data;
    	index++;
    }
       
    fclose(fptr) ;
          
    
    return 0;   
	
	
}

/*
int main()
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, execute, NULL);
    return 0;
}
*/


