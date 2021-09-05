#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>



#define SLOT_SIZE 5
#define BARBER_THREADS 1
#define PROGRAM_TIME_IN_SEC 10

pthread_t slot[SLOT_SIZE];
pthread_mutex_t mutex;
pthread_cond_t notifyBarber;

int emptySeat[SLOT_SIZE];
int emptySeatStart = 0;
int emptySeatEnd = 0;

int occupiedSeat[SLOT_SIZE];
int occupiedSeatStart = 0;
int occupiedSeatEnd = 0;

int status = 1;
int numCustomer = 0;
int isBarberAsleep = 1;

void* barber(void* args) {

    pthread_t id = pthread_self();

    while (status) {


        pthread_mutex_lock(&mutex);
        if (occupiedSeat[occupiedSeatStart] == -1) {
            printf("Barber (ID: %lu) is sleeping\n", id);
            pthread_cond_wait(&notifyBarber, &mutex);
            printf("Barber (ID: %lu) is waking up\n", id);
        }

        printf("Barber (ID: %lu) serving customer on slot %d\n", id, occupiedSeat[occupiedSeatStart]);
        sleep((double)(rand() % 1000) / 1000);
        slot[occupiedSeat[occupiedSeatStart]] = 0;


        emptySeat[emptySeatEnd] = occupiedSeat[occupiedSeatStart];
        emptySeatEnd = (emptySeatEnd + 1) % SLOT_SIZE;

        occupiedSeat[occupiedSeatStart] = -1;
        occupiedSeatStart = (occupiedSeatStart + 1) % SLOT_SIZE;

        pthread_mutex_unlock(&mutex);
    }


    return EXIT_SUCCESS;
}

void* customer(void* args) {

    pthread_t id = pthread_self();
    // if (numCustomer < SLOT_SIZE) {

    pthread_mutex_lock(&mutex);
    if (emptySeat[emptySeatStart] != -1) {
        slot[emptySeat[emptySeatStart]] = id;
        printf("Customer (ID: %lu) has arrived the barber shop and is sitting on seat %d\n", id, emptySeat[emptySeatStart]);


        occupiedSeat[occupiedSeatEnd] = emptySeat[emptySeatStart];
        occupiedSeatEnd = (occupiedSeatEnd + 1) % SLOT_SIZE;
        emptySeat[emptySeatStart] = -1;
        emptySeatStart = (emptySeatStart + 1) % SLOT_SIZE;

    } else {
        printf("Customer (ID: %lu) left because no space available\n", id);
    }
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&notifyBarber);

    return EXIT_SUCCESS;
}



void* customerGenerator(void* args) {

    while (status) {

        sleep((double)(rand() % 1000) / 1000);
        pthread_t customerThread;
        pthread_create(&customerThread, NULL, &customer, NULL);
        // pthread_join(customerThread, NULL);

    }

    return EXIT_SUCCESS;
}

int main(void) {

    srand(time(NULL));

    for (int i = 0; i < SLOT_SIZE; i++) {
        emptySeat[i] = i;
        occupiedSeat[i] = -1;
    }


    // for (int i = 0; i < SLOT_SIZE; i++) {
    //     printf("%d\n", occupiedSeat[i]);
    // }


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

    pthread_join(customerGeneratorThread, NULL);
    for (int i = 0; i < BARBER_THREADS; i++) {
        pthread_join(barberThreads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&notifyBarber);

    return EXIT_SUCCESS;
}