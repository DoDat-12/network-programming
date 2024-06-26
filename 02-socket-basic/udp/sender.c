#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    char buf[256];
    while (1)
    {
        printf("sending: ");
        fgets(buf, sizeof(buf), stdin);
        if (strncmp(buf, "exit", 4) == 0)
            break;

        // sending
        int ret = sendto(sender, buf, strlen(buf), 0,
                         (struct sockaddr *)&addr, sizeof(addr));
        if (ret == -1)
            printf("sendto() failed\n");
        else
            printf("%d bytes send\n", ret);
    }

    return 0;
}