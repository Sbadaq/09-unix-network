#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    mqd_t mqd;
    void *ptr;
    size_t len;
    u_int8_t prior;

    if(argc != 4)
    {
        printf("Usage: %s <name> <len> <prio>\n", argv[0]);
        return 1;
    
    }
    len = atoi(argv[2]);
    prior = atoi(argv[3]);
    mqd = mq_open(argv[1], O_WRONLY);
    ptr = malloc(len);
    int rt = mq_send(mqd, ptr, len, prior);
    printf("mq_send return %d\n", rt);
    if (rt == -1)
    {
        perror("mq_send");
        return 1;
    }
       
    return 0;
}