
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>

volatile sig_atomic_t mq_flag = 0;
void sig_proc(int signo);
int main(int argc, char **argv)
{
    mqd_t mq;
    void *buff;
    ssize_t  n;
    sigset_t zeromask,oldmask,newmask;

    struct mq_attr attr;
    struct sigevent sev;

    if(argc != 2)
    {
        printf("usage: %s <message queue name>\n",argv[0]);
        exit(1);
    }

    mq = mq_open(argv[1],O_RDONLY|O_NONBLOCK);
    mq_getattr(mq, &attr);
    buff = malloc(attr.mq_msgsize);

    sigemptyset(&newmask);
    sigemptyset(&oldmask);
    sigemptyset(&zeromask);

    sigaddset(&newmask, SIGUSR1);
    signal(SIGUSR1, sig_proc);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;
    mq_notify(mq, &sev);

    for(;;)
    {
        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        printf("block and wait for signal\n");
        while(mq_flag == 0)
            sigsuspend(&zeromask);

        mq_flag = 0;
        mq_notify(mq, &sev);
        while(( n = mq_receive(mq, buff, attr.mq_msgsize, NULL)) >= 0){
            printf("read %ld bytes\n",n);
        }
        if(errno != EAGAIN)
        {
            printf("mq_receive error\n" );

        }else{
           // mq_notify(mq, &sev);
           printf("mq_receive EAGAIN,this means read no data more.\n");
        }

        sigprocmask(SIG_UNBLOCK, &newmask, NULL);
        printf("unblocked\n");
    }


	return 0;
}

void sig_proc(int signo)
{
    mq_flag = 1;

    return ;
}