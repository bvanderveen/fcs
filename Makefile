default: 
	gcc -g -std=c99 -o fcs \
	main.c \
	core.c \
	energy.c \
	pid_controller.c \
	turn_coordinator.c \
	heading.c \
	config.c \
	xplane.c

run: default
	./fcs