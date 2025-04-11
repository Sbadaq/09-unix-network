
int main(int argc, char *argv[])
{
    mqd_t mq = mq_open("/test", O_RDWR|O_CREAT, 0666, NULL);
    if (mq == -1) {
        perror("mq_open");
        return 1;
    }

    pid_t poducer = fork();
    if (poducer == 0) {//子进程
        mq_send(mq, "Hello", 5, 0);
        exit(0);
    }
    else if (poducer > 0) {//父进程
        pid_t consumer = fork();
        
    }
}