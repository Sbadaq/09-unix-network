#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#define SERVER_FIFO "/tmp/server_fifo"
#define MAXLINE 1024
#define CLIENT_FIFO_TEMPLATE "/tmp/client_%d_fifo"

void cleanup() {
    unlink(SERVER_FIFO);
}

typedef struct {
    pid_t pid;
    char data[MAXLINE];
} Request;

int main() {
    mkfifo(SERVER_FIFO, 0666);
    atexit(cleanup);
    
    int server_fd = open(SERVER_FIFO, O_RDONLY);
    printf("Server started, waiting for requests...\n");
    
    while (1) {
        Request req;
        if (read(server_fd, &req, sizeof(Request)) > 0) {
            printf("[Server] Received request from PID %d: %s", req.pid, req.data);
            
            // 处理请求
            char client_fifo[256];
            snprintf(client_fifo, sizeof(client_fifo), CLIENT_FIFO_TEMPLATE, req.pid);
            
            // 打开客户端FIFO（可能需创建）
            int client_fd = open(client_fifo, O_WRONLY);
            if (client_fd == -1) {
                if (mkfifo(client_fifo, 0666) == -1) {
                    perror("mkfifo");
                    continue;
                }
                client_fd = open(client_fifo, O_WRONLY);
            }
            
            // 发送响应
            char response[MAXLINE];
            snprintf(response, sizeof(response), 
                    "Response to PID %d: %s", req.pid, req.data);
            write(client_fd, response, strlen(response)+1);
            
            close(client_fd);
        }
    }
    
    close(server_fd);
    return 0;
}