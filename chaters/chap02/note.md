
### **第二章：POSIX IPC**
#### **核心内容**
1. **三种机制**：
   • **消息队列**：`mq_open`, `mq_send`, `mq_receive`
   • **信号量**：`sem_open`, `sem_wait`, `sem_post`
   • **共享内存**：`shm_open`, `mmap`, `munmap`

2. **关键特性**：
   • **命名规则**：必须以`/`开头（如`/my_queue`）
   • **持久性**：IPC对象可持久化到文件系统
   • **权限控制**：通过`mode`参数设置（类似文件权限）

#### **难点与代码示例**
##### **难点1：信号量同步陷阱**
• **问题**：多个进程竞争共享资源时需严格同步。
• **代码片段（生产者-消费者模型）**：
  ```c
  sem_t *empty = sem_open("/empty", O_CREAT, 0644, 5); // 缓冲区容量5
  sem_t *full = sem_open("/full", O_CREAT, 0644, 0);
  
  // 生产者
  sem_wait(empty);  // 等待空槽
  write_to_buffer();
  sem_post(full);   // 增加数据

  // 消费者
  sem_wait(full);
  read_from_buffer();
  sem_post(empty);
  ```

##### **难点2：共享内存竞争**
• **解决方法**：必须配合信号量或互斥锁。
• **记忆技巧**：  
  *"共享内存无锁必炸，信号量是救命稻草。"*

---
队列名必须以/开头（如/queue_A），符合POSIX规范。
程序退出时需调用 mq_close 和 mq_unlink，否则队列会残留在系统中。
如果需要某个机制的完整代码示例，可以告诉我具体场景（如“用POSIX消息队列实现聊天程序”），我会提供更详细的实现！