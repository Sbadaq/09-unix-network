
#include <unistd.h>
#include <pthread.h> 
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
 


static mqd_t send_queue, recv_queue;

void* send_thread(void *arg)
{
    char buffer[1024];
    while(1){
        fgets(buffer, sizeof(buffer), stdin);
        if(mq_send(send_queue,buffer,strlen(buffer)+ 1, 0) == -1){
            perror("mq_send");
            exit(1);
        }
    }
    return NULL;
}

void* recv_thread(void *arg){
    char buffer[1024];
    while(1){
        if(mq_receive(recv_queue,buffer,sizeof(buffer),NULL) == -1){
            perror("mq_receive");
            exit(1);
        }
        printf("Received: %s\n",buffer);
    }
    return NULL;
}

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage: %s <send> <receive>\n",argv[0]);
        exit(1);
    }

    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = 1024,
        .mq_curmsgs = 0
    };
    send_queue = mq_open(argv[1], O_CREAT | O_WRONLY, 0666, &attr);
    if(send_queue == -1){
        perror("mq_open");
        exit(1);
    }
    recv_queue = mq_open(argv[2], O_CREAT | O_RDONLY, 0666, &attr);
    if(recv_queue == -1){
        perror("mq_open");
        exit(1);
    }
    pthread_t send_tid, recv_tid;
    pthread_create(&send_tid, NULL, send_thread, NULL);
    pthread_create(&recv_tid, NULL, recv_thread, NULL);
    pthread_join(send_tid, NULL);
    pthread_join(recv_tid, NULL);

    mq_close(send_queue);
    mq_close(recv_queue);
    mq_unlink(argv[1]);
    mq_unlink(argv[2]);
    return 0;
}