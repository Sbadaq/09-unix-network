
### **第三章：System V IPC**
#### **核心内容**
1. **三种机制**：

   • **消息队列**：`msgget`, `msgsnd`, `msgrcv`

   • **信号量**：`semget`, `semop`

   • **共享内存**：`shmget`, `shmat`, `shmdt`

2. **关键区别（与POSIX对比）**：
   | 特性           | POSIX IPC         | System V IPC       |
   |----------------|-------------------|--------------------|
   | **命名方式**   | 路径名（/name）   | 整数键值（key_t）  |
   | **函数前缀**   | `mq_`, `sem_`    | `msg_`, `sem_`     |
   | **持久性**     | 基于文件系统      | 内核维护           |

#### **难点与代码示例**
##### **难点1：System V消息队列复杂结构**
• **消息结构强制要求**：
  ```c
  struct msgbuf {
    long mtype;     // 必须为long且是第一个字段
    char mtext[100];
  };
  
  // 发送消息
  msgsnd(qid, &msg, sizeof(msg.mtext), 0);
  ```

##### **难点2：System V信号量操作**
• **原子操作数组**：通过`semop`一次性操作多个信号量。
• **代码片段**：
  ```c
  struct sembuf ops[2];
  // 信号量0减1（P操作）
  ops[0].sem_num = 0;
  ops[0].sem_op = -1;
  ops[0].sem_flg = SEM_UNDO;
  
  // 信号量1加1（V操作）
  ops[1].sem_num = 1;
  ops[1].sem_op = 1;
  ops[1].sem_flg = SEM_UNDO;
  
  semop(semid, ops, 2); // 原子执行两个操作
  ```

##### **记忆技巧**
• **System V函数名规律**：  
  *"get创建，ctl控制，op操作。"*  
  如：`msgget`创建队列，`msgctl`设置属性，`msgsnd/msgrcv`收发数据。

---

### **辅助记忆工具**
#### 1. **对比表格（POSIX vs System V）**
| **特性**       | POSIX IPC               | System V IPC           |
|----------------|-------------------------|------------------------|
| **消息队列**   | `mq_open`               | `msgget`               |
| **信号量**     | `sem_open` + 有名信号量| `semget` + 集合操作    |
| **共享内存**   | `shm_open` + `mmap`    | `shmget` + `shmat`     |

#### 2. **常见错误速查**
• **权限问题**：  
  System V IPC的权限在`xxxget`调用时设置（如`msgget(key, 0666 | IPC_CREAT)`）。
• **资源泄漏**：  
  用`ipcs`命令查看残留的IPC对象，`ipcrm`手动删除。

---

### **总结建议**
1. **动手实验**：对每个机制写一个最小Demo（如用共享内存传递字符串）。
2. **调试工具**：
   • `strace`跟踪系统调用（如`strace -e trace=ipc ./demo`）
   • `ipcs`和`ipcrm`管理System V IPC对象。


以下是 **System V IPC** 的必知必会知识整理，用最简单的方式帮你掌握核心内容，拒绝复杂细节！

---

### **System V IPC 三大件**
1. **消息队列**：进程间传递结构化消息（类似邮箱）。
2. **信号量**：协调进程对共享资源的访问（类似钥匙）。
3. **共享内存**：多个进程直接读写同一块内存（类似黑板）。

---

### **一、消息队列（Message Queues）**
#### **核心操作**
• **创建/获取队列**：`msgget(key, flags)`  
  ```c
  key_t key = ftok("/some/file", 'A');  // 生成唯一key
  int msgid = msgget(key, 0666 | IPC_CREAT);  // 0666是权限
  ```
• **发送消息**：`msgsnd(msgid, &msg, size, flags)`  
• **接收消息**：`msgrcv(msgid, &msg, size, msg_type, flags)`  

#### **必知要点**
• **消息结构强制格式**：  
  ```c
  struct msgbuf {
    long mtype;     // 必须为long且是第一个字段！
    char mtext[100]; // 自定义数据
  };
  ```
• **消息类型过滤**：`msgrcv` 根据 `mtype` 选择性接收消息（`mtype=0` 表示任意类型）。

• **常见错误**：  
  • 消息结构未以 `long mtype` 开头 → 崩溃！  
  • 忘记用 `ftok` 生成唯一key → 队列冲突！

#### **记忆口诀**  
> *"消息队列靠key找，mtype开头不能少；发送接收按类型，结构不对全乱套。"*

---

### **二、信号量（Semaphores）**
#### **核心操作**
• **创建/获取信号量集合**：`semget(key, num_sems, flags)`  
  ```c
  int semid = semget(key, 2, 0666 | IPC_CREAT); // 创建包含2个信号量的集合
  ```
• **操作信号量**：`semop(semid, ops, num_ops)`  
  ```c
  struct sembuf ops[1] = {
    {0, -1, SEM_UNDO}  // 对第0个信号量减1（P操作）
  };
  semop(semid, ops, 1);
  ```

#### **必知要点**
• **信号量是“集合”**：一次可管理多个信号量（如生产者-消费者需要两个信号量）。

• **SEM_UNDO的作用**：进程崩溃时自动撤销未完成的操作（防止死锁）。

• **常见错误**：  
  • 操作非原子性（应一次性操作多个信号量）。  
  • 忘记初始化信号量初值 → 死锁！

#### **记忆口诀**  
> *"信号量是集合体，P减V加要牢记；SEM_UNDO防崩溃，操作原子是真理。"*

---

### **三、共享内存（Shared Memory）**
#### **核心操作**
• **创建/获取共享内存**：`shmget(key, size, flags)`  
  ```c
  int shmid = shmget(key, 4096, 0666 | IPC_CREAT); // 创建4KB共享内存
  ```
• **挂接到进程地址空间**：`shmat(shmid, NULL, 0)`  
  ```c
  char *ptr = shmat(shmid, NULL, 0); // 返回内存指针
  ```
• **分离内存**：`shmdt(ptr)`  

#### **必知要点**
• **需要同步**：必须用信号量或锁保护共享内存的读写。
• **内核不自动释放**：即使所有进程分离，共享内存仍存在，需手动删除：
  ```c
  shmctl(shmid, IPC_RMID, NULL); // 删除共享内存
  ```
• **常见错误**：  
  • 多进程同时写共享内存未加锁 → 数据混乱！  
  • 忘记删除残留的共享内存 → 占用系统资源！

#### **记忆口诀**  
> *"共享内存快如飞，挂接分离要配对；不同步就必翻车，用完不删是犯罪。"*

---

### **System V vs POSIX IPC 快速对比**
| **特性**         | System V IPC               | POSIX IPC                 |
|------------------|---------------------------|--------------------------|
| **命名方式**     | 整数key（`ftok`生成）      | 路径名（如`/my_queue`）   |
| **持久性**       | 内核维护，需手动删除       | 可基于文件系统持久化      |
| **函数前缀**     | `msg_`, `sem_`, `shm_`     | `mq_`, `sem_`, `shm_`     |
| **易用性**       | 复杂                      | 简单直观                 |

---

### **必会命令**
1. **查看所有IPC对象**：
   ```bash
   ipcs -a  # 查看消息队列、信号量、共享内存
   ```
2. **删除残留的IPC对象**：
   ```bash
   ipcrm -Q <key>    # 删除消息队列
   ipcrm -S <key>    # 删除信号量
   ipcrm -M <key>    # 删除共享内存
   ```

---

### **总结：System V IPC 核心三问**
1. **怎么创建对象？** → 用 `xxxget(key, ...)`（如 `msgget`）。
2. **如何操作对象？** → 用 `xxxop`（如 `semop`）或 `xxxrcv`（如 `msgrcv`）。
3. **用完怎么清理？** → 用命令 `ipcrm` 或代码中调用 `xxxctl(..., IPC_RMID)`。

---

记住这些就能应付面试和基础开发了！如果需要更具体的代码片段（如生产者-消费者模型），随时告诉我！   