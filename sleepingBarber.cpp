#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <vector>
#include <queue>

#define SLOT_SIZE 5
#define BARBER_THREADS 2
#define PROGRAM_TIME_IN_SEC 10
#define MAX_THREADS 32000


pthread_t slot[SLOT_SIZE];
pthread_mutex_t mutex;
pthread_cond_t notifyBarber;

std::queue<int>* emptySeat;
std::queue<int>* occupiedSeat;

int status = 1;
int numCustomer = 0;
int maxServed = 0;

void* barber(void* args) {

    pthread_t id = pthread_self();
    int numServed = 0;

    while (status) {

        if (numServed > maxServed) {
            maxServed = numServed;
        }

        pthread_mutex_lock(&mutex);
        if (numCustomer <= 0 && status) {
            maxServed = numServed;
            printf("Barber (ID: %lu) is sleeping\n", id);

            if (status) {
                pthread_cond_wait(&notifyBarber, &mutex);
            }

        }

        if (numServed > maxServed && status) {
            pthread_cond_signal(&notifyBarber);
            pthread_cond_wait(&notifyBarber, &mutex);
        }

        if (status) {
            pthread_mutex_unlock(&mutex);
            sleep((double)(rand() % 1000) / 1000);
            pthread_mutex_lock(&mutex);
            numCustomer--;
            numServed++;
            if (occupiedSeat->size() > 0) {
                int slotNum = occupiedSeat->front();
                emptySeat->push(slotNum);
                printf("Barber (ID: %lu) served customer on slot %d. Total Served: %d. Slots left %ld\n", id, slotNum, numServed, emptySeat->size() + 1);
                slot[slotNum] = 0;
                occupiedSeat->pop();
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    printf("Barber (ID: %lu) has servered a total of %d customers\n", id, numServed);

    return EXIT_SUCCESS;
}

void* customer(void* args) {

    pthread_t id = pthread_self();
    int customerEntered = 0;
    pthread_mutex_lock(&mutex);
    if (status) {
        if (emptySeat->size() > 0) {
            int slotNum = emptySeat->front();
            slot[slotNum] = id;
            occupiedSeat->push(slotNum);
            numCustomer++;
            printf("Customer (ID: %lu) has arrived the barber shop and is sitting on slot %d. Slots left %ld\n", id, slotNum, emptySeat->size());
            emptySeat->pop();
            customerEntered = 1;
        } else {
            printf("Customer (ID: %lu) left because of no space\n", id);
        }
    }
    pthread_mutex_unlock(&mutex);
    if (customerEntered) {
        // for (int i = 0; i < BARBER_THREADS; i++) {
        pthread_cond_signal(&notifyBarber);
        // }
    }

    return EXIT_SUCCESS;
}



void* customerGenerator(void* args) {
    std::vector<pthread_t> threadsToJoin;

    int threadCount = 0;
    while (status && threadCount < MAX_THREADS) {
        sleep((double)(rand() % 1000) / 1000);
        pthread_t customerThread;
        if (pthread_create(&customerThread, NULL, &customer, NULL) == 0) {
            threadsToJoin.push_back(customerThread);
            threadCount++;
        } else {
            printf("Operating System denied creating new thread\n");
        }

    }

    for (unsigned int i = 0; i < threadsToJoin.size(); i++) {
        pthread_join(threadsToJoin.at(i), NULL);
    }

    if (threadCount == MAX_THREADS) {
        printf("\n\n\nMax threads creation reached. No more customers! Waiting for %d seconds to complete\n\n\n\n", PROGRAM_TIME_IN_SEC);
    }

    return EXIT_SUCCESS;
}

int runSleepingBarber() {

    // Seeding the random generator
    srand(time(NULL));

    // Initializing data structures
    emptySeat = new std::queue<int>();
    occupiedSeat = new std::queue<int>();
    for (int i = 0; i < SLOT_SIZE; i++) {
        emptySeat->push(i);
    }

    pthread_t barberThreads[BARBER_THREADS];
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&notifyBarber, NULL);

    for (int i = 0; i < BARBER_THREADS; i++) {
        pthread_create(&barberThreads[i], NULL, &barber, NULL);
        printf("Barber thread (ID: %lu) is created\n", barberThreads[i]);
    }

    pthread_t customerGeneratorThread;
    pthread_create(&customerGeneratorThread, NULL, &customerGenerator, NULL);

    sleep(PROGRAM_TIME_IN_SEC);
    printf("\nTimes Up! Barber shop closing\n\n");
    status = 0;

    pthread_cond_broadcast(&notifyBarber);

    pthread_join(customerGeneratorThread, NULL);
    for (int i = 0; i < BARBER_THREADS; i++) {
        pthread_join(barberThreads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&notifyBarber);
    delete emptySeat;
    delete occupiedSeat;

    return EXIT_SUCCESS;
}