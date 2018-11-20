
all: wrapper

wrapper:
	g++ -c rapl-wrapper.c -o rapl-wrapper.o -I. -lm 

clean:
	rm -rf *.o
