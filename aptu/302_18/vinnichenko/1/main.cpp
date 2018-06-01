#include <pthread.h>
#include <iostream>
#include <vector>
#include <bits/unique_ptr.h>
#include <cassert>
#include <sstream>

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

class ValueWrapper {
public:
    int get() {
        assert(ready);
        int res = _value.get();
        ready = false;
        return res;
    }

    void update(int val) {
        assert(!ready);
        _value.update(val);
        ready = true;
    }

    bool is_ready() const {
        return ready;
    }

private:
    Value _value;
    bool ready = false;
};

pthread_t producer_tid;
pthread_t consumer_tid;
pthread_t consumer_interruptor_tid;
pthread_barrier_t consumer_start_barrier;

pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_produce = PTHREAD_COND_INITIALIZER;

volatile bool producer_finished = false;

// Wait for consumer to start
// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
void* producer_routine(void* arg) {
    pthread_barrier_wait(&consumer_start_barrier);

    auto value = reinterpret_cast<ValueWrapper*>(arg);

    std::string data;
    std::getline(std::cin, data);
    std::istringstream iss(data);

    int x;
    while (iss >> x) {
        while (value->is_ready()) {
            pthread_cond_wait(&can_produce, &value_mutex);
        }
        value->update(x);
        pthread_cond_broadcast(&can_consume);
    }

    producer_finished = true;

    pthread_mutex_unlock(&value_mutex);
    pthread_cond_broadcast(&can_consume);
    
    pthread_exit(nullptr);
}

// notify about start
// for every update issued by producer, read the value and add to sum
// return pointer to result
void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    pthread_barrier_wait(&consumer_start_barrier);

    auto value = reinterpret_cast<ValueWrapper*>(arg);

    int* sum_ptr = new int(0);
    while (!producer_finished) {
        while (!value->is_ready()) {
            pthread_cond_wait(&can_consume, &value_mutex);
        }
        (*sum_ptr) += value->get();
        pthread_cond_broadcast(&can_produce);
    }

    pthread_mutex_unlock(&value_mutex);
    pthread_cond_broadcast(&can_consume);

    pthread_exit(sum_ptr);
}

// wait for consumer to start, interrupt consumer while producer is running
void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&consumer_start_barrier);

    while (!producer_finished) {
        pthread_cancel(consumer_tid);
    }
    pthread_exit(nullptr);
}

// start 3 threads and wait until they're done
// return sum of update values seen by consumer
int run_threads() {
    std::vector<int> vec = {1001, 2, 3, 4, 5};

    ValueWrapper value;

    pthread_barrier_init(&consumer_start_barrier, nullptr, 3);

    pthread_create(&producer_tid, nullptr, producer_routine, &value);
    pthread_create(&consumer_tid, nullptr, consumer_routine, &value);
    pthread_create(&consumer_interruptor_tid, nullptr, consumer_interruptor_routine, nullptr);

    void* result = nullptr;

    pthread_join(producer_tid, nullptr);
    pthread_join(consumer_tid, &result);
    pthread_join(consumer_interruptor_tid, nullptr);

    auto res_ptr = reinterpret_cast<int*>(result);
    int res = *res_ptr;
    delete res_ptr;
    return res;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}