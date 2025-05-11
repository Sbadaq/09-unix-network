#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define BUFFER_SIZE 10
#define SEM_MUTEX_NAME "mutex"
#define SEM_EMPTY_SLOTS "empty_slots"
#define SEM_FILLED_SLOTS "filled_slots"

int item_count;
#define SHOW_DEADLOCK 1

struct {
    int buffer[BUFFER_SIZE];
    sem_t *mutex, *empty_slots, *filled_slots;
} shared;

void *producer(void*), *consumer(void*);

void cleanup_semaphores() {
    if (sem_close(shared.mutex) == -1) perror("sem_close mutex");
    if (sem_close(shared.empty_slots) == -1) perror("sem_close empty_slots");
    if (sem_close(shared.filled_slots) == -1) perror("sem_close filled_slots");
    
    if (sem_unlink(SEM_MUTEX_NAME) == -1) perror("sem_unlink mutex");
    if (sem_unlink(SEM_EMPTY_SLOTS) == -1) perror("sem_unlink empty_slots");
    if (sem_unlink(SEM_FILLED_SLOTS) == -1) perror("sem_unlink filled_slots");
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <item_count>\n", argv[0]);
        return EXIT_FAILURE;
    }

    item_count = atoi(argv[1]);
    
    cleanup_semaphores();
    
    shared.mutex = sem_open(SEM_MUTEX_NAME, O_CREAT | O_EXCL, 0600, 1);
    shared.empty_slots = sem_open(SEM_EMPTY_SLOTS, O_CREAT | O_EXCL, 0600, BUFFER_SIZE);
    shared.filled_slots = sem_open(SEM_FILLED_SLOTS, O_CREAT | O_EXCL, 0600, 0);
    
    if (shared.mutex == SEM_FAILED || shared.empty_slots == SEM_FAILED || shared.filled_slots == SEM_FAILED) {
        perror("sem_open");
        return EXIT_FAILURE;
    }

    pthread_t producer_thread, consumer_thread;
    if (pthread_create(&producer_thread, NULL, producer, NULL) != 0 ||
        pthread_create(&consumer_thread, NULL, consumer, NULL) != 0) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    cleanup_semaphores();
    return EXIT_SUCCESS;
}

void *producer(void *arg) {
    for(int i = 0; i < item_count; i++) {
#if SHOW_DEADLOCK    
        sem_wait(shared.mutex); //P 操作，减1
        sem_wait(shared.empty_slots);// P 操作，减1
#else
        //应该先确保有空间，再进入临界区
        sem_wait(shared.empty_slots);
        sem_wait(shared.mutex);
#endif        
        shared.buffer[i % BUFFER_SIZE] = i;
        printf("[%ld] Produced: %d\n", syscall(SYS_gettid), i);
        
        sem_post(shared.mutex);
        sem_post(shared.filled_slots);
    }
    return NULL;
}

void *consumer(void *arg) {
    for(int i = 0; i < item_count; i++) {
#if SHOW_DEADLOCK
        sem_wait(shared.mutex);
        sem_wait(shared.filled_slots);
#else
        //应该先确保有数据，再进入临界区
        sem_wait(shared.filled_slots);
        sem_wait(shared.mutex);
#endif        
        printf("[%ld]                Consumed: %d\n", syscall(SYS_gettid), shared.buffer[i % BUFFER_SIZE]);
        
        sem_post(shared.mutex);
        sem_post(shared.empty_slots);
    }
    return NULL;
}