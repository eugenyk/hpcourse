#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iterator>

class Value {
public:
    Value() : _value(0) { }

    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }

private:
    int _value;
};

pthread_t producer;
pthread_t consumer;
pthread_t interruptor;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t is_modified_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t is_read_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t is_started_cond = PTHREAD_COND_INITIALIZER;

bool is_modified = false;
bool is_read = false;
bool is_finished = false;
bool is_started = false;

void* producer_routine(void* arg) {
    pthread_mutex_lock(&mutex);
    while(!is_started) {
        pthread_cond_wait(&is_started_cond, &mutex);
    };
    pthread_mutex_unlock(&mutex);

    std::vector<int> numbers;
    std::copy(std::istream_iterator<int>(std::cin),
              std::istream_iterator<int>(),
              std::back_inserter(numbers));

    for (int idx = 0; idx < numbers.size(); ++idx) {
        pthread_mutex_lock(&mutex);
        ((Value*)arg)->update(numbers[idx]);
        is_modified = true;
        pthread_cond_broadcast(&is_modified_cond);

        while (!is_read) {
            pthread_cond_wait(&is_read_cond, &mutex);
        }
        is_read = false;
        pthread_mutex_unlock(&mutex);
    }
    pthread_cond_signal(&is_modified_cond);
    pthread_mutex_unlock(&mutex);
    is_finished = true;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_mutex_lock(&mutex);
    if (!is_started) {
        pthread_cond_broadcast(&is_started_cond);
    }
    is_started = true;
    pthread_mutex_unlock(&mutex);

    auto *result = new int;

    while (true) {
        pthread_mutex_lock(&mutex);
        while (!is_modified && !is_finished) {
            pthread_cond_wait(&is_modified_cond, &mutex);
        }
        if (is_finished) {
            pthread_mutex_unlock(&mutex);
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            break;
        }
        is_modified = false;
        *result += ((Value*)arg)->get();
        is_read = true;
        pthread_cond_broadcast(&is_read_cond);
        pthread_mutex_unlock(&mutex);
    }
    return (void*)result;
}

void* consumer_interruptor_routine(void* arg) {
    (void)arg;
    pthread_mutex_lock(&mutex);
    while (!is_started) {
        pthread_cond_wait(&is_started_cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    while (!is_finished) {
        pthread_cancel(consumer);
    }
    return NULL;
}

int run_threads() {
    auto * data = new Value();
    pthread_create(&producer, nullptr, producer_routine, data);
    pthread_create(&consumer, nullptr, consumer_routine, data);
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, data);
    pthread_join(producer, nullptr);
    void* result;
    pthread_join(consumer, &result);
    pthread_join(interruptor, nullptr);
    delete data;

  return *(int*)result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}