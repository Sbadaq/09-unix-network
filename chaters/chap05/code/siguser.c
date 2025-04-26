

/**
 * P69: 5.6.1 简单的信号 通知 
 * 当有一个消息放入到队列 中，它就产生一个siguser1信号 
 * 注意 这里有一个错误 
 * 
 * 注意 队列 中内容 由空变为非空时会有一个信号产生
 * 
 * */
/**
 * {{filename}} - 
 * Copyright (c) {{year}}, haloo.liu
 * Contact: liujiahao.oak@gmail.com
 * All rights reserved.
 * 
 */

 #include <mqueue.h>
 #include <signal.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <errno.h>
 
 
 void *buff;
 
  mqd_t mqd;
  //struct mq_attr attr;
  struct sigevent sev;
  static void sig_handler(int signo);
  
  struct mq_attr attr = {
     .mq_flags = 0,
     .mq_maxmsg = 10,
     .mq_msgsize = 50,
     .mq_curmsgs = 0
 };
 
  int main(int argc, char **argv)
  {
     if(argc != 2) {
         printf("Usage: %s <cmd>\n", argv[0]);
         return 1;
     }
 
 
 
     //if WRONLY, it cannot read signal
     mqd = mq_open(argv[1], O_CREAT | O_RDWR, 0666, &attr);
     mq_getattr(mqd, &attr);
     buff = malloc(attr.mq_msgsize);
     signal(SIGUSR2, sig_handler);
     sev.sigev_notify = SIGEV_SIGNAL;
     sev.sigev_signo = SIGUSR2;
     mq_notify(mqd, &sev);
     while(1) {
         sleep(1);
     }
     return 0;
 
 }
 
 /**如果被监听 的队列中消息改变，则发送信号，并调用信号处理函数，并且 这个队列中的消息被清空。
  * 也就是说，可能原本 发给其它人的，被监听者收到并清除。
  */
 static void sig_handler(int signo)
 {
     mq_notify(mqd, &sev);
     printf("Received a signal\n");
     mq_receive(mqd, buff, attr.mq_msgsize, NULL);
     printf("Received a message: %s\n", buff);
 }