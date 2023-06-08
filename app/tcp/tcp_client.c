#include "tcp_socket.h"

int main(int argc, char **argv)
{
    printf("==================tcp cient==================\n");
    if (argc < 3)
    {
        printf("usage:./tcp_client ip port\n");
        exit(EXIT_FAILURE);
    }

    char ip_buf[32] = {0};
    int port = 0;

    memcpy(ip_buf, argv[1], strlen(argv[1]));
    port = atoi(argv[2]);

    int server_fd = tcp_connect(ip_buf, port);
    if (server_fd < 0)
    {
        printf("tcp_connect error!\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        char buf[128] = {0};
        if (scanf("%s", buf))
        {
            int send_len = tcp_send(server_fd, buf, strlen(buf));
            if (send_len <= 0)
            {
                printf("tcp_send error!\n");
                tcp_close(server_fd);
                exit(EXIT_FAILURE);  
            }
            else
            {
                printf("send success! send: %s, send_len: %d\n", buf, send_len);
            }

            bzero(buf, sizeof(buf));
            int recv_len = tcp_blocking_recv(server_fd, buf, sizeof(buf));
            if (recv_len <= 0)
            {
                printf("tcp_blocking_recv error!\n");
                tcp_close(server_fd);
                exit(EXIT_FAILURE);
            }
            printf("recv : %s\n", buf);
        } 
    }

    return 0;
}