#ifndef __MY_PTHREAD_RWLOCK_H
#define __MY_PTHREAD_RWLOCK_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t rw_mutex;
    pthread_cond_t rw_condreaders;
    pthread_cond_t rw_condwriters;
    int rw_magic;
    int rw_nwaitreaders;
    int rw_nwaitwriters;
    int rw_refcount;
} my_rwlock_t;

#define RW_MAGIC 0x19283756

#define MY_RWLOCK_INITIALIZER {   \ 
    PTHREAD_MUTEX_INITIALIZER,    \ 
    PTHREAD_COND_INITIALIZER,     \ 
    PTHREAD_COND_INITIALIZER,     \ 
    RW_MAGIC, 0, 0, 0 }

typedef struct {
    int attr_flags;
} my_rwlockattr_t;

int my_rwlockattr_init(my_rwlockattr_t *attr);

int my_rwlock_destroy(my_rwlock_t *rw);
int my_rwlock_init(my_rwlock_t *__restrict rw, 
                  const my_rwlockattr_t *__restrict attr);
int my_rwlock_rdlock(my_rwlock_t *rw);
int my_rwlock_tryrdlock(my_rwlock_t *rw);
int my_rwlock_trywrlock(my_rwlock_t *rw);
int my_rwlock_unlock(my_rwlock_t *rw);
int my_rwlock_wrlock(my_rwlock_t *rw);

#endif