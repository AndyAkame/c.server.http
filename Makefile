export CC=clang
export CFLAGS=

server: server.c
	${CC} ${CFLAGS} $^ -o $@
