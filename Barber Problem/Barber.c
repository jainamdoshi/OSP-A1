#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>



#define SLOT_SIZE 5
#define BARBER_THREADS 1
#define PROGRAM_TIME_IN_SEC 2

pthread_t slot[SLOT_SIZE];
pthread_mutex_t mutex;
pthread_cond_t notifyBarber;

int emptySeat[SLOT_SIZE];
int emptySeatIndex = 0;

int status = 1;
int numCustomer = 0;
int isBarberAsleep = 1;

void* barber(void* args) {

    pthread_t id = pthread_self();

    while (status) {


        pthread_mutex_lock(&mutex);
        if (numCustomer == 0) {
            printf("Barber (ID: %lu) is sleeping\n", id);
            pthread_cond_wait(&notifyBarber, &mutex);
            printf("Barber (ID: %lu) is waking up\n", id);
        }


        for (int i = 0; i < SLOT_SIZE && numCustomer != 0; i++) {

            if (slot[i] != 0) {
                printf("Barber (ID: %lu) serving customer on slot %d\n", id, i);
                sleep(0.5);
                slot[i] = 0;
                numCustomer--;
            }
        }
        pthread_mutex_unlock(&mutex);
    }


    return EXIT_SUCCESS;
}

void* customer(void* args) {

    pthread_t id = pthread_self();
    // if (numCustomer < SLOT_SIZE) {

    pthread_mutex_lock(&mutex);
    if (slot[emptySeatIndex] == 0) {
        slot[i] = id;
        printf("Customer (ID: %lu) has arrived the barber shop and is sitting on seat %d\n", id, i);
        numCustomer++;
    }
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&notifyBarber);
    // }

} else {
    printf("Customer (ID: %lu) left because no space available\n", id);
}
return EXIT_SUCCESS;
}


void* customerGenerator(void* args) {

    while (status) {

        sleep(1);
        pthread_t customerThread;
        pthread_create(&customerThread, NULL, &customer, NULL);
        printf("---------\n");
        pthread_join(customerThread, NULL);

    }

    return EXIT_SUCCESS;
}

int main(void) {

    srand(time(NULL));

    for (int i = 0; i < SLOT_SIZE; i++) {
        emptySeat[i] = i;
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

    pthread_join(customerGeneratorThread, NULL);
    for (int i = 0; i < BARBER_THREADS; i++) {
        pthread_join(barberThreads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&notifyBarber);

    return EXIT_SUCCESS;
}