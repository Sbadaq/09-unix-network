
# System V 消息队列

---

## 1. 消息队列概述
跟posix 消息队列一样，System V 消息队列也是一种IPC机制。不同的是，它是由System V UNIX系统支持的，而非POSIX标准的一部分。
我们可以将内核中某个特定的消息队列刻画成一个单链表

## 2. msgget
msgget函数用于创建或打开一个消息队列。

## 3. msgsnd
msgsnd函数用于向消息队列中发送消息。

## 4. msgrcv
msgrcv函数用于从消息队列中接收消息。

## 5. msgctl
msgctl函数用于控制消息队列，它可以用来删除一个消息队列或者查询其属性。

---

### 1. msgget
```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int msgget(key_t key, int msgflg);
```
- `key`：消息队列的键值，通常是通过ftok()函数生成的。如果为0，则创建一个新的消息队列。
- `msgflg`：标志位，可以是IPC_CREAT、IPC_EXCL和S_IRUSR | S_IWUSR等组合。
  - IPC_CREAT：如果指定的键不存在，就创建它。
  - IPC_EXCL：与IPC_CREAT一起使用，表示只有当键不存在时才创建新队列。
  - S_IRUSR | S_IWUSR：设置权限，例如只读或可读写。

返回值：成功返回消息队列标识符；失败返回-1并设置errno。

### 2
