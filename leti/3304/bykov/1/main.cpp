#include <iostream>
#include <pthread.h>
#include <sstream>
#include <vector>
#include <unistd.h>

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

struct thread_data {
    unsigned thread_id;
    unsigned max_consumer_sleep_time;
    Value *value;
    long *sum;
};

pthread_mutex_t consumer_start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consumer_start_condition = PTHREAD_COND_INITIALIZER;
bool is_consumer_started = false;

pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t value_write_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t value_read_cond = PTHREAD_COND_INITIALIZER;
bool is_data_ready = false;

bool is_data_over = false;

void *producer_routine(void *arg) {
    Value *value = (Value *) arg;

    // Wait for consumer to start
    pthread_mutex_lock(&consumer_start_mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&consumer_start_condition, &consumer_start_mutex);
    }
    pthread_mutex_unlock(&consumer_start_mutex);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    std::string line;
    std::getline(std::cin, line);

    std::istringstream line_stream(line);

    std::vector<int> numbers;
    int n;
    while (line_stream >> n) {
        numbers.push_back(n);
    }

    pthread_mutex_lock(&value_mutex);

    for (int i : numbers) {
        value->update(i);
        is_data_ready = true;

        pthread_cond_signal(&value_write_cond);

        while (is_data_ready) {
            pthread_cond_wait(&value_read_cond, &value_mutex);
        }
    }

    is_data_over = true;
    pthread_cond_broadcast(&value_write_cond);

    pthread_mutex_unlock(&value_mutex);

    pthread_exit(NULL);
}

void *consumer_routine(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    thread_data *data = (thread_data *) arg;

    // notify about start
    pthread_mutex_lock(&consumer_start_mutex);
    if (!is_consumer_started) {
        is_consumer_started = true;
        pthread_cond_broadcast(&consumer_start_condition);
    }
    pthread_mutex_unlock(&consumer_start_mutex);

    // for every update issued by producer, read the value and add to sum
    // return pointer to result (aggregated result for all consumers)

    while (true) {
        pthread_mutex_lock(&value_mutex);

        while (!is_data_ready && !is_data_over) {
            pthread_cond_wait(&value_write_cond, &value_mutex);
        }

        if (is_data_over) {
            pthread_mutex_unlock(&value_mutex);
            pthread_exit(data->sum);
        }

        *(data->sum) += data->value->get();
        is_data_ready = false;

        pthread_cond_signal(&value_read_cond);

        pthread_mutex_unlock(&value_mutex);

        double sleep_time = data->max_consumer_sleep_time * 1000 * rand_r(&(data->thread_id)) / double(RAND_MAX);
        usleep((__useconds_t) sleep_time);
    }
}

void *consumer_interruptor_routine(void *arg) {
    std::vector<pthread_t> *consumers = (std::vector<pthread_t> *) arg;

    // wait for consumer to start
    pthread_mutex_lock(&consumer_start_mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&consumer_start_condition, &consumer_start_mutex);
    }
    pthread_mutex_unlock(&consumer_start_mutex);

    // interrupt consumer while producer is running
    while (!is_data_over) {
        pthread_cancel(consumers->at(rand() % consumers->size()));
    }

    pthread_exit(NULL);
}

int run_threads(int N, int max_consumer_sleep_time) {
    srand((unsigned) time(NULL));

    Value *value = new Value();
    long *sum = new long(0);

    // start N threads and wait until they're done
    pthread_t producer;
    pthread_create(&producer, NULL, producer_routine, value);

    std::vector<pthread_t> consumers;
    for (unsigned i = 0; i < N; i++) {
        thread_data *data = new thread_data();
        data->value = value;
        data->max_consumer_sleep_time = (unsigned) max_consumer_sleep_time;
        data->sum = sum;
        data->thread_id = i;

        pthread_t consumer;
        pthread_create(&consumer, NULL, consumer_routine, (void *) data);
        consumers.push_back(consumer);
    }

    pthread_t consumer_interruptor;
    pthread_create(&consumer_interruptor, NULL, consumer_interruptor_routine, &consumers);

    void *result = NULL;

    pthread_join(producer, NULL);
    pthread_join(consumer_interruptor, NULL);
    pthread_join(consumers[0], &result);
    for (int i = 1; i < N; i++) {
        pthread_join(consumers[i], NULL);
    }

    // return aggregated sum of values
    return *(int *) result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Incorrect number of arguments" << std::endl;
        return -1;
    }

    int num_of_consumers = atoi(argv[1]);
    int max_consumer_sleep_time = atoi(argv[2]);

    if (num_of_consumers < 1 || max_consumer_sleep_time < 0) {
        std::cout << "Incorrect arguments" << std::endl;
    }

    std::cout << run_threads(num_of_consumers, max_consumer_sleep_time) << std::endl;
    return 0;
}