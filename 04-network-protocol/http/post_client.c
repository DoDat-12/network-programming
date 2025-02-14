#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct addrinfo *info;
    if (getaddrinfo("httpbin.org", "http", NULL, &info)) {
        puts("getaddrinfo() failed\n");
        return 1;
    }

    int ret = connect(client, info->ai_addr, info->ai_addrlen);
    if (ret == -1) {
        perror("connect() failed\n");
        return 1;
    }

    char req[] = "POST /post HTTP/1.1\r\nHost: httpbin.org\r\nContent-Length: 27\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nparam1=value1&param2=value2";
    send(client, req, strlen(req), 0);

    char buf[2048];
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    printf("%d bytes received\n%s\n", ret, buf);

    close(client);
}