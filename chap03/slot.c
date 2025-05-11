#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    int i,msqid;

    for(i = 0 ; i < 10 ; i++)
    {
        msqid = msgget(IPC_PRIVATE, 0666);
        printf("msqid = %d\n", msqid);

        msgctl(msqid, IPC_RMID, NULL);
    }
    return 0;
}