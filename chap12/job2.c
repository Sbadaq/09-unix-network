/**write hand : pruducer and consumer , share memory instance */


#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#define BUFFER_SIZE 100
struct shared *data;

struct shared{
    sem_t mutex;//mutex lock
    sem_t empty;//empty buffer
    sem_t full; //full buffer
    int iters[BUFFER_SIZE];
    int in; //producer write from here
    int out;//consumer read from here
};
// pruducer wait empty signal, if get,then write, then send full signal
// consumer wait full signal, if get, then read ,then send empty signal


void producer(int id){
    for(int i=0;i<10;++i){
       // printf("producer %d waiting empty (value=%d)\n",id,sem_getvalue(&data->empty));
        sem_wait(&data->empty);
       // printf("producer %d got empty, now waiting mutex (value=%d)\n",id,sem_getvalue(&data->mutex));
        
        sem_wait(&data->mutex);
       
        data->iters[data->in] = id * 100 + i;
        
       // printf("process %d write %d at slot %d\n",id,data->iters[data->in],data->in);
        data->in = (data->in+1)%BUFFER_SIZE;
       
        
        sem_post(&data->full);
        sem_post(&data->mutex);
    }
}

void consumer(int id){
    for(int i=0;i<10;++i){
        
        sem_wait(&data->full);
        sem_wait(&data->mutex);
        int iterm = data->iters[data->out];
        printf("                consumer %d read %d from slot %d\n", id, iterm, data->out);

        data->out = (data->out+1)%BUFFER_SIZE;
        
       
        sem_post(&data->empty);
         sem_post(&data->mutex);
    }
}

int main(int argc, char const *argv[])
{
/*
#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```
- **关键参数解析**：
  - `addr`：建议映射地址（通常设为`NULL`由内核选择）  
  - `prot`：内存保护标志（`PROT_READ | PROT_WRITE`）  
  - `flags`：  
    - `MAP_SHARED`：修改对其他进程可见  
    - `MAP_PRIVATE`：写时复制私有映射  
    - `MAP_ANONYMOUS`：创建匿名映射（BSD风格，需`fd=-1`）  
  - `fd`：文件描述符（匿名映射时设为`-1`）  
  - `offset`：文件偏移量（需按页大小对齐）  

*/    
    data = mmap(NULL,sizeof(struct shared),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    if(data == MAP_FAILED){
        perror("mmap");
        return 1;
    }
    data->in = 0;
    data->out = 0;

    // sem_init(&data->mutex,1,1); /// 初始值1（互斥锁）/ 初始值1（互斥锁）
    // sem_init(&data->empty,1,BUFFER_SIZE);// // 初始空槽=缓冲区大小
    // sem_init(&data->full,1,0);/ 初始满槽=0
    if(sem_init(&data->mutex,1,1) == -1 || sem_init(&data->empty,1,BUFFER_SIZE) == -1|| sem_init(&data->full,1,0) ==-1){
        perror("sem_init");
        return 1;
    }
    int processes ;
    if(argc < 2){
        processes = 2;
    }
    else{
        processes = atoi(argv[1]);
    }
   
    for(int i = 1; i < processes; i++){
        if(fork() == 0){
            if (i % 2 == 0) producer(i);  // even i -> producer
            else consumer(i);             // odd i -> consumer
            
        }
    }

    while(wait(NULL) > 0);
   
    sem_destroy(&data->mutex);
    sem_destroy(&data->empty);
    sem_destroy(&data->full);

   // ❌ munmap(&data, sizeof(struct shared)) 是错误的
   // munmap(&data,sizeof(struct shared)); 
   //&data，这是一个指针变量的地址（struct shared**），而 munmap 需要传入你映射得到的共享内存地址（void*），也就是 data。
   munmap(data, sizeof(struct shared));

    printf("done\n");
    return 0;
}

/*
使用 Helgrind 检测死锁：
bash

valgrind --tool=helgrind ./prod_cons

确保输出中无 Possible deadlock 警告。
如果死锁由业务逻辑（而非锁顺序）导致（如资源分配错误），Helgrind 可能无法发现。

增加消费者数量 &data，这是一个指针变量的地址（struct shared**），而 munmap 需要传入你映射得到的共享内存地址（void*），也就是 data。
修改代码启动多个消费者，观察负载均衡情况。

缓冲区溢出测试
将 BUFFER_SIZE 设为较小的值（如5），验证信号量是否能正确限制生产速度。

性能分析
对比使用共享内存与消息队列（如管道）的吞吐量差异


打印日志：
在生产/消费操作前后添加日志，观察执行顺序。

strace跟踪系统调用：
bash

strace -f ./program  # 跟踪多进程信号量 &data，这是一个指针变量的地址（struct shared**），而 munmap 需要传入你映射得到的共享内存地址（void*），也就是 data。操作

ipcs监控共享内存：
bash

    ipcs -m  # 查看共享内存段状态

通过这两个实验，你不仅能掌握共享内存的使用，还能深入理解同步机制的必要性。
务必手动实现并尝试破坏同步逻辑（如移除信号量），观察结果异常，从而加深对竞态条件和锁机制的理解！
*/