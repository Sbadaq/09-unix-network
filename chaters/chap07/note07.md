### **《UNIX网络编程卷2：进程间通信》第7章复习大纲——互斥锁与条件变量**

---

### **一、互斥锁（`pthread_mutex_t`）**
#### **1. 核心函数与用法**
```c
#include <pthread.h>
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);   // 阻塞加锁
int pthread_mutex_trylock(pthread_mutex_t *mutex);// 非阻塞加锁（失败返回EBUSY）
int pthread_mutex_unlock(pthread_mutex_t *mutex); // 解锁
```
- **功能**：保护临界区，确保同一时间仅一个线程访问共享资源。
- **初始化方式**：
  - **静态初始化**：`pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;`（适用于全局或静态变量）。
  - **动态初始化**：通过`pthread_mutex_init()`，需配合`pthread_mutex_destroy()`释放资源。
- **注意**：
  - **粒度控制**：锁的持有时间需平衡性能与安全性，避免过粗或过细。
  - **动态分配互斥锁**：需在释放内存前调用`pthread_mutex_destroy()`。

#### **2. 生产者-消费者问题**
**经典模型**：  
- **顺序启动**：生产者全部生产完成后消费者再消费（示例代码见下方）。
- **同时启动**：生产者与消费者并行运行，需条件变量协调。

**代码示例（顺序启动）**：
```c
struct {
    pthread_mutex_t mutex;
    int buffer[MAX_ITEMS];
    int nput; // 下一个存放位置
} shared = {PTHREAD_MUTEX_INITIALIZER};

void* producer(void *arg) {
    pthread_mutex_lock(&shared.mutex);
    if (shared.nput >= MAX_ITEMS) {
        pthread_mutex_unlock(&shared.mutex);
        return NULL;
    }
    shared.buffer[shared.nput++] = data;
    pthread_mutex_unlock(&shared.mutex);
    return NULL;
}
```

---

### **二、条件变量（`pthread_cond_t`）**
#### **1. 核心函数与用法**
```c
#include <pthread.h>
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex); // 等待条件
int pthread_cond_signal(pthread_cond_t *cond);  // 唤醒一个等待线程
int pthread_cond_broadcast(pthread_cond_t *cond);// 唤醒所有等待线程
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, 
                           const struct timespec *abstime); // 限时等待
```
- **功能**：与互斥锁配合，实现线程间的条件等待与通知。
- **使用模式**：
  ```c
  pthread_mutex_lock(&mutex);
  while (条件不满足) { // 避免虚假唤醒
      pthread_cond_wait(&cond, &mutex);
  }
  // 处理临界区
  pthread_mutex_unlock(&mutex);
  ```

#### **2. 避免虚假唤醒**
- **原因**：某些系统实现可能导致`pthread_cond_wait()`返回时条件未满足。
- **解决方法**：始终在循环中检查条件，而非`if`语句。

#### **3. 生产者-消费者（同时启动）**
**代码示例**：
```c
struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int buffer[MAX_ITEMS];
    int count = 0; // 当前缓冲区数据量
} shared;

void* producer(void *arg) {
    pthread_mutex_lock(&shared.mutex);
    while (shared.count >= MAX_ITEMS) { // 缓冲区满则等待
        pthread_cond_wait(&shared.cond, &shared.mutex);
    }
    shared.buffer[shared.count++] = data;
    pthread_cond_signal(&shared.cond); // 通知消费者
    pthread_mutex_unlock(&shared.mutex);
    return NULL;
}

void* consumer(void *arg) {
    pthread_mutex_lock(&shared.mutex);
    while (shared.count <= 0) { // 缓冲区空则等待
        pthread_cond_wait(&shared.cond, &shared.mutex);
    }
    data = shared.buffer[--shared.count];
    pthread_cond_signal(&shared.cond); // 通知生产者
    pthread_mutex_unlock(&shared.mutex);
    return NULL;
}
```

---

### **三、互斥锁与条件变量的属性**
#### **1. 属性设置**
- **进程共享属性**：
  ```c
  pthread_mutexattr_t mattr;
  pthread_mutexattr_init(&mattr);
  pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED); // 进程间共享
  pthread_mutex_init(&mutex, &mattr);
  ```
  - `PTHREAD_PROCESS_SHARED`允许跨进程同步（需共享内存）。

#### **2. 动态初始化与销毁**
```c
pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
pthread_mutex_init(mutex, NULL);
// 使用后需销毁
pthread_mutex_destroy(mutex);
free(mutex);
```

---

### **四、关键问题与解决方案**
#### **1. 上锁与等待的对比**
- **仅用互斥锁**：生产者需完全生产后消费者才能消费，效率低。
- **结合条件变量**：允许生产与消费并行，通过条件变量通知状态变化。

#### **2. 条件变量的定时等待**
```c
struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
ts.tv_sec += 5; // 等待5秒
int ret = pthread_cond_timedwait(&cond, &mutex, &ts);
if (ret == ETIMEDOUT) {
    // 超时处理
}
```
- **应用场景**：避免无限等待，如实时任务调度。

---

### **五、必须掌握的代码模式**
1. **互斥锁保护共享数据**：
   ```c
   pthread_mutex_lock(&mutex);
   // 临界区操作
   pthread_mutex_unlock(&mutex);
   ```

2. **条件变量协调生产消费**：
   ```c
   // 生产者
   pthread_cond_signal(&cond);
   // 消费者
   while (条件不满足) pthread_cond_wait(&cond, &mutex);
   ```

3. **属性设置（跨进程同步）**：
   ```c
   pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
   ```

---

### **六、常见错误与调试**
- **死锁**：确保加锁后所有路径都有解锁操作。
- **未初始化锁**：静态初始化或显式调用`pthread_mutex_init()`。
- **忽略虚假唤醒**：始终在循环中检查条件。

---

通过结合代码示例与理论分析，可深入理解互斥锁和条件变量在多线程同步中的核心作用。建议通过实际调试生产者-消费者问题，观察线程调度与资源竞争行为，以巩固知识点。