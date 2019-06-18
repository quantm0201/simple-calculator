all: calc.o hsm.o
	gcc calc.o hsm.o -o calc -lm
calc.o: calc.c hsm.h
	gcc -c calc.c -lm
hsm.o: hsm.c hsm.h
	gcc -c hsm.c
clean:
	rm -f *.o calc
