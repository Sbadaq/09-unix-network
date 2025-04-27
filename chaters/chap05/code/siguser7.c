#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// 信号处理函数原型
static void sig_rt(int, siginfo_t *, void *);

// 自定义信号注册函数
typedef void (*sigfunc_rt)(int, siginfo_t *, void *);
sigfunc_rt signal_rt(int signo, sigfunc_rt func, sigset_t *mask) {
    struct sigaction act, oact;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = func;
    act.sa_mask = *mask;

    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }

    // 调用 sigaction 注册信号处理函数，并检查错误
    if (sigaction(signo, &act, &oact) < 0) {
        perror("sigaction");
        return SIG_ERR;
    }
    return oact.sa_sigaction;
}

int main(int argc, char *argv[]) {
    int i, j;
    pid_t pid;
    sigset_t mask;
    union sigval value;

    // 输出系统支持的最小和最大实时信号编号
    printf("SIGRTMIN = %ld, SIGRTMAX = %ld\n", (long)SIGRTMIN, (long)SIGRTMAX);

    // 创建子进程
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // 子进程
        // 初始化信号集
        if (sigemptyset(&mask) == -1) {
            perror("sigemptyset");
            return 1;
        }
        // 添加要阻塞的信号
        if (sigaddset(&mask, SIGRTMAX) == -1 ||
            sigaddset(&mask, SIGRTMAX - 1) == -1 ||
            sigaddset(&mask, SIGRTMAX - 2) == -1) {
            perror("sigaddset");
            return 1;
        }
        // 阻塞信号
        if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
            perror("sigprocmask");
            return 1;
        }

        // 注册信号处理函数
        if (signal_rt(SIGRTMAX, sig_rt, &mask) == SIG_ERR ||
            signal_rt(SIGRTMAX - 1, sig_rt, &mask) == SIG_ERR ||
            signal_rt(SIGRTMAX - 2, sig_rt, &mask) == SIG_ERR) {
            return 1;
        }

        // 休眠 6 秒
        sleep(6);
        // 解除信号阻塞
        if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1) {
            perror("sigprocmask");
            return 1;
        }
        // 休眠 3 秒
        sleep(3);
        exit(0);
    } else {
        // 父进程
        // 休眠 3 秒
        sleep(3);
        // 发送 9 个信号给子进程
        for (i = SIGRTMAX; i >= SIGRTMAX - 2; i--) {
            for (j = 0; j < 3; j++) {
                value.sival_int = i + j * 10;
                // 发送信号
                if (sigqueue(pid, i, value) == -1) {
                    perror("sigqueue");
                    return 1;
                }
                printf("sent signal %d, val = %d\n", i, value.sival_int);
            }
        }
        // 等待子进程结束
        wait(NULL);
        exit(0);
    }
}

// 信号处理函数
static void sig_rt(int signo, siginfo_t *info, void *context) {
    printf("caught signal %d, val = %d\n", signo, info->si_value.sival_int);
}
    