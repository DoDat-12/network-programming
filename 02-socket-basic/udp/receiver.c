#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

/*
2 arguments
    argv[0] - filename
    argv[1] - port
*/

int main(int argc, char *argv[])
{
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char buf[256];
    // recv
    printf("receiving...\n");
    while (1)
    {
        int ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);
        if (ret == -1)
            printf("recvfrom() failed\n");
        else
        {
            buf[ret] = 0;
            printf("sender: ");
            puts(buf);
        }
    }

    return 0;
}