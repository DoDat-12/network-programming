// Using perfork
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define PREFORK 8

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

    char buf[256];
    for (int i = 0; i < PREFORK; i++)
    {
        if (fork() == 0) // child process
        {
            while (1)
            {
                int client = accept(listener, NULL, NULL);
                printf("client %d connected", client);

                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    continue;

                buf[ret] = 0;
                printf("client %d: %s", client, buf);

                close(client);
            }
        }
    }

    getchar();
    killpg(0, SIGKILL);
    return 0;
}