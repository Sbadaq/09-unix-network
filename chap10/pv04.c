/**10.5 多个生产者，多个消费者*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/syscall.h>

#define NBUFF  10
#define MAXTHREADERS 100

#define min(a,b) ((a)<(b)?(a):(b))
int ntimes,nproduces,nconsumes;

struct{
    int buff[NBUFF];
    int nput;     // 生产者放入的个数
    int nputval;  // 生产者放入的值
    int nget;     // 消费者取出的个数
    int ngetval;  // 消费者取出的值
    sem_t mutex,nempty,nstored;
}shared;

void *produce(void*),*consume(void*);

int main(int argc,char *argv[]){
    int i,prod_count[MAXTHREADERS],cons_count[MAXTHREADERS];
    pthread_t prods[MAXTHREADERS],cons[MAXTHREADERS];
    if(argc!=4){
        fprintf(stderr,"usage: %s <ntimes> <nproduces> <nconsumes>\n",argv[0]);
        exit(1);
    }
    ntimes=atoi(argv[1]);
    nproduces=min(atoi(argv[2]),MAXTHREADERS);
    nconsumes=min(atoi(argv[3]),MAXTHREADERS);

    sem_init(&shared.mutex,0,1);
    sem_init(&shared.nempty,0,NBUFF);
    sem_init(&shared.nstored,0,0);

    for(i=0;i<nproduces;i++){
        prod_count[i]=ntimes;
        pthread_create(&prods[i],NULL,produce,(void*)(&prod_count[i]));
    }
    for(i=0;i<nconsumes;i++){
        cons_count[i]=ntimes;
        pthread_create(&cons[i],NULL,consume,(void*)(&cons_count[i]));
    }
    for(i=0;i<nproduces;i++){
        pthread_join(prods[i],NULL);
        printf("produces count[%d] = %d\n",i,prod_count[i]);
    }
    for(i=0;i<nconsumes;i++){
        pthread_join(cons[i],NULL);
        printf("consumes count[%d] = %d\n",i,cons_count[i]);
    }

    sem_destroy(&shared.mutex);
    sem_destroy(&shared.nempty);
    sem_destroy(&shared.nstored);
    exit(0);
}

void *produce(void *arg){
    for(;;)
    {
        sem_wait(&shared.nempty);
        sem_wait(&shared.mutex);
        if(shared.nput >= ntimes)
        {
            sem_post(&shared.nstored);
            sem_post(&shared.nempty);
            sem_post(&shared.mutex);
            return NULL;
        }
        shared.buff[shared.nput % NBUFF ]=shared.nputval;
        printf("[%d] produce %d = %d \n",syscall(SYS_gettid),shared.nputval,shared.buff[shared.nput]);
        shared.nputval++;
        shared.nput++;
        sem_post(&shared.mutex);
        sem_post(&shared.nstored);

        *((int*)arg) = *((int*)arg) + 1;
    }
}

void *consume(void *arg)
{
    int i;
    for(;;)
    {
        sem_wait(&shared.nstored);
        sem_wait(&shared.mutex);
        if(shared.nget >= ntimes)
        {
            sem_post(&shared.nstored);
            sem_post(&shared.mutex);
            return NULL;
        }
        i=shared.nget % NBUFF;
        if(shared.buff[i] != shared.ngetval)
        {
            printf("buff[%d] = %d, ngetval = %d\n",i,shared.buff[i],shared.ngetval);
            exit(1);
        }
        printf("[%d] consume %d = %d\n",syscall(SYS_gettid),shared.ngetval,shared.buff[i]);
        shared.nget++;
        shared.ngetval++;
        sem_post(&shared.mutex);
        sem_post(&shared.nempty);

        *((int*)arg) = *((int*)arg) + 1;
    }
}