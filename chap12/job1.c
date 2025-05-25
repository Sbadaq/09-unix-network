/**多进程共享 计数器 ，需要 同步 */
/* 在多个进程  共享 一个计算器变量 ，每个进程 都 对其加1,最终确保 结果正确  ，无竞态条件 */

//技术栈 
//- 共享内存 mmap 
//- posix无名信号 量 sem_init

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <wait.h>

#define COUNT_SIZE  sizeof(struct counter)

//    将信号量mutex与计数器value封装在同一共享内存区域，确保多进程能访问同一信号量。
struct counter{
    sem_t mutex;
    int value;
};

int main()
{

    int fd = shm_open("/my_counter", O_CREAT | O_RDWR , 0666);
    ftruncate(fd, COUNT_SIZE);//设置共享内存大小
    struct counter *counter = mmap(NULL, COUNT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if(fork() == 0)//子进程 
    {   
        sem_init(&counter->mutex, 1, 1);
        counter->value = 0;
    }else{
        sleep(1);
    }

    if(fork() == 0 || fork() == 0)//子进程 2,3
    {
        for(int i = 0 ; i < 1000 ; i++ )
        {//每次修改计数器前用sem_wait加锁，操作后sem_post解锁。
            sem_wait(&counter->mutex);
            ++counter->value;
           // printf("thread %d: %d\n", getpid(), counter->value);
            sem_post(&counter->mutex);
        }
        exit(0);
    }

    wait(NULL);
    wait(NULL);
    printf("final %d counter value %d\n",getpid(), counter->value);

    sem_destroy(&counter->mutex);
    munmap(counter, COUNT_SIZE);
    shm_unlink("/my_counter");

    return 0;
}
