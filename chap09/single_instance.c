/*记录上锁（特别是文件锁）是确保守护程序单实例运行的经典方法。以下是分步说明及示例：

​1. 实现原理
​锁定文件：选择一个固定路径的文件（如 /var/run/my_daemon.lock）。
​独占写锁：启动时尝试获取该文件的 ​写锁（F_WRLCK）​。
​锁的互斥性：若锁已被占用，说明已有实例运行，新进程退出。

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define LOCK_FILE "/tmp/my_daemon.lock"

int acquire_lock() {
    int fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("无法打开锁文件");
        exit(1);
    }
    
    struct flock fl;
    fl.l_type = F_WRLCK;  // 写锁（独占）
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;         // 锁定整个文件

    // 非阻塞方式尝试获取锁
    if (fcntl(fd, F_SETLK, &fl) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            printf("守护程序已在运行，退出。\n");
            close(fd);
            exit(0);
        } else {
            perror("锁操作失败");
            close(fd);
            exit(1);
        }
    }
    else {
        printf("成功获取锁。\n");
    }
    // 成功获取锁，保留文件描述符不关闭
    return fd;
}

void daemon_work() {
    // 守护程序的实际工作（示例：无限循环）
    while (1) {
        sleep(60);
        printf("守护程序运行中...\n");
    }
}

/**
 * @brief 主函数
 *
 * 该函数实现了守护进程的创建和执行。
 *
 * @return int 返回值0表示成功，非0表示失败。
 */
int main() {
    // 1. 获取锁
    // 获取文件锁
    int lock_fd = acquire_lock();

    // 2. 守护进程化（可选）
    // 尝试将当前进程守护进程化
    /* The  daemon()  function  is for programs wishing to detach themselves from the controlling terminal and run in
       the background as system daemons.

       If nochdir is zero, daemon() changes the process's current working directory to the root directory ("/"); oth‐
       erwise, the current working directory is left unchanged.

       If  noclose is zero, daemon() redirects standard input, standard output, and standard error to /dev/null; oth‐
       erwise, no changes are made to these file descriptors.
       */
    // if (daemon(1, 1) == -1) {
    //     // 如果守护进程化失败
    //     perror("守护进程化失败");  // 输出错误信息
    //     printf("守护程序将退出。\n");  // 输出提示信息
    //     close(lock_fd);  // 关闭文件锁
    //     exit(1);  // 退出程序，并返回错误码 1
    // }
    // else {
    //     printf("守护程序已成功启动。\n");  // 输出提示信息，表示守护程序已成功启动
    // }
    // 3. 执行守护程序任务
    // 执行守护程序的核心任务
    daemon_work();

    // 4. 清理（通常不会执行到这里）
    // 在正常情况下，守护程序不会执行到这里
    close(lock_fd);  // 关闭文件锁
    return 0;  // 返回 0，表示程序正常退出
}