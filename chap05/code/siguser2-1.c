
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
 int mqflag = 0;
 struct sigevent sev;
 struct mq_attr attr = {
     .mq_flags = 0,
     .mq_maxmsg = 10,
     .mq_msgsize = 50,
     .mq_curmsgs = 0
 };
 
 sigset_t zeromask, newmask, oldmask;
 
 static void sig_handler(int signo) {
     mqflag = 1;
 }
 
 int main(int argc, char **argv) {
     if (argc != 2) {
         printf("Usage: %s <cmd>\n", argv[0]);
         return 1;
     }
 
     mqd = mq_open(argv[1], O_CREAT | O_RDONLY, 0666, &attr);
     if (mqd == (mqd_t)-1) {
         perror("mq_open failed");
         return 1;
     }
 
     if (mq_getattr(mqd, &attr) == -1) {
         perror("mq_getattr failed");
         mq_close(mqd);
         return 1;
     }
 
     buff = malloc(attr.mq_msgsize);
     if (buff == NULL) {
         perror("malloc failed");
         mq_close(mqd);
         return 1;
     }
 
     struct sigaction sa;
     sa.sa_handler = sig_handler;
     sa.sa_flags = 0;
     sigemptyset(&sa.sa_mask);
 
     if (sigaction(SIGUSR2, &sa, NULL) == -1) {
         perror("sigaction failed");
         free(buff);
         mq_close(mqd);
         return 1;
     }
 
     sev.sigev_notify = SIGEV_SIGNAL;
     sev.sigev_signo = SIGUSR2;
 
     sigemptyset(&zeromask);
     sigemptyset(&newmask);
     sigemptyset(&oldmask);
     sigaddset(&newmask, SIGUSR2);
 
     while (1) {
         if (mq_notify(mqd, &sev) == -1) {
             perror("mq_notify failed");
             free(buff);
             mq_close(mqd);
             return 1;
         }
 
         sigprocmask(SIG_BLOCK, &newmask, &oldmask);
         while (mqflag == 0) {
             sigsuspend(&zeromask);
         }
         mqflag = 0;
         sigprocmask(SIG_SETMASK, &oldmask, NULL);
 
         ssize_t n = mq_receive(mqd, buff, attr.mq_msgsize, NULL);
         if (n == -1) {
             perror("mq_receive failed");
             free(buff);
             mq_close(mqd);
             return 1;
         }
 
         ((char *)buff)[n] = '\0'; // Null-terminate the received message
         printf("Received a message: %s\n", (char *)buff);
     }
 
     free(buff);
     mq_close(mqd);
     return 0;
 }
