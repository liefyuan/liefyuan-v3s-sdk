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
    int sockfd;//服务器套接字描述符
    int client_sockfd;//通信套接字描述符
    char buffer[MAX_SIZE];//数据缓冲区
    struct sockaddr_in server_addr;//服务器套接字信息
    struct sockaddr_in client_addr;//客户端套接字信息

    /* 获取服务器套接字文件描述符 */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket error");
        exit(1);
    }

    /* 初始化服务器套接字信息 */
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;//使用IPv4
    server_addr.sin_port = htons(PORT);//对端口进行字节序转化
    server_addr.sin_addr.s_addr = inet_addr(IP);//对IP地址进行格式转化

    /* 因为最后需要服务器主动关闭连接，所以要设置服务器套接字为可重用 */
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* 绑定服务器套接字信息 */
    if (bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr_in)) < 0) {
        perror("bind error");
        exit(1);
    }

    /* 将服务器套接字转化为被动监听 */
    if (listen(sockfd, 3) < 0) {
        perror("listen error!");
        exit(1);
    }

    /* 与客户端进行串行连接 */
    while(1) {
        printf("[server] Server is waiting······\n");

        /* 等待客户端的连接 */
        int len = sizeof(struct sockaddr_in);//通信套接字结构体长度
        memset(&client_addr, 0, sizeof(struct sockaddr_in));
        if ((client_sockfd = accept(sockfd, (struct sockaddr *)(&client_addr), &len)) < 0) {
            perror("accept error");
            exit(1);
        }
        printf("[server] Client's port is %d, ip is %s\n", ntohs(client_addr.sin_port), inet_ntoa(client_addr.sin_addr));

        /* 接收客户端的消息 */
        memset(buffer, 0, MAX_SIZE);
        recv(client_sockfd, buffer, sizeof(buffer), 0);
        printf("[server] Client's message:%s\n", buffer);
        
        /* 向客户端发送消息 */
        send(client_sockfd, "I have received your message.", 30, 0);

        /* 关闭连接 */
        shutdown(client_sockfd, SHUT_RDWR);
    }

    return 0;    
}

