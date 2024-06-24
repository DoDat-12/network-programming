#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <malloc.h>

int main()
{
    // OpenSSL lib init
    SSL_library_init();
    // ssl encode method
    const SSL_METHOD *meth = TLS_client_method();
    // create context
    SSL_CTX *ctx = SSL_CTX_new(meth);

    // ssl pointer - used to send/recv encoded data
    SSL *ssl = SSL_new(ctx);
    if (!ssl)
    {
        printf("error creating ssl\n");
        return 1;
    }

    // create socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct addrinfo *info;

    // source: https://lebavui.github.io/videos/ecard.mp4
    if (getaddrinfo("lebavui.github.io", "https", NULL, &info))
    {
        puts("getaddrinfo() failed\n");
        return 1;
    }

    if (connect(client, info->ai_addr, info->ai_addrlen))
    {
        perror("connect() failed\n");
        return 1;
    }

    // Binding ssl pointer with socket
    SSL_set_fd(ssl, client);
    int err = SSL_connect(ssl); // create ssl connection
    if (err <= 0)
    {
        printf("error creating ssl connection\nerr = %x\n", err);
        return 1;
    }

    char req[] = "GET /videos/ecard.mp4 HTTP/1.1\r\nHost: lebavui.github.io\r\nConnection: close\r\n\r\n";
    SSL_write(ssl, req, strlen(req));

    char buf[2048];

    // get header's content
    char *header = NULL;
    int len = 0;

    while (1)
    {
        int ret = SSL_read(ssl, buf, sizeof(buf));
        if (ret <= 0)
        {
            close(client);
            printf("disconnected\n");
            return 1;
        }

        header = realloc(header, len + ret);
        memcpy(header + len, buf, ret);
        len += ret;

        // check if get all header ("\r\n\r\n" is end of header)
        if (memmem(header, len, "\r\n\r\n", 4) != NULL)
            break;
    }
    puts(header);

    // get video content
    char *pos = memmem(header, len, "\r\n\r\n", 4); // pointer at the end of header
    FILE *f = fopen("test_ssl.mp4", "wb");

    int header_length = pos - header;
    if (header_length + 4 < len)                        // header only has nothing but "\r\n\r\n"
        fwrite(pos + 4, 1, len - header_length - 4, f); // data ssl_read is part of video

    while (1)
    {
        int ret = SSL_read(ssl, buf, sizeof(buf));
        if (ret <= 0)
            break;
        fwrite(buf, 1, ret, f);
    }

    free(header);
    fclose(f);
    close(client);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
}

// gcc test_ssl.c - o test_ssl -lssl
// ./test_ssl