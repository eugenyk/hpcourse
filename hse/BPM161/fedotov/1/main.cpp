#include <iostream>
#include <sstream>
#include <unistd.h>
#include <pthread.h>

pthread_barrier_t allThreadsBarrier;
bool finish = false;
bool containerReady = false;
pthread_cond_t containerReadyToWrite;
pthread_cond_t containerReadyToRead;
pthread_mutex_t containerMutex;

int maxSleepTime, threadCount;


void *producer_routine(void *arg) {
    int *container = (int *) arg;
    pthread_barrier_wait(&allThreadsBarrier);
    std::string line;
    getline(std::cin, line);
    std::istringstream is(line);
    int next;
    while (is >> next) {
        pthread_mutex_lock(&containerMutex);
        while (containerReady) {
            pthread_cond_wait(&containerReadyToWrite, &containerMutex);
        }
        *container = next;
        containerReady = true;
        pthread_cond_signal(&containerReadyToRead);
        pthread_mutex_unlock(&containerMutex);
    }
    pthread_mutex_lock(&containerMutex);
    while (containerReady) {
        pthread_cond_wait(&containerReadyToWrite, &containerMutex);
    }
    finish = true;
    pthread_cond_broadcast(&containerReadyToRead);
    pthread_mutex_unlock(&containerMutex);
}


thread_local int sum = 0;

void *consumer_routine(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    int *container = (int *) arg;
    pthread_barrier_wait(&allThreadsBarrier);
    while (!finish) {
        pthread_mutex_lock(&containerMutex);
        while (!containerReady && !finish) {
            pthread_cond_wait(&containerReadyToRead, &containerMutex);
        }
        if (containerReady) {
            sum += *container;
            containerReady = false;
            pthread_cond_signal(&containerReadyToWrite);
        }
        pthread_mutex_unlock(&containerMutex);
        if (maxSleepTime && !finish) {
            usleep(rand() % (maxSleepTime * 1000));
        }
    }
    int *mySum = new int;
    *mySum = sum;
    return mySum;
}

void *consumer_interruptor_routine(void *arg) {
    pthread_t *threads = (pthread_t *) arg;
    pthread_barrier_wait(&allThreadsBarrier);
    while (!finish) {
        int poorThreadIndex = rand() % threadCount;
        pthread_cancel(threads[poorThreadIndex]);
    }
}

int run_threads(int threadCount) {
    pthread_t interruptor, producer;
    pthread_t threads[threadCount];
    pthread_barrier_init(&allThreadsBarrier, NULL, threadCount + 2);
    int container = 0;
    pthread_create(&producer, NULL, producer_routine, (void *) &container);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, (void *) &threads);
    for (auto &thread: threads) {
        pthread_create(&thread, NULL, consumer_routine, (void *) &container);
    }
    pthread_join(interruptor, NULL);
    pthread_join(producer, NULL);
    int consumersSum = 0;
    for (auto &thread: threads) {
        void *res;
        pthread_join(thread, &res);
        consumersSum += *((int *) res);
        delete (int *) res;
    }
    return consumersSum;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Not enough args" << std::endl;
        return 1;
    }
    threadCount = std::stoi(argv[1]);
    maxSleepTime = std::stoi(argv[2]);
    std::cout << run_threads(threadCount) << std::endl;
    return 0;
}