#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h> // for non-blocking mode
#include <errno.h>

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
    if (ret == -1)
    {
        perror("connect() failed\n");
        return 1;
    }

    // turn client to non-blocking mode
    unsigned long ul = 1;
    ioctl(client, FIONBIO, &ul);
    ioctl(STDIN_FILENO, FIONBIO, &ul); // non-blocking for stdin

    char buf[256];
    while (1)
    {
        // sending
        if (fgets(buf, sizeof(buf), stdin) != NULL)
        {
            send(client, buf, sizeof(buf), 0);
            if (strncmp(buf, "exit", 4) == 0)
                break;
        }

        // recving
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret != -1)
        {
            if (ret == 0)
            {
                printf("disconnect\n");
                break; // disconnect
            }
            buf[ret] = 0;
            printf("recv: %s\n", buf);
        }
        else
        {
            if (errno == EWOULDBLOCK)
            {
                // recv nothing
                // do nothing
            }
            else
            {
                // error while recv
                printf("error rcving\n");
                break;
            }
        }
    }

    close(client);
    return 0;
}