
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
static const char*  FILE_MODE = "0666";
int main(int argc,char** argv)
{
    int c,flags;
    mqd_t mq;

    flags = O_RDWR | O_CREAT;
    while(( c = getopt(argc,argv,"e")) != -1){
        switch(c){
            case 'e':
                flags |= O_EXCL;
                break;
            default:
                break;
        }
    }

    if(optind != argc - 1){
        fprintf(stderr,"Usage: %s [-e] <queue name>\n",argv[0]);
        exit(1);
    }
     
    printf("Creating queue %s\n",argv[optind]);


    mq = mq_open(argv[optind],flags,FILE_MODE,NULL);
}