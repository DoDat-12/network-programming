#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h> // for select()

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

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);

    // not reset
    // fdtest = fdread;

    char buf[256];
    while (1)
    {
        // reset each loop
        fdtest = fdread;

        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select() failed\n");
            return 1;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            // check event in each component of fdtest
            if (FD_ISSET(i, &fdtest))
            {
                if (i == listener)
                {
                    // new connection
                    int client = accept(listener, NULL, NULL);
                    if (client >= FD_SETSIZE) // max connections, can not add
                        close(client);
                    else
                    {
                        FD_SET(client, &fdtest);
                        printf("client %d connected\n", client);
                    }
                }
                else
                {
                    // i is one of the client
                    int client = i;
                    ret = recv(client, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        printf("client %d disconnected\n", client);
                        close(client);
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("client %d: %s", client, buf);
                    }
                }
            }
        }
    }

    return 0;
}