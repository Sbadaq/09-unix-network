#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAXLINE 1024

int main(void)
{
    int pipe1[2]; // 父进程写，子进程读
    int pipe2[2]; // 子进程写，父进程读
    pid_t pid;
    char line[MAXLINE];
    
    // 创建两个管道
    if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    
    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // 父进程 - 服务器
        close(pipe1[0]); // 关闭pipe1的读端
        close(pipe2[1]); // 关闭pipe2的写端
        
        // 服务器读取客户端请求
        ssize_t n;
        if ((n = read(pipe2[0], line, MAXLINE)) > 0) {
            line[n] = 0; // 添加字符串结束符
            printf("[Server] Received request: %s", line);
            
            // 处理请求并发送响应
            char response[MAXLINE];
            snprintf(response, sizeof(response), "Response to: %s", line);
            write(pipe1[1], response, strlen(response));
        }
        
        close(pipe1[1]);
        close(pipe2[0]);
        
        waitpid(pid, NULL, 0); // 等待子进程结束
    } else { // 子进程 - 客户端
        close(pipe1[1]); // 关闭pipe1的写端
        close(pipe2[0]); // 关闭pipe2的读端
        
        // 客户端发送请求
        char request[] = "Client request 123\n";
        write(pipe2[1], request, strlen(request));
        
        // 客户端接收响应
        ssize_t n;
        if ((n = read(pipe1[0], line, MAXLINE)) > 0) {
            line[n] = 0; // 添加字符串结束符
            printf("[Client] Received response: %s", line);
        }
        
        close(pipe1[0]);
        close(pipe2[1]);
        
        exit(EXIT_SUCCESS);
    }
    
    return 0;
}