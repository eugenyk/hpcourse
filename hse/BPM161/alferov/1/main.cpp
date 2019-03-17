#include <cassert>
#include <functional>
#include <iostream>
#include <random>

#include <pthread.h>
#include <unistd.h>

int max_sleep;

class countdown_latch {
    int value_;
    pthread_mutex_t lock_;
    pthread_cond_t zero_;

public:
    countdown_latch() {
        assert(pthread_mutex_init(&lock_, nullptr) == 0);
        assert(pthread_cond_init(&zero_, nullptr) == 0);
    }

    ~countdown_latch() {
        assert(pthread_mutex_destroy(&lock_) == 0);
        assert(pthread_cond_destroy(&zero_) == 0);
    }

    void init_with(int n) {
        value_ = n;
    }

    void wait() {
        assert(pthread_mutex_lock(&lock_) == 0);
        while (value_ != 0) {
            assert(pthread_cond_wait(&zero_, &lock_) == 0);
        }
        assert(pthread_mutex_unlock(&lock_) == 0);
    }

    void countdown() {
        assert(pthread_mutex_lock(&lock_) == 0);
        value_--;
        if (value_ == 0) {
            pthread_cond_broadcast(&zero_);
        }
        assert(pthread_mutex_unlock(&lock_) == 0);
    }
};

countdown_latch all_consumers;

// The purpose is to wait for a consumer until it wakes up and only then to notify it.
// It seems that pthread_cond_signal does nothing in case nobody's waiting.
class queueing_cond {
    pthread_mutex_t lock_;
    pthread_cond_t wrapped_;
    int count_waiting_ = 0;
    bool broadcast_happened = false;

public:
    queueing_cond() {
        assert(pthread_mutex_init(&lock_, nullptr) == 0);
        assert(pthread_cond_init(&wrapped_, nullptr) == 0);
    }

    ~queueing_cond() {
        assert(pthread_mutex_destroy(&lock_) == 0);
        assert(pthread_cond_destroy(&wrapped_) == 0);
    }

    void signal() {
        assert(pthread_mutex_lock(&lock_) == 0);
        if (!broadcast_happened) {
            while (count_waiting_ == 0) {
                pthread_cond_wait(&wrapped_, &lock_);
            }
            assert(pthread_cond_signal(&wrapped_) == 0);
        }
        assert(pthread_mutex_unlock(&lock_) == 0);
    }

    void broadcast() {
        assert(pthread_mutex_lock(&lock_) == 0);
        broadcast_happened = true;
        pthread_cond_broadcast(&wrapped_);
        assert(pthread_mutex_unlock(&lock_) == 0);
    }

    void lock() {
        assert(pthread_mutex_lock(&lock_) == 0);
    }

    void wait() {
        if (broadcast_happened)
            return;
        count_waiting_++;
        if (count_waiting_ == 1) {
            assert(pthread_cond_signal(&wrapped_) == 0);
        }
        assert(pthread_cond_wait(&wrapped_, &lock_) == 0);
        count_waiting_--;
    }

    void unlock() {
        assert(pthread_mutex_unlock(&lock_) == 0);
    }
};

queueing_cond consumer_cond;
queueing_cond processed_cond;

bool finished = false;

void* producer_routine(void* arg) {
    // Wait for consumer to start

    all_consumers.wait();

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    int& input_value = *(int*)arg;

    // We don't need to lock a mutex here as at this point it is guaranteed that all consumers are either sleeping
    // or waiting for value to add.
    while (std::cin >> input_value) {
        consumer_cond.signal();

        processed_cond.lock();
        processed_cond.wait();
        processed_cond.unlock();
    }

    consumer_cond.lock();
    finished = true;
    consumer_cond.unlock();

    return nullptr;
}

thread_local int sum = 0;

void* consumer_routine(void* arg) {
    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    all_consumers.countdown();

    int& input_value = *(int*)arg;

    std::mt19937 rnd(pthread_self());
    std::uniform_int_distribution<int> dist(0, max_sleep);

    while (!finished) {
        consumer_cond.lock();
        consumer_cond.wait();
        if (!finished) {
            sum += input_value;
            consumer_cond.unlock();
            processed_cond.signal();

            int to_sleep = dist(rnd);
            usleep(to_sleep * 1000);
        } else {
            consumer_cond.unlock();
        }
    }

    return &sum;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumers to start

    all_consumers.wait();

    // interrupt random consumer while producer is running
    const std::vector<pthread_t>& consumers = *(std::vector<pthread_t>*)arg;

    std::mt19937 rnd;
    std::uniform_int_distribution<int> dist(0, consumers.size() - 1);

    consumer_cond.lock();
    while (!finished) {
        consumer_cond.unlock();
        int i = dist(rnd);
        assert(pthread_cancel(consumers[i]) == 0);
        consumer_cond.lock();
    }
    consumer_cond.unlock();

    return nullptr;
}

int run_threads(int n) {
    // start N threads and wait until they're done
    // return aggregated sum of values

    int input_value;

    all_consumers.init_with(n);

    pthread_t producer, interruptor;
    std::vector<pthread_t> consumers(n);
    assert(pthread_create(&producer, nullptr, producer_routine, &input_value) == 0);
    assert(pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers) == 0);
    for (int i = 0; i < n; i++) {
        assert(pthread_create(&consumers[i], nullptr, consumer_routine, &input_value) == 0);
    }

    void* retval;
    int ans = 0;
    pthread_join(producer, &retval);
    pthread_join(interruptor, &retval);
    consumer_cond.broadcast();
    for (const auto& consumer : consumers) {
        pthread_join(consumer, &retval);
        ans += *(int*)retval;
    }

    return ans;
}

int main(int argc, char *argv[]) {
    assert(argc == 3);
    int n = std::atoi(argv[1]);
    max_sleep = std::atoi(argv[2]);

    std::cout << run_threads(n) << std::endl;
    return 0;
}
