#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <random>
#include <iostream>
#include <vector>
#include <sstream>
using namespace std;

pthread_barrier_t barrier;

pthread_cond_t cond_upd;
pthread_cond_t cond_added;
pthread_mutex_t mutex;

unsigned int max_sleep_time;
unsigned int num_of_consumers;

bool is_finish = false, was_updated = false;

// TLS variable
__thread int part_res;
int cur_val;

void* producer_routine(void* arg) {
    pthread_barrier_wait(&barrier);

    string input;
    getline(cin, input);
    istringstream stream(input);

    while (!stream.eof()) {
        int n = 0;
        stream >> n;

        pthread_mutex_lock(&mutex);
        while (was_updated) {
            pthread_cond_wait(&cond_added, &mutex);
        }

        cur_val = n;
        was_updated = true;
        pthread_cond_signal(&cond_upd);

        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);

    while (was_updated) {
        pthread_cond_wait(&cond_added, &mutex);
    }

    is_finish = true;
    pthread_cond_broadcast(&cond_upd);

    pthread_mutex_unlock(&mutex);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    auto random_sleep_time = []() -> useconds_t {
        return max_sleep_time ? static_cast<useconds_t>(rand()) % max_sleep_time : 0;
    };

    pthread_barrier_wait(&barrier);
    const auto& arg_val = *static_cast<int *>(arg);

    while (true) {
        pthread_mutex_lock(&mutex);

        if (is_finish) {
           pthread_mutex_unlock(&mutex);
           break;
        }

        while (!was_updated && !is_finish) {
            pthread_cond_wait(&cond_upd, &mutex);
        }
        if (was_updated) {
            part_res += arg_val;
            was_updated = false;
            pthread_cond_signal(&cond_added);
        }
        
        pthread_mutex_unlock(&mutex);

        usleep(random_sleep_time());
    }

    return &part_res;
}

void* consumer_interruptor_routine(void* arg) {
   const auto& consumers = *static_cast<vector<pthread_t>*>(arg);    

    pthread_barrier_wait(&barrier);

    while(true) {
        pthread_mutex_lock(&mutex);

        if (is_finish) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);

        unsigned int ind = static_cast<unsigned int>(random() % consumers.size());
        pthread_cancel(consumers[ind]);
    }
}

void init() {
    pthread_barrier_init(&barrier, NULL, num_of_consumers + 2);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_added, NULL);
    pthread_cond_init(&cond_upd, NULL);
}

void destroy() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_added);
    pthread_cond_destroy(&cond_upd);
}

int run_threads() {
    init();

    pthread_t producer, interruptor;
    vector<pthread_t> consumers(num_of_consumers);

    pthread_create(&producer, NULL, producer_routine, NULL);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, &consumers);
    for (pthread_t& consumer : consumers) {
        pthread_create(&consumer, NULL, consumer_routine, &cur_val);
    }

    pthread_join(producer, NULL);
    pthread_join(interruptor, NULL);

    long long sum = 0;
    for (pthread_t& consumer : consumers) {
        void *part_res;
        pthread_join(consumer, &part_res);
        sum += *((long long *) part_res);
    }

    destroy();

    return sum;
}

int main(int argc, char **argv) {
    num_of_consumers = atoi(argv[1]);
    max_sleep_time = atoi(argv[2]);
    
    cout << run_threads() << endl;

    return 0;
}