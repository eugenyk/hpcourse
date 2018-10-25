#include <pthread.h>
#include <iostream>
#include <assert.h>
#include <sstream>
#include <unistd.h>
#include <vector>

class Value {
public:
    Value() : _value(0) {}

    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }

private:
    int _value;
};

enum states {
    BEFORE_STARTED, STARTED, PRODUCED, CONSUMED, FINISHED
};

static int N_CONSUMERS;
static int MAX_DELAY;
int result;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_producer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_consumer = PTHREAD_COND_INITIALIZER;
states state = BEFORE_STARTED;

void unlock_and_sleep() {
    unsigned int microseconds = static_cast<unsigned int>(rand() % MAX_DELAY) * 1000;
    pthread_mutex_unlock(&m);
    usleep(microseconds);
}

void *producer_routine(void *arg) {

    Value *value = static_cast<Value *>(arg);

    // Wait for consumer to start
    pthread_mutex_lock(&m);
    while (state != STARTED) {
        pthread_cond_wait(&cond_producer, &m); // [tip: wait releases mutex while waiting]
    }
    pthread_mutex_unlock(&m);

    // Read data, loop through each value and update the value
    std::string line;
    getline(std::cin, line);
    std::istringstream iss(line);
    int c;

    while (iss >> c) {
        pthread_mutex_lock(&m);

        value->update(c);
        state = PRODUCED;

        // notify consumer
        pthread_cond_signal(&cond_consumer);
        //  wait for consumer to process
        while (state != CONSUMED)
            pthread_cond_wait(&cond_producer, &m);
        pthread_mutex_unlock(&m);
    }

    pthread_mutex_lock(&m);
    state = FINISHED;
    pthread_cond_broadcast(&cond_consumer);
    pthread_mutex_unlock(&m);
    return nullptr;
}

void *consumer_routine(void *arg) {
    Value *value = static_cast<Value *>(arg);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    // notify about start
    pthread_mutex_lock(&m);
    if (state == BEFORE_STARTED) {
        state = STARTED;
        pthread_cond_broadcast(&cond_producer);
    }
    pthread_mutex_unlock(&m);


    // for every update issued by producer, read the value and add to sum
    while (true) {
        pthread_mutex_lock(&m);

        while (state != PRODUCED && state != FINISHED) {
            pthread_cond_wait(&cond_consumer, &m);
        }

        if (state == PRODUCED) {
            result += value->get();
            state = CONSUMED;
            pthread_cond_signal(&cond_producer);

            unlock_and_sleep();

        } else if (state == FINISHED) {
            pthread_mutex_unlock(&m);
            // return pointer to result (aggregated result for all consumers)
            return &result;
        }
    }
}

void *consumer_interruptor_routine(void *arg) {
    std::vector<pthread_t> &consumers = *static_cast<std::vector<pthread_t> *> (arg);

    // Wait for consumer to start
    pthread_mutex_lock(&m);
    while (state != STARTED)
        pthread_cond_wait(&cond_producer, &m);
    pthread_mutex_unlock(&m);

    // interrupt consumer while producer is running
    pthread_mutex_lock(&m);
    while (state != FINISHED) {
        unsigned int random_index = static_cast<unsigned int>(rand() % consumers.size());
        pthread_cancel(consumers[random_index]);

        unlock_and_sleep();
        pthread_mutex_lock(&m);
    }
    pthread_mutex_unlock(&m);

    return nullptr;
}

int run_threads() {
    Value value;
    void *local_result;

    pthread_t producer;
    pthread_t interrupter;
    std::vector<pthread_t> consumers(static_cast<unsigned long>(N_CONSUMERS));

    // start N threads
    pthread_create(&producer, NULL, producer_routine, &value);
    pthread_create(&interrupter, NULL, consumer_interruptor_routine, &consumers);
    for (int i = 0; i < N_CONSUMERS; ++i) {
        pthread_create(&consumers[i], NULL, consumer_routine, &value);
    }

    // wait until they're done
    pthread_join(producer, nullptr);
    pthread_join(interrupter, nullptr);
    for (int i = 0; i < N_CONSUMERS; ++i) {
        pthread_join(consumers[i], &local_result);
    }

    // return aggregated sum of values
    return *((int *) local_result);
}

int main(int argc, char *argv[]) {
    assert(argc == 3);
    N_CONSUMERS = std::atoi(argv[1]);
    MAX_DELAY = std::atoi(argv[2]);

    std::cout << run_threads() << std::endl;

    return 0;
}