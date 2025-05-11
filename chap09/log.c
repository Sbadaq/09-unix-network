/*使用 fcntl 记录锁实现多进程安全写入日志的示例：*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void write_log(const char *filename, const char *message, int pid) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // 定义写锁（独占锁）
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;  // 写锁
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;        // 锁住整个文件
    lock.l_len = 0;

    // 非阻塞获取锁（F_SETLK）或阻塞等待（F_SETLKW）
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("fcntl");
        close(fd);
        exit(1);
    }

    // 写入日志
    char buf[256];
    snprintf(buf, sizeof(buf), "PID %d: %s\n", pid, message);
    write(fd, buf, strlen(buf));
    printf("PID %d wrote: %s", pid, buf);

    lock.l_type = F_UNLCK;    // 释放锁
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("fcntl unlock");
    }

    close(fd);
}


#if 1
/**
 * @brief 主函数，演示进程创建和进程间通信
 *
 * 该程序首先调用fork函数创建一个子进程，然后父进程和子进程分别向同一个日志文件中写入不同的消息。
 *
 * @return 0 表示程序成功执行完毕，1 表示程序执行过程中出现错误
 */
int main() {
    const char *filename = "demo.log";
    pid_t pid = fork();
/* fork - create a child process */
    // 创建一个子进程
    if (pid == 0) {  // 子进程
        // 在子进程中写日志
        write_log(filename, "Hello from child!", getpid());
    } else if (pid > 0) {  // 父进程
        // 在父进程中写日志
        write_log(filename, "Hello from parent!", getpid());
        // 等待子进程结束
        wait(NULL);
    } else {
        // fork函数调用失败，输出错误信息
        perror("fork");
        return 1;
    }
    return 0;
}
#endif

#if 0
int main() {
    int x = 100;  // 父子进程初始共享同一物理内存页
    pid_t pid = fork();
    //On  success, the PID of the child process is returned in the parent, 
    //and 0 is returned in the child.
    if (pid == 0) {  // 子进程
        x = 200;     // 触发 COW，子进程的 x 指向新物理页
        printf("i am a Child x = %d\n", x);  // 输出 200
    } else {         // 父进程
        sleep(1);    // 等待子进程先执行
        printf("Parent x = %d\n aha, i be a father!!! i borned a baby %d\n", x,pid); // 输出 100（原物理页未修改）
    }
    return 0;
}
#endif