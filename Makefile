
all: wrapper

wrapper:
	gcc -c rapl-wrapper.c -o rapl-wrapper.o -I. -lm 

clean:
	rm -rf *.o
