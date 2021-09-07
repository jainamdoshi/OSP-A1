all: main.cpp
	g++ -g -Wall -Werror -o simulation main.cpp producerConsumer.cpp sleepingBarber.cpp -lpthread

clean:
	rm -rf simulation *.o