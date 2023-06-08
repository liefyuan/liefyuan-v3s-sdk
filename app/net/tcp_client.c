#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 2015
#define IP "192.168.1.103"
#define MAX_SIZE 1024

int main() {
    int sockfd;//客户端套接字描述符
    char buffer[MAX_SIZE];//收发数据缓冲区
    struct sockaddr_in server_addr;//服务器套接字描述符

    /* 获取客户端套接字文件描述符 */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    /* 初始化服务器套接字信息 */
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(IP);

    /* 向服务器发送连接请求 */
    if (connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr_in)) < 0) {
        perror("connect error");
        exit(1);
    }
    printf("[client] Connect successfully.\n");

    /* 向服务器发送消息 */
    printf("[client] Please input your message>>>");
    memset(buffer, 0, MAX_SIZE);
    scanf("%[^\n]%*c", buffer);
    send(sockfd, buffer, strlen(buffer), 0);

    /* 接收服务端的消息 */
    memset(buffer, 0, MAX_SIZE);
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("[client] Server's message:%s\n", buffer);

    return 0;
}

