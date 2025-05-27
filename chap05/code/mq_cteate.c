#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

static const mode_t FILE_MODE = 0666;

int main(int argc, char **argv) {
    int c, flags;
    mqd_t mq;
    struct mq_attr attr;

    // 初始化队列属性
    attr.mq_maxmsg = 10;  // 最大消息数量
    attr.mq_msgsize = 256; // 每条消息的最大字节数
    attr.mq_flags = 0;

    flags = O_RDWR | O_CREAT;
    while ((c = getopt(argc, argv, "e")) != -1) {
        switch (c) {
            case 'e':
                flags |= O_EXCL;
                break;
            default:
                break;
        }
    }

    if (optind != argc - 1) {
        fprintf(stderr, "Usage: %s [-e] <queue name>\n", argv[0]);
        exit(1);
    }

    // 检查名称是否以 / 开头
    if (argv[optind][0] != '/') {
        fprintf(stderr, "Queue name must start with /\n");
        exit(1);
    }

    printf("Creating queue %s\n", argv[optind]);

    // 打开消息队列并指定属性
    mq = mq_open(argv[optind], flags, FILE_MODE, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    // 关闭消息队列
    if (mq_close(mq) == -1) {
        perror("mq_close");
        exit(1);
    }

    return 0;
}