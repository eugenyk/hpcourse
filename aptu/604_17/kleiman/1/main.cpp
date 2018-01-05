#include <pthread.h>
#include <iostream>
#include <vector>

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

static bool endp = false;
static bool startc = false;
static bool endc = false;
static bool isUpdate = false;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* producer_routine(void* arg) {
    // Wait for consumer to start

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::vector<int> buffer;
    copy(std::istream_iterator<int>(std::cin), std::istream_iterator<int>(), back_inserter(buffer));
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        pthread_mutex_lock(&mutex);
        while(isUpdate)
            pthread_cond_wait(&cond, &mutex);
        ((Value*)arg)->update(buffer[i]);
        if (i == buffer.size() - 1)
            endp = true;
        isUpdate = true;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(nullptr);
}

void* consumer_routine(void* arg) {
    // notify about start
    // allocate value for result
    // for every update issued by producer, read the value and add to sum
    // return pointer to result
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    startc = true;
    static int result = 0;
    while(true) {
        pthread_mutex_lock(&mutex);
        while(!isUpdate)
            pthread_cond_wait(&cond, &mutex);
        result += ((Value*)arg)->get();
        isUpdate = false;
        if (endp)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    endc = true;
    return reinterpret_cast<void *>(result);
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumer to start

    // interrupt consumer while producer is running
    auto* p = (pthread_t*) arg;
    while (!endc)
        if (startc)
            pthread_cancel(*p);
    pthread_exit(nullptr);
}

int run_threads() {
    // start 3 threads and wait until they're done
    // return sum of update values seen by consumer
    Value val;
    void* sum;
    pthread_t prod, cons, inter;
    pthread_create(&prod, nullptr, producer_routine, &val);
    pthread_create(&cons, nullptr, consumer_routine, &val);
    pthread_create(&inter, nullptr, consumer_interruptor_routine, &cons);
    pthread_join(prod, nullptr);
    pthread_join(cons, &sum);
    pthread_join(inter, nullptr);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return reinterpret_cast<int64_t>(sum);
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}