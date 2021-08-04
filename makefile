CC=gcc
CFLAGS=-g -I. -lpthread -lrt

DEPS = chc_compiler/parser.tab.h chc_compiler/ast.h chc_compiler/display.h chc_compiler/semantics.h chc_compiler/ir_generator.h chc_compiler/hr_interpreter.h chc_compiler/code_generator.h chc_compiler/code_interpreter.h chc_compiler/code_output.h cpu.h 2by2sim.h

OBJ = cpu_F.o 2by2sim.o

OBJH = cpu_H.o many_core.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sim: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

sim_h: $(OBJH)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o sim sim_h
