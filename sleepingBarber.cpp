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

pthread_t slot[SLOT_SIZE];
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
pthread_cond_t notifyBarber;
pthread_cond_t notifyCustomer;

std::queue<int>* emptySeat;
std::queue<int>* occupiedSeat;

int status = 1;
int numCustomer = 0;
int isBarberAsleep = 1;

void* barber(void* args) {

    pthread_t id = pthread_self();
    int numServed = 0;

    while (status) {


        pthread_mutex_lock(&mutex);

        if (numCustomer <= 0 && status) {
            printf("Barber (ID: %lu) is sleeping\n", id);
            pthread_cond_wait(&notifyBarber, &mutex);
            printf("Barber (ID: %lu) is waking up\n", id);
        }

        pthread_mutex_unlock(&mutex);

        sleep((double)((rand() % 1000)) / 1000);
        numServed++;

        pthread_mutex_lock(&mutex);
        numCustomer--;
        if (occupiedSeat->size() > 0) {
            int slotNum = occupiedSeat->front();
            printf("Barber (ID: %lu) served customer on slot %d. Total Served: %d\n", id, slotNum, numServed);
            slot[slotNum] = 0;
            emptySeat->push(slotNum);
            occupiedSeat->pop();
        }

        pthread_mutex_unlock(&mutex);

    }


    return EXIT_SUCCESS;
}

void* customer(void* args) {

    pthread_t id = pthread_self();

    pthread_mutex_lock(&mutex);
    if (emptySeat->size() > 0) {
        int slotNum = emptySeat->front();
        slot[slotNum] = id;
        occupiedSeat->push(slotNum);
        numCustomer++;
        printf("Customer (ID: %lu) has arrived the barber shop and is sitting on slot %d\n", id, slotNum);
        emptySeat->pop();
    } else {
        printf("Customer (ID: %lu) left because no space available\n", id);
    }

    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&notifyBarber);

    return EXIT_SUCCESS;
}



void* customerGenerator(void* args) {
    std::vector<pthread_t> threadsToJoin;

    while (status) {

        sleep((double)((rand() % 1000)) / 1000);
        pthread_t customerThread;
        pthread_create(&customerThread, NULL, &customer, NULL);
        threadsToJoin.push_back(customerThread);
    }

    for (unsigned int i = 0; i < threadsToJoin.size(); i++) {
        pthread_join(threadsToJoin.at(i), NULL);
    }

    return EXIT_SUCCESS;
}

int runSleepingBarber() {

    srand(time(NULL));

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
    status = 0;

    for (int i = 0; i < BARBER_THREADS; i++) {
        pthread_cond_signal(&notifyBarber);
    }

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