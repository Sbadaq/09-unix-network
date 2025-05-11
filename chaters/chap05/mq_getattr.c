
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    mqd_t mqd;
    struct mq_attr attr;

    if(argc != 2)
    {
        printf("Usage: %s <mq_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    mqd = mq_open(argv[1], O_RDONLY);
    mq_getattr(mqd, &attr);

    printf("max #msgs = %ld, max #bytes = %ld\n #currently on quueue = %ld\n",
        attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
    mq_close(mqd);

    return 0;
}