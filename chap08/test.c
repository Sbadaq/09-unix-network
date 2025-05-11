#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "pthread_rdlock.h"

#define NUM_READERS 3
#define NUM_WRITERS 2
#define READ_DELAY 2 // seconds
#define WRITE_DELAY 1 // seconds

my_rwlock_t rwlock;
int shared_resource = 0;

void* reader(void* arg) {
    int id = *(int*)arg;
    free(arg);

    printf("Reader %d: Trying to acquire read lock...\n", id);
    my_rwlock_rdlock(&rwlock);
    printf("Reader %d: Acquired read lock.\n", id);

    // Simulate reading from shared resource
    printf("Reader %d: Reading shared resource (value: %d).\n", id, shared_resource);
    sleep(READ_DELAY);

    printf("Reader %d: Releasing read lock.\n", id);
    my_rwlock_unlock(&rwlock);

    return NULL;
}

void* writer(void* arg) {
    int id = *(int*)arg;
    free(arg);

    printf("Writer %d: Trying to acquire write lock...\n", id);
    my_rwlock_wrlock(&rwlock);
    printf("Writer %d: Acquired write lock.\n", id);

    // Simulate writing to shared resource
    shared_resource++;
    printf("Writer %d: Writing to shared resource (new value: %d).\n", id, shared_resource);
    sleep(WRITE_DELAY);

    printf("Writer %d: Releasing write lock.\n", id);
    my_rwlock_unlock(&rwlock);

    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int* ids;

    // Initialize the read-write lock
    if (pthread_rdlock_init(&rwlock, NULL) != 0) {
        fprintf(stderr, "Failed to initialize read-write lock.\n");
        return EXIT_FAILURE;
    }

    // Create reader threads
    for (int i = 0; i < NUM_READERS; i++) {
        ids = malloc(sizeof(int));
        *ids = i + 1;
        if (pthread_create(&readers[i], NULL, reader, ids) != 0) {
            fprintf(stderr, "Failed to create reader thread %d.\n", i + 1);
            return EXIT_FAILURE;
        }
        printf("Reader %d: Created.\n", i + 1);
    }

    // Create writer threads
    for (int i = 0; i < NUM_WRITERS; i++) {
        ids = malloc(sizeof(int));
        *ids = i + 1;
        if (pthread_create(&writers[i], NULL, writer, ids) != 0) {
            fprintf(stderr, "Failed to create writer thread %d.\n", i + 1);
            return EXIT_FAILURE;
        }
        printf("Writer %d: Created.\n", i + 1);
    }

    // Wait for all reader threads to finish
    for (int i = 0; i < NUM_READERS; i++) {
        printf("Waiting for reader %d to finish...\n", i + 1);
        pthread_join(readers[i], NULL);
    }

    // Wait for all writer threads to finish
    for (int i = 0; i < NUM_WRITERS; i++) {
        printf("Waiting for writer %d to finish...\n", i + 1);
        pthread_join(writers[i], NULL);
    }

    // Destroy the read-write lock
    my_rwlock_destroy(&rwlock);

    printf("All threads have finished. Final shared resource value: %d\n", shared_resource);

    return EXIT_SUCCESS;
}