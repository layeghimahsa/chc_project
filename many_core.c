#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#include "many_core.h"
//#include "2by2sim.h"
#include "cpu.h"


//dynamic code array (starts as copy of original code)
int *runtime_code;
//cpu status array
int *cpu_status;
//used to keep track of node numbers
int list_index;
//This is the number of dead nodes (0 destinations) that were removed (needed for node_dest allignment
int nodes_removed;
//the list of all tasks (that have more than 0 destinations)
struct AGP_node *program_APG_node_list;
//main mutex
pthread_mutex_t buss_lock;
//buss master in
struct FIFO *buss_Min;
//buss master out
struct FIFO *buss_Mout;

struct FIFO **buss;

pthread_t *thread_id;

int NUM_CPU;
//FOR OUTPUT DISPLAY
int MESSAGE;
int GRAPH;
struct data_entry **data;
clock_t BEGIN;

//DO NOT REMOVE THE LINE BELLOW!! File may become corrupt if it is (used to write code array in)
//CODE BEGINE//
const int code[] = {//End main:
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0xa4,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x74,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x44,
0x7fffffff,
0x0,
0x1,
0x20,
0xc,
0x0,
0x1,
0x14,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0xc8,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xc,
0x1,
0x0,
0x2,
0xc4,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x24,
0xd,
0x2,
0x0,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x850,
0x1,
0x78,
0x160,
0x4a4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x850,
0x1,
0x78,
0x13c,
0x4a4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x850,
0x1,
0x78,
0x118,
0x4a4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x850,
0x1,
0x78,
0xf0,
0x4a4,
//Start main @(661):
//End single:
0x7fffffff,
0x0,
0x14,
0x20,
0xc,
0x0,
0x1,
0x810,
0x7fffffff,
0x0,
0xfffffffc,
0x24,
0x1,
0x0,
0x2,
0x434,
0x814,
0x7fffffff,
0x2,
0xfffffffc,
0x80,
0x3,
0x2,
0x0,
0x0,
0x17,
0x14,
0x44,
0x74,
0xa4,
0xd4,
0x104,
0x134,
0x164,
0x194,
0x1c4,
0x1f4,
0x224,
0x254,
0x284,
0x2b4,
0x2e4,
0x314,
0x344,
0x374,
0x3a4,
0x3d4,
0x404,
0x464,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0x488,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0x48c,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x1,
0xfffffffc,
0x20,
0xc,
0x1,
0x0,
0x0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x3,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x794,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x770,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x74c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x72c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x70c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x6ec,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x6cc,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x6ac,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x68c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x66c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x64c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x62c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x60c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x5ec,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x5cc,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x5ac,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x58c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x56c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x54c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x52c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x50c,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x4ec,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x4cc,
0x1b4,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0x4ac,
0x1b4,
//Start single @(120):
//End fact:
0x7fffffff,
0x0,
0xfffffffc,
0x28,
0x1,
0x0,
0x3,
0xa8,
0xf4,
0x124,
0x7fffffff,
0x1,
0xfffffffc,
0x28,
0xb,
0x2,
0x0,
0x0,
0x1,
0xffffffff,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x8,
0x2,
0x0,
0x0,
0x1,
0x19c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x9,
0x2,
0x0,
0x0,
0x1,
0x198,
0x7fffffff,
0x2,
0xfffffffc,
0x30,
0x5,
0x2,
0x0,
0x0,
0x3,
0x80,
0x14c,
0x174,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x4,
0x2,
0x0,
0x0,
0x1,
0x148,
0x7fffffff,
0x1,
0xfffffffc,
0x24,
0xc,
0x1,
0x0,
0x1,
0xf0,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0xa,
0x2,
0x0,
0x0,
0x1,
0x7c,
0x7fffffff,
0x2,
0xfffffffc,
0x28,
0x9,
0x2,
0x0,
0x0,
0x1,
0x14,
0x7fffffff,
0x0,
0x0,
0x20,
0xc,
0x0,
0x1,
0x120,
0x7fffffff,
0x0,
0x1,
0x24,
0xc,
0x0,
0x2,
0xa4,
0x170,
0x7fffffff,
0x1,
0xfffffffc,
0x30,
0x0,
0x1,
0x0,
0x1dc,
0x1,
0x0,
0xcc,
0x1b4
//Start fact @(0):
};
int code_size = 788;
int main_addr = 661;
int main_num_nodes = 13;
int dictionary[][3] = {{661,127,13},
{120,541,52},
{0,120,12}
};
int num_dict_entries = 3;
//CODE END//
//DO NOT REMOVE THE LINE ABOVE!!//

/**
 * @brief size function
 *
 * This function is called to calculate the size of each node. (how many entries does the node have)
 * @param [in] addr is the beggining of the node.
 * @param [out] size of the node.
 * @return int
 **/
 int size(int addr){
 	//find size
 	int i = addr + 1;
 	int size = 1;
 	while(code[i] != NODE_BEGIN_FLAG && i < code_size){
 		size++;
 		i++;
 	}
 	return size;
 }

int main(int argc, char **argv)
{
    printf("\n***SIMULATION START***\n\n");
		NUM_CPU;
		MESSAGE = 0;
		GRAPH=0;
		int KG=0;
		int h=0;
		int n = 0;
		int NODE_NUM_MAX = 2;

		int opt;
		while ((opt = getopt(argc, argv, "mhngK:")) != -1) {
             switch (opt) {
             case 'm':
                 MESSAGE = 1;
                 break;
						 case 'h':
 								 h=1;
 								 break;
						 case 'n':
						 		 n = 1;
								 break;
						 case 'g':
						 		 GRAPH = 1;
								 break;
             default: /* '?' */
                 printf("Usage: %s [-m] [-g] [-h] [-n] num_cpu\n",argv[0]);
                 exit(EXIT_FAILURE);
             }
    }
		if(h==1){
			printf("Usage: ./sim [-m] [-n] [-g] [-h] num_cpu  (ex: ./sim 4 or ./sim -m 4)\n[-m]: Display all core messages\n[-n]: Display node details\n[-g]: Create graphs and save them to pdf (Requires gnuplot)\n[-h]: Display all options\n\n");
			return 0;
		}
		if (optind >= argc) {
             fprintf(stderr, "Expected argument!\nhint: add -h to see all options\n\n");
             exit(EXIT_FAILURE);
  	}else{
			NUM_CPU = atoi(argv[optind]);
		}

    if(NUM_CPU < 1){
			printf("NUM CPU %d\n",NUM_CPU);
			printf("YOU MUST HAVE AT LEAST 1 CPU\n");
			return 1;
    }
    double sq = sqrt(NUM_CPU);
    if(sq != (int)sq){
      printf("There must be NxN cores defined.\nex: 1,4,9,16\n");
      return 0;
    }

    //create array of thread id
    thread_id = (pthread_t *) malloc(NUM_CPU*sizeof(pthread_t));

    buss = (struct FIFO**) malloc(NUM_CPU*sizeof(struct FIFO*));
    for(int i = 0; i<NUM_CPU;i++){
      buss[i] = create_FIFO();
    }

		//create cpu struct
    struct CPU_SA *cpus[NUM_CPU];
    for(int i = 0; i<NUM_CPU; i++){
        struct CPU_SA *cpu_t = (struct CPU_SA*) malloc(sizeof(struct CPU_SA));
        cpu_t->cpu_num = i+1;
				cpu_t->main_addr = main_addr;
				cpu_t->num_dict_entries = num_dict_entries;
        cpu_t->PM = (int *) malloc(ADDRASABLE_SPACE*sizeof(int));
				cpu_t->dictionary = dictionary;
        cpu_t->num_cpu = NUM_CPU;
				cpu_t->code_size = 0;
        cpu_t->routing_table = set_up_routing_table(i+1,buss);
        cpus[i] = cpu_t;
    }

		/**********************************************************************************************************/
		int num_nodes_to_make = 0;
		for(int i = 0; i<num_dict_entries; i++){
				num_nodes_to_make += dictionary[i][2];
		}

		//using random coloruing for all nodes (including those in subgraph)
		srand(time(NULL)); //resetting the seed to avoid same result.
		printf("total number of nodes: %d\n", num_nodes_to_make);
		int colouring_random[num_nodes_to_make];//holds node allocations to cpus.
		int counter = 0;
		for(int i = 0; i< num_nodes_to_make; i++){
				colouring_random[i] = rand() % NUM_CPU; //random allocation
				printf("cpu rand[%d]: %d\n", i, colouring_random[i]+1);
				counter++;
		}



		//printf("counter: %d\n", counter);

		int i = 0;
		int node_counter = 0;
		int add;
		int j;
		int rand_cpu;
		int s,si;

    int func_offset = dictionary[0][0];
    int dict_ent = 0;
		//num_nodes_to_make--;
    while(num_nodes_to_make != 0){
      rand_cpu = colouring_random[node_counter];
      j = cpus[rand_cpu]->code_size;
      s = j+4;
      cpus[rand_cpu]->PM[j] = code[i]; //this is the new node flag
			cpus[rand_cpu]->PM[j+1] = code_size - i; //this is the MM offset
			j+=2;i++;
      add = i+5;
      for(i; i<add; i++){
        cpus[rand_cpu]->PM[j] = code[i];
				j++;
      }
      if(code[i-2]==code_expansion){
        add = i + (code[i-1]*2);
        int suggraph_offset = code[i+(code[i-1]*2)+1];
        for(i; i<add; i+=2){
          cpus[rand_cpu]->PM[j] = code[i];
          cpus[rand_cpu]->PM[j+1] = code[i+1];
          cpus[rand_cpu]->PM[j+2] = colouring_random[find_cpu_num(suggraph_offset,code[i+1])]+1;
  				j+=3;
        }
        add = i + (code[i]*3);
        cpus[rand_cpu]->PM[j] = code[i];
        i++;j++;
        for(i; i<add; i+=3){
          cpus[rand_cpu]->PM[j] = code[i];
          cpus[rand_cpu]->PM[j+1] = code[i+1];
          cpus[rand_cpu]->PM[j+2] = colouring_random[find_cpu_num(func_offset,code[i+1])]+1;
          cpus[rand_cpu]->PM[j+3] = code[i+2];
          cpus[rand_cpu]->PM[j+4] = colouring_random[find_cpu_num(suggraph_offset,code[i+2])]+1;
  				j+=5;
        }
      }else{
        add = i + code[i-1] + 1;
        for(i; i<add; i++){
          cpus[rand_cpu]->PM[j] = code[i];
  				j++;
        }
        add = i + code[i-1];
        for(i; i<add; i++){
          cpus[rand_cpu]->PM[j] = code[i];
          cpus[rand_cpu]->PM[j+1] = colouring_random[find_cpu_num(func_offset,code[i])]+1;
  				j+=2;
        }
      }
      cpus[rand_cpu]->PM[s] = j - cpus[rand_cpu]->code_size;
      cpus[rand_cpu]->code_size += cpus[rand_cpu]->PM[s]+1; //+1 for 1 extra entry and +1 to be one cell past
      cpus[rand_cpu]->PM[j] = func_offset; //this is the function offset this node is placed at node_size-1
      if(i == (code_size-func_offset) && i>0){
        dict_ent++;
        func_offset = dictionary[dict_ent][0];
      }
      node_counter++;
			num_nodes_to_make--;
    }

		/**********************************************************************************************************/

    pthread_mutex_init(&buss_lock, NULL);

		if(MESSAGE == 1)
    	printf("\n\nLAUNCHING THREADS!!!\n\n");


		BEGIN = clock();

    for(int i = 0; i<NUM_CPU; i++){
				pthread_create(&thread_id[i], NULL, &CPU_SA_start, cpus[i]);
    }//*/

    /***********************/
    /**** Simulation end ***/
    /***********************/

		//this should prob be in but it causes a seg fault
    for(int i = 0; i<NUM_CPU; i++){
				//pthread_cancel(thread_id[i]); //cancel all threads
				pthread_join(thread_id[i], NULL); //wait for all threads to clean and cancel safely
    }

		printf("\n***SIMULATION COMPLETE***\n\n");

		clock_t finish = clock();
		double elapsed = (double)(finish - BEGIN)/CLOCKS_PER_SEC;

    //pthread_mutex_destroy(&mem_lock);

		printf("TIME ELAPSED: %f\n\n", elapsed);

    //printf("%d AGP nodes created\n",list_index-1);

    return 0;
}

struct FIFO **set_up_routing_table(int cpu_num, struct FIFO **rt){
  struct FIFO **r_table = (struct FIFO**) malloc(NUM_CPU*sizeof(struct FIFO*));
  int N = sqrt(NUM_CPU);
  //printf("NxN: %d\n",N);
  int table[N][N];
  int num = 1;
  int x,y;
  int up,down,left,right;
  for(int i=0; i<N; i++){
    for(int j=0; j<N; j++){
      table[i][j] = num;
      if(num == cpu_num){
        x=i;y=j;
      }
      num++;
    }
  }
  if(x-1>-1){up = table[x-1][y];}
  if(x+1<N){down = table[x+1][y];}
  if(y-1>-1){left = table[x][y-1];}
  if(y+1<N){right = table[x][y+1];}
  num = 0;
  for(int i=0; i<N; i++){
    for(int j=0; j<N; j++){
      if(i<x){
        r_table[num] = rt[up-1];
        //printf("Num: %d -> %d\n",num,up-1);
      }
      else if(i>x){
        r_table[num] = rt[down-1];
        //printf("Num: %d -> %d\n",num,down-1);
      }
      else if(j<y){
        r_table[num] = rt[left-1];
        //printf("Num: %d -> %d\n",num,left-1);
      }
      else if(j>y){
        r_table[num] = rt[right-1];
        //printf("Num: %d -> %d\n",num,right-1);
      }
      else{
        r_table[num] = rt[cpu_num-1];
        //printf("Num: %d -> %d\n",num,cpu_num-1);
      }
      num++;
    }
  }
  return r_table;
}

int find_cpu_num(int func_offset, int dest_offset){

  if(dest_offset == -1){return 0;}
  int end = code_size - func_offset - dest_offset/4;
  int count=0;
  int i=1;
  while(i!=end){
    if(code[i]==NODE_BEGIN_FLAG){count++;}
    i++;
  }
  return count;
}

struct FIFO *create_FIFO(){
	struct FIFO *fifo = (struct FIFO*)malloc(sizeof(struct FIFO));
	pthread_mutex_init(&fifo->fifo_lock, NULL);
	fifo->front = fifo->back = NULL;
	fifo->size = 0;
	return fifo;
}

void sendMessageOnBuss(int cpu_num,struct Message *m){
  struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *m;
  for(int i=0;i<NUM_CPU;i++){
    if(i!=cpu_num-1){
      pthread_mutex_lock(&buss[i]->fifo_lock);
      sendMessage(buss[i],new);
      pthread_mutex_unlock(&buss[i]->fifo_lock);
    }
  }
}
void sendMessage(struct FIFO *fifo, struct Message *m){
	struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *m;
	if(fifo->back == NULL){
		fifo->front = fifo->back = new;
		fifo->size+=1;
		fifo->message_counter+=1;
	}else{
		fifo->back->next = new;
		fifo->back = fifo->back->next;
		fifo->size+=1;
		fifo->message_counter+=1;
	}
	//printf("message added\n");
}
struct Message *peekMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		return NULL;
	}
	struct Message *new = (struct Message*)malloc(sizeof(struct Message));
	*new = *fifo->front;
	new->next = NULL;
	return new;
}
void removeMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		//return NULL;
		printf("NO MESSAGES TO REMOVE");
	}else{
		struct Message *m = fifo->front;
		fifo->front = fifo->front->next;
		m->next = NULL;

		if(fifo->front == NULL)
			fifo->back = NULL;

		fifo->size-=1;
		free(m);
	}
}
struct Message *popMessage(struct FIFO *fifo){
	if(fifo->front == NULL){
		return NULL;
	}else{
    struct Message *new = (struct Message*)malloc(sizeof(struct Message));
  	*new = *fifo->front;
  	new->next = NULL;

  	struct Message *remove = fifo->front;
  	fifo->front = fifo->front->next;
  	remove->next = NULL;
  	free(remove);

  	if(fifo->front == NULL)
  		fifo->back = NULL;

  	fifo->size-=1;
  	return new;
  }

}

int getFifoSize(struct FIFO *fifo){
	return fifo->size;
}
