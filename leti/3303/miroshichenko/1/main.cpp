#include <pthread.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <random>

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

enum class Status {
    PRODUCER_READY,
    CONSUMER_READY,
};

static size_t N_THREADS;
static long int MAX_SLEEP;

static bool started = false;
static bool finished = false;
Status status{Status::CONSUMER_READY};

pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t start_signal = PTHREAD_COND_INITIALIZER;
pthread_cond_t producer_signal = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_signal = PTHREAD_COND_INITIALIZER;

void* producer_routine(void* arg) {
    // Wait for consumer to start
    if (!started) {
        pthread_mutex_lock(&value_mutex);
        while (!started) {
            pthread_cond_wait(&start_signal, &value_mutex);
        }
        pthread_mutex_unlock(&value_mutex);
    }

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    auto value = static_cast<Value*>(arg);
    std::string input;
    std::getline(std::cin, input);
    std::istringstream ss(input);

    std::vector<int> numbers;

    int current;
    while (ss >> current) {
        numbers.push_back(current);
    }

    pthread_mutex_lock(&value_mutex);

    for (int number : numbers) {
        value->update(number);
        status = Status::PRODUCER_READY;
        pthread_cond_signal(&producer_signal);
        while (status != Status::CONSUMER_READY) {
            pthread_cond_wait(&consumer_signal, &value_mutex);
        }
    }

    finished = true;
    pthread_cond_broadcast(&producer_signal); // wake up all consumers
    pthread_mutex_unlock(&value_mutex);

    return nullptr;
}

void sleep_with_mutex_release(pthread_mutex_t& mutex) {
    pthread_mutex_unlock(&mutex);
    timespec sleep_amount{0, rand() % (MAX_SLEEP + 1)};
    if (nanosleep(&sleep_amount, nullptr) == -1) {
        std::cout << "AWAKEN" << std::endl;
    }

    pthread_mutex_lock(&mutex);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    int sum = 0;
    auto value = static_cast<Value const*>(arg);

    pthread_mutex_lock(&value_mutex);

    // notify about start
    if (!started) {
        started = true;
        pthread_cond_broadcast(&start_signal);
    }

    // for every update issued by producer, read the value and add to sum
    // return pointer to result (aggregated result for all consumers)
    while (!finished) {
        while (!(status == Status::PRODUCER_READY || finished)) {
            pthread_cond_wait(&producer_signal, &value_mutex);
        }

        if (finished) { break; }

        sum += value->get();
        status = Status::CONSUMER_READY;
        pthread_cond_signal(&consumer_signal);

        sleep_with_mutex_release(value_mutex);
    }

    pthread_mutex_unlock(&value_mutex);

    return new int(sum);
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumer to start
    // interrupt consumer while producer is running
    auto threads = static_cast<std::vector<pthread_t> const*>(arg);

    pthread_mutex_lock(&value_mutex);
    while (!started) {
        pthread_cond_wait(&start_signal, &value_mutex);
    }

    while (!finished) {
        pthread_cancel(threads->at(rand() % threads->size()));
        sleep_with_mutex_release(value_mutex);
    }

    pthread_mutex_unlock(&value_mutex);

    return nullptr;
}

int run_threads(Value* value) {
    // start N threads and wait until they're done
    // return aggregated sum of values
    pthread_t producer;
    pthread_t interruptor;
    std::vector<pthread_t> consumers(N_THREADS);

    pthread_create(&producer, nullptr, producer_routine, value);
    for (auto& t: consumers) {
        pthread_create(&t, nullptr, consumer_routine, value);
    }
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    int total = 0;

    for (auto& t: consumers) {
        int* result;
        pthread_join(t, reinterpret_cast<void**>(&result));
        total += (*result);
        delete result;
    }

    return total;
}


int main(int argc, char** argv) {
    using namespace std;

    if (argc != 3) {
        cerr << "Invalid number of arguments" << endl;
        return 1;
    }

    N_THREADS = std::atoi(argv[1]);
    MAX_SLEEP = std::atol(argv[2]) * 1'000'000;
    cout << "Number of threads is " << N_THREADS << endl;
    cout << "Max sleep value is " << MAX_SLEEP << "us" << endl;

    Value value;
    int total = run_threads(&value);
    cout << "Total sum is " << total << endl;

    return 0;
}