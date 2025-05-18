#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define SEM_NAME "mysem"

int count = 0;

int main(int argc, char *argv[])
{
    int i,nloop;
    sem_t *mutext;

    if(argc != 2)
    {
        printf("usage: %s <nloop>\n", argv[0]);
        exit(1);
    }

    nloop = atoi(argv[1]);
    mutext = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
    sem_unlink(SEM_NAME);

    setbuf(stdout, NULL);
    if(fork() == 0)
    {
        for(i = 0; i < nloop; i++)
        {
            sem_wait(mutext);
            count++;
            printf("CHILD: count = %d\n", count);
            sem_post(mutext);
        }
        exit(0);
    }
    for(i = 0; i < nloop; i++)
    {
        sem_wait(mutext);
        count--;
        printf("PARENT: count = %d\n", count);
        sem_post(mutext);
    }
    wait(NULL);
    exit(0);
}