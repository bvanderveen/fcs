yajl-static:
	cd yajl && cmake . && make

fcs-main: yajl-static
	gcc -g -std=c99 -o fcs-main.o \
	-Iyajl/yajl-2.1.1/include \
	yajl/yajl-2.1.1/lib/libyajl_s.a \
	fcs/debug.c \
	fcs/state.c \
	fcs/json.c \
	fcs/net/udp_socket.c \
	fcs/net/json_socket.c \
	fcs/net/xplane_socket.c \
	fcs/bus/xplane.c \
	fcs/bus/message.c \
	main.c

fcs-test: yajl-static
	gcc -g -std=c99 -o fcs-tests.o \
	-DDEBUG \
	-Iyajl/yajl-2.1.1/include \
	yajl/yajl-2.1.1/lib/libyajl_s.a \
	fcs/debug.c \
	fcs/state.c \
	fcs/json.c \
	fcs/net/udp_socket.c \
	fcs/net/json_socket.c \
	fcs/net/xplane_socket.c \
	fcs/bus/xplane.c \
	fcs/bus/message.c \
	tests.c

run: fcs-main
	./fcs.o

test: fcs-test
	./fcs-tests.o