#include "daemon.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>

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

void read_load(char *buf){
    FILE* fp = fopen("/proc/loadavg", "r");
    if(!fp){
        strcpy(buf, "0.0 0.0 0.0");
        return;
    }

    fgets(buf, 128, fp);
    fclose(fp);
}

int main(int argc, char *argv[])
{
    daemonsize();
    
    mqd_t mq_sender = init_queue(MONITOR_QUEUE, 1);//初始化消息队列
    mqd_t mq_recv = init_queue(LOGGER_QUEUE, 0);//
    struct daemon_msg msg;
    while(1){
        msg.type = MSG_LOAD;
        read_load(msg.data);
        if(mq_send(mq_sender, (char*)&msg, sizeof(msg),0) == - 1){
            sleep(5);//发送失败，等待5秒后重试
            continue;
        }
 
        if(mq_receive(mq_recv, (char*)&msg, sizeof(msg), 0) == -1){
            if(msg.type == MSG_CONTROL){
                if(strcmp(msg.data, "RELOAD") == 0){   
                    printf("RELOAD");
                    continue;
                }
            }
        }
        sleep(3);
    }

    return 0;
}