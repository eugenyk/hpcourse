#include <iostream>
#include <string>
#include <pthread.h>
#include <sstream>
#include <vector>
#include <unistd.h>

using namespace std;

int COUNT;
int SLEEP;
pthread_mutex_t mmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t p_cond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;
int result;
bool is_produced, end_of_data = false;


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


void *producer_routine(void *arg) {
    pthread_barrier_wait(&barrier);

    auto *value = static_cast<Value *>(arg);
    std::string input;
    std::getline(std::cin, input);
    std::stringstream stream(input);

    int val;
    while (!stream.eof()) {
        stream >> val;
        pthread_mutex_lock(&mmutex);
        while (is_produced)
            pthread_cond_wait(&p_cond, &mmutex);
        value->update(val);
        is_produced = true;
        pthread_cond_signal(&c_cond);
        pthread_mutex_unlock(&mmutex);
    }
    pthread_mutex_lock(&mmutex);
    while (is_produced) {
        pthread_cond_wait(&p_cond, &mmutex);
    }
    value->update(0);
    is_produced = true;
    end_of_data = true;
    pthread_cond_broadcast(&c_cond);
    pthread_mutex_unlock(&mmutex);
    return nullptr;
}

void *consumer_routine(void *arg) {
    pthread_barrier_wait(&barrier);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    auto *value = static_cast<Value *>(arg);
    while (!end_of_data) {
        pthread_mutex_lock(&mmutex);

        while (!is_produced)
            pthread_cond_wait(&c_cond, &mmutex);
        result += value->get();
        is_produced = false; // consumed
        if (end_of_data)
            is_produced = true;
        pthread_cond_broadcast(&p_cond);
        pthread_mutex_unlock(&mmutex);
        usleep(static_cast<useconds_t>((rand() % SLEEP) * 10000));
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    return &result;
}

void *consumer_interruptor_routine(void *arg) {
    pthread_barrier_wait(&barrier);

    auto *consumers = static_cast<pthread_t *>(arg);
    while (!end_of_data)
        pthread_cancel(consumers[ rand() % COUNT ]);
    return nullptr;
}

int run_threads() {
    pthread_t producer;
    pthread_t consumers[COUNT];
    pthread_t interrupter;

    Value value;
    int *sum = nullptr;

    pthread_mutex_init(&mmutex, nullptr);
    pthread_cond_init(&p_cond, nullptr);
    pthread_cond_init(&c_cond, nullptr);
    pthread_barrier_init(&barrier, nullptr, COUNT + 1 + 1); // consumers + producer + interrupter

    for (int i = 0; i < COUNT; ++i)
        pthread_create(&consumers[i], nullptr, consumer_routine, &value);

    pthread_create(&producer, nullptr, producer_routine, &value);
    pthread_create(&interrupter, nullptr, consumer_interruptor_routine, consumers);


    pthread_join(interrupter, nullptr);
    pthread_join(producer, nullptr);
    for (int i = 0; i < COUNT; ++i)
        pthread_join(consumers[i], (void **) (&sum));

    pthread_mutex_destroy(&mmutex);
    pthread_cond_destroy(&p_cond);
    pthread_cond_destroy(&c_cond);

    return *sum;
}

int main(int argc, char *argv[]) {
    COUNT = atoi(argv[1]);
    SLEEP = atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}