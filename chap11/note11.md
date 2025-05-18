以下是《UNIX网络编程卷2：进程间通信》第11章 **System V信号量** 的复习大纲，包含重点内容总结和必须掌握的代码示例及说明，帮助你全面掌握这一章节的核心知识。

---

### **第11章 System V信号量 复习大纲**

---

#### **11.1 概述**
- **核心概念**  
  System V信号量通过**信号量集合**（Semaphore Set）实现，每个集合包含多个信号量，而POSIX信号量仅支持单个信号量。
- **关键特性**  
  - 支持原子操作多个信号量（通过 `semop`）。
  - 提供复杂的同步机制（如进程间同步）。
- **与POSIX信号量区别**  
  - POSIX信号量更轻量，但System V信号量功能更强大（如跨进程共享）。

---

#### **11.2 semget：创建/获取信号量集合**
- **函数原型**  
  ```c
  #include <sys/sem.h>
  int semget(key_t key, int nsems, int semflg);
  ```
- **关键参数**  
  - `key`：唯一标识信号量集合的键值（可用 `ftok` 生成或 `IPC_PRIVATE`）。
  - `nsems`：集合中信号量的数量。
  - `semflg`：权限标志（如 `IPC_CREAT | 0666`）。
- **返回值**  
  成功返回信号量集合标识符（`semid`），失败返回 `-1`。
- **必须掌握的代码**  
  ```c
  key_t key = ftok("/tmp/semfile", 'a');  // 生成键值
  int semid = semget(key, 1, IPC_CREAT | 0666); // 创建包含1个信号量的集合
  if (semid == -1) {
      perror("semget error");
      exit(1);
  }
  ```
  **说明**：若信号量已存在，`semget` 会直接获取其标识符。

---

#### **11.3 semop：操作信号量集合**
- **函数原型**  
  ```c
  int semop(int semid, struct sembuf *sops, unsigned nsops);
  ```
- **关键结构体**  
  ```c
  struct sembuf {
      unsigned short sem_num;  // 信号量在集合中的索引
      short sem_op;   // 操作值（+1: V操作，-1: P操作）
      short sem_flg;  // 标志（如 IPC_NOWAIT, SEM_UNDO）
  };
  ```
- **原子操作**  
  `semop` 可一次性对多个信号量进行原子操作（通过 `nsops` 指定操作数）。
- **必须掌握的代码**  
  ```c
  // P操作（申请资源）
  struct sembuf p_op = {0, -1, SEM_UNDO}; // 操作集合中的第0个信号量
  if (semop(semid, &p_op, 1) == -1) {
      perror("semop P error");
  }

  // V操作（释放资源）
  struct sembuf v_op = {0, 1, SEM_UNDO};
  if (semop(semid, &v_op, 1) == -1) {
      perror("semop V error");
  }
  ```
  **说明**：`SEM_UNDO` 标志确保进程异常终止时自动撤销操作。

---

#### **11.4 semctl：控制信号量集合**
- **函数原型**  
  ```c
  int semctl(int semid, int semnum, int cmd, ... /* union semun arg */);
  ```
- **关键命令（`cmd`）**  
  - `IPC_RMID`：删除信号量集合。
  - `SETVAL`：设置单个信号量的值。
  - `GETVAL`：获取单个信号量的值。
  - `IPC_STAT`：获取信号量集合的状态信息。
- **联合体 `semun`**  
  ```c
  union semun {
      int val;                // 用于 SETVAL
      struct semid_ds *buf;   // 用于 IPC_STAT/IPC_SET
      unsigned short *array;  // 用于 SETALL/GETALL
  };
  ```
- **必须掌握的代码**  
  ```c
  // 初始化信号量值为1
  union semun arg;
  arg.val = 1;
  if (semctl(semid, 0, SETVAL, arg) == -1) {
      perror("semctl SETVAL error");
  }

  // 删除信号量集合
  if (semctl(semid, 0, IPC_RMID) == -1) {
      perror("semctl IPC_RMID error");
  }
  ```

---

#### **11.5 信号量集合**
- **信号量集合结构体**  
  ```c
  struct semid_ds {
      struct ipc_perm sem_perm;  // 权限信息
      unsigned short sem_nsems;  // 集合中信号量数量
      time_t sem_otime;          // 最后一次操作时间
      time_t sem_ctime;          // 最后一次修改时间
  };
  ```
- **原子操作的优点**  
  通过 `semop` 对多个信号量进行原子操作，避免死锁（如同时申请多个资源）。

---

#### **11.6 文件上锁**
- **使用信号量实现文件锁**  
  通过信号量协调对文件的访问（类似互斥锁）。
- **对比其他锁机制**  
  - `fcntl` 记录锁：更轻量，但仅限单机。
  - 信号量锁：支持跨进程复杂同步。

---

#### **11.7 信号量限制**
- **系统限制**  
  - `SEMVMX`：信号量值的最大值（通常为32767）。
  - `SEMMNI`：系统允许的信号量集合数量。
  - `SEMMSL`：单个集合允许的最大信号量数量。
- **查看限制**  
  ```bash
  sysctl kernel.sem  # 输出示例：250 32000 32 128
  ```
  对应参数：`SEMMSL`, `SEMMNS`, `SEMOPM`, `SEMMNI`。

---

### **必须掌握的代码总结**
1. **创建信号量集合**  
   ```c
   int semid = semget(key, nsems, flags);
   ```
2. **PV操作**  
   ```c
   semop(semid, &sembuf, nsops);
   ```
3. **初始化和删除**  
   ```c
   semctl(semid, semnum, SETVAL, arg);
   semctl(semid, 0, IPC_RMID);
   ```
4. **文件锁示例**  
   使用信号量实现文件访问的互斥（代码略，需结合 `open` 和 `semop`）。

---

### **重点注意事项**
1. **原子操作**  
   确保 `semop` 操作的原子性，避免死锁。
2. **资源泄漏**  
   进程终止前必须释放信号量（或使用 `SEM_UNDO`）。
3. **键值管理**  
   使用 `ftok` 生成唯一键值时，确保文件路径稳定。
4. **信号量删除**  
   只有创建者或超级用户可删除信号量集合（`IPC_RMID`）。
