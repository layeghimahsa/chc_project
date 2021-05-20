CC=gcc

#CREATE BIN AND BUILD FOLDERS TO SAVE THE COMPILED FILES DURING RUNTIME
bin_folder := $(shell mkdir -p bin)
build_folder := $(shell mkdir -p build)
#results_folder := $(shell mkdir -p simulation_results)

#TARGET TO COMPILE ALL THE TESTS TOGETHER 
# Data Structures

2by2sim.o: 2by2sim.c
	$(CC) -g -c 2by2sim.c -o build/2by2sim.o

cpu.o: cpu.c
	$(CC) -g -c cpu.c -o build/cpu.o

#Compile sim
sim: 2by2sim.o cpu.o
	$(CC) -pthread -g -o bin/2BY2_SIM build/2by2sim.o build/cpu.o

#CLEAN COMMANDS
clean:
	rm -f bin/* build/*




