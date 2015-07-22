CC=clang

server: server.c
	${CC} $^ -o $@
