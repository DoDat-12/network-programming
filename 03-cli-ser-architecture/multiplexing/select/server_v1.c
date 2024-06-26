#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h> // for select function

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

    // fd_set initialization
    fd_set fdread;

    int clients[FD_SETSIZE]; // array of client socket values
    int numClients = 0;

    int maxSocketValue;
    char buf[256]; // buffer for data

    while (1)
    {
        FD_ZERO(&fdread);          // init fdread set
        FD_SET(listener, &fdread); // add socket listener to fd_set

        // find max socket value
        maxSocketValue = listener;
        for (int i = 0; i < numClients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (clients[i] > maxSocketValue)
                maxSocketValue = clients[i];
        }

        // select() for waiting events (block here until event appear)
        int ret = select(maxSocketValue + 1, &fdread, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select() failed\n");
            return 1;
        }

        // connection event
        if (FD_ISSET(listener, &fdread))
        {
            // new connection
            int client = accept(listener, NULL, NULL);
            if (client >= FD_SETSIZE)
                close(client);
            else
            {
                clients[numClients] = client;
                numClients++;
                printf("client %d connected\n", client);
            }
        }

        // recv data events
        for (int i = 0; i < numClients; i++)
        {
            if (FD_ISSET(clients[i], &fdread))
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0) // disconnect
                {
                    printf("client %d disconnected\n", clients[i]);
                    close(clients[i]);

                    if (i < numClients - 1)
                        clients[i] = clients[numClients - 1]; // replace client i with client (numClients - 1) (the last client in array)
                    numClients--;
                    i--;

                    continue;
                }
                else
                {
                    buf[ret] = 0;
                    printf("client %d: %s\n", clients[i], buf);
                }
            }
        }
    }

    return 0;
}

// client: nc -v 127.0.0.1 8000