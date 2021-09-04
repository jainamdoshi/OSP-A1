#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>



#define PRODUCERS_THEARDS 5
#define CONSUMERS_THEARDS 5
#define ARRAY_SIZE 10
#define PROGRAM_TIME_IN_SEC 10

#define PRODUCING_TIME 0
#define CONSUMING_TIME 0

time_t startTime;
int array[ARRAY_SIZE];
int arrayIndexToAdd = 0;
int arrayIndexToRemove = 0;

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

int flag = 0;

void* producer(void* args) {

    pthread_t currentThreadID = pthread_self();

    while (flag == 0) {
        int random = rand();
        printf("Producer (ID: %lu) is producing a number %d\n", currentThreadID, random);

        sem_wait(&empty);
        pthread_mutex_lock(&mutex);

        array[arrayIndexToAdd] = random;
        printf("Producer (ID: %lu) is adding %d to the array (index: %d)\n", currentThreadID, random, arrayIndexToAdd);
        arrayIndexToAdd++;
        arrayIndexToAdd %= ARRAY_SIZE;
        sleep(PRODUCING_TIME);
        pthread_mutex_unlock(&mutex);
        sem_post(&full);

    }

    return EXIT_SUCCESS;
}

void* consumer(void* args) {
    pthread_t currentThreadID = pthread_self();

    while (flag == 0) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        int item = array[arrayIndexToRemove];
        printf("Consumer (ID: %lu) has consumed a number %d from the array (index: %d)\n", currentThreadID, item, arrayIndexToRemove);
        arrayIndexToRemove++;
        arrayIndexToRemove %= ARRAY_SIZE;
        sleep(CONSUMING_TIME);
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
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
        printf("Producer (ID: %lu) is created\n", producerThreads[i]);
    }

    for (int i = 0; i < CONSUMERS_THEARDS; i++) {
        pthread_create(&consumerThreads[i], NULL, &consumer, NULL);
        printf("Consumer (ID: %lu) is created\n", consumerThreads[i]);
    }

    sleep(10);
    flag = 1;

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