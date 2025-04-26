#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

int main(int argc, char *argv[])
{
    const char *mq_name = "/test";

    // 删除现有的消息队列
    //mq_unlink(mq_name);

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = 50,
        .mq_curmsgs = 0
    };

    mqd_t mq = mq_open(mq_name, O_RDWR|O_CREAT, 0666, &attr);
    if (mq == -1) {
        perror("mq_open");
        return 1;
    }

    // 检查消息队列的属性
    struct mq_attr current_attr;
    if (mq_getattr(mq, &current_attr) == -1) {
        perror("mq_getattr");
        mq_close(mq);
        mq_unlink(mq_name);
        return 1;
    }

    if (current_attr.mq_msgsize != attr.mq_msgsize || current_attr.mq_maxmsg != attr.mq_maxmsg) {
        fprintf(stderr, "Message queue attributes do not match expected values\n");
        mq_close(mq);
        mq_unlink(mq_name);
        return 1;
    }

    pid_t producer = fork();
    if (producer == 0) { // 子进程
        const char *message = "HelloHelloHelloHelloHelloHelloHello";
        if (mq_send(mq, message, strlen(message), 0) == -1) {
            perror("mq_send");
            exit(1);
        }
        printf("Producer sent: %s\n", message);
        sleep(1);
        exit(0);
    }
    else if (producer > 0) { // 父进程
        sleep(3); // 等待子进程发送消息

        char buf[50]; // 调整缓冲区大小为 50 字节
        ssize_t bytes_received = mq_receive(mq, buf, 50, 0); // 使用 50 字节的缓冲区
        if (bytes_received == -1) {
            perror("mq_receive");
            exit(1);
        }

        buf[bytes_received] = '\0'; // 添加空终止符
        printf("Father received: %s\n", buf);
        mq_close(mq);
        mq_unlink(mq_name);
        exit(0);
    }
    return 0;
}