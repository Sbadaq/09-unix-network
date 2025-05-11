### **《UNIX网络编程卷2：进程间通信》第6章复习大纲——System V消息队列**

---

### **一、消息队列概述** 
- **核心概念**：
  - System V消息队列是内核维护的链表结构，通过唯一的**键值（key）**标识。
  - 与POSIX消息队列的区别：
    - **持久性**：System V队列在内核中持久存在，需显式删除（`msgctl(IPC_RMID)`）。
    - **跨进程通信**：可通过键值访问，无需共享文件描述符。
  - 消息结构：每条消息包含**类型（mtype）**和**数据（mtext）**，支持按类型选择性读取。

---

### **二、核心函数详解**
#### **1. `msgget`：创建/打开队列**
```c
#include <sys/msg.h>
int msgget(key_t key, int msgflg);
```
- **参数**：
  - `key`：通过`ftok`生成或直接指定（如`0x1234`），若为`IPC_PRIVATE`则创建私有队列。
  - `msgflg`：权限标志组合（`IPC_CREAT | 0666`），`IPC_EXCL`确保唯一创建。
- **返回值**：成功返回队列标识符`msqid`，失败返回`-1`。
- **代码示例**：
  ```c
  key_t key = ftok(".", 'a'); // 当前目录生成key
  int msqid = msgget(key, IPC_CREAT | 0666);
  ```

#### **2. `msgsnd`：发送消息**
```c
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
```
- **参数**：
  - `msgp`：指向消息结构体（需包含`long mtype`）。
  - `msgsz`：消息数据长度（不含`mtype`）。
  - `msgflg`：`IPC_NOWAIT`非阻塞，队列满时立即返回`-1`。
- **代码示例**：
  ```c
  struct msgbuf {
      long mtype;
      char mtext[512];
  };
  struct msgbuf msg = {1, "Hello, System V!"};
  msgsnd(msqid, &msg, sizeof(msg.mtext), 0);
  ```

#### **3. `msgrcv`：接收消息**
```c
ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
```
- **参数**：
  - `msgtyp`：
    - `0`：读取队列第一条消息。
    - `>0`：读取类型等于`msgtyp`的第一条消息。
    - `<0`：读取类型小于等于`|msgtyp|`的最小消息。
  - `msgflg`：`MSG_NOERROR`允许截断超长消息。
- **代码示例**：
  ```c
  struct msgbuf msg;
  msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0); // 接收类型为1的消息
  ```

#### **4. `msgctl`：控制队列**
```c
int msgctl(int msqid, int cmd, struct msqid_ds *buf);
```
- **命令（cmd）**：
  - `IPC_STAT`：获取队列属性（如`msg_qnum`当前消息数）。
  - `IPC_SET`：修改队列属性（如`msg_qbytes`最大字节数）。
  - `IPC_RMID`：立即删除队列（需权限）。
- **代码示例**：
  ```c
  struct msqid_ds info;
  msgctl(msqid, IPC_STAT, &info); // 获取状态
  msgctl(msqid, IPC_RMID, NULL);  // 删除队列
  ```

---

### **三、关键代码与示例**
#### **1. 基本发送-接收流程**
- **发送端**：
  ```c
  key_t key = ftok(".", 'a');
  int msqid = msgget(key, IPC_CREAT | 0666);
  struct msgbuf msg = {1, "Test message"};
  msgsnd(msqid, &msg, sizeof(msg.mtext), 0);
  ```
- **接收端**：
  ```c
  int msqid = msgget(key, 0666);
  struct msgbuf msg;
  msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0);
  printf("Received: %s\n", msg.mtext);
  msgctl(msqid, IPC_RMID, NULL); // 删除队列
  ```

#### **2. Client-Server模型**
- **Server**：创建队列并循环接收消息：
  ```c
  while (1) {
      msgrcv(msqid, &msg, sizeof(msg.mtext), 0, 0);
      // 处理消息并回复
  }
  ```
- **Client**：发送请求并等待响应（需设计消息类型区分请求/响应）。

#### **3. 复用消息（按类型处理）**
- **示例**：接收不同类型消息并分发处理：
  ```c
  struct msgbuf msg;
  while (1) {
      msgrcv(msqid, &msg, sizeof(msg.mtext), -100, 0); // 接收类型≤100的消息
      switch (msg.mtype) {
          case 1: /* 处理类型1 */ break;
          case 2: /* 处理类型2 */ break;
      }
  }
  ```

---

### **四、高级主题与注意事项**
#### **1. 消息队列限制**
- **内核参数**：通过`msgctl(IPC_STAT)`获取`msg_qbytes`（最大字节数）、`msg_qnum`（当前消息数）。
- **调整限制**：使用`msgctl(IPC_SET)`修改`msg_qbytes`（需超级用户权限）。

#### **2. 错误处理**
- **常见错误码**：
  - `EACCES`：权限不足。
  - `ENOMSG`：队列空且`IPC_NOWAIT`被设置。
  - `EIDRM`：队列被删除。

#### **3. 与`select`/`poll`的整合**
- **限制**：System V消息队列无法直接与`select`/`poll`结合（需通过线程或信号驱动I/O）。
- **替代方案**：使用管道或信号通知消息到达。

---

### **五、必须掌握的要点总结**
1. **队列生命周期管理**：显式删除队列避免资源泄漏（`IPC_RMID`）。
2. **消息类型设计**：合理规划`mtype`实现优先级或分类处理。
3. **阻塞与非阻塞模式**：`IPC_NOWAIT`的使用场景。
4. **权限控制**：`msg_perm`结构体中的用户/组权限设置。

---

通过此大纲，结合代码实践和系统调用手册（`man msgget`等），可全面掌握System V消息队列的核心机制与应用场景。