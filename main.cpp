#include <stdio.h>
#include <stdlib.h>

int runProducerConsumer();
int runSleepingBarber();

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("No arguments provided. Usage: ./simulation [1 or 2]. 1 for Producer-Consumer and 2 for Sleeping Barber\n");
        return EXIT_FAILURE;
    }

    if (*argv[1] == '1') {
        runProducerConsumer();
    } else if (*argv[1] == '2') {
        runSleepingBarber();
    }

    return EXIT_SUCCESS;
}