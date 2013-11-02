yajl-static:
	cmake yajl
	cd yajl && make

fcs-main: yajl-static
	gcc -g -std=c99 -o fcs-main.o \
	-Iyajl/yajl-2.0.5/include \
	yajl/yajl-2.0.5/lib/libyajl_s.a \
	fcs/state.c \
	fcs/udp.c \
	xplane/xplane.c \
	main.c

fcs-test: yajl-static
	gcc -g -std=c99 -o fcs-tests.o \
	-Iyajl/yajl-2.0.5/include \
	yajl/yajl-2.0.5/lib/libyajl_s.a \
	fcs/state.c \
	fcs/udp.c \
	xplane/xplane.c \
	tests.c

run: fcs-main
	./fcs.o

test: fcs-test
	./fcs-tests.o