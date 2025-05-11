
#include <stdlib.h>
#include <stdio.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>

int main(int argc, char **argv)
{
    mqd_t mq;
    void *buff;
    ssize_t  n;
    sigset_t newmask;
    int signo;

    struct mq_attr attr;
    struct sigevent sev;

    if(argc != 2)
    {
        printf("usage: %s <message queue name>\n",argv[0]);
        exit(1);
    }

    mq = mq_open(argv[1],O_RDONLY|O_NONBLOCK);
    if(mq == -1)
    {
        printf("mq_open error\n");
        exit(1);
    }
    mq_getattr(mq, &attr);
    printf("mq_msgsize = %ld\n",attr.mq_msgsize);

    buff = malloc(attr.mq_msgsize);

    sigemptyset(&newmask);

    sigaddset(&newmask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &newmask, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;
    mq_notify(mq, &sev);
    for(;;)
    {
        sigwait(&newmask, &signo);

        if(signo == SIGUSR1)
        {
            mq_notify(mq, &sev);
            while(( n = mq_receive(mq, buff, attr.mq_msgsize, NULL)) >= 0 )
            {
                printf("received %d bytes\n", (long)n);

            }
            if(errno != EAGAIN)
            {
                perror("mq_receive error.");
                exit(1);
            }
        }
        //sigprocmask(SIG_UNBLOCK, &newmask, NULL);
    }


	return 0;
}
