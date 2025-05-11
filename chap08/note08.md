
你了解《unix网络编程卷2：进程间通信》第2版中文版本的第8章吗，第8章主要讲以下章节，请根据你了解的章节重点内容，帮我写一个复习大纲，必须掌握的代码部分也帮我提供并添加说明；使我能充分掌握和理解
### 第8章 读写锁 复习大纲

---

#### **一、读写锁核心概念**
1. 两种锁模式：
   - 共享锁（读锁）：允许多个线程同时读
   - 独占锁（写锁）：只允许单个线程写
2. 适用场景：读操作频繁，写操作较少的场景（如配置管理、缓存系统）
3. 锁优先级策略：
   - 写优先（可能导致读线程饥饿）
   - 读优先（可能导致写线程饥饿）

---

#### **二、必须掌握的API及代码**
```c
/* 初始化与销毁 */
int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);

/* 加锁操作 */
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);  // 阻塞获取读锁
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);  // 阻塞获取写锁

/* 非阻塞尝试 */
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock); // 尝试读锁（返回EBUSY表示失败）
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock); // 尝试写锁（返回EBUSY表示失败）

/* 释放锁 */
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
```

**代码示例：**
```c
pthread_rwlock_t rwlock;
pthread_rwlock_init(&rwlock, NULL);

// 读线程操作
pthread_rwlock_rdlock(&rwlock);
/* 读操作 */
pthread_rwlock_unlock(&rwlock);

// 写线程操作
pthread_rwlock_wrlock(&rwlock);
/* 写操作 */
pthread_rwlock_unlock(&rwlock);

pthread_rwlock_destroy(&rwlock);
```

---

#### **三、读写锁属性**
1. 进程共享属性：
   ```c
   int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared);
   int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared);
   ```
   - `PTHREAD_PROCESS_PRIVATE`（默认）：仅限进程内线程使用
   - `PTHREAD_PROCESS_SHARED`：允许跨进程使用（需配合共享内存）

2. 属性使用流程：
   ```c
   pthread_rwlockattr_t attr;
   pthread_rwlockattr_init(&attr);
   pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
   pthread_rwlock_init(&rwlock, &attr);
   pthread_rwlockattr_destroy(&attr);
   ```

---

#### **四、实现原理（重点理解）**
1. 使用互斥锁 + 条件变量实现读写锁的核心思路：
   - 维护状态变量：
     - `readers`（当前读者数）
     - `writer`（是否有写者）
   - 互斥锁保护状态变量
   - 条件变量用于等待锁可用

2. 关键实现逻辑：
   - 获取读锁时：等待无写者且无写等待
   - 获取写锁时：等待无读者和写者
   - 避免写者饥饿的常见策略：优先处理写请求
```c
// 读锁加锁逻辑
pthread_mutex_lock(&mutex);
while (writer || waiting_writers > 0) {
    pthread_cond_wait(&read_cond, &mutex);
}
readers++;
pthread_mutex_unlock(&mutex);
```   

---

#### **五、线程取消处理**
1. 关键问题：线程被取消时可能未释放锁，导致死锁
2. 解决方案：
   - 使用`pthread_cleanup_push`注册清理函数
   - 在清理函数中释放锁
   ```c
   void cleanup(void *arg) {
       pthread_rwlock_unlock((pthread_rwlock_t *)arg);
   }

   pthread_cleanup_push(cleanup, &rwlock);
   pthread_rwlock_wrlock(&rwlock);
   /* 临界区操作 */
   pthread_cleanup_pop(1); // 执行清理
   ```

---

#### **六、重点注意事项**
1. 错误处理：
   - 检查所有API返回的`int`值（0成功，非0错误码）
   - 特别注意`EBUSY`（trylock失败）、`EDEADLK`（死锁风险）

2. 性能优化：
   - 避免长时间持有写锁
   - 读多写少场景更适合读写锁

3. 典型陷阱：
   - 忘记解锁导致死锁
   - 嵌套加锁导致未定义行为

---

#### **七、自测问题**
1. 读写锁与互斥锁的主要区别是什么？
2. 如何避免写线程饥饿？实现读写锁时需平衡读者和写者的优先级（如写者优先策略）
`解决方案：`
```c
写者优先策略（需在读写锁的实现中设计）：

标记写者等待状态：当有写者等待时，禁止新读者获取锁。

唤醒优先级：解锁时优先唤醒写者（而非读者）。

状态管理：

维护 waiting_writers 计数器。

读者加锁前检查是否有写者等待，若有则阻塞。

伪代码逻辑（基于条件变量实现）：

c
// 写者加锁逻辑
pthread_mutex_lock(&mutex);
waiting_writers++;  // 标记有写者等待
while (readers > 0 || active_writers > 0) {
    pthread_cond_wait(&write_cond, &mutex);
}
waiting_writers--; 
active_writers = 1; // 获取写锁
pthread_mutex_unlock(&mutex);

// 读者加锁逻辑
pthread_mutex_lock(&mutex);
while (active_writers > 0 || waiting_writers > 0) { 
    // 若有写者等待，禁止新读者
    pthread_cond_wait(&read_cond, &mutex);
}
readers++;
pthread_mutex_unlock(&mutex);
```

3. 如果要在父子进程间共享读写锁，需要设置什么属性？
 
#### **3. 父子进程间共享读写锁的属性设置**

- **关键步骤**：
  1. **设置进程共享属性**：
     ```c
     pthread_rwlockattr_t attr;
     pthread_rwlockattr_init(&attr);
     pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
     ```
  2. **初始化锁时传入属性**：
     ```c
     pthread_rwlock_t *rwlock = mmap(NULL, sizeof(pthread_rwlock_t), 
                                   PROT_READ | PROT_WRITE, 
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
     pthread_rwlock_init(rwlock, &attr);
     ```
  3. **父子进程共享内存**：  
     必须将锁分配在共享内存中（如通过 `mmap` 或 `shm_open`），否则即使设置属性，父子进程的锁仍指向不同内存地址。

- **注意事项**：
  - 销毁锁前需确保所有进程已释放锁。
  - 共享内存的同步需结合其他 IPC 机制（如信号量）。

---
4. `pthread_rwlock_trywrlock`返回EBUSY时如何处理？




#### **4. `pthread_rwlock_trywrlock` 返回 EBUSY 的处理**

- **EBUSY 的含义**：  
  写锁已被其他线程持有，或存在读锁。

- **处理策略**：
  - **策略1：直接放弃**  
    适用于非关键操作，直接返回错误：
    ```c
    if (pthread_rwlock_trywrlock(&rwlock) == EBUSY) {
        // 记录日志或执行备用逻辑
        return -1;
    }
    ```
  - **策略2：重试机制**  
    结合忙等待或延时重试（需谨慎避免 CPU 空转）：
    ```c
    while (pthread_rwlock_trywrlock(&rwlock) == EBUSY) {
        usleep(1000); // 休眠1ms后重试
    }
    ```
  - **策略3：回退到阻塞等待**  
    若无法立即获取锁，转为调用 `pthread_rwlock_wrlock`：
    ```c
    if (pthread_rwlock_trywrlock(&rwlock) != 0) {
        pthread_rwlock_wrlock(&rwlock); // 阻塞直到获取锁
    }
    ```

---
5. 为什么线程取消时需要特别处理读写锁？


#### **5. 线程取消时需特别处理读写锁的原因**

- **风险**：  
  线程被取消（如调用 `pthread_cancel`）时若持有锁，锁不会被自动释放，导致其他线程永久阻塞。

- **解决方案**：  
  使用 **线程清理函数**（Thread Cleanup Handlers）确保锁被释放：
  ```c
  // 清理函数：释放写锁
  void unlock_cleanup(void *arg) {
      pthread_rwlock_t *lock = (pthread_rwlock_t *)arg;
      pthread_rwlock_unlock(lock);
  }

  // 线程函数示例
  void *writer_thread(void *arg) {
      pthread_rwlock_t *lock = (pthread_rwlock_t *)arg;
      
      // 注册清理函数（压栈）
      pthread_cleanup_push(unlock_cleanup, lock);
      
      pthread_rwlock_wrlock(lock);
      // ... 临界区操作（可能被取消） ...
      
      // 弹出并执行清理函数（参数1表示执行）
      pthread_cleanup_pop(1); 
      return NULL;
  }
  ```

- **关键点**：
  - `pthread_cleanup_push` 和 `pthread_cleanup_pop` 必须成对出现。
  - 即使线程未被取消，`pthread_cleanup_pop(1)` 也会执行清理函数。
 
---

通过这个大纲可以系统掌握读写锁的：
- 核心概念与适用场景
- POSIX API的正确使用方式
- 底层实现原理
- 线程安全与异常处理
- 性能调优方向

建议配合书中示例代码进行实践编程练习。