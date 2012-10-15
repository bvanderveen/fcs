default: 
	gcc -g -std=c99 -o fcs \
	main.c \
	xplane.c

run: default
	./fcs