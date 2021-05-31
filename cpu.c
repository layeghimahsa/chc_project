#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <pthread.h>
#include <string.h>
#include "cpu.h"
#include "2by2sim.h"


void *CPU_start(struct cpu *CPU){
	
	printf("CPU %d 	START!!\n",CPU->assigned_cpu);
	struct cpu_out *output;
	output = (struct cpu_out *)malloc(sizeof(struct cpu_out));
	
	
	/* ************ returning cpu_output value ********************** */
	
	if(CPU->has_dependent){
		int operation = CPU->code[4];
		switch(operation)
		{
			//case code_input: 		printf("\t<< "); scanf("%d",(sp + 2)); break;
			case code_plus: 		CPU->code[2] = CPU->code[6]+ CPU->code[7]; break;
			case code_times:		CPU->code[2] = CPU->code[6] * CPU->code[7]; break;
			case code_is_equal:		(CPU->code[6] == CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
			case code_is_less:		(CPU->code[6] < CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
			case code_is_greater:	(CPU->code[6] > CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
			/*case code_if:			if((code[6] != 0))
									{ 
										(code[2]=code[7]);
									}
									else
									{ 
										
										propagate_death(sp);
										*(sp+1) = DEAD; 
									} break; 
			case code_else:			if(*(sp + 6) == 0)
									{
										(*(sp + 2)=*(sp + 7));
									}
									else
									{ 
										
										propagate_death(sp);
										*(sp+1) = DEAD; 
									} break; */
			case code_minus:		CPU->code[2] = CPU->code[6] - CPU->code[7]; break;
			//TODO: Fix merge so it has a single argument
			/*case code_merge:		*(sp + 2) = (*(sp + 6) | *(sp + 7)); break;
			case code_identity:		*(sp + 2) = *(sp + 6); break;*/
			default: printf("Error: unknown code found during interpretation\n");
		}
		
		output->value = CPU->code[2];
		
	} else{
		output->value = CPU->code[2]; //the value of the node
	}
	
	
	/* ************ returning cpu_output result ********************** */
	while(CPU->next != NULL){
		
		if(CPU->has_dependent == false){
			CPU = CPU->next;
		}
			
		//to be implemented!
		
	}
	
	return output;
	
	
}




















