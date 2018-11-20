
all: a.out

wrapper:
	make -C ../ 

a.out: wrapper
	gcc example1.c ../rapl-wrapper.o -I.. -lm 
	gcc thread_based_reading.c -L.. -lraplwrapper -I.. -lm  -lpthread -o thread_based_reading


a.out.2:
	gcc -c thread_based_reading.c -I..
	gcc -L.. -lraplwrapper  -lm  -lpthread -o thread_based_reading thread_based_reading.o

	
run: all
	sudo ./a.out

clean:
	rm -rf a.out

