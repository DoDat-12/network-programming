// Multithread chat server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

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

int clientSockets[1024];
char *clientNames[1024];
int numClients = 0;

int remove_client(int client, int *clientSockets, char **clientNames, int *numClients)
{
    int i = 0;
    for (; i < *numClients; i++)
        if (clientSockets[i] == client)
            break;

    if (i < *numClients)
    {
        if (i < *numClients - 1)
        {
            clientSockets[i] = clientSockets[*numClients - 1];
            strcpy(clientNames[i], clientNames[*numClients - 1]);
        }

        free(clientNames[*numClients - 1]);
        *numClients -= 1;
    }
}

void *client_proc(void *arg)
{
    int client = *(int *)arg;
    char buf[256];

    // recving data
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            remove_client(client, clientSockets, clientNames, &numClients);
            break;
        }

        buf[ret] = 0;
        printf("client %d: %s\n", client, buf);

        // login status
        int i = 0;
        for (; i < numClients; i++)
            if (clientSockets[i] == client)
                break;
        if (i == numClients)
        {
            // not login
            char cmd[16], id[32], tmp[32];
            int n = sscanf(buf, "%s %s %s", cmd, id, tmp);
            if (n == 2)
            {
                if (strcmp(cmd, "client_id:") == 0)
                {
                    // check if id exists
                    int k = 0;
                    for (; k < numClients; k++)
                        if (strcmp(clientNames[k], id) == 0)
                            break;
                    if (k < numClients)
                    {
                        // id exists
                        char *msg = "id exists, input another id\n";
                        send(client, msg, strlen(msg), 0);
                    }
                    else
                    {
                        // id not exists
                        char *msg = "sign in successfully\n";
                        send(client, msg, strlen(msg), 0);

                        // turn client to logined
                        clientSockets[numClients] = client;
                        clientNames[numClients] = malloc(strlen(id) + 1);
                        memcpy(clientNames[numClients], id, strlen(id) + 1);
                        // strcpy(client_names[num_clients], id);
                        numClients++;
                    }
                }
                else
                {
                    char *msg = "wrong syntax\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                char *msg = "wrong syntax\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else
        {
            // logined
            char receiver[32];
            sscanf(buf, "%s", receiver);

            if (strcmp(receiver, "all") == 0)
            {
                // send to other clients
                for (int k = 0; k < numClients; k++)
                    if (clientSockets[k] != client)
                    {
                        char msg[512];
                        sprintf(msg, "%s: %s\n", clientNames[i], buf + strlen(receiver) + 1);
                        send(clientSockets[k], msg, strlen(msg), 0);
                    }
            }
            else
            {
                // send to specific client
                for (int k = 0; k < numClients; k++)
                    if (strcmp(clientNames[k], receiver) == 0)
                    {
                        char msg[512];
                        sprintf(msg, "%s: %s\n", clientNames[i], buf + strlen(receiver) + 1);
                        send(clientSockets[k], msg, strlen(msg), 0);
                    }
            }
        }
    }

    close(client);
}

// nc -v 127.0.0.1 8000
// client_id: 12 dodat