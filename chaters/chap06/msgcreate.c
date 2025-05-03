#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SVMSG_MODE 0666


int main(int argc , char **argv)
{
    int c,oflag , mqid;

    oflag = SVMSG_MODE | IPC_CREAT;

    while(( c = getopt(argc , argv , "e")) != -1){
        switch(c){
            case 'e':
                oflag |= IPC_EXCL;
                break;
        }
    }
    if ( optind != argc - 1)
    {
        fprintf(stderr , "Usage : %s [-e] <key>\n" , argv[0]);
        exit(-1);
    }
    mqid = msgget(atoi(argv[optind]) , oflag);
    return 0;
}  