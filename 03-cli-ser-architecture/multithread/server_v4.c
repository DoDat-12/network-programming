// Multithread time server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

void *client_proc(void *);

int clients[64];

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
        perror("bind() failed");
        return 1;
    }

    // waiting for connection
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client);
        pthread_detach(tid);
    }

    return 0;
}

void *client_proc(void *arg)
{
    int client = *(int *)arg;
    char buf[256];

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;

        buf[ret] = 0;
        printf("client %d: %s\n", client, buf);

        char cmd[32], format[32], tmp[32];
        int n = sscanf(buf, "%s%s%s", cmd, format, tmp);
        if (n != 2)
        {
            char *msg = "wrong syntax\n";
            send(client, msg, strlen(msg), 0);
        }
        else
        {
            if (strcmp(cmd, "GET_TIME") != 0)
            {
                char *msg = "wrong syntax\n";
                send(client, msg, strlen(msg), 0);
            }
            else
            {
                time_t t = time(NULL);
                struct tm *tinfo = localtime(&t);

                if (strcmp(format, "dd/mm/yyyy\n") == 0)
                {
                    sprintf(buf, "%02d/%02d/%4d",
                            tinfo->tm_mday, tinfo->tm_mon + 1, tinfo->tm_year + 1900);
                    send(client, buf, strlen(buf), 0);
                }
                else if (strcmp(format, "dd/mm/yy\n") == 0)
                {
                    sprintf(buf, "%02d/%02d/%02d",
                            tinfo->tm_mday, tinfo->tm_mon + 1, tinfo->tm_year - 100);
                    send(client, buf, strlen(buf), 0);
                }
                else if (strcmp(format, "mm/dd/yyyy\n") == 0)
                {
                    sprintf(buf, "%02d/%02d/%4d",
                            tinfo->tm_mon + 1, tinfo->tm_mday, tinfo->tm_year + 1900);
                    send(client, buf, strlen(buf), 0);
                }
                else if (strcmp(format, "mm/dd/yy\n") == 0)
                {
                    sprintf(buf, "%02d/%02d/%02d",
                            tinfo->tm_mon + 1, tinfo->tm_mday, tinfo->tm_year - 100);
                    send(client, buf, strlen(buf), 0);
                }
                else
                {
                    char *msg = "Dinh dang khong hop le. Hay nhap lai.\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
        }
    }

    close(client);
}