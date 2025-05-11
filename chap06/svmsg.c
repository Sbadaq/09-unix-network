
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#define SVMSG_MODE 0666
#define MAX_TEXT 512
struct msgbuf{
    long mtype;
    char mtext[MAX_TEXT];
};
// struct msqid_ds{
//     struct msg_perm msg_perm;
//     time_t msg_stime;
//     time_t msg_rtime;
//     time_t msg_ctime;
//     unsigned long int __msg_cbytes;
//     msgqnum_t msg_qnum;
//     msglen_t msg_qbytes;
//     pid_t msg_lspid;
//     pid_t msg_lrpid;
// };

// 方式 1：直接使用标准类型
//printf("cbytes = %lu", (unsigned long)info.__msg_cbytes);

// 方式 2：定义跨平台类型别名（需在头文件中统一）
#if defined(_WIN32)
  typedef unsigned long ulong_t;  // Windows 下 long 为 4 字节
#else
  typedef unsigned long ulong_t;  // Linux 下 long 为 8 字节（注意潜在风险！）
#endif
int main(int argc,char **argv)
{
    int msqid;
    struct msqid_ds info;
    struct msgbuf buf;

    msqid = msgget(IPC_PRIVATE,SVMSG_MODE| IPC_CREAT);
    buf.mtype = 1;
    buf.mtext[0] = 'a';
    msgsnd(msqid,&buf,1,0);
    msgctl(msqid,IPC_STAT,&info);
    printf("read-write %03o, cbytes = %lu, qnum = %lu,qbytes = %lu\n",
    info.msg_perm.mode & 0777 ,(ulong_t)info.msg_cbytes,(ulong_t)info.msg_qnum,(ulong_t)info.msg_qbytes);
    system("ipcs -q");
    msgctl(msqid,IPC_RMID,&info);
    return 0;
}