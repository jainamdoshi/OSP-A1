#include <stdio.h>
#include <stdlib.h>

int runProducerConsumer();
int runSleepingBarber();

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("No arguments provided. Usage: ./simulation [1-3]\n. 1 for Producer-Consumer, 2 for Sleeping Barber and 3 for both\n");
        return EXIT_FAILURE;
    }

    if (*argv[1] == '1') {
        runProducerConsumer();
    } else if (*argv[1] == '2') {
        runSleepingBarber();
    } else if (*argv[1] == '3') {
        runProducerConsumer();
        runSleepingBarber();
    } else {
        printf("Wrong arguments. Usage: ./simulation [1-3]\n. 1 for Producer-Consumer and 2 for Sleeping Barber and 3 for both\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}