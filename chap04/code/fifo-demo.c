#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
/**FIFO方案与匿名管道的主要区别

    创建方式：

        匿名管道：pipe()系统调用

        FIFO：mkfifo()函数或mkfifo命令

    持久性：

        匿名管道：随进程结束而消失

        FIFO：存在于文件系统中，需要显式删除

    进程关系：

        匿名管道：只能用于有亲缘关系的进程

        FIFO：可用于任意进程间通信
 */
#define SERVER_FIFO "/tmp/server_fifo"
#define CLIENT_FIFO "/tmp/client_fifo"
#define MAXLINE 1024

void remove_fifo() {
    unlink(SERVER_FIFO);
    unlink(CLIENT_FIFO);
}

int main(void)
{
    char line[MAXLINE];
    int readfd, writefd;
    
    // 创建两个FIFO，如果已存在则忽略错误
    mkfifo(SERVER_FIFO, 0666);
    mkfifo(CLIENT_FIFO, 0666);
    
    // 注册退出时清理FIFO
    atexit(remove_fifo);
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // 父进程 - 服务器
        // 服务器打开自己的读管道和客户端的写管道
        readfd = open(SERVER_FIFO, O_RDONLY);
        writefd = open(CLIENT_FIFO, O_WRONLY);
        
        // 读取客户端请求
        ssize_t n;
        if ((n = read(readfd, line, MAXLINE)) > 0) {
            line[n] = 0;
            printf("[Server] Received request: %s", line);
            
            // 处理并发送响应
            char response[MAXLINE];
            snprintf(response, sizeof(response), "Response to: %s", line);
            write(writefd, response, strlen(response));
        }
        
        close(readfd);
        close(writefd);
        waitpid(pid, NULL, 0); // 等待子进程
    } else { // 子进程 - 客户端
        // 客户端打开服务器的写管道和自己的读管道
        writefd = open(SERVER_FIFO, O_WRONLY);
        readfd = open(CLIENT_FIFO, O_RDONLY);
        
        // 发送请求
        char request[] = "Client request via FIFO\n";
        write(writefd, request, strlen(request));
        
        // 接收响应
        ssize_t n;
        if ((n = read(readfd, line, MAXLINE)) > 0) {
            line[n] = 0;
            printf("[Client] Received response: %s", line);
        }
        
        close(readfd);
        close(writefd);
        exit(EXIT_SUCCESS);
    }
    
    return 0;
}