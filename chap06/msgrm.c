
#include <stdio.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc,char** argv)
{
    int mqid;
    if(argc != 2)
    {
        printf("Usage: %s <msg>\n",argv[0]);
        return 1;
    }

    key_t key = strtol(argv[1],NULL, 16);

    mqid = msgget(key,0666);
    if(mqid < 0)
    {
        perror("msgget");
        return 1;
    }
    printf("will rm mqid:%d\n",mqid);

    //msgctl(mqid,IPC_RMID,NULL);
    if(-1 == msgctl(mqid,IPC_RMID,NULL))
    {
        perror("msgctl");
        return 1;
    }
    printf("rm mqid:%d ok\n",mqid);
    return 0;
}