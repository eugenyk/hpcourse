#include <pthread.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sstream>

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


pthread_mutex_t mutex;
pthread_cond_t producer_signal, consumer_signal, consumer_start;


bool alive = false;
bool is_updated = false;
bool finished = false;

unsigned int max_sleep;
int amount_of_threads;
int threads_started;

void *producer_routine(void *arg) {
    // Wait for consumer to start
    if (!alive) {
        pthread_mutex_lock(&mutex);
        while (!alive) {
            pthread_cond_wait(&consumer_start, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::vector<int> values;

    std::string mystr;
    std::getline(std::cin, mystr);
    std::stringstream ss(mystr);

    int cur_val;
    while (ss >> cur_val) {
        values.emplace_back(cur_val);
    }

    auto *val = static_cast<Value *>(arg);
    pthread_mutex_lock(&mutex);
    for (int num: values) {
//        std::cout << "Update " << num << std::endl;
        val->update(num);
        is_updated = true;
        pthread_cond_broadcast(&producer_signal);
        while (is_updated) {
            pthread_cond_wait(&consumer_signal, &mutex);
        }

    }

    finished = true;
    pthread_cond_broadcast(&producer_signal);
    pthread_mutex_unlock(&mutex);
    return nullptr;
}

void *consumer_routine(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    pthread_mutex_lock(&mutex);
    // notify about start
    if (!alive) {
        alive = true;
        pthread_cond_broadcast(&consumer_start);
    }
    threads_started += 1;

    // for every update issued by producer, read the value and add to sum
    // return pointer to result (aggregated result for all consumers)
    int total = 0;
    auto *val = static_cast<Value *>(arg);

    while (!finished) {
        while (!finished && !is_updated) {
            pthread_cond_wait(&producer_signal, &mutex);
        }

        if (finished) break;

//        std :: cout << "value is : " << val->get() << std::endl;
        total += val->get();
        is_updated = false;
        pthread_cond_signal(&consumer_signal);

        pthread_mutex_unlock(&mutex);
        usleep(rand() % max_sleep);
        pthread_mutex_lock(&mutex);
    }

    pthread_mutex_unlock(&mutex);

    return new int(total);
}

void *consumer_interruptor_routine(void *arg) {
    auto thread = static_cast<pthread_t *>(arg);
    // wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!alive) {
        pthread_cond_wait(&consumer_start, &mutex);
    }

    // interrupt consumer while producer is running
    while (!finished) {
//        std::cout << "Interruptor waits" << std::endl;
        pthread_cancel(thread[rand() % threads_started]);

        pthread_mutex_unlock(&mutex);
        usleep(rand() % max_sleep);
        pthread_mutex_lock(&mutex);
    }
    pthread_mutex_unlock(&mutex);
    return nullptr;

}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&consumer_start, nullptr);
    pthread_cond_init(&consumer_signal, nullptr);
    pthread_cond_init(&producer_signal, nullptr);

    pthread_t *consumers = new pthread_t[amount_of_threads];
    Value value;

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, &value);

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, consumers);

    for (int i = 0; i < amount_of_threads; ++i) {
        pthread_create(&consumers[i], nullptr, consumer_routine, &value);
    }
//    std::cout << "Here";

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    int total = 0;
    for (int i = 0; i < amount_of_threads; ++i) {
        int *result;
        pthread_join(consumers[i], reinterpret_cast<void **>(&result));
        total += (*result);
    }

    delete[] consumers;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&consumer_start);
    pthread_cond_destroy(&consumer_signal);
    pthread_cond_destroy(&producer_signal);


    return total;
}

int main(int argc, char *argv[]) {
    amount_of_threads = atoi(argv[1]);
    max_sleep = atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}