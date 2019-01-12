#include <pthread.h>
#include <iostream>
#include <unistd.h>

//int sum = 0;
int num_threads;
pthread_cond_t cond_prod = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_cons = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mt = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
bool changed = false;
bool the_end = false;

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

Value sum;

void* producer_routine(void* arg) {
    // Wait for consumer to start
    Value* val = static_cast<Value*>(arg);
    std::string data;
    getline(std::cin, data);
    for (int i = 0; i < data.size(); ++i) {
        if (data[i] == ' ') continue;
        int elem;
        for (int j = i; j < data.size(); ++j) {
            if (data[j] == ' ' || j == data.size() - 1) {
                if (j == data.size() - 1) j++;
                elem = std::stoi(data.substr(i, j));
                i = j;
                break;
            }
        }
        pthread_mutex_lock(&mt);
        while (changed) pthread_cond_wait(&cond_prod, &mt);
        val->update(elem);
        changed = true;
        pthread_cond_signal(&cond_cons);
        pthread_mutex_unlock(&mt);
    }
//    while (changed);
    pthread_mutex_lock(&mt);
    while (changed) pthread_cond_wait(&cond_prod, &mt);
    the_end = true;
    pthread_cond_broadcast(&cond_cons);
    pthread_mutex_unlock(&mt);
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    pthread_exit(NULL);
}

void* consumer_routine(void* arg) {
    // notify about start
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&barrier);
    while (true) {
        pthread_mutex_lock(&mt);
        while (!changed && !the_end) pthread_cond_wait(&cond_cons, &mt);
        if (the_end && !changed) {
            pthread_cond_signal(&cond_prod);
            pthread_mutex_unlock(&mt);
            break;
        }
        sum.update(sum.get() + static_cast<Value*>(arg)->get());
        changed = false;
        pthread_cond_signal(&cond_prod);
        pthread_mutex_unlock(&mt);
        sleep(std::rand() % 1);
    }
    return &sum;
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (aggregated result for all consumers)
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumer to start
    pthread_barrier_wait(&barrier);
    while (!the_end) pthread_cancel(static_cast<pthread_t*>(arg)[std::rand() % num_threads]);
    pthread_exit(NULL);
    // interrupt consumer while producer is running
}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values
    pthread_t prod;
    pthread_t cons[num_threads];
    pthread_t inter;
    pthread_barrier_init(&barrier, nullptr, num_threads + 1);
    Value val;
    val.update(0);
    pthread_create(&prod, nullptr, producer_routine, &val);
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(cons + i, nullptr, consumer_routine, &val);
    }
    pthread_create(&inter, nullptr, consumer_interruptor_routine, cons);
    pthread_join(prod, nullptr);
    int* res;
    pthread_join(inter, nullptr);
    pthread_join(*cons, (void**)&res);
    for (int i = 1; i < num_threads; ++i) pthread_join(cons[i], nullptr);
    return *res;
}

int main(int argc, char* argv[]) {

    num_threads = std::atoi(argv[1]);
    std::cout << run_threads() << std::endl;
    return 0;
}