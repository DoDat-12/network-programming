#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h> // for non-block mode
#include <errno.h>

int main(int argc, char *argv[])
{
    // create socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed\n");
        return 1;
    }

    // change socket to non-blocking mode
    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

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

    int clients[64]; // array of socket value
    int numClients = 0;
    char buf[256]; // buffer for data

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        // listener is on non-blocking mode -> server run accept & don't wait for listener
        if (client != -1)
        {
            printf("client %d connected\n", client);

            // turn client to non-blocking
            ul = 1;
            ioctl(client, FIONBIO, &ul);

            clients[numClients] = client;
            numClients++;
        }
        else
        {
            if (errno == EWOULDBLOCK)
            {
                // zero client connect
                // do nothing
            }
            else
            {
                // got some error while connecting
                printf("error connecting\n");
                break;
            }
        }

        // recv data
        for (int i = 0; i < numClients; i++)
        {
            int ret = recv(clients[i], buf, sizeof(buf), 0);
            if (ret != -1)
            {
                if (ret == 0) // connection closed
                    continue;

                buf[ret] = 0;
                printf("client %d: %s\n", clients[i], buf);
            }
            else
            {
                if (errno == EWOULDBLOCK)
                {
                    // client've not send anything
                    // do nothing
                }
                else
                {
                    continue; // error while recving
                }
            }
        }
    }

    close(listener);
    return 1;
}