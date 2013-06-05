all: mkbkp
FLAGS = -Wall -L./ -g -std=gnu99

mkbkp: mkbkp.c mkbkp.h
	gcc $(FLAGS) -o mkbkp mkbkp.c  

clean:
	rm -f *.o 
	rm -f a.out
	rm -f *~
	rm -f mkbkp
	rm -f *a