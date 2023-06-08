#include "tcp_socket.h"

int main(int argc, char **argv)
{
    printf("==================tcp server==================\n");
    int server_fd = tcp_init(NULL, 4321);
    if (server_fd < 0)
    {
        printf("tcp_init error!\n");
        exit(EXIT_FAILURE);
    }

    int client_fd = tcp_accept(server_fd);
    if (client_fd < 0)
    {
        printf("tcp_accept error!\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        char buf[128] = {0};
        
        int recv_len = tcp_blocking_recv(client_fd, buf, sizeof(buf));
        if (recv_len <= 0)
        {
            printf("recv error!\n");
            tcp_close(client_fd);
            tcp_close(server_fd);
            exit(EXIT_FAILURE);
        }
        printf("recv : %s\n", buf);

        int send_len = tcp_send(client_fd, buf, strlen(buf));
        if (send_len <= 0)
        {
            printf("send error!\n");
            tcp_close(client_fd);
            tcp_close(server_fd);
            exit(EXIT_FAILURE);  
        }
        else
        {
            printf("send success! send: %s, send_len: %d\n", buf, send_len);
        }
    }
    tcp_close(server_fd);

    return 0;
}