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

int buff[MIN_TIMES];
struct {
    pthread_mutex_t mutex;
    int nput;
    int nval;
} put = {
    PTHREAD_MUTEX_INITIALIZER
};
struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready;
} nready = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER
};

// struct {
//     pthread_mutex_t mutex;
//     int buff[MIN_TIMES];
//     int nput;
//     int nval;
// } shared = {
//     PTHREAD_MUTEX_INITIALIZER
// };

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
        pthread_mutex_lock(&put.mutex);
        if(put.nput >= ntimes)
        {
            pthread_mutex_unlock(&put.mutex);
          //  break;
            return NULL;
        }
        buff[put.nput] = put.nval++;
        put.nput++;
        put.nval++;
        pthread_mutex_unlock(&put.mutex);
        
        pthread_mutex_lock(&nready.mutex);
        if(nready.nready == 0)
            pthread_cond_signal(&nready.cond);
        nready.nready++;
        pthread_mutex_unlock(&nready.mutex);
        *((int *)arg) += 1;
        //*((int *)arg) ++；为什么这一句不行？
    }
}

void * consumer(void *arg)
{
    int i;
    for(i = 0 ; i < ntimes ; i++ )
    {
        pthread_mutex_lock(&nready.mutex);
        while(nready.nready == 0)
        {
            pthread_cond_wait(&nready.cond, &nready.mutex);
        }
        nready.nready--;

        pthread_mutex_unlock(&nready.mutex);
        if(buff[i] != i)
        {
            printf("buff[%d] = %d (should be %d)\n", i, buff[i], i);
        }
    }
}