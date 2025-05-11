#include "daemon.h"
#include "stdlib.h"

#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define LOG_FILE "./daemon.log"

void daemonsize(){
    pid_t pid = fork();
    if(pid < 0){//fork失败
        perror("fork");
        exit(1);
    }
    if(pid > 0){ //父进程退出
        exit(0);
    }
    sleep(10);

    setsid();//创建会话，脱离终端
    umask(0);//屏蔽权限
    chdir("/");//改变工作目录

    close(STDIN_FILENO);//关闭标准输入
    close(STDOUT_FILENO);//关闭标准输出
    close(STDERR_FILENO);//关闭标准错误
}

int main(int argc, char *argv[]){
    printf("daemon start\n");
    daemonsize();
    printf("daemon running\n");
    mqd_t mq_recv = init_queue(LOGGER_QUEUE, 0);
    mqd_t mq_send = init_queue(MONITOR_QUEUE, 1);

    struct daemon_msg msg;
    time_t now;
    char timestamp[64];

    while(1){
        if(mq_receive(mq_recv, (char*)&msg, sizeof(msg), NULL) == -1){
            continue;
        }
        if(msg.type == MSG_LOAD){
            time(&now);
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
            FILE *fp = fopen(LOG_FILE, "a");
            if(fp){
                fprintf(fp, "%s: %s\n", timestamp, msg.data);
                fclose(fp);
            }
        }
    }

    mq_close(mq_recv);
    mq_close(mq_send);
    return 0;
}