#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *client_proc(void *);

int get_index(int client);
int remove_client(int client, int *client_sockets, char **client_names, int *num_clients);

int check_join(int client);
int process_join(int client, char *buf);
int process_msg(int client, char *buf);
int process_pmsg(int client, char *buf);
int process_op(int client, char *buf);
int process_kick(int client, char *buf);
int process_topic(int client);
int process_quit(int client);

int client_sockets[1024];
char *client_names[1024];
int num_clients = 0;
int owner_client;

int main()
{
    // create socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

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

int remove_client(int client, int *client_sockets, char **client_names, int *num_clients)
{
    int i = 0;
    for (; i < *num_clients; i++)
        if (client_sockets[i] == client)
            break;

    if (i < *num_clients)
    {
        if (i < *num_clients - 1)
        {
            client_sockets[i] = client_sockets[*num_clients - 1];
            strcpy(client_names[i], client_names[*num_clients - 1]);
        }

        free(client_names[*num_clients - 1]);
        *num_clients -= 1;
    }
}

void *client_proc(void *arg)
{
    int client = *(int *)arg;
    char buf[256];

    // recv data
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            remove_client(client, client_sockets, client_names, &num_clients);
            break;
        }

        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

        if (strncmp(buf, "JOIN ", 5) == 0)
            process_join(client, buf);
        else if (strncmp(buf, "MSG ", 4) == 0)
            process_msg(client, buf);
        else if (strncmp(buf, "PMSG ", 5) == 0)
            process_pmsg(client, buf);
        else if (strncmp(buf, "OP ", 3) == 0)
            process_op(client, buf);
        else if (strncmp(buf, "KICK ", 5) == 0)
            process_kick(client, buf);
        else if (strncmp(buf, "TOPIC ", 6) == 0)
            process_topic(client);
        else if (strncmp(buf, "QUIT ", 5) == 0)
            process_quit(client);
        else
        {
            char *msg = "999 Unknown command\n";
            send(client, msg, strlen(msg), 0);
        }
    }

    close(client);
}

int check_join(int client)
{
    int i = 0;
    for (; i < num_clients; i++)
        if (client_sockets[i] == client)
            break;
    return i < num_clients;
}

int get_index(int client)
{
    int i = 0;
    for (; i < num_clients; i++)
        if (client_sockets[i] == client)
            break;
    return i;
}

int process_join(int client, char *buf)
{
    if (!check_join(client))
    {
        // not login yet
        char cmd[16], id[32], tmp[32];
        int n = sscanf(buf, "%s %s %s", cmd, id, tmp);
        if (n == 2)
        {
            // check valid
            // id contains number and normal char
            int k = 0;
            for (; k < strlen(id); k++)
                if (id[k] < '0' || id[k] > 'z' || (id[k] > '9' && id[k] < 'a'))
                    break;
            if (k < strlen(id))
            {
                // not valid
                char *msg = "201 Invalid nickname\n";
                send(client, msg, strlen(msg), 0);
            }
            else
            {
                // check id exists
                k = 0;
                for (; k < num_clients; k++)
                    if (strcmp(client_names[k], id) == 0)
                        break;
                if (k < num_clients)
                {
                    // id exists
                    char *msg = "200 Nickname in use\n";
                    send(client, msg, strlen(msg), 0);
                }
                else
                {
                    // id not exists
                    char *msg = "100 OK\n";
                    send(client, msg, strlen(msg), 0);

                    // turn client to logined
                    client_sockets[num_clients] = client;
                    client_names[num_clients] = malloc(strlen(id) + 1);
                    memcpy(client_names[num_clients], id, strlen(id) + 1);
                    // strcpy(client_names[num_clients], id);
                    num_clients++;

                    // first client -> room owner
                    if (owner_client == 0)
                    {
                        owner_client = client;
                        char *reponse = "You are owner\n";
                        send(client, reponse, strlen(reponse), 0);
                    }

                    // send to other clients
                    for (int k = 0; k < num_clients; k++)
                        if (client_sockets[k] != client)
                        {
                            char msg[512];
                            sprintf(msg, "JOIN %s\n", id);
                            send(client_sockets[k], msg, strlen(msg), 0);
                        }
                }
            }
        }
        else
        {
            char *msg = "999 Unknown error\n";
            send(client, msg, strlen(msg), 0);
        }
    }
    else
    {
        char *msg = "888 Wrong state\n";
        send(client, msg, strlen(msg), 0);
    }

    return 0;
}

int process_msg(int client, char *buf)
{
    if (check_join(client))
    {
        int idx = get_index(client);
        // send to other clients
        for (int k = 0; k < num_clients; k++)
            if (client_sockets[k] != client)
            {
                char msg[512];
                sprintf(msg, "MSG %s %s\n", client_names[idx], buf + 4);
                send(client_sockets[k], msg, strlen(msg), 0);
            }

        char *msg = "100 OK\n";
        send(client, msg, strlen(msg), 0);
    }
    else
    {
        char *msg = "999 Unknown error\n";
        send(client, msg, strlen(msg), 0);
    }
    return 0;
}

int process_pmsg(int client, char *buf)
{
    if (check_join(client))
    {
        int idx = get_index(client);

        char receiver[32];
        sscanf(buf + 5, "%s", receiver);

        // send to specific client
        int k = 0;
        for (; k < num_clients; k++)
            if (strcmp(client_names[k], receiver) == 0)
                break;

        if (k < num_clients)
        {
            // found that client
            char msg[512];
            sprintf(msg, "PMSG %s %s\n", client_names[idx], buf + strlen(receiver) + 6);
            send(client_sockets[k], msg, strlen(msg), 0);

            char *response = "100 OK\n";
            send(client, response, strlen(response), 0);
        }
        else
        {
            // not found
            char *msg = "202 Unknown nickname\n";
            send(client, msg, strlen(msg), 0);
        }
    }
    else
    {
        char *msg = "999 Unknown error\n";
        send(client, msg, strlen(msg), 0);
    }
    return 0;
}

int process_op(int client, char *buf)
{
    if (check_join(client))
    {
        if (client == owner_client)
        {
            // owner
            int idx = get_index(client);

            char new_op[32];
            sscanf(buf + 3, "%s", new_op);

            // gui thong bao cho client chuyen thanh chu phong
            int k = 0;
            for (; k < num_clients; k++)
                if (strcmp(client_names[k], (new_op)) == 0)
                    break;

            if (k < num_clients)
            {
                // Tim thay
                if (client_sockets[k] != client)
                {
                    // khong phai chinh minh
                    char msg[512];
                    sprintf(msg, "OP %s\n", client_names[idx]);
                    send(client_sockets[k], msg, strlen(msg), 0);

                    // cap nhat chu phong
                    owner_client = client_sockets[k];
                    // gui thong bao thanh cong
                    char *response = "100 OK\n";
                    send(client, response, strlen(response), 0);
                }
                else
                {
                    // tu chuyen cho chinh minh
                    char *msg = "203 DENIED\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                // Khong thay nguoi nhan
                char *msg = "202 Unknown nickname\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else
        {
            // khong phai la chu phong
            char *msg = "203 DENIED\n";
            send(client, msg, strlen(msg), 0);
        }
    }
    else
    {
        char *msg = "999 Unknown error\n";
        send(client, msg, strlen(msg), 0);
    }

    return 0;
}

int process_kick(int client, char *buf)
{
    if (check_join(client))
    {
        if (client == owner_client)
        {
            // dung la chu phong
            int idx = get_index(client);

            // lay ra name ng bi kick
            char id_kick[32];
            sscanf(buf + 5, "%s", id_kick);

            int k = 0;
            for (; k < num_clients; k++)
                if (strcmp(client_names[k], (id_kick)) == 0)
                    break;

            if (k < num_clients)
            {
                // Tim thay nguoi bi kick
                if (client_sockets[k] != client)
                {
                    // khong phai chinh minh
                    remove_client(client_sockets[k], client_sockets, client_names, &num_clients);
                    char msg[512];
                    sprintf(msg, "KICK %s %s\n", client_names[k], client_names[idx]);
                    for (int j = 0; j < num_clients; j++)
                    {
                        if (client_sockets[j] != client)
                        {
                            send(client_sockets[j], msg, strlen(msg), 0);
                        }
                    }
                    char *response = "100 OK\n";
                    send(client, response, strlen(response), 0);
                }
                else
                {
                    // tu kick chinh minh
                    char *msg = "203 DENIED\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            else
            {
                // Khong thay nguoi nhan
                char *msg = "202 Unknown nickname\n";
                send(client, msg, strlen(msg), 0);
            }
        }
        else
        {
            // khong phai la chu phong
            char *msg = "203 DENIED\n";
            send(client, msg, strlen(msg), 0);
        }
    }
    else
    {
        char *msg = "999 Unknown error\n";
        send(client, msg, strlen(msg), 0);
    }
    return 0;
}

int process_topic(int client)
{
    return 0;
}

int process_quit(int client)
{
    if (check_join(client))
    {
        int idx = get_index(client);
        // ten nguoi quit
        char id_quit[32];
        memcpy(id_quit, client_names[idx], sizeof(id_quit));

        // thoat
        remove_client(client, client_sockets, client_names, &num_clients);

        if (client == owner_client)
        {
            // nguoi quit la chu phong
            if (num_clients == 0)
                // khong con ai trong phong
                owner_client = 0;
            else
                // chuyen quyen chu phong cho nguoi dau mang
                owner_client = client_sockets[0];
        }

        // gui thong bao cho cac client khac
        char msg[512];
        sprintf(msg, "QUIT %s\n", id_quit);
        for (int j = 0; j < num_clients; j++)
        {
            send(client_sockets[j], msg, strlen(msg), 0);
        }

        // gui thong bao thanh cong
        char *response = "100 OK\n";
        send(client, response, strlen(response), 0);
    }
    else
    {
        char *msg = "999 Unknown error\n";
        send(client, msg, strlen(msg), 0);
    }
    return 0;
}