#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

/** send and receive msg in a mq */
int main(int argc, char *argv[])
{
    struct mq_attr attr = {.mq_maxmsg = 10, .mq_msgsize = 1024};
    mqd_t mq = mq_open("/est.mq", O_CREAT | O_RDWR, 0666, &attr);
    if( mq == (mqd_t)-1 ){
        perror("mq_open");
        return 1;
    }
    char msg[] = "Hello, world!";
    int e = mq_send(mq, msg, strlen(msg) + 1, 0);
    if( e < 0 ){
        perror("mq_send");
        return 1;
    }
    char buf[1024] = {0};
    e = mq_receive(mq, buf, sizeof(buf), NULL);
    if(e < 0){
        perror("mq_receive");
        return 1;
    }

    printf("Received: %s\n", buf);
    mq_close(mq);
    mq_unlink("/est.mq");
    return 0;
}
