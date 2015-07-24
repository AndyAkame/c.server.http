/* Common: */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Socketing: */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


#define FOREVAH for(;;)


typedef int SocketHandle;
typedef uint16_t TCPPort;


bool isGoodSocket(SocketHandle handle);
SocketHandle setupAcceptingSocket(TCPPort port);
SocketHandle acceptClients(SocketHandle accepting_socket);
void serveClient(SocketHandle client_socket);
void parseAndReact(SocketHandle client_socket, char* to_parse);


int
main(int argc, char* argv[])
{
    TCPPort port = (argc > 1) ? atoi(argv[ 1 ]) : 8081;
    SocketHandle accepting_socket = setupAcceptingSocket(port);
    SocketHandle client_socket = acceptClients(accepting_socket);
    serveClient(client_socket);
    close(accepting_socket);
    return 0;
}


bool
isGoodSocket(SocketHandle handle)
{
    if (handle < 0)
    {
        return false;
    }
    return true;
}


SocketHandle
setupAcceptingSocket(TCPPort port)
{
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    SocketHandle accepting_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (!isGoodSocket(accepting_socket))
    {
        return -2; // TODO
    }

    int reuse = 1;
    setsockopt(accepting_socket, SOL_SOCKET,
               SO_REUSEADDR, (char*) &reuse, sizeof(int));

    if (!~bind(accepting_socket,
               (struct sockaddr*) &servaddr,
               sizeof(servaddr)))
    {
        return -3; // TODO
    }

    if (!~listen(accepting_socket, SOMAXCONN))
    {
        return -4; // TODO
    }

    return accepting_socket;
}


void
serveClient(SocketHandle client_socket)
{
    struct {
        const size_t recv;
        size_t pars;
        ssize_t received;
        size_t buffered;
    } sizes = {1024, 4096, 0, 0};
    struct {
        char* recv;
        char* pars;
    } buffers = {
        malloc(sizes.recv),
        malloc(sizes.pars)
    };
    char* to_parse;
    while ((sizes.received = recv(client_socket, buffers.recv,
                                  sizes.recv   , 0           )) > 0)
    {
        buffers.recv[ sizes.received ] = '\0';
        if (strstr(buffers.recv, "\r\n\r\n"))
        {
            if (!sizes.buffered)
            {
                to_parse = buffers.recv;
            }
            else
            {
                if ((sizes.buffered + sizes.received) > sizes.pars)
                {
                    sizes.pars <<= 1;
                    buffers.pars = realloc(buffers.pars, sizes.pars);
                    to_parse = strcat(buffers.pars, buffers.recv);
                }
            }
            parseAndReact(client_socket, to_parse);
            sizes.buffered = 0;
            break;
        }
        if ((sizes.buffered + sizes.received) > sizes.pars)
        {
            sizes.pars <<= 1;
            buffers.pars = realloc(buffers.pars, sizes.pars);
        }
        strcat(buffers.pars, buffers.recv);
        sizes.buffered += sizes.received;
    }
    close(client_socket);
}


SocketHandle
acceptClients(SocketHandle accepting_socket)
{
    {
        if (!isGoodSocket(accepting_socket))
        {
            return -1; // TODO
        }
    }
    {
        struct sockaddr_in client_sockaddr_in;
        socklen_t client_sockaddr_in_len = sizeof(client_sockaddr_in);
        SocketHandle accepted_socket;
        FOREVAH
        {
            accepted_socket = (
                accept(accepting_socket,
                       (struct sockaddr*) &client_sockaddr_in,
                       &client_sockaddr_in_len)
            );
            if (!isGoodSocket(accepted_socket))
            {
                return -1; // TODO
            }

            return accepted_socket;
        }
    }
}


void
parseAndReact(SocketHandle client_socket, char* to_parse)
{
    puts(to_parse);
    char* get = strstr(to_parse, "\r\n"); *get = '\0';
    puts(to_parse);
    const char* resp = (
        "HTTP/1.1 200 OK\r\n"
        "Content-type: text/html; charset=utf-8\r\n\r\n"
        "<!doctype html>                             \n"
        "<html>                                      \n"
        "<head> <title>=^.^=</title> </head>         \n"
        "<body><p>really fantastic!</p></body></html>\n"
    );
    send(client_socket, resp, strlen(resp) - 1, 0);
}


