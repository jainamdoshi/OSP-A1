#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

// All constants used in the program
#define PRODUCERS_THEARDS 5
#define CONSUMERS_THEARDS 5
#define ARRAY_SIZE 10
#define PROGRAM_TIME_IN_SEC 10
#define PRODUCING_TIME 0
#define CONSUMING_TIME 0

// All global variables shared by all threads
int array[ARRAY_SIZE];
int arrayIndexToAdd = 0;
int arrayIndexToRemove = 0;
int pStatus = 1;

pthread_mutex_t arrayMutex;
sem_t empty;
sem_t full;

void* producer(void* args) {
    pthread_t currentThreadID = pthread_self();

    while (pStatus) {

        printf("Producer (ID: %lu) is producing an item\n", currentThreadID);

        // Adding more time for producing
        sleep(PRODUCING_TIME);
        int random = rand();

        // Waiting till there is space in the array
        sem_wait(&empty);

        // Locks and adds the item into the array
        pthread_mutex_lock(&arrayMutex);
        array[arrayIndexToAdd] = random;
        printf("Producer (ID: %lu) is adding %d to the array (index: %d)\n", currentThreadID, random, arrayIndexToAdd);
        arrayIndexToAdd++;
        arrayIndexToAdd %= ARRAY_SIZE;

        // Unlock the mutex and signal other threads who are waiting on full semaphore
        pthread_mutex_unlock(&arrayMutex);
        sem_post(&full);
    }
    return EXIT_SUCCESS;
}

void* consumer(void* args) {
    pthread_t currentThreadID = pthread_self();

    while (pStatus) {
        // Adding more time for producing
        sleep(CONSUMING_TIME);

        // Wait till there is an item in the array
        sem_wait(&full);

        // Locks and removes item from the array
        pthread_mutex_lock(&arrayMutex);
        int item = array[arrayIndexToRemove];
        printf("Consumer (ID: %lu) has consumed a number %d from the array(index: % d)\n", currentThreadID, item, arrayIndexToRemove);
        arrayIndexToRemove++;
        arrayIndexToRemove %= ARRAY_SIZE;

        // Unlock the mutex and signal other threads who are waiting on empty semaphore
        pthread_mutex_unlock(&arrayMutex);
        sem_post(&empty);
    }
    return EXIT_SUCCESS;
}

int runProducerConsumer() {

    // Seeding the random generator
    srand(time(NULL));

    // Intializing mutex and semaphores
    pthread_mutex_init(&arrayMutex, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, ARRAY_SIZE);

    // List of all producer and consumer threads
    pthread_t producerThreads[PRODUCERS_THEARDS];
    pthread_t consumerThreads[CONSUMERS_THEARDS];


    // Creating Producer threads
    for (int i = 0; i < PRODUCERS_THEARDS; i++) {
        pthread_create(&producerThreads[i], NULL, &producer, NULL);
        printf("Producer (ID: %lu) is created\n", producerThreads[i]);
    }

    // Creating Consumer threads
    for (int i = 0; i < CONSUMERS_THEARDS; i++) {
        pthread_create(&consumerThreads[i], NULL, &consumer, NULL);
        printf("Consumer (ID: %lu) is created\n", consumerThreads[i]);
    }

    // Sleep and then change the status
    sleep(PROGRAM_TIME_IN_SEC);
    pStatus = 0;

    // Joining all the threads for no memory loss
    for (int i = 0; i < PRODUCERS_THEARDS; i++) {
        pthread_join(producerThreads[i], NULL);
    }
    for (int i = 0; i < CONSUMERS_THEARDS; i++) {
        pthread_join(consumerThreads[i], NULL);
    }

    // Destorying mutex and semaphores
    pthread_mutex_destroy(&arrayMutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return EXIT_SUCCESS;
}