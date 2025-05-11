#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define SERVER_FIFO "/tmp/server_fifo"
#define MAXLINE 1024
#define CLIENT_FIFO_TEMPLATE "/tmp/client_%d_fifo"

typedef struct {
    pid_t pid;
    char data[MAXLINE];
} Request;

int main() {
    // 创建唯一的客户端FIFO
    char client_fifo[256];
    snprintf(client_fifo, sizeof(client_fifo), CLIENT_FIFO_TEMPLATE, getpid());
    mkfifo(client_fifo, 0666);
    
    // 打开服务器FIFO发送请求
    int server_fd = open(SERVER_FIFO, O_WRONLY);
    
    Request req;
    req.pid = getpid();
    snprintf(req.data, sizeof(req.data), "Request from PID %d\n", getpid());
    
    // 发送请求
    write(server_fd, &req, sizeof(Request));
    
    // 打开自己的FIFO接收响应
    int client_fd = open(client_fifo, O_RDONLY);
    
    char response[MAXLINE];
    if (read(client_fd, response, MAXLINE) > 0) {
        printf("[Client %d] Received: %s", getpid(), response);
    }
    
    close(client_fd);
    close(server_fd);
    unlink(client_fifo); // 清理
    
    return 0;
}