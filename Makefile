default: 
	gcc -g -std=c99 -o fcs.o \
	main.c \
	fcs/*.c \
	xplane/*.c

default-test:
	gcc -g -std=c99 -o fcs-tests.o \
	tests.c \
	fcs/state.c \
	fcs/udp.c

run: default
	./fcs.o

test: default-test
	./fcs-tests.o