CC=gcc
CFLAGS=-g -I. -lpthread -lrt
DEPS = chc_compiler/parser.tab.h chc_compiler/ast.h chc_compiler/display.h chc_compiler/semantics.h chc_compiler/ir_generator.h chc_compiler/hr_interpreter.h chc_compiler/code_generator.h chc_compiler/code_interpreter.h chc_compiler/code_output.h cpu.h 2by2sim.h
OBJ = chc_compiler/lex.yy.o chc_compiler/parser.tab.o chc_compiler/chc.o chc_compiler/ast.o chc_compiler/display.o chc_compiler/semantics.o chc_compiler/ir_generator.o chc_compiler/hr_interpreter.o chc_compiler/code_generator.o chc_compiler/code_interpreter.o chc_compiler/code_output.o cpu.o 2by2sim.o

chc_compiler/lex.yy.c:	chc_compiler/scanner.l $(DEPS)
	flex chc_compiler/scanner.l

chc_compiler/parser.tab.c: chc_compiler/parser.y
	bison -d chc_compiler/parser.y

chc_compiler/parser.tab.h: chc_compiler/parser.y
	bison -d chc_compiler/parser.y

chc_compiler/parser.tab.o:	chc_compiler/parser.tab.c
	$(CC) -c chc_compiler/parser.tab.c $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chc: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) 
	rm -f *.o  chc_compiler/lex.yy.c chc_compiler/parser.tab.h chc_compiler/parser.tab.c  
	
clean:
	rm -f *.o chc_compiler/lex.yy.c chc_compiler/parser.tab.h chc_compiler/parser.tab.c chc_compiler/chc chc_compiler/chc_output.c chc_compiler/output
