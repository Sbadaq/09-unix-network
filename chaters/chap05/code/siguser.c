#include <bits/mqueue.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <bits/sigevent-consts.h>

mqd_t mqd;
void *buff;
struct mq_attr attr;
struct sigevent sev;

static void sig_usr(int);

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s <queue_name>\n", argv[0]);
        exit(1);
    }

    mqd = mq_open(argv[1], O_RDONLY);
    mq_getattr(mqd, &attr);
    buff = malloc(attr.mq_msgsize);
    signal(SIGUSR1, sig_usr);
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;
    mq_notify(mqd, &sev);

    for(;;)
        pause();
    
    return 0;
}

static void sig_usr(int signo)
{
    ssize_t nr;
    mq_notify(mqd, &sev);
    nr = mq_receive(mqd, buff, attr.mq_msgsize, NULL);
    printf("Received %ld bytes\n", nr);
    return;
}