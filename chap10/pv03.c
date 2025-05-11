/*10.4 多个生产者，单个消费者*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define min(a,b) ((a)<(b)?(a):(b))
#include <sys/syscall.h>

#define NBUFF   10
#define MAXTHREADERS  100
#define NO_DEADLOCK  0

int ntimes,nproduces;   //只读变量，生产者和消费者都不能修改这2个变量；
struct{
    int buff[NBUFF];
    int nput;
    int nputval;
    sem_t mutex,nempty,nstored;//信号量而并非指针
}shared;

void *produce(void*),*consume(void*);
int main(int argc,char **argv)
{
    int i,count[MAXTHREADERS];
    pthread_t  tid_produce[MAXTHREADERS],tid_consume;

    if(argc != 3)
    {
        printf("usage : pv03 <#items> <#produces>");
        return -1;
    }
    ntimes = atoi(argv[1]);
    nproduces = min(atoi(argv[2]),MAXTHREADERS);

    sem_init(&shared.mutex,0,1);
    sem_init(&shared.nempty,0,NBUFF);
    sem_init(&shared.nstored,0,0);

    for(i=0;i<nproduces;i++)
    {
        count[i] = 0;
        pthread_create(&tid_produce[i],NULL,produce,(void *)&count[i]);
    }
    pthread_create(&tid_consume,NULL,consume,NULL);
    for(i=0;i<nproduces;i++)
    {
        pthread_join(tid_produce[i],NULL);
        printf("producer %d produced %d items\n",i,count[i]);
    }
    pthread_join(tid_consume,NULL);
    sem_destroy(&shared.mutex);
    sem_destroy(&shared.nempty);
    sem_destroy(&shared.nstored);
    return 0;
}

void *produce(void *arg)
{
    for(;;)
    {
#if NO_DEADLOCK     
        //生产者先获取空槽信号量 nempty，确保缓冲区有空位，再获取互斥锁 mutex 进行生产。
        sem_wait(&shared.nempty);
        sem_wait(&shared.mutex);
#else
        sem_wait(&shared.mutex);
        sem_wait(&shared.nempty);
#endif            
        if(shared.nput >= ntimes)
        { 
#if NO_DEADLOCK        
        sem_post(&shared.nempty);
        sem_post(&shared.mutex);
#else
        sem_post(&shared.mutex);
        sem_post(&shared.nempty);
#endif    
            return NULL;  //all done
        }
        shared.buff[shared.nput % NBUFF] = shared.nputval;
        shared.nputval += 1;
        shared.nput += 1;
        printf("[%d] produced %d\n",syscall(SYS_gettid),shared.buff[shared.nput % NBUFF]);
        //因为在最后一个循环时并没有往缓冲区中写入数据，所以这里要再加1
        //否则 生产线程数大于 缓冲区数量时，多余的生产线程会陷入等待
#if NO_DEADLOCK        
        sem_post(&shared.mutex);
        sem_post(&shared.nstored);
#else        
        sem_post(&shared.nstored);
        sem_post(&shared.mutex);
#endif    
        *((int *)arg) += 1;
    }
}

void *consume(void *arg)
{
    int i;
    for(i=0;i<ntimes;i++)
    {
#if NO_DEADLOCK     
//消费者先获取存储信号量 nstored，确保缓冲区有数据，再获取互斥锁 mutex 进行消费。   
        sem_wait(&shared.nstored);
        sem_wait(&shared.mutex);
#else
        sem_wait(&shared.mutex);
        sem_wait(&shared.nstored);
#endif    

        if(shared.buff[i % NBUFF] != i)
        {
            printf("error : buff[%d] = %d\n",i,shared.buff[i%NBUFF]);
             
        }
        printf("[%d] consumed %d\n",syscall(SYS_gettid),shared.buff[shared.nput % NBUFF]);
#if NO_DEADLOCK        
        sem_post(&shared.mutex);
        sem_post(&shared.nempty);
#else
        sem_post(&shared.nempty);
        sem_post(&shared.mutex);
#endif        
    }

    return NULL;
}