#ifndef DAEMON_H
#define DAEMON_H

#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>

#define MONITOR_QUEUE  "/load_monnitor_queue"
#define LOGGER_QUEUE   "/log_control_queue"
#define QUEUE_PERM 0666
#define MAX_MSG_SIZE 1024

enum msg_type{
    MSG_LOAD,
    MSG_CONTROL
};

struct daemon_msg{
    enum msg_type type;
    char data[MAX_MSG_SIZE];
};

mqd_t init_queue(const char* name, int is_sender){
    struct mq_attr attr = {
        .mq_maxmsg = 10,
        .mq_msgsize = sizeof(struct daemon_msg)
    };
    int flags = is_sender ? O_CREAT | O_WRONLY : O_RDONLY;
    mqd_t q = mq_open(name, flags, QUEUE_PERM, &attr);
    if(q == (mqd_t) - 1){
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    return q;
}

#endif