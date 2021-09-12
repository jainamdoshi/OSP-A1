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
        // Set this barber's serves if their serves is higher than the max
        if (numServed > maxServed) {
            maxServed = numServed;
        }

        // Locks the mutex and goes to sleep if there are no customers
        pthread_mutex_lock(&mutex);
        if (numCustomer <= 0 && status) {
            maxServed = numServed;
            printf("Barber (ID: %lu) is sleeping\n", id);

            if (status) {
                pthread_cond_wait(&notifyBarber, &mutex);
            }
        }

        // This barber wakes another barber and goes to if this barber has served more than maxServed
        if (numServed > maxServed && status) {
            pthread_cond_signal(&notifyBarber);
            pthread_cond_wait(&notifyBarber, &mutex);
        }

        if (status) {
            // Unlocks mutex, serves (sleeps) customer and locks mutex
            pthread_mutex_unlock(&mutex);
            sleep((double)(rand() % 1000) / 1000);
            pthread_mutex_lock(&mutex);

            if (occupiedSeat->size() > 0) {
                numCustomer--;
                numServed++;
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

    // Locking all the global variables
    pthread_mutex_lock(&mutex);

    // If the there an empty seat, then the customer will add themself into the slot
    if (status && emptySeat->size() > 0) {
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

    // Unlocking mutex and signaling a barber
    pthread_mutex_unlock(&mutex);
    if (customerEntered) {
        pthread_cond_signal(&notifyBarber);
    }

    return EXIT_SUCCESS;
}



void* customerGenerator(void* args) {

    std::vector<pthread_t> customers;
    int threadCount = 0;

    // Create customers till the time runs out or the thread's threshold is reached
    while (status && threadCount < MAX_THREADS) {
        sleep((double)(rand() % 1000) / 1000);
        pthread_t customerThread;

        // Create a customer thread
        if (pthread_create(&customerThread, NULL, &customer, NULL) == 0) {
            customers.push_back(customerThread);
            threadCount++;
        } else {
            printf("Operating System denied creating new thread\n");
        }
    }

    // Join all the customer threads to prevent memory leaks
    for (unsigned int i = 0; i < customers.size(); i++) {
        pthread_join(customers.at(i), NULL);
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

    // Intializing mutex and conditional variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&notifyBarber, NULL);

    // Creating barber threads
    pthread_t barberThreads[BARBER_THREADS];
    for (int i = 0; i < BARBER_THREADS; i++) {
        pthread_create(&barberThreads[i], NULL, &barber, NULL);
        printf("Barber thread (ID: %lu) is created\n", barberThreads[i]);
    }

    // Creating Customer generator thread
    pthread_t customerGeneratorThread;
    pthread_create(&customerGeneratorThread, NULL, &customerGenerator, NULL);

    // Sleep and then change the status
    sleep(PROGRAM_TIME_IN_SEC);
    printf("\nTimes Up! Barber shop closing\n\n");
    status = 0;

    // Wake and join all barber threads
    pthread_cond_broadcast(&notifyBarber);
    for (int i = 0; i < BARBER_THREADS; i++) {
        pthread_join(barberThreads[i], NULL);
    }
    pthread_join(customerGeneratorThread, NULL);

    // Freeing up all allocated memory. 
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&notifyBarber);
    delete emptySeat;
    delete occupiedSeat;

    return EXIT_SUCCESS;
}