OBJECTS = assembler.o pre_processor.o first_pass.o second_pass.o utilitys.o errors.o
OBJECTS_FLAGS = -c -Wall -ansi -pedantic

assembler: $(OBJECTS)
	gcc -g -Wall -ansi -pedantic $(OBJECTS) -lm -o assembler

assembler.o: assembler.c
	gcc $(OBJECTS_FLAGS) assembler.c -lm 

pre_processor.o: pre_processor.c
	gcc $(OBJECTS_FLAGS) pre_processor.c -lm
	
first_pass.o: first_pass.c
	gcc $(OBJECTS_FLAGS) first_pass.c -lm 
	
second_pass.o: second_pass.c
	gcc $(OBJECTS_FLAGS) second_pass.c -lm 

utilitys.o: utilitys.c
	gcc $(OBJECTS_FLAGS) utilitys.c -lm 
	
errors.o: errors.c
	gcc $(OBJECTS_FLAGS) errors.c -lm 

clean:
	rm -f assembler $(OBJECTS)

