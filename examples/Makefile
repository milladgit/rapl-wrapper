
all: a.out

wrapper:
	make -C ../ 

a.out: wrapper
	gcc example1.c ../rapl-wrapper.o -I.. -lm 
	
run: all
	sudo ./a.out

clean:
	rm -rf a.out

