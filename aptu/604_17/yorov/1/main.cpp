#include <iostream>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>

enum State {GOT, UPDATED, END};

class Value {
public:
    Value()
        : _value(0)
        , state(GOT)
    {}

    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    State state;

private:
    int _value;
};

static bool running = false;
static pthread_mutex_t mutex_running = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_running = PTHREAD_COND_INITIALIZER;

void* producer_routine(void* arg) {
    pthread_mutex_lock(&mutex_running);
    while (!running) {
        pthread_cond_wait(&cond_running, &mutex_running);
    }
    pthread_cond_signal(&cond_running);
    pthread_mutex_unlock(&mutex_running);

    std::string line;
    std::getline(std::cin, line);
    std::vector<int> number{};
    std::stringstream iss(line);
    std::copy(std::istream_iterator<int>(iss), std::istream_iterator<int>(),
              std::back_inserter(number));

    Value* value = static_cast<Value*>(arg);
    pthread_mutex_lock(&value->mutex);
    for (auto x: number) {
        value->update(x);
        value->state = UPDATED;
        pthread_cond_signal(&value->cond);
        while (value->state == UPDATED) {
            pthread_cond_wait(&value->cond, &value->mutex);
        }
    }
    value->state = END;
    pthread_cond_signal(&value->cond);
    pthread_mutex_unlock(&value->mutex);
    pthread_mutex_lock(&mutex_running);
    running = false;
    pthread_mutex_unlock(&mutex_running);
    return nullptr;

  // Wait for consumer to running
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_mutex_lock(&mutex_running);
    running = true;
    pthread_cond_signal(&cond_running);
    pthread_mutex_unlock(&mutex_running);

    Value* value = static_cast<Value*>(arg);
    int* result = new int(0);
    pthread_mutex_lock(&value->mutex);
    while (value->state != END) {
        while (value->state == GOT) {
            pthread_cond_wait(&value->cond, &value->mutex);
        }
        if (value->state == END) {
            break;
        }
        *result += value->get();
        value->state = GOT;
        pthread_cond_signal(&value->cond);
    }
    pthread_mutex_unlock(&value->mutex);
    return result;

  // notify about running
  // allocate value for result
  // for every update issued by producer, read the value and add to sum
  // return pointer to result
}

void* consumer_interruptor_routine(void* arg) {
    pthread_mutex_lock(&mutex_running);
    while (!running) {
        pthread_cond_wait(&cond_running, &mutex_running);
    }
    pthread_cond_signal(&cond_running);
    pthread_mutex_unlock(&mutex_running);

    pthread_t* consumer = static_cast<pthread_t*>(arg);
    while (true) {
        pthread_cancel(*consumer);
        pthread_mutex_lock(&mutex_running);
        if (!running) {
            pthread_mutex_unlock(&mutex_running);
            break;
        }
        pthread_mutex_unlock(&mutex_running);
    }
  // wait for consumer to running
  // interrupt consumer while producer is running
}

int run_threads() {
    Value* value = new Value{};
    pthread_mutex_init(&value->mutex, nullptr);
    pthread_cond_init(&value->cond, nullptr);

    pthread_t threads[3];
    pthread_create(&threads[0], nullptr, producer_routine, value);
    pthread_create(&threads[1], nullptr, consumer_routine, value);
    pthread_create(&threads[2], nullptr, consumer_interruptor_routine, &threads[1]);
    void* result;
    pthread_join(threads[0], nullptr);
    pthread_join(threads[1], &result);
    pthread_join(threads[2], nullptr);
    pthread_cond_destroy(&value->cond);
    pthread_mutex_destroy(&value->mutex);
    delete value;
    int res = *static_cast<int*>(result);
    delete static_cast<int*>(result);
    return res;
  // running 3 threads and wait until they're done
  // return sum of update values seen by consumer
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
