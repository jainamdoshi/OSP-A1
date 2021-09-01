#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>


#define PRODUCERS_THEARDS 5
#define CONSUMERS_THEARDS 5
#define ARRAY_SIZE 10
#define PROGRAM_TIME_IN_SEC 1.0000

time_t startTime;
int array[ARRAY_SIZE];
int arrayIndexToAdd = 0;
int arrayIndexToRemove = 0;

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

void* producer(void* args) {

    pthread_t currentThreadID = pthread_self();

    while ((double)(clock() - startTime) / CLOCKS_PER_SEC < PROGRAM_TIME_IN_SEC) {
        int random = rand();
        printf("%f - Producer (ID: %lu) is producing a number %d\n", (double)(clock() - startTime) / CLOCKS_PER_SEC, currentThreadID, random);

        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        array[arrayIndexToAdd] = random;
        printf("%f - Producer (ID: %lu) is adding %d to the array (index: %d)\n", (double)(clock() - startTime) / CLOCKS_PER_SEC, currentThreadID, random, arrayIndexToAdd);
        pthread_mutex_unlock(&mutex);
        sem_post(&full);

        arrayIndexToAdd++;
        arrayIndexToAdd %= ARRAY_SIZE;
    }

    return EXIT_SUCCESS;
}

void* consumer(void* args) {
    pthread_t currentThreadID = pthread_self();

    while ((double)(clock() - startTime) / CLOCKS_PER_SEC < PROGRAM_TIME_IN_SEC) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        int item = array[arrayIndexToRemove];
        printf("%f - Consumer (ID: %lu) has consumed a number %d from the array (index: %d)\n", (double)(clock() - startTime) / CLOCKS_PER_SEC, currentThreadID, item, arrayIndexToRemove);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);



        arrayIndexToRemove++;
        arrayIndexToRemove %= ARRAY_SIZE;
    }

    return EXIT_SUCCESS;
}

int main(void) {


    srand(time(NULL));
    pthread_t producerThreads[PRODUCERS_THEARDS];
    pthread_t consumerThreads[CONSUMERS_THEARDS];

    pthread_mutex_init(&mutex, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, ARRAY_SIZE);

    startTime = clock();

    for (int i = 0; i < PRODUCERS_THEARDS; i++) {
        pthread_create(&producerThreads[i], NULL, &producer, NULL);
        printf("%f - Producer (ID: %lu) is created\n", (double)(clock() - startTime) / CLOCKS_PER_SEC, producerThreads[i]);
    }

    for (int i = 0; i < CONSUMERS_THEARDS; i++) {
        pthread_create(&consumerThreads[i], NULL, &consumer, NULL);
        printf("%f - Consumer (ID: %lu) is created\n", (double)(clock() - startTime) / CLOCKS_PER_SEC, consumerThreads[i]);
    }

    for (int i = 0; i < PRODUCERS_THEARDS; i++) {
        pthread_join(producerThreads[i], NULL);
    }

    for (int i = 0; i < CONSUMERS_THEARDS; i++) {
        pthread_join(consumerThreads[i], NULL);
    }


    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return EXIT_SUCCESS;
}