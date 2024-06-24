#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

// threads function
void *client_proc(void *);

int main()
{
    // create socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // socket address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // binding
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed\n");
        return 1;
    }

    // waiting for connection
    if (listen(listener, 10))
    {
        perror("listen() failed\n");
        return 1;
    }

    while (1)
    {
        printf("waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("new client accepted, client = %d\n", client);

        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client);
        pthread_detach(tid);
    }
    return 0;
}

void *client_proc(void *arg)
{
    int client = *(int *)arg;

    char buf[2048];
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        pthread_exit(NULL);
    }

    buf[ret] = 0;
    printf("received from %d: %s\n", client, buf);

    // compare first 6 chars
    if (strncmp(buf, "GET / ", 6) == 0)
    {
        strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Hello World</h1>");
        send(client, buf, strlen(buf), 0);
    }
    else if (strncmp(buf, "GET /image ", 11) == 0)
    {
        strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");
        send(client, buf, strlen(buf), 0);

        FILE *f = fopen("./files/test.jpg", "rb");
        while (1)
        {
            int len = fread(buf, 1, sizeof(buf), f);
            if (len <= 0)
                break;
            send(client, buf, len, 0);
        }
        fclose(f);
    }
    else if (strncmp(buf, "GET /audio ", 11) == 0)
    {
        FILE *f = fopen("./files/test.mp3", "rb");

        fseek(f, 0, SEEK_END);    // seek to end
        long filesize = ftell(f); // size of f
        fseek(f, 0, SEEK_SET);

        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: audio/mp3\r\nContent-Length: %ld\r\n\r\n", filesize);
        send(client, buf, strlen(buf), 0);

        char fbuf[2048];
        while (1)
        {
            int len = fread(fbuf, 1, sizeof(fbuf), f);
            if (len <= 0)
                break;
            send(client, buf, len, MSG_NOSIGNAL); // big file needs MSG_NOSIGNAL to avoid Broken Pipe
        }
        fclose(f);
    }
    else if (strncmp(buf, "GET /video ", 11) == 0)
    {
        FILE *f = fopen("./files/test.mp4", "rb");

        fseek(f, 0, SEEK_END);
        long filesize = ftell(f);
        fseek(f, 0, SEEK_SET);

        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: video/mp4\r\nContent-Length: %ld\r\n\r\n", filesize);
        send(client, buf, strlen(buf), 0);

        while (1)
        {
            int len = fread(buf, 1, sizeof(buf), f);
            if (len <= 0)
                break;
            send(client, buf, len, MSG_NOSIGNAL);
        }
        fclose(f);
    }
    close(client);
    pthread_exit(NULL);
}

// start server: ./server_v1

// request
// -------curl -o ./curl_files/curl.jpg http://localhost:8000/image
// -------use web: http://localhost:8000/image