# 第五章 posix消息队列 

## 5.1 概述
posix消息队列是linux系统提供的一种消息传递机制，消息队列是先进先出（FIFO）的，消息队列中的消息是可读可写的，消息队列中的消息是可共享的，消息队列中的消息是可持久化的。
消息队列的创建和删除，以及消息的读写都是原子操作，所以消息队列是线程安全的。
posix消息队列和管道的区别：
 
消息队列和管道都是用于进程间通信的机制，但存在一些区别。
- 管道是字节流，消息队列是字节块；
- 管道是半双工的，只能用于读写；
消息队列的创建和删除，以及消息的读写都是原子操作，所以消息队列是线程安全的。

消息队列具有 随内核的一致性，这一点与fifo不同，fifo是随文件系统的一致性。
## 5.2 posix消息队列的创建
mq_open函数用于创建或打开一个消息队列。
mq_clo函数用于关闭一个消息队列。
mq_unlink函数用于删除一个消息队列。

## 5.3 posix消息队列的四个属性
mq_getattr函数用于获取消息队列的属性。
mq_setattr函数用于设置消息队列的属性。

## 5.4 mq_send函数和mq_receive函数
mq_send函数用于向消息队列中发送消息。
mq_receive函数用于从消息队列中接收消息。
mq_send函数和mq_receive函数都是原子操作，所以它们是线程安全的。

## 5.5 消息队列 的限制
消息队列的最大长度是1MB，消息队列的最大数量是1024。
mq_mqxmsgs函数用于获取消息队列的最大数量。
mq_msgsize 函数用于获取消息队列的最大长度。

## 5.6 mq_notify函数
mq_notify函数用于注册一个信号，当消息队列中有消息时，会发送这个信号。
mq_notify函数是异步的，所以它可以在消息队列中有消息时，立即发送信号。
mq_notify函数的参数是信号和消息队列。

### 5.6.1 简单信号通知
这个例子是先注册一个信号，SIGUSR1信号。当消息队列中有消息时，会发送这个信号。
处理信号的函数是sig_usr函数。
```
    signal(SIGUSR1, sig_usr);
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGUSR1;
    mq_notify(mqd, &sev);
```
这个例子的关键问题是 ，信号和处理是自导自演的。
实际上不可能注册和处理都是在一个程序中完成的。
 
### 5.6.2 异步信号安全函数
### 5.6.3 信号通知
避免从信号处理程序中调用任何函数 的方法之一是：让处理程序仅设置一个全局标志，
由某信线程检查这个标志，以确定何时收到消息。
这个例子中 我们使用了3个信号集 sigset_t 
```
 struct sigevent {
                  int             sigev_notify; /* Notification type */
                  int             sigev_signo;  /* Signal number */
                  union sigval    sigev_value;  /* Signal value */
                  void          (*sigev_notify_function)(union sigval);
                                                /* Notification function */
                  pthread_attr_t *sigev_notify_attributes;
                                                /* Notification attributes */
              };
```
The use of sigprocmask() is unspecified in a multithreaded process; see pthread_sigmask(3).

### 5.6.4 使用非阻塞的mq_receive信号通知
打开队列 就是创建一个非阻塞的队列，接收数据时判断EAGAIN ；
在这段代码中，sigprocmask(SIG_BLOCK, &newmask, &oldmask); 这一行代码将 SIGUSR1 信号添加到信号掩码中，从而阻塞了该信号。

```
mq = mq_open(argv[1],O_RDONLY|O_NONBLOCK);
sigprocmask(SIG_BLOCK, &newmask, &oldmask);
if(errno != EAGAIN)
{
    printf("mq_receive error\n" );

}else{
    // mq_notify(mq, &sev);
    printf("mq_receive EAGAIN,this means read no data more.\n");
}

 sigprocmask(SIG_UNBLOCK, &newmask, NULL);
```
`为什么读完消息再次把队列打开（why ?? `

**阻塞信号的原因**
- 在这段代码中，sigprocmask(SIG_BLOCK, &newmask, &oldmask); 这一行代码将 SIGUSR1 信号添加到信号掩码中，从而阻塞了该信号。
- 阻塞信号的主要目的是为了确保在处理信号相关的操作时，不会被其他信号干扰。例如，在 sigsuspend 函数调用期间，程序会暂停执行，等待信号的到来。如果此时不阻塞 SIGUSR1 信号，可能会导致信号被多次处理或处理顺序混乱。
**处理完信号后解除阻塞的原因**
-当程序处理完 SIGUSR1 信号后，需要再次调用 sigprocmask(SIG_UNBLOCK, &newmask, NULL); 来解除对 SIGUSR1 信号的阻塞。
-这是因为如果不解除阻塞，SIGUSR1 信号将一直被屏蔽，即使后续有新的 SIGUSR1 信号到达，也不会被处理。通过解除阻塞，程序可以继续正常接收和处理 SIGUSR1 信号。

### 5.6.5 使用sigwait 代替信号处理程序的信号通知
运用 sigprocmask(SIG_BLOCK, &newmask, NULL); 把 SIGUSR1 信号给阻塞了，这是为了让 sigwait 函数可以等待这个信号。sigwait 函数自身就会阻塞线程，直至接收到指定信号，当接收到信号之后，它会把信号从信号掩码里移除，并且返回接收到的信号编号。
当 sigwait 接收到 SIGUSR1 信号之后，这个信号就已经不在信号掩码里了，也就是处于未阻塞状态。
这时要再次调用  mq_notify(mq, &sev); 就是重新注册消息队列通知的操作。在处理完当前消息队列里的消息之后，再次调用 mq_notify 函数，使得消息队列可以在下次有新消息到来时，继续通过发送 SIGUSR1 信号来通知程序。

### 5.6.6 使用select的posix消息队列 
### 5.6.7 启动线程 

## 5.7 posix 实时信号
signal_rt

## 5.8 使用内存映射io实现posix消息队列
第七章介绍 互斥锁和条件变量，第13章介绍内存映射io。可以先阅读过这2章再阅读本章。

### 5.8.1 mq_open函数
### 5.8.2 mq_close函数
### 5.8.3 mq_unlink函数
### 5.8.4 mq_getattr函数
### 5.8.5 mq_setattr函数
### 5.8.6 mq_notify函数
### 5.8.7 mq_send函数
### 5.8.8 mq_receive函数


## 5.4 posix消息队列的消息读写
mq_send函数用于向消息队列中发送消息。
mq_receive函数用于从消息队列中接收消息。

## 5.5 posix消息队列的结构体

## 5.6 posix消息队列的通知
my_notify函数用于设置消息队列的通知。
