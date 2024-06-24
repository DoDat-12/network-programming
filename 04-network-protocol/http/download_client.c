#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct addrinfo *info;
    if (getaddrinfo("storage.googleapis.com", "http", NULL, &info))
    {
        perror("getaddrinfo() failed\n");
        return 1;
    }

    int ret = connect(client, info->ai_addr, info->ai_addrlen);
    if (ret == -1)
    {
        perror("connect() failed\n");
        return 1;
    }

    char req[] = "GET /it4060_samples/ecard.mp4 HTTP/1.1\r\nHost: storage.googleapis.com\r\nConnection: close\r\n\r\n";
    send(client, req, strlen(req), 0);

    char buf[2048];
    char *header = NULL; // header content
    int len = 0;

    while (1)
    {
        // First recv the http respond's header
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            close(client);
            printf("disconnected\n");
            return 1;
        }

        // allocate memory for header
        header = realloc(header, len + ret);
        memcpy(header + len, buf, ret);
        len += ret;

        // need _GNU_SOURCE
        // memmem(string, string-len, sub-string, sub-string-len)
        // find index of sub-string in string
        if (memmem(header, len, "\r\n\r\n", 4) != NULL)
            break;
    }

    puts(header);

    char *pos = (char *)memmem(header, len, "\r\n\r\n", 4);
    FILE *f = fopen("download.mp4", "wb");

    // pos - pointer at "\r\n\r\n"
    // header - pointer at header[0]
    int header_len = pos - header;
    // check if header has only "\r\n\r\n"
    if (header_len + 4 < len)
        fwrite(pos + 4, 1, len - header_len - 4, f);

    while (1)
    {
        // receive the body - the .mp4
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        fwrite(buf, 1, ret, f);
    }

    free(header);
    fclose(f);
    close(client);

    printf("Done\n");
}