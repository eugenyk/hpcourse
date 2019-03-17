#include <pthread.h>
#include <iostream>
#include <random>
#include <sstream>
#include <unistd.h>

thread_local int partialSum;
unsigned long consumersCount, maxSleepDuration;
int commonValue;

pthread_barrier_t barrier;
pthread_mutex_t valueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t valueEmbarked = PTHREAD_COND_INITIALIZER;
pthread_cond_t valueDisembarked = PTHREAD_COND_INITIALIZER;


enum DataState {
    EMBARKED, DISEMBARKED, ENDED
};

DataState valueState = DISEMBARKED;

std::vector<int> read_numbers() {
    std::string line;
    std::vector<int> numbers;
    int curNumber;
    getline(std::cin, line);
    std::istringstream iss(line);
    while (iss >> curNumber) {
        numbers.push_back(curNumber);
    }
    return numbers;
}

unsigned long generate_random_int(unsigned long maxValue) {
    static thread_local std::random_device dev;
    static thread_local std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, maxValue);

    return dist(rng);
}

void *producer_routine(void *arg) {
    auto numbers = read_numbers();

    pthread_barrier_wait(&barrier);

    for (auto &number:numbers) {
        pthread_mutex_lock(&valueLock);

        while (valueState == EMBARKED) {
            pthread_cond_wait(&valueDisembarked, &valueLock);
        }
        commonValue = number;

        valueState = EMBARKED;
        pthread_cond_broadcast(&valueEmbarked);
        pthread_mutex_unlock(&valueLock);
    }

    pthread_mutex_lock(&valueLock);

    while (valueState == EMBARKED) {
        pthread_cond_wait(&valueDisembarked, &valueLock);
    }

    valueState = ENDED;
    pthread_cond_broadcast(&valueEmbarked);
    pthread_mutex_unlock(&valueLock);

    return nullptr;
}

void *consumer_routine(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&barrier);

    auto value = (int *) arg;

    while (true) {
        pthread_mutex_lock(&valueLock);
        while (valueState == DISEMBARKED) {
            pthread_cond_wait(&valueEmbarked, &valueLock);
        }

        if (valueState == EMBARKED) {
            partialSum += *value;
        }

        if (valueState == ENDED) {
            pthread_mutex_unlock(&valueLock);
            return &partialSum;
        }
        valueState = DISEMBARKED;
        pthread_cond_signal(&valueDisembarked);
        pthread_mutex_unlock(&valueLock);
        usleep(generate_random_int(maxSleepDuration));
    }
}

void *consumer_interruptor_routine(void *arg) {
    pthread_barrier_wait(&barrier);

    auto *consumers = (pthread_t *) arg;

    while (true) {
        pthread_mutex_lock(&valueLock);

        if (valueState == ENDED) {
            pthread_mutex_unlock(&valueLock);
            return nullptr;
        }
        pthread_mutex_unlock(&valueLock);

        auto target = generate_random_int(consumersCount - 1);
        pthread_cancel(consumers[target]);
    }
}

int run_threads() {
    pthread_t consumers[consumersCount], producer, interruptor;
    pthread_barrier_init(&barrier, nullptr, consumersCount + 2);

    for (auto &consumer: consumers) {
        pthread_create(&consumer, nullptr, consumer_routine, &commonValue);
    }

    pthread_create(&producer, nullptr, producer_routine, nullptr);
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);

    int totalSum = 0;

    for (auto &consumer: consumers) {
        void *ret;
        pthread_join(consumer, &ret);
        totalSum += *((int *) ret);
    }

    return totalSum;
}

void parse_args(int argc, char **argv) {
    if (argc != 3)
        exit(-1);

    consumersCount = std::strtoul(argv[1], nullptr, 10);
    maxSleepDuration = std::strtoul(argv[2], nullptr, 10);
}

int main(int argc, char **argv) {
    parse_args(argc, argv);
    std::cout << run_threads() << std::endl;
    return 0;
}