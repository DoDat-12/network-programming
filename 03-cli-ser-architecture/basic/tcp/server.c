#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

// 2 arguments: argv[0] - filename
//              argv[1] - ports

int main(int argc, char *argv[])
{
    // create socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("connect() failed\n");
        return 1; // exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // binding
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("bind() failed\n");
        return 1;
    }

    // listen
    if (listen(listener, 5))
    {
        printf("listen() failed\n");
        return 1;
    }
    printf("waiting for connection...\n");

    int client = accept(listener, NULL, NULL);
    if (client == -1)
    {
        printf("accept() failed\n");
        return 1;
    }
    printf("new client connect: %d\n", client);

    char buf[2048];

    // recv from client
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        printf("recv() failed\n");
        return 1;
    }
    buf[ret] = 0;
    printf("client: ");
    puts(buf);

    // send back to client
    send(client, buf, strlen(buf), 0);

    close(client);
    close(listener);

    return 0;
}

// gcc server.c -o server
// ./server 8000 (before running client)