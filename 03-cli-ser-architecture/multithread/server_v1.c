#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *clientProc(void *);

int clients[64];

int main(int argc, char *argv[])
{
    // create socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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

    while (1)
    {
        printf("waiting for connection...\n");
        int client = accept(listener, NULL, NULL);
        printf("client %d connected\n", client);

        pthread_t tid;
        pthread_create(&tid, NULL, clientProc, &client);
        pthread_detach(tid);
    }

    return 0;
}

void *clientProc(void *arg)
{
    int client = *(int *)arg;

    char buf[256];
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;

        buf[ret] = 0;
        printf("client %d: %s\n", client, buf);
    }

    close(client);
}