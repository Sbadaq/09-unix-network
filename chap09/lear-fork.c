
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief 主函数
 *
 * 该函数演示了如何在Unix-like系统中创建子进程，并处理子进程退出时的信号。
 *
 * @return 0 表示程序成功退出，非0值表示程序异常退出。
 */
int main(void)
{
    pid_t pid;

    // 为什么要忽略子进程结束信号？
    // 忽略子进程结束信号的原因是为了避免父进程在子进程结束时收到SIGCHLD信号，从而导致父进程阻塞。
    // 忽略SIGCHLD信号可以避免父进程在子进程结束时收到SIGCHLD信号，从而导致父进程阻塞。
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    switch (pid)
    {
    case -1:
        // fork失败的原因可能有哪些？
        // fork失败的原因可能有以下几种：
        // 1. 系统资源不足：如果系统资源不足，fork可能会失败。
        // 2. 进程数达到上限：如果进程数达到上限，fork可能会失败。
        // 3. 进程表已满：如果进程表已满，fork可能会失败。
        perror("fork");
        exit(EXIT_FAILURE);
    case 0:
        // 子进程执行代码
        puts("Child exiting.");
        exit(EXIT_SUCCESS);
    default:
        // 父进程执行代码
        printf("Child is PID %jd\n", (intmax_t)pid);//为什么要用intmax_t 类型，用int不行吗？
        // 为什么要用intmax_t 类型，用int不行吗？
        // 在32位系统中，intmax_t 和 int 的取值范围相同，都是负数到正数的最大值的一半。
        // 在64位系统中，intmax_t 的取值范围是负数到正数的最大值，而 int 的取值范围仍然是负数到正数的最大值的一半。
        
        puts("Parent exiting.");
        exit(EXIT_SUCCESS);
    }
}