

#include <bits/types/__sigval_t.h>
#include <cerrno>
mqd_t mq;

struct mq_attr attr;
struct sigevent sigev;

static void notify_thread(union sigval val)
{
	ssize_t n;
    void *buff;
    printf("notify_thread: %d\n", val.sival_int);

    buff = malloc(attr.mq_msgsize);
    mq_notify(mq, &sigev);

    while(( n = mq_receive(mq, buff, attr.mq_msgsize, NULL) ) >= 0)
    {
        printf("notify_thread: %s\n", (char *)buff);
    }
    if(errno != EAGAIN)
    {
          
    }
}

