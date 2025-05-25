### **第12章 复习大纲**
#### **一、核心概念**
1. **共享内存区的优势与挑战**  
   - **最快IPC**：直接通过内存地址访问，无需内核数据拷贝  
   - **必须同步**：需配合互斥锁/信号量等机制防止竞态条件  

2. **mmap核心作用**  
   - **文件映射I/O**：将文件映射到内存，实现高效读写  
   - **匿名内存映射**：不依赖文件，用于进程间/线程间共享内存（BSD/SVR4实现差异）  
   - **POSIX共享内存**：通过`shm_open`创建无亲缘关系进程的共享内存  

---

#### **二、必须掌握的API与参数**
##### **1. `mmap`函数**  
```c
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

##### **2. `munmap`函数**  
```c
int munmap(void *addr, size_t length);  // 释放映射区域
```
- **重点**：`addr`必须与`mmap`返回地址对齐，`length`可部分释放  

##### **3. `msync`函数**  
```c
int msync(void *addr, size_t length, int flags);  // 同步内存与文件
```
- **用途**：确保修改写入磁盘（`flags=MS_SYNC`阻塞写，`MS_ASYNC`异步写）

---

#### **三、关键代码实例与说明**
##### **1. 文件映射计数器自增（需同步）**
```c
#include <sys/mman.h>
#include <fcntl.h>

int main() {
    int fd = open("counter.file", O_RDWR | O_CREAT, 0666);
    ftruncate(fd, sizeof(int));  // 扩展文件大小
    int *counter = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);  // 映射后立即关闭不影响

    // 需要同步机制（如信号量）保护此操作！
    for (int i = 0; i < 1000; i++) {
        (*counter)++;  // 直接操作内存地址
    }

    munmap(counter, sizeof(int));
    return 0;
}
```
- **注意**：实际应用中必须添加互斥锁或信号量，否则多进程并发会导致计数错误。

##### **2. 匿名内存映射（BSD风格）**
```c
int *shared = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, 
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
```
- **特点**：不依赖文件，适用于父子进程共享内存。

##### **3. SVR4匿名映射（通过`/dev/zero`）**
```c
int fd = open("/dev/zero", O_RDWR);  // 特定于System V系统
int *shared = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
close(fd);
```

---

#### **四、重点对比与问题**
1. **匿名映射的两种方式**  
   - **BSD**：`MAP_ANONYMOUS` + `fd=-1`  
   - **SVR4**：映射`/dev/zero`文件  

2. **内存映射同步问题**  
   - **写回时机**：默认由内核延迟写回，`msync`强制同步  
   - **竞态条件**：必须使用锁机制（如`pthread_mutex`或信号量）  

3. **地址对齐要求**  
   - `offset`必须是页大小的整数倍（通过`sysconf(_SC_PAGE_SIZE)`获取页大小）  

---

#### **五、常见陷阱**
1. **忘记`ftruncate`文件**：映射文件前需确保文件大小足够。  
2. **忽略同步机制**：直接操作共享内存会导致数据不一致。  
3. **错误处理返回值**：`mmap`失败返回`MAP_FAILED`（非`NULL`）。  

---

#### **六、扩展学习建议**
1. **对比System V共享内存**：`shmget`/`shmat`与`mmap`的异同。  
2. **实战调试**：通过`strace`追踪`mmap`系统调用行为。  
3. **性能测试**：比较共享内存与管道/消息队列的传输速率差异。

掌握12章内容后，可通过编写多进程计数器、生产者-消费者模型等实验巩固知识。务必手动实现同步机制以加深理解！