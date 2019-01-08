#include <pthread.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <ctime>
#include <cassert>

class Value {
public:
    Value() : _value(0) {}

    void update(int value) { _value = value; }

    int get() const { return _value; }

private:
    int _value;
};

int cons_thread_num = 0;
int cons_sleep_limit = 0;
bool is_empty_data = false;
bool is_ready_data = false;
pthread_mutex_t mutex;
pthread_cond_t cond1;
pthread_barrier_t val_barrier;
pthread_barrier_t cancel_barrier;

void *producer_routine(void *arg) {
    auto *val = (Value *) (arg);

    std::string str;
    std::getline(std::cin, str);
    std::istringstream ss(str);
    std::vector<int> numbers;
    int input;
    while (ss >> input) {
        numbers.push_back(input);
    }


    for (auto num : numbers) {
        pthread_mutex_lock(&mutex);
        val->update(num);
        is_ready_data = true;
        pthread_cond_broadcast(&cond1);
        pthread_mutex_unlock(&mutex);

        pthread_barrier_wait(&val_barrier);

        pthread_mutex_lock(&mutex);
        is_ready_data = false;
        pthread_mutex_unlock(&mutex);

        pthread_barrier_wait(&val_barrier);
    }

    pthread_mutex_lock(&mutex);
    is_ready_data = true;
    is_empty_data = true;
    pthread_cond_broadcast(&cond1);
    pthread_mutex_unlock(&mutex);

    pthread_barrier_wait(&val_barrier);

    pthread_mutex_lock(&mutex);
    is_ready_data = false;
    pthread_mutex_unlock(&mutex);

    pthread_barrier_wait(&val_barrier);

    return nullptr;
}

void *consumer_routine(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&cancel_barrier);
    auto *val = (Value *) (arg);

    int *result_sum = new int(0);
    while (true) {
        bool need_break = false;

        pthread_mutex_lock(&mutex);
        while (!is_ready_data) {
            pthread_cond_wait(&cond1, &mutex);
        }
        if (!is_empty_data) {
            *result_sum += val->get();
        } else {
            need_break = true;
        }
        pthread_mutex_unlock(&mutex);

        pthread_barrier_wait(&val_barrier);

        pthread_barrier_wait(&val_barrier);

        if (need_break) break;
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    return result_sum;
}

void *consumer_interruptor_routine(void *arg) {
    auto threads = (std::vector<pthread_t> *) (arg);
    pthread_barrier_wait(&cancel_barrier);

    while (true) {
        pthread_cancel(threads->at(std::rand() % threads->size()));
        timespec sleep_amount{0, std::rand() % (cons_sleep_limit * 1000)};
        nanosleep(&sleep_amount, nullptr);
    }
    return nullptr;
}

int run_threads() {
    pthread_t producer;
    std::vector<pthread_t> consumers(static_cast<unsigned long>(cons_thread_num));
    pthread_t interruptor;

    // start N threads and wait until they're done
    auto *val = new Value();

    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond1, nullptr);
    pthread_barrier_init(&val_barrier, nullptr, static_cast<unsigned int>(cons_thread_num + 1));
    pthread_barrier_init(&cancel_barrier, nullptr, static_cast<unsigned int>(cons_thread_num + 1));


    pthread_create(&producer, nullptr, producer_routine, val);
    for (auto &cons : consumers) {
        pthread_create(&cons, nullptr, consumer_routine, val);
    }
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);

    int consumer_result = 0;
    pthread_join(producer, nullptr);

    pthread_cancel(interruptor);
    pthread_join(interruptor, nullptr);

    for (auto &cons : consumers) {
        int *result;
        pthread_join(cons, (void **) &result);
        consumer_result = (*result);
        delete result;
    }


    return consumer_result;
}

int main(int argc, char *argv[]) {
    cons_thread_num = std::atoi(argv[1]);
    cons_sleep_limit = std::atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}