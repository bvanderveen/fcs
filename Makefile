default: 
	gcc -g -std=c99 -o fcs.o \
	main.c \
	fcs/*.c \
	xplane/*.c

run: default
	./fcs.o