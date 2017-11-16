all: procesosVM swap

procesosVM: procesos.c mmu.o pagefault.o
	gcc -no-pie -o procesosVM procesos.c mmu.o pagefault.o

pagefault.o: pagefault.c mmu.h
	gcc -c pagefault.c


swap:
	./createswap
