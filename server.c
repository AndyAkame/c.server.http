/* Common: */
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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


int
main(int argc, char* argv[])
{
    TCPPort port = (argc > 1) ? atoi(argv[ 1 ]) : 8081;
    SocketHandle accepting_socket = setupAcceptingSocket(port);
    SocketHandle client_socket = acceptClients(accepting_socket);
    serveClient(client_socket);
    close(client_socket);
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
    const size_t buff_len = 1024;
    int n;
    char buff[ buff_len ] = {0};
    while ((n = recv(client_socket, buff, buff_len, 0)))
    {
        buff[ n ] = '\0';
        printf("%s", buff);
    }
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


