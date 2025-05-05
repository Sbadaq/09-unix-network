#define _GNU_SOURCE   // 启用 GNU 扩展
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>    // CPU 亲和性相关函数
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>


#define MIN_TIMES 10000000
#define MAX_THREADS 100

int ntimes = MIN_TIMES;

struct {
    pthread_mutex_t mutex;
    int buff[MIN_TIMES];
    int nput;
    int nval;
} shared = {
    PTHREAD_MUTEX_INITIALIZER
};

void * producer(void *arg);
void * consumer(void *arg);

 

void set_affinity(pthread_t thread, int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

int main(int argc, char *argv[])
{
    int i,nthreads,count[MAX_THREADS];
    pthread_t th[MAX_THREADS], tid_consumer;

    if(argc != 3)
    {
        printf("Usage: %s <#threads> <#iterations>\n", argv[0]);
        exit(1);
    }
    ntimes = atoi(argv[1]);
    nthreads = atoi(argv[2]);

    //set_concurrency(nthreads);
    for(i=0; i<nthreads; i++)
    {
        count[i] = 0;
        pthread_create(&th[i], NULL, producer, (void *)&count[i]);
        // 在线程创建后调用
        //pthread_create(&th[i], NULL, producer, &count[i]);
        set_affinity(th[i], i % 4); // 绑定到 0-3 号 CPU 核心
    }
    pthread_create(&tid_consumer, NULL, consumer, NULL);

    for(i = 0 ; i < nthreads; i++  )
    {
        pthread_join(th[i], NULL);
        printf("Producer %d produced %d values\n", i, count[i]);
    }

    pthread_join(tid_consumer, NULL);

    return 0;
}

void * producer(void *arg)
{
    for(;;) { //这个例子会导致全部的产品都是在一个生产线程中完成，无法使用set_affinity函数
        pthread_mutex_lock(&shared.mutex);
        if(shared.nput >= ntimes)
        {
            pthread_mutex_unlock(&shared.mutex);
          //  break;
            return NULL;
        }
        shared.buff[shared.nput] = shared.nval++;
        shared.nput++;
        shared.nval++;
        pthread_mutex_unlock(&shared.mutex);
        *((int *)arg) += 1;
    }
}

void consume_wait(int i)
{
    for(;;)
    {
        pthread_mutex_lock(&shared.mutex);
        if(i < shared.nput)
        {
            pthread_mutex_unlock(&shared.mutex);
            break;
        }
        pthread_mutex_unlock(&shared.mutex);
    }
}

void * consumer(void *arg)
{ 
    int i;
    for(i = 0 ; i < ntimes; i++ )
    {
        consume_wait(i);
        if(shared.buff[i] != i)
        {
            printf("buff[%d] = %d (should be %d)\n", i, shared.buff[i], i);
        }
    }
}