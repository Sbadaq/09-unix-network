#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define MAX_TRIES   10
#define MSGSIZE(x) (x)
#define FILESIZE(x, y) (sizeof(struct mq_hdr) + x * (sizeof(struct msg_hdr) + y))
#define MQI_MAGIC 0x12345678

// 假设的结构体定义
struct mq_hdr {
    struct mq_attr mqh_attr;
    int mqh_nwait;
    pid_t mqh_pid;
    long mqh_head;
    long mqh_free;
    pthread_mutex_t mqh_lock;
    pthread_cond_t mqh_wait;
    struct sigevent mqh_event;
};

struct msg_hdr {
    long msg_next;
    long msg_prev;
    unsigned int msg_prio;
    size_t msg_len;
};

struct mq_info {
    struct mq_hdr *mqi_hdr;
    int mqi_magic;
    int mqi_flags;
};

struct mq_attr defattr = {0, 128, 1024, 0};

mqd_t mq_open(const char* path_name, int oflag, ...) {
    int i, fd, nonblock, created, save_errno;
    long msgsize, filesize, index;
    va_list ap;
    mode_t mode;
    int8_t *mptr;
    struct stat statbuff;
    struct mq_hdr *mqhdr;
    struct msg_hdr *msghdr;
    struct mq_attr *attr;
    struct mq_info *mqinfo;
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    created = 0;
    nonblock = oflag & O_NONBLOCK;
    oflag &= ~O_NONBLOCK;
    mptr = (int8_t *)MAP_FAILED;
    mqinfo = NULL;

    int retryies = 0;
again:
    if (oflag & O_CREAT) {
        printf("%d, Creating a new message queue\n  ",__LINE__);

        va_start(ap, oflag);
        mode = va_arg(ap, mode_t) & ~S_IXUSR;
        attr = va_arg(ap, struct mq_attr *);
        va_end(ap);

        fd = open(path_name, O_CREAT | O_EXCL | O_RDWR, mode);
        const char *errmsg = strerror(errno);
        if (fd < 0) {
            if (errmsg != NULL) {
                printf("%d,error: %s\n", __LINE__, errmsg);
            } else {
                printf("%d,error: Unknown error\n", __LINE__);
            }
            retryies++;
            if (retryies < MAX_TRIES) {
                printf("%d,retrying ? no \n", __LINE__);
            } else if (errno == ENOENT) {
                printf("%d,error: %s\n", __LINE__, strerror(errno));
                return -1;
            }
            return -1;
        }
        if (fd < 0) {
            if (errno == EEXIST && (oflag & O_EXCL == 0)) {
                printf("%d,already exist\n",__LINE__);
                goto again;
            } else {
                printf("%d,error: %s\n", __LINE__, strerror(errno));
                return -1;
            }
        }
        printf("%d,oepn fd:%d\n",__LINE__,fd);
        created = 1;

        if (attr == NULL) {
            attr = &defattr;
        } else {
            if (attr->mq_maxmsg <= 0 || attr->mq_msgsize <= 0) {
                printf("%d,error: %s\n", __LINE__, strerror(errno));
                return -1;
            }
        }

        printf("%d,attr->mq_maxmsg:%ld\n",__LINE__,attr->mq_maxmsg);

        msgsize = MSGSIZE(attr->mq_msgsize);
        filesize = FILESIZE(attr->mq_maxmsg, msgsize);
        if (lseek(fd, filesize - 1, SEEK_SET) == -1) {
            printf("%d,error: %s\n",__LINE__, strerror(errno));
            return -1;
        }

        if (write(fd, "", 1) == -1) {
            printf("%d,error: %s\n",__LINE__, strerror(errno));
            return -1;
        }

        printf("%d,mmap\n",__LINE__);
        mptr = (int8_t*)mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mptr == MAP_FAILED) {
            printf("%d,error: %s\n", __LINE__, strerror(errno));
            return -1;
        }

        if ((mqinfo = (struct mq_info*)malloc(sizeof(struct mq_info))) == NULL) {
            printf("%d,error: %s\n", __LINE__, strerror(errno));
            return -1;
        }

        printf("%d,mqinfo:%p\n",__LINE__,mqinfo);

        mqinfo->mqi_hdr = mqhdr = (struct mq_hdr *)mptr;
        mqinfo->mqi_magic = MQI_MAGIC;
        mqinfo->mqi_flags = nonblock;

        mqhdr->mqh_attr.mq_flags = 0;
        mqhdr->mqh_attr.mq_maxmsg = attr->mq_maxmsg;
        mqhdr->mqh_attr.mq_msgsize = attr->mq_msgsize;
        mqhdr->mqh_attr.mq_curmsgs = 0;
        mqhdr->mqh_nwait = 0;
        mqhdr->mqh_pid = 0;
        mqhdr->mqh_head = 0;

        index = sizeof(struct mq_hdr);
        mqhdr->mqh_free = index;
        for (i = 0; i < attr->mq_maxmsg; i++) {
            msghdr = (struct msg_hdr *)(mptr + index);
            index += sizeof(struct msg_hdr) + msgsize;
            msghdr->msg_next = index;
        }
        msghdr = (struct msg_hdr *)(mptr + index - 1);
        msghdr->msg_next = 0;
        printf("%d,mqhdr:%p\n",__LINE__,mqhdr);

        if ((i = pthread_mutexattr_init(&mattr)) != 0) {
            printf("%d,error: %s\n", __LINE__, strerror(errno));
            return -1;
        }

        if ((i = pthread_condattr_init(&cattr)) != 0) {
            printf("%d,error: %s\n", __LINE__, strerror(errno));
            return -1;
        }

        pthread_condattr_destroy(&cattr);
        if (i != 0) {
            printf("%d,error: %s\n", __LINE__, strerror(errno));
            return -1;
        }

        if (fchmod(fd, mode) == -1) {
            printf("%d,error: %s\n", __LINE__, strerror(errno));
            return -1;
        }

        close(fd);
        printf("%d,done\n",__LINE__);
        return ((mqd_t)-1);
    }

exists:
    if ((fd = open(path_name, O_RDWR)) < 0) {
        if (errno == ENOENT && (oflag & O_CREAT)) {
            goto again;
        } else {
            return -1;
        }
    }

    for (i = 0; i < MAX_TRIES; i++) {
        if (stat(path_name, &statbuff) == -1) {
            if (errno == ENOENT && (oflag & O_CREAT)) {
                goto again;
            } else {
                return -1;
            }
        }
        sleep(1);
    }

    if (i == MAX_TRIES) {
        errno = ETIMEDOUT;
        return -1;
    }

    filesize = statbuff.st_size;
    mptr = (int8_t*)mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mptr == MAP_FAILED) {
        return -1;
    }

    close(fd);
    errno = save_errno;
    return ((mqd_t)-1);
}

int mq_close(mqd_t mqd) {
    long msgsize, filesize;
    struct mq_hdr *mqhdr;
    struct mq_attr* attr;
    struct mq_info *mqinfo;

    mqinfo = (struct mq_info *)mqd;

    if (mqinfo->mqi_magic != MQI_MAGIC) {
        return -1;
    }

    mqhdr = mqinfo->mqi_hdr;
    attr = &mqhdr->mqh_attr;

    if (mq_notify(mqd, NULL) != 0) {
        return -1;
    }

    msgsize = MSGSIZE(attr->mq_msgsize);
    filesize = FILESIZE(attr->mq_maxmsg, msgsize);

    if (munmap((void *)mqinfo->mqi_hdr, filesize) == -1) {
        return -1;
    }

    mqinfo->mqi_hdr = NULL;
    free(mqinfo);
    return 0;
}

int mq_unlink(const char *path_name) {
    if (unlink(path_name) == -1) {
        return -1;
    }
    return 0;
}

int mq_getattr(mqd_t mqd, struct mq_attr *mqstat) {
    struct mq_hdr *mqhdr;
    struct mq_info *mqinfo;
    struct mq_attr *attr;

    mqinfo = (struct mq_info *)mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        return -1;
    }

    mqhdr = mqinfo->mqi_hdr;
    attr = &mqhdr->mqh_attr;

    mqstat->mq_flags = mqinfo->mqi_flags;
    mqstat->mq_maxmsg = attr->mq_maxmsg;
    mqstat->mq_msgsize = attr->mq_msgsize;
    mqstat->mq_curmsgs = attr->mq_curmsgs;

    return 0;
}

int mq_setattr(mqd_t mqd, const struct mq_attr *newattr, struct mq_attr *oldattr) {
    int n;
    struct mq_hdr *mqhdr;
    struct mq_attr *attr;
    struct mq_info *mqinfo;

    mqinfo = (struct mq_info *)mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        return -1;
    }

    mqhdr = mqinfo->mqi_hdr;
    attr = &mqhdr->mqh_attr;

    if ((n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) {
        return -1;
    }

    if (oldattr != NULL) {
        oldattr->mq_flags = mqinfo->mqi_flags;
        oldattr->mq_maxmsg = attr->mq_maxmsg;
        oldattr->mq_msgsize = attr->mq_msgsize;
        oldattr->mq_curmsgs = attr->mq_curmsgs;
    }

    if (newattr->mq_flags & O_NONBLOCK) {
        mqinfo->mqi_flags |= O_NONBLOCK;
    } else {
        mqinfo->mqi_flags &= ~O_NONBLOCK;
    }

    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return 0;
}

int mq_notify(mqd_t mqd, const struct sigevent *notification) {
    int n;
    pid_t pid;
    struct mq_hdr *mqhdr;
    struct mq_info *mqinfo;

    mqinfo = (struct mq_info *)mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        errno = EINVAL;
        return -1;
    }

    mqhdr = mqinfo->mqi_hdr;
    if ((n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) {
        errno = n;
        return -1;
    }

    pid = getpid();
    if (notification == NULL) {
        if (mqhdr->mqh_pid == pid) {
            mqhdr->mqh_pid = 0; /* unregister calling process */
        } /* no error if caller not registered */
    } else {
        if (mqhdr->mqh_pid != 0) {
            if (kill(mqhdr->mqh_pid, 0) != -1 || errno != ESRCH) {
                errno = EBUSY;
                goto err;
            }
        }
        mqhdr->mqh_pid = pid;
        mqhdr->mqh_event = *notification;
    }

    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return 0;

err:
    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return -1;
}


int mq_send(mqd_t mqd, const char *ptr, size_t len, unsigned int prio) {
    struct mq_info *mqinfo = (struct mq_info *)mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        errno = EINVAL;
        return -1;
    }

    struct mq_hdr *mqhdr = mqinfo->mqi_hdr;
    struct mq_attr *attr = &mqhdr->mqh_attr;
    int8_t *mptr = (int8_t *)mqhdr;

    pthread_mutex_lock(&mqhdr->mqh_lock);

    /* 检查消息长度 */
    if (len > attr->mq_msgsize) {
        errno = EMSGSIZE;
        pthread_mutex_unlock(&mqhdr->mqh_lock);
        return -1;
    }

    /* 检查队列是否满 */
    if (attr->mq_curmsgs >= attr->mq_maxmsg) {
        if (mqinfo->mqi_flags & O_NONBLOCK) {
            errno = EAGAIN;
            pthread_mutex_unlock(&mqhdr->mqh_lock);
            return -1;
        }
        while (attr->mq_curmsgs >= attr->mq_maxmsg) {
            pthread_cond_wait(&mqhdr->mqh_wait, &mqhdr->mqh_lock);
        }
    }

    /* 从空闲链表获取节点 */
    long freeindex = mqhdr->mqh_free;
    if (freeindex == 0) {
        errno = EAGAIN; // 实际应不可能发生，因前面已检查
        pthread_mutex_unlock(&mqhdr->mqh_lock);
        return -1;
    }

    struct msg_hdr *msghdr = (struct msg_hdr *)(mptr + freeindex);
    mqhdr->mqh_free = msghdr->msg_next;

    /* 填充消息内容 */
    msghdr->msg_prio = prio;
    msghdr->msg_len = len;
    memcpy(msghdr + 1, ptr, len);

    /* 插入优先级队列 */
    long prev = 0;
    long current = mqhdr->mqh_head;
    while (current != 0) {
        struct msg_hdr *cur = (struct msg_hdr *)(mptr + current);
        if (prio > cur->msg_prio) break;
        prev = current;
        current = cur->msg_next;
    }

    msghdr->msg_next = current;
    msghdr->msg_prev = prev;

    if (prev == 0) {
        mqhdr->mqh_head = freeindex;
    } else {
        ((struct msg_hdr *)(mptr + prev))->msg_next = freeindex;
    }

    if (current != 0) {
        ((struct msg_hdr *)(mptr + current))->msg_prev = freeindex;
    }

    attr->mq_curmsgs++;

    /* 唤醒接收者 */
    if (attr->mq_curmsgs == 1) {
        pthread_cond_signal(&mqhdr->mqh_wait);
    }

    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return 0; 

}

ssize_t mq_receive(mqd_t mqd, char *ptr, size_t maxlen, unsigned int *priop)
{
    int n;
    long index;
    int8_t *mptr;
    ssize_t len;
    struct mq_info *mqinfo;
    struct mq_hdr *mqhdr;
    struct mq_attr *attr;
    struct msg_hdr *msghdr;

    mqinfo = (struct mq_info *)mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        errno = EINVAL;
        return(-1);
    }
    mqhdr = mqinfo->mqi_hdr; /* struct pointer */
    mptr = (int8_t *)mqhdr; /* byte pointer */

    attr = &mqhdr->mqh_attr;
    if ((n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) {
        errno = n;
        return(-1);
    }
    if (maxlen < attr->mq_msgsize) {
        errno = EMSGSIZE;
        goto err;
    }
    if (attr->mq_curmsgs == 0) { /* queue is empty */
        if (mqinfo->mqi_flags & O_NONBLOCK) {
            errno = EAGAIN;
            goto err;
        }
        /* wait for a message to be placed onto queue */
        mqhdr->mqh_nwait++;
        while (attr->mq_curmsgs == 0)
            pthread_cond_wait(&mqhdr->mqh_wait, &mqhdr->mqh_lock);
        mqhdr->mqh_nwait--;
    }
    if ((index = mqhdr->mqh_head) == 0)
    {
        printf("mq_receive: curmsgs = %ld; head = 0", attr->mq_curmsgs);
        goto err;
    }
    msghdr = (struct msg_hdr *) &mptr[index];
    mqhdr->mqh_head = msghdr->msg_next; /* new head of list */
    len = msghdr->msg_len;
    memcpy(ptr, msghdr + 1, len); /* copy the message itself */
    if (priop != NULL)
        *priop = msghdr->msg_prio;
    /* just-removed message goes to front of free list */
    msghdr->msg_next = mqhdr->mqh_free;
    mqhdr->mqh_free = index;

    /* wake up anyone blocked in mq_send waiting for room */
    if (attr->mq_curmsgs == attr->mq_maxmsg)
        pthread_cond_signal(&mqhdr->mqh_wait);
    attr->mq_curmsgs--;

    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return(len);
err:
    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return(-1);
}
    