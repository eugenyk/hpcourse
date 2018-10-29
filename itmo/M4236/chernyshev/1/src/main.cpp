#include <pthread.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <random>

#define NOT_STARTED 0
#define WAIT_FOR_PRODUCER 1
#define WAIT_FOR_CONSUMMER 2
#define FINISHED 3

class Value {
   public:
    Value() : _value(0) {}

    void update(int value) { _value = value; }

    int get() const { return _value; }

   private:
    int _value;
};

int current_status;
int cons_thread_num = 0;
int cons_sleep_limit = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

void* producer_routine(void* arg) {
    auto * val = (Value*)(arg);

    // Wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (current_status == NOT_STARTED) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    // Read data, loop through each value and update the value, notify consumer,
    // wait for consumer to process
    std::string str;
    std::getline(std::cin, str);
    std::istringstream ss(str);
    std::vector<int> numbers;
    int input;
    while (ss >> input) {
        numbers.push_back(input);
    }

    pthread_mutex_lock(&mutex);
    for (auto num : numbers) {
        val->update(num);
        current_status = WAIT_FOR_CONSUMMER;
        pthread_cond_broadcast(&cond);
        while (current_status != WAIT_FOR_PRODUCER) {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    current_status = FINISHED;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    return nullptr;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    auto * val = (Value*)(arg);

    // notify about start
    pthread_mutex_lock(&mutex);
    current_status = WAIT_FOR_PRODUCER;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (aggregated result for all consumers)

    int* result_sum = new int(0);
    while (true) {
        pthread_mutex_lock(&mutex);
        if (current_status == FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        if (current_status == WAIT_FOR_CONSUMMER) {
            *result_sum += val->get();
            current_status = WAIT_FOR_PRODUCER;
            pthread_cond_broadcast(&cond);

            pthread_mutex_unlock(&mutex);
            timespec sleep_amount{0, rand() % (cons_sleep_limit + 1)};
            nanosleep(&sleep_amount, nullptr);
            pthread_mutex_lock(&mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    return result_sum;
}

void* consumer_interruptor_routine(void* arg) {
    auto threads = (std::vector<pthread_t>*)(arg);

    // wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (current_status == NOT_STARTED) {
        pthread_cond_wait(&cond, &mutex);
    }

    // interrupt consumer while producer is running
    while (current_status != FINISHED) {
        pthread_cancel(threads->at(static_cast<unsigned long>(rand() % threads->size())));
        timespec sleep_amount{0, rand() % (cons_sleep_limit + 1)};
        nanosleep(&sleep_amount, nullptr);
    }
    pthread_mutex_unlock(&mutex);
    return nullptr;
}

int run_threads() {
    pthread_t producer;
    std::vector<pthread_t> consumers(static_cast<unsigned long>(cons_thread_num));
    pthread_t interruptor;

    // start N threads and wait until they're done
    auto * val = new Value();
    pthread_create(&producer, nullptr, producer_routine, val);
    for (auto& cons : consumers) {
        pthread_create(&cons, nullptr, consumer_routine, val);
    }
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);

    int consumer_result = 0;
    pthread_join(producer, nullptr);
    for (auto& cons : consumers) {
        int* result;
        pthread_join(cons, reinterpret_cast<void**>(&result));
        consumer_result += (*result);
        delete result;
    }

    pthread_join(interruptor, nullptr);

    return consumer_result;
}

int main(int argc, char* argv[]) {
    cons_thread_num = std::atoi(argv[1]);
    cons_sleep_limit = std::atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}