default: 
	gcc -g -std=c99 -o fcs main.c

run: default
	./fcs