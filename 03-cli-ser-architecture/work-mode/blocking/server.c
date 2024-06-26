#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

/*
2 argument
    argv[0] - filename
    argv[1] - port
*/

int main(int argc, char *argv[])
{
    // create socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed\n");
        return 1;
    }
    // default mode: blocking

    // socket address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // binding
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed\n");
        return 1;
    }

    // waiting for connection
    if (listen(listener, 5))
    {
        perror("listen() failed\n");
        return 1;
    }

    int clients[64]; // array of clients
    int numClients = 0;
    char buf[256]; // buffer for recv

    while (1)
    {
        printf("waiting for connection...\n");
        int client = accept(listener, NULL, NULL);

        clients[numClients] = client; // socket number
        numClients++;

        printf("waiting data...\n");
        for (int i = 0; i < numClients; i++)
        {
            int ret = recv(clients[i], buf, sizeof(buf), 0);
            if (ret <= 0)
                continue;

            buf[ret] = 0;
            printf("client %d: %s\n", clients[i], buf);
        }
    }
    // after recv data from 1 client, while loop makes listener wait for another connect -> BLOCKING recv

    close(listener);
    return 0;
}

// gcc server.c -o server
// ./server 8000