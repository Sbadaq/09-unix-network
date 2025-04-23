

#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile sig_atomic_t  mqflag;
static void sig_user_1(int);

int main(int argc, char **argv)
{
    mqd_t mq;
    void *buff;
    ssize_t n;
    sigset_t zeromask,newmask,oldmask;

    struct mq_attr attr;
    struct sigevent sev;

    if(argc != 2)
    {
        printf("usage: siguser2 <pathname>");
        exit(1);
    }

    mq = mq_open(argv[1],O_RDONLY);
    mq_getattr(mq, &attr);
    buff = malloc(attr.mq_msgsize);
    sigemptyset(&zeromask);
    sigemptyset(&newmask);
    sigaddset(&newmask,SIGUSR1);

    signal(SIGUSR1, sig_user_1);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;
    mq_notify(mq,&sev);

    for(;;)
    {
        sigprocmask(SIG_BLOCK,&newmask,&oldmask);
        if((n = mq_receive(mq,buff,attr.mq_msgsize,NULL)) < 0)
        while(mqflag == 0)
            sigsuspend(&zeromask);
        mqflag = 0;
        mq_notify(mq, &sev);
        n = mq_receive(mq, buff, attr.mq_msgsize, NULL);
        printf("read %ld bytes\n", (long)n );


        sigprocmask(SIG_SETMASK,&oldmask,NULL);
        
    }
    exit(0);
}

static void sig_user_1(int signo)
{
    printf("sig_user_1: got SIGUSR1\n");
    mqflag = 1;
    return;
}