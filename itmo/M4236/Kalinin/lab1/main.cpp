#include <iostream>
#include <string>
#include <pthread.h>
#include <sstream>
#include <vector>
#include <unistd.h>

using namespace std;

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

pthread_mutex_t pmutex;
pthread_cond_t consumer_cond, producer_cond;
int threads_count, max_sleep_time, sum;
bool is_update, finish = false;
pthread_barrier_t barrier;

void read_data(vector<int> *buffer) {
    std::string input;
    std::getline(std::cin, input);
    std::stringstream ss(input);

    int cur_val;
    while (ss >> cur_val) {
        buffer->push_back(cur_val);
    }
}

void *producer_routine(void *arg) {

    // Wait for consumer to start
    pthread_barrier_wait(&barrier);
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    auto *value = static_cast<Value *>(arg);
    std::vector<int> numbers;
    read_data(&numbers);

    for (int num : numbers) {
        pthread_mutex_lock(&pmutex);
        value->update(num);
        is_update = true;
        pthread_cond_broadcast(&consumer_cond);
        while (is_update) {
            pthread_cond_wait(&producer_cond, &pmutex);
        }
        pthread_mutex_unlock(&pmutex);
    }

    finish = true;
    pthread_cond_broadcast(&consumer_cond);

    pthread_exit(0);
}

void *consumer_routine(void *arg) {
    // notify about start
    pthread_barrier_wait(&barrier);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    // for every update issued by producer, read the value and add to sum
    auto *value = static_cast<Value *>(arg);
    while (!finish) {
        pthread_mutex_lock(&pmutex);
        while (!is_update) {
            if (finish) {
                pthread_mutex_unlock(&pmutex);
                return &sum;
            }
            pthread_cond_wait(&consumer_cond, &pmutex);
        }
        sum += value->get();
        is_update = false;
        pthread_cond_signal(&producer_cond);
        pthread_mutex_unlock(&pmutex);
        usleep(static_cast<useconds_t>(rand() % max_sleep_time) * 1000);
    }
    // return pointer to result (aggregated result for all consumers)
    return &sum;
}

void *consumer_interruptor_routine(void *arg) {
    // wait for consumer to start
    pthread_barrier_wait(&barrier);
//     interrupt consumer while producer is running
    auto *consumers_id = static_cast<pthread_t *>(arg);
    while (!finish) {
        int i = rand() % threads_count;
        pthread_cancel (consumers_id[i]);
    }
    pthread_exit(0);
}

int run_threads() {
    // start N threads and wait until they're done
    pthread_t producer_id;
    pthread_t consumers_id[threads_count];
    pthread_t interuptor_id;
    Value value;
    int *consumers_result = nullptr;

    pthread_mutex_init(&pmutex, nullptr);
    pthread_cond_init(&producer_cond, nullptr);
    pthread_cond_init(&consumer_cond, nullptr);
    pthread_barrier_init(&barrier, nullptr, threads_count + 2);

    pthread_create(&producer_id, nullptr, producer_routine, &value);

    for (int i = 0; i < threads_count; ++i) {
        pthread_create(&consumers_id[i], nullptr, consumer_routine, &value);
    }

    pthread_create(&interuptor_id, nullptr, consumer_interruptor_routine, consumers_id);

    pthread_join(producer_id, nullptr);
    for (int i = 0; i < threads_count; ++i) {
        pthread_join(consumers_id[i], (void **) (&consumers_result));
    }
    pthread_join(interuptor_id, nullptr);

    pthread_mutex_destroy(&pmutex);
    pthread_cond_destroy(&producer_cond);
    pthread_cond_destroy(&consumer_cond);

    // return aggregated sum of values
    return *consumers_result;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Error. Not enough arguments: please pass number of threads and maximum number of milliseconds of sleep"
             << endl;
    } else if (argc > 3){
        cout << "Error. Too many arguments" << endl;
    } else {
        threads_count = atoi(argv[1]);
        max_sleep_time = atoi(argv[2]);
        std::cout << run_threads() << std::endl;
    }
    return 0;
}