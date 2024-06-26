#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // create socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1)
    {
        perror("socket() failed\n");
        return 1;
    }

    // socket address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed\n");
        return 1;
    }

    // sending data
    char buf[256];
    while (1)
    {
        printf("input: ");
        fgets(buf, sizeof(buf), stdin);

        send(client, buf, strlen(buf), 0);
        if (strncmp(buf, "exit", 4) == 0)
            break;
    }

    close(client);
    return 0;
}