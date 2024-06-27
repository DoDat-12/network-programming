#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h> // for poll()

int main(int argc, char *argv[])
{
    // create socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // socket address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    // connect
    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));

    // pollfd init
    struct pollfd fds[2]; // client & stdin
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = client;
    fds[1].events = POLLIN;

    char buf[256];
    while (1)
    {
        int ret = poll(fds, 2, -1);
        if (fds[0].revents & POLLIN) // sending data
        {
            fgets(buf, sizeof(buf), stdin);
            fflush(stdin);
            send(client, buf, sizeof(buf), 0);
        }

        else if (fds[1].revents & POLLIN) // recving data
        {
            ret = recv(client, buf, sizeof(buf), 0);
            if (ret <= 0)
                break;
            buf[ret] = 0;
            printf("recv: ");
            puts(buf);
        }
    }

    close(client);
    return 0;
}