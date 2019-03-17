#include <pthread.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sstream>
#include <memory>

//#define DEBUG

#ifdef DEBUG
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define eprintf(...)
#endif

struct ArgForConsumer {
    ArgForConsumer() {
        pthread_cond_init(&process_cond, nullptr);
        pthread_cond_init(&update_cond, nullptr);
        pthread_mutex_init(&update_mutex, nullptr);
        pthread_mutex_init(&start_mutex, nullptr);
        pthread_mutex_init(&start_mutex, nullptr);
        pthread_cond_init(&start_cond, nullptr);
        pthread_key_create(&key, NULL);
    }

    pthread_cond_t process_cond;

    pthread_cond_t update_cond;
    pthread_mutex_t update_mutex;

    pthread_cond_t start_cond;
    pthread_mutex_t start_mutex;

    pthread_key_t key;

    int current_value;
    bool producer_is_working = true;
    bool is_finished = false;
    int number_of_started_consumers = 0;
};


struct ArgForProducer {
    ArgForConsumer *arg_for_consumer;
    std::vector<int> numbers;
    ArgForProducer(const std::vector<int>& values) : numbers(values) {
        arg_for_consumer = new ArgForConsumer();
    }

    ~ArgForProducer() {
        delete(arg_for_consumer);
    }
};


pthread_t producer;
pthread_t interrupter;
size_t millisec;
std::vector<pthread_t> threads;

void *producer_routine(void *arg) {
    eprintf("Producer: started\n");
    ArgForConsumer *arg_for_consumer = ((ArgForProducer *) arg)->arg_for_consumer;
    std::vector<int> numbers = ((ArgForProducer *) arg)->numbers;
    pthread_mutex_lock(&arg_for_consumer->start_mutex);
    while (!arg_for_consumer->number_of_started_consumers) {
        pthread_cond_wait(&arg_for_consumer->start_cond, &arg_for_consumer->start_mutex);
    }
    pthread_mutex_unlock(&arg_for_consumer->start_mutex);

    eprintf("Producer: started iterating\n");
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    for (int number : numbers) {
        pthread_mutex_lock(&arg_for_consumer->update_mutex);
        arg_for_consumer->current_value = number;
        arg_for_consumer->producer_is_working = false;
        eprintf("Producer: set value to %d\n", number);
        pthread_cond_signal(&arg_for_consumer->update_cond);
        while (!arg_for_consumer->producer_is_working) {
            pthread_cond_wait(&arg_for_consumer->process_cond, &arg_for_consumer->update_mutex);
        }
        pthread_mutex_unlock(&arg_for_consumer->update_mutex);
    }
    pthread_mutex_lock(&arg_for_consumer->update_mutex);
    arg_for_consumer->is_finished = true;
    arg_for_consumer->producer_is_working = false;
    pthread_cond_broadcast(&arg_for_consumer->update_cond);
    pthread_mutex_unlock(&arg_for_consumer->update_mutex);
    eprintf("Producer: finished %d\n", arg_for_consumer->is_finished);
    return nullptr;
}


void *consumer_routine(void *argument) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    int id = -1;
    for (int i = 0; i < threads.size(); i++) {
        if (pthread_equal(threads[i], pthread_self())) {
            id = i;
            break;
        }
    }
    eprintf("Consumer %d: started\n", id);
    auto arg = (ArgForConsumer *) argument;
    pthread_mutex_lock(&arg->start_mutex);
    arg->number_of_started_consumers++;
    pthread_cond_broadcast(&arg->start_cond);
    pthread_mutex_unlock(&arg->start_mutex);
    eprintf("Consumer %d: ready to loop \n", id);
    int* tl = new int(0);
    pthread_setspecific(arg->key, tl);
    while (true) {
        pthread_mutex_lock(&arg->update_mutex);
        while (arg->producer_is_working) {
            pthread_cond_wait(&arg->update_cond, &arg->update_mutex);
        }
        if (arg->is_finished) {
            pthread_mutex_unlock(&arg->update_mutex);
            break;
        }
        int* cur_sum = (int *)pthread_getspecific(arg->key);
        eprintf("Consumer %d: current sum = %d\n", id, *cur_sum);
        *tl = arg->current_value + *cur_sum;
        eprintf("Consumer %d: new sum %d\n", id, *tl);
        pthread_setspecific(arg->key, tl);
        arg->producer_is_working = true;
        pthread_cond_signal(&arg->process_cond);
        pthread_mutex_unlock(&arg->update_mutex);
        size_t time_to_sleep = rand() % millisec;
        if (time_to_sleep != 0) {
            usleep(time_to_sleep);
        }
    }
    int* res = (int *)pthread_getspecific(arg->key);
    eprintf("Consumer %d: final result=%d\n", id, *res);
    return res;
}


void *consumer_interruptor_routine(void *arg) {
    eprintf("Interruptor: started\n");
    auto argument = ((ArgForConsumer*) arg);
    pthread_mutex_lock(&argument->start_mutex);
    while(argument->number_of_started_consumers < threads.size()) {
        pthread_cond_wait(&argument->start_cond, &argument->start_mutex);
    }
    pthread_mutex_unlock(&argument->start_mutex);
    eprintf("Interruptor: started looping\n");
    while(true) {
        int thread_id = rand() % threads.size();
        pthread_mutex_lock(&argument->update_mutex);
        bool is_finished = argument->is_finished;
        if (! is_finished) {
            pthread_cancel(threads[thread_id]);
        }
        pthread_mutex_unlock(&argument->update_mutex);
        if (is_finished) {
            break;
        }
    }
    eprintf("Interruptor: finished\n");
    return nullptr;
}

int run_threads() {
    // start N threads and wait until they're done
    eprintf("Starting run_threads()\n");
    std::string line_with_numbers;
    std::getline(std::cin, line_with_numbers);
    eprintf("Got line %s\n", line_with_numbers.c_str());
    std::stringstream ss;
    ss.str(line_with_numbers);
    std::vector<int> numbers;
    while(! ss.eof()) {
        int i;
        ss >> i;
        numbers.push_back(i);
    }
    ArgForProducer* arg_for_producer = new ArgForProducer(std::move(numbers));
    pthread_create(&producer, nullptr, producer_routine, arg_for_producer);
    for (int i = 0; i < threads.size(); i++) {
        pthread_create(&threads[i], nullptr, consumer_routine, arg_for_producer->arg_for_consumer);
    }
    pthread_create(&interrupter, nullptr, consumer_interruptor_routine, arg_for_producer->arg_for_consumer);
    // return aggregated sum of values
    int sum = 0;
    for (int i = 0; i < threads.size(); i++) {
        int* res = nullptr;
        pthread_join(threads[i], (void**) &res);
        sum += *res;
        delete(res);
    }
    pthread_join(producer, nullptr);
    pthread_join(interrupter, nullptr);

    delete(arg_for_producer);

    return sum;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: first argument -- number of consumers, upper bound of time to sleep\n";
        return 0;
    }
    unsigned long n = std::stoul(argv[1]);
    millisec = std::stoul(argv[2]);
    threads.resize(n);
    srand(time(NULL));
    std::cout << run_threads() << std::endl;
    return 0;
}