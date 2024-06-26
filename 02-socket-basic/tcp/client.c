#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

/*
3 arguments:
    argv[0] - filename
    argv[1] - address
    argv[2] - port
*/

int main(int argc, char *argv[])
{
    // create socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    // connect to server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1)
    {
        printf("connect() failed\n");
        return 1;
    }

    // send to server
    char *msg = "hello server\n";
    send(client, msg, strlen(msg), 0);

    // recv from server
    char buf[2048];
    int len = recv(client, buf, sizeof(buf), 0);
    if (len == -1)
    {
        printf("recv() failed\n");
        return 1;
    }
    buf[len] = 0;
    printf("server: %s\n", buf);

    close(client);
    return 0;
}

// gcc client.c -o client
// ./client 127.0.0.1 8000 (after running server)