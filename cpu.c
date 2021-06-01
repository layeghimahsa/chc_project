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
	
	if(CPU->has_dependent == true){
		int operation = CPU->code[4];
		switch(operation)
		{
			case code_input: 		printf("\t<< "); scanf("%d", CPU->code[2]); break;
			case code_plus: 		CPU->code[2] = CPU->code[6]+ CPU->code[7]; break;
			case code_minus:		CPU->code[2] = CPU->code[6] - CPU->code[7]; break;
			case code_times:		CPU->code[2] = CPU->code[6] * CPU->code[7]; break;
			case code_is_equal:		(CPU->code[6] == CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
			case code_is_less:		(CPU->code[6] < CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
			case code_is_greater:	(CPU->code[6] > CPU->code[7]) ? (CPU->code[2]=1): (CPU->code[2]=0); break;
			case code_if:			if((CPU->code[6] != 0))
									{ 
										(CPU->code[2] = CPU->code[7]);
									}
									else
									{ 
										
										//propagate_death(CPU->code[0]); 
										CPU->code[1] = DEAD; 
									} break; 
			case code_else:			if(CPU->code[6] == 0)
									{
										(CPU->code[2] = CPU->code[7]);
									}
									else
									{ 
										
										//propagate_death(CPU->code[0]);
										CPU->code[1] = DEAD; 
									} break;

			//TODO: Fix merge so it has a single argument
			case code_merge:		CPU->code[2] = (CPU->code[6] | CPU->code[7]); break;
			//case code_identity:		CPU->code[2] = CPU->code[6]; break;
			default: printf("Error: unknown code found during interpretation\n");
		}
		
		output->value = CPU->code[2];
		
	} else{
		output->value = CPU->code[2]; //the value of the node
	}
	
	
	/* ************ returning cpu_output destination and address ********************** */
	
	output->dest = CPU->cpu_dest;//cpu number, it is either a cpu number or -99 for writing back to memory
	output->addr = CPU->code[CPU->node_size-1];//stack destination address, it is either a positive offset or -1 for "writing back to memory" state
	
	//printf("\n\nTESTING OUTPUT RESULT\n\n"); 
	printf("\t CPU %d 's VALUE: %d\n", CPU->assigned_cpu, output->value);
	printf("\t CPU %d 's DEST: %d\n", CPU->assigned_cpu, output->dest);
	printf("\t CPU %d 's ADDR: %d\n", CPU->assigned_cpu, output->addr);
		
	
	return output;
	
	
}




















