#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

int pipefd[2];
static void sig_user(int);

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <message>\n", argv[0]);
        exit(1);
    }

    mqd_t mqd;
    void *buff;
    ssize_t n;
    struct mq_attr attr;
    struct sigevent sigev;

    // 打开消息队列（阻塞模式）
    mqd = mq_open(argv[1], O_RDONLY);
    if (mqd == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    if (mq_getattr(mqd, &attr) == -1) {
        perror("mq_getattr");
        exit(1);
    }

    buff = malloc(attr.mq_msgsize);
    if (!buff) {
        perror("malloc");
        exit(1);
    }

    // 清空队列中的已有消息
    while ((n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >  0) {
        printf("Received existing message: %.*s\n", (int)n, buff);
    }
    if (errno != EAGAIN) {
        perror("mq_receive");
        exit(1);
    }

    pipe(pipefd);

    // 使用实时信号 SIGRTMIN
    signal(SIGRTMIN, sig_user);
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGRTMIN;

    // 注册通知
    if (mq_notify(mqd, &sigev) == -1) {
        perror("mq_notify");
        exit(1);
    }

    fd_set rfds;
    FD_ZERO(&rfds);

    for (;;) {
        FD_SET(pipefd[0], &rfds);
        printf("Waiting for notification\n");
        if (select(pipefd[0] + 1, &rfds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(pipefd[0], &rfds)) {
            printf("Got notification\n");
            read(pipefd[0], buff, 1); // 清空管道

            // 立即重新注册通知
            if (mq_notify(mqd, &sigev) == -1) {
                perror("mq_notify");
                exit(1);
            }

            // 读取所有新消息
            while ((n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) {
                printf("Received: %.*s\n", (int)n, buff);
            }

            if (errno != EAGAIN) {
                perror("mq_receive");
                exit(1);
            }
        }
    }

    free(buff);
    mq_close(mqd);
    return 0;
}

static void sig_user(int signo) {
    write(pipefd[1], "a", 1);
}