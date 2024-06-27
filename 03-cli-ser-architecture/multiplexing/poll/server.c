#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h> // for poll()

#define MAX_DFS 2048

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

    // listen
    if (listen(listener, 5))
    {
        perror("listen() failed\n");
        return 1;
    }

    // pollfd init
    struct pollfd fds[MAX_DFS];
    int nfds = 0; // number of fds

    // add socket listener into pollfd
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    nfds++;

    char buf[245];
    while (1)
    {
        int ret = poll(fds, nfds, -1); // -1 so ain't care about timeout
        if (ret == -1)                 // if error
            break;

        else if (ret == 0)
        {
            // used when set timer
            // printf("Time out\n");
            continue;
        }

        for (int i = 0; i < nfds; i++)
        {
            // revents true when event occur, POLLIN true when having data to read
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == listener)
                {
                    // new connection
                    int client = accept(listener, NULL, NULL);
                    if (client >= MAX_DFS)
                        close(client);
                    else
                    {
                        fds[nfds].fd = client;
                        fds[nfds].events = POLLIN;
                        nfds++;
                        printf("client %d connected\n", client);
                    }
                }
                else
                {
                    // got some data to recv
                    int client = fds[i].fd;
                    ret = recv(client, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        // client disconnected
                        fds[i] = fds[nfds - 1];
                        nfds--;
                        i--;
                        printf("client %d disconnected\n", client);
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("client %d:", client);
                        puts(buf);

                        // send to other clients
                        for (int j = 0; j < nfds; j++)
                        {
                            if (fds[j].fd != client && fds[j].fd != listener)
                            {
                                printf("sending to client %d...\n", fds[j].fd);
                                send(fds[j].fd, buf, strlen(buf), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    close(listener);
    return 0;
}