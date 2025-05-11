
#include <fcntl.h>
#include <mqueue.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int c,flags;
    mqd_t mq;

    ssize_t n;
    u_int8_t prior;
    void *buff;
    struct mq_attr attr;

    flags = O_RDONLY;
    while( ( c = getopt(argc,argv,"n") ) != -1 )
    {
        switch (c) {
        case 'n':
            flags |= O_NONBLOCK;
            break;
        default:
            break;
        }
    }
    if(optind != argc - 1){
        fprintf(stderr,"Usage: %s [-e] <queue name>\n",argv[0]);
        exit(1);
    }
    mq = mq_open(argv[1],flags);
    mq_getattr(mq, &attr);
    buff = malloc(attr.mq_msgsize);
    while(1)
    {
        n = mq_receive(mq,buff,attr.mq_msgsize,&prior);
        if(n > 0)
        {
            printf("received %d bytes\n",n);
        }
    }

    return 0;
}