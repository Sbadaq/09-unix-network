
#include "pthread_rdlock.h"

#define EINVAL  -1
#define EBUSY   -2

/**
 * @brief 初始化读写锁为读锁定状态
 *
 * 该函数用于初始化一个读写锁，并设置其初始状态为读锁定。
 *
 * @param rw 指向需要初始化的读写锁的指针
 * @param attr 指向读写锁属性的指针，若传入NULL则使用默认属性
 *
 * @return 初始化成功返回0，失败返回错误码
 *         - EINVAL: 若attr不为NULL，则传入非空属性时返回该错误码
 *         - 其他错误码：pthread_mutex_init或pthread_cond_init返回的错误码
 */
int pthread_rdlock_init(my_rwlock_t *rw, my_rwlockattr_t *attr)
{
    int result;
    if(attr != NULL)
    {
        return EINVAL;
    }

    if(( result = pthread_mutex_init(&rw->rw_mutex,NULL )) != 0)
    {
        goto err1;
    }

    if(( result = pthread_cond_init(&rw->rw_condreaders, NULL )) != 0)
    {
        goto err2;
    }

    if(( result = pthread_cond_init(&rw->rw_condwriters, NULL )) != 0 )
    {
        goto err3;
    }
    rw->rw_nwaitreaders = 0;
    rw->rw_nwaitwriters = 0;
    rw->rw_refcount = 0;
    rw->rw_magic = RW_MAGIC;

    return 0;
err3:
    pthread_cond_destroy(&rw->rw_condreaders);
err2:
    pthread_cond_destroy(&rw->rw_mutex);
err1:
    return result; 
}

/**
 * @brief 销毁读写锁
 *
 * 销毁指定的读写锁，释放其占用的资源。
 *
 * @param rw 指向要销毁的读写锁的指针
 *
 * @return 返回0表示成功，返回EINVAL表示参数无效，返回EBUSY表示当前无法销毁
 */
int my_rwlock_destroy(my_rwlock_t *rw)
{
    if(rw->rw_magic != RW_MAGIC)
    {
        return EINVAL;
    }
    /*如果引用计数 或者等待读写者计数不为0，则返回EBUSY，当前不能销毁*/
    if(rw->rw_refcount != 0 || rw->rw_nwaitreaders != 0 || rw->rw_nwaitwriters != 0)
    {
        return EBUSY;
    }
    printf("rwlock destroy...\n");
    
    pthread_mutex_destroy(&rw->rw_mutex);
    pthread_cond_destroy(&rw->rw_condreaders);
    pthread_cond_destroy(&rw->rw_condwriters);
    rw->rw_magic = 0;

    return 0;
}

/**
 * @brief 对读写锁进行读锁定
 *
 * 此函数用于对给定的读写锁进行读锁定。如果读写锁已经处于写锁定状态，则当前线程将被阻塞，直到锁被释放。
 *
 * @param rw 指向my_rwlock_t类型的指针，表示要进行读锁定的读写锁。
 *
 * @return 成功时返回0，失败时返回错误码。可能的错误码包括：
 *         - EINVAL：参数无效。
 *         - 其他错误码：由pthread_mutex_lock函数返回。
 */
int my_rwlock_rdlock(my_rwlock_t *rw)
{
    int result;
    if(rw->rw_magic != RW_MAGIC)
    {
        return EINVAL;
    }
    //获取互斥锁
    if(( result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
    {
        return result;
    }
    //rw_refcount 共享锁数量；如果rw_refcount < 0 表示有写锁存在；因为写锁会--rw_refcount
    //如果已经有写锁 或者 有线程在等待写锁，则当前读锁阻塞，意思是：当前线程进入等待队列
    while(rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0)
    {
        rw->rw_nwaitreaders++;
        //调用wait使当前线程等待在变量rw_condreaders上，阻塞
        //这条指令还会释放rw_mutex锁，直到rw_condreaders被唤醒后重新获取该互斥锁
        pthread_cond_wait(&rw->rw_condreaders, &rw->rw_mutex);
        rw->rw_nwaitreaders--;//减少等待读锁的线程数，因为当前线程被唤醒，所以减少等待计数
        if(result != 0)
        {
            break;
        }
    }

    if(result == 0)
    {
        rw->rw_refcount++;//读锁 会 增加rw_refcount的值 
    }

    pthread_mutex_unlock(&rw->rw_mutex);
    return result;
}

int my_rwlock_tryrdlock(my_rwlock_t *rw)
{
    int result;
    if(rw->rw_magic != RW_MAGIC)
    {
        return EINVAL;
    }

    if(( result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
    {
        return result;
    }

    if(rw->rw_refcount <0 || rw->rw_nwaitwriters > 0)
    {
        result = EBUSY;
    }
    else
    {
        rw->rw_refcount++;
    }
    pthread_mutex_unlock(&rw->rw_mutex);
    return result;
}

/**
 * @brief 尝试对读写锁加写锁
 *
 * 此函数尝试对给定的读写锁对象 `rw` 加写锁。如果成功，当前线程将独占访问资源。
 *
 * @param rw 指向要操作的读写锁对象的指针
 *
 * @return 成功返回 0，失败返回错误码。可能的错误码包括：
 *         - `EINVAL`: 传入的读写锁对象无效或已损坏
 *         - 其他错误码由 `pthread_mutex_lock` 或 `pthread_cond_wait` 返回
 */
int my_rwlock_wrlock(my_rwlock_t *rw)
{
    int result;
    if(rw->rw_magic != RW_MAGIC)
    {
        return EINVAL;
    }

    if((result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
    {
        return result;
    }

    while(rw->rw_refcount != 0 )
    {
        rw->rw_nwaitwriters++;
        pthread_cond_wait(&rw->rw_condwriters, &rw->rw_mutex);
        rw->rw_nwaitwriters--;
        if(result != 0) 
            break;
    }
    
    if(result == 0)
    {
        rw->rw_refcount = -1;
    }
    pthread_mutex_unlock(&rw->rw_mutex);
    return result; 
}

int my_rwlock_trywrlock(my_rwlock_t *rw)
{
    int result;
    if(rw->rw_magic != RW_MAGIC)
    {
        return EINVAL;
    }

    if((result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
    {
        return result;
    }

    if(rw->rw_refcount != 0)
    {
        result = EBUSY;
    }
    else
    {
        rw->rw_refcount = -1;
    }
    pthread_mutex_unlock(&rw->rw_mutex);

    return result;
}

int my_rwlock_unlock(my_rwlock_t *rw)
{
    if(rw->rw_magic != RW_MAGIC)
    {
        return EINVAL;
    }

    pthread_mutex_lock(&rw->rw_mutex);
    if(rw->rw_refcount > 0)
    {
        //如果是读锁，则减少计数并唤醒等待的写线程
        rw->rw_refcount--;
    }else if(rw->rw_refcount == -1)
    {
        rw->rw_refcount = 0; 
    }
    else
    {
        pthread_mutex_unlock(&rw->rw_mutex);
        return EINVAL;
    }

    if(rw->rw_nwaitwriters > 0)//写线程在等待
    {
        if(rw->rw_refcount == 0)//没有读锁存在，唤醒写线程
        {
            pthread_cond_signal(&rw->rw_condwriters);
        }
    }else if(rw->rw_nwaitreaders > 0)//读线程在等待
    {
        pthread_cond_broadcast(&rw->rw_condreaders);//读锁可以被多个线程持有
    }

    pthread_mutex_unlock(&rw->rw_mutex);

    return 0;

}