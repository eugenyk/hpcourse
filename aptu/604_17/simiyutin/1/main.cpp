#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>

pthread_t id_producer;
pthread_t id_consumer;
pthread_t id_interruptor;

pthread_mutex_t mut;
pthread_mutex_t mut_producer;

bool consumer_started = false;
pthread_cond_t consumer_start_cond;

bool producer_finished = false;

bool has_new_value = false;
pthread_cond_t has_new_value_cond;

bool finished_processing = false;
pthread_cond_t finished_processing_cond;

bool end_of_the_line = false;

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

void* producer_routine(void* arg) {

    Value * val = (Value *) arg;

    // Wait for consumer to start
    pthread_mutex_lock(&mut);
    while (!consumer_started)
        pthread_cond_wait(&consumer_start_cond, &mut);
    pthread_mutex_unlock(&mut);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::string line;
    std::getline(std::cin, line);
    std::istringstream ss(line);
    int el;
    while (ss >> el) {
        pthread_mutex_lock(&mut);
        val->update(el);
        has_new_value = true;
        pthread_cond_signal(&has_new_value_cond);
        while (!finished_processing)
            pthread_cond_wait(&finished_processing_cond, &mut);
        finished_processing = false;
        pthread_mutex_unlock(&mut);
    }

    pthread_mutex_lock(&mut);
    end_of_the_line = true;
    has_new_value = true;
    pthread_cond_signal(&has_new_value_cond);
    pthread_mutex_unlock(&mut);


    pthread_mutex_lock(&mut_producer);
    producer_finished = true;
    pthread_mutex_unlock(&mut_producer);
}

void* consumer_routine(void* arg) {

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    Value * val = (Value *) arg;

    // notify about start
    pthread_mutex_lock(&mut);
    consumer_started = true;
    pthread_cond_broadcast(&consumer_start_cond);
    pthread_mutex_unlock(&mut);

    // allocate value for result
    int * result = new int;

    // for every update issued by producer, read the value and add to sum
    while (true) {
        pthread_mutex_lock(&mut);
        while (!has_new_value)
            pthread_cond_wait(&has_new_value_cond, &mut);
        if (end_of_the_line) {
            pthread_mutex_unlock(&mut);
            break;
        };
        has_new_value = false;
        *result += val->get();
        finished_processing = true;
        pthread_cond_signal(&finished_processing_cond);
        pthread_mutex_unlock(&mut);
    }

    // return pointer to result
    return result;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumer to start
    pthread_mutex_lock(&mut);
    while (!consumer_started)
        pthread_cond_wait(&consumer_start_cond, &mut);
    pthread_mutex_unlock(&mut);

    // interrupt consumer while producer is running
    while (true) {
        pthread_mutex_lock(&mut_producer);
        if (producer_finished) {
            pthread_mutex_unlock(&mut_producer);
            return nullptr;
        }
        pthread_cancel(id_consumer);
        pthread_mutex_unlock(&mut_producer);
    }
}

int run_threads() {

    Value value;

    // start 3 threads and wait until they're done
    pthread_create(&id_producer, nullptr, producer_routine, (void *) &value);
    pthread_create(&id_consumer, nullptr, consumer_routine, (void *) &value);
    pthread_create(&id_interruptor, nullptr, consumer_interruptor_routine, nullptr);

    void * result_ptr = nullptr;

    pthread_join(id_producer, nullptr);
    pthread_join(id_consumer, &result_ptr);
    pthread_join(id_interruptor, nullptr);

    int result = *(int *) result_ptr;
    delete (int *) result_ptr;

    // return sum of update values seen by consumer
    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
