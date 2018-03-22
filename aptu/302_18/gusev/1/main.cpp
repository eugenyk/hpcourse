#include <pthread.h>
#include <iostream>

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

class FinishedException {};

class SumProcess {
private:
    Value *value;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
    bool done;
    bool pending;
    bool started;
public:
    SumProcess() {
        value = new Value();
        done = false;
        pending = false;
        mutex = new pthread_mutex_t;
        pthread_mutex_init(mutex, nullptr);
        cond = new pthread_cond_t;
        pthread_cond_init(cond, nullptr);
    }

    void updateValue(int val) {
        pthread_mutex_lock(mutex);
        while (pending) {
            if (done) {
                pthread_mutex_unlock(mutex);
                throw FinishedException();
            }
            pthread_cond_wait(cond, mutex);
        }
        value->update(val);
        pending = true;
        pthread_cond_broadcast(cond);
        pthread_mutex_unlock(mutex);
    }

    int getValue() {
        pthread_mutex_lock(mutex);
        while (!pending) {
            if (done) {
                pthread_mutex_unlock(mutex);
                throw FinishedException();
            }
            pthread_cond_wait(cond, mutex);
        }
        int val = value->get();
        pending = false;
        pthread_cond_broadcast(cond);
        pthread_mutex_unlock(mutex);
        return val;
    }

    bool isDone() const {
        pthread_mutex_lock(mutex);
        bool _done = done;
        pthread_mutex_unlock(mutex);
        return _done;
    }

    void setDone() {
        pthread_mutex_lock(mutex);
        done = true;
        pthread_cond_broadcast(cond);
        pthread_mutex_unlock(mutex);
    }

    void start() {
        pthread_mutex_lock(mutex);
        started = true;
        pthread_cond_broadcast(cond);
        pthread_mutex_unlock(mutex);
    }

    void waitToStart() {
        pthread_mutex_lock(mutex);
        while (!started) {
            pthread_cond_wait(cond, mutex);
        }
        pthread_mutex_unlock(mutex);
    }

    ~SumProcess() {
        pthread_cond_destroy(cond);
        delete cond;
        pthread_mutex_destroy(mutex);
        delete mutex;
        delete value;
    }
};

struct InterrupterArg {
    pthread_t *consumer;
    SumProcess *sumProcess;
};

void* producer_routine(void* arg) {
    int data;
    SumProcess *sumProcess = (SumProcess*)arg;
    sumProcess->waitToStart();
    while (std::cin >> data) {
        sumProcess->updateValue(data);
    }
    sumProcess->setDone();
    return nullptr;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    SumProcess *sumProcess = (SumProcess*)arg;
    int *sum = new int;
    *sum = 0;
    sumProcess->start();
    while (!sumProcess->isDone()) {
        try {
            *sum += sumProcess->getValue();
        } catch (FinishedException e) {}
    }
    return (void*)sum;
}

void* consumer_interruptor_routine(void* arg) {
    SumProcess *sumProcess = ((InterrupterArg*)arg)->sumProcess;
    pthread_t *consumer = ((InterrupterArg*)arg)->consumer;
    sumProcess->waitToStart();
    while (!sumProcess->isDone()) {
        pthread_cancel(*consumer);
    }
}

int run_threads() {
    pthread_t *producer_thread = new pthread_t;
    pthread_t *consumer_thread = new pthread_t;
    pthread_t *interrupter_thread = new pthread_t;

    SumProcess *sumProcess = new SumProcess();
    InterrupterArg *interrupter_arg = new InterrupterArg;
    interrupter_arg->sumProcess = sumProcess;
    interrupter_arg->consumer = consumer_thread;

    pthread_create(producer_thread, nullptr, producer_routine, sumProcess);
    pthread_create(consumer_thread, nullptr, consumer_routine, sumProcess);
    pthread_create(interrupter_thread, nullptr, consumer_interruptor_routine, interrupter_arg);

    void **result_pointer = new void*;
    pthread_join(*consumer_thread, result_pointer);
    pthread_join(*producer_thread, nullptr);
    pthread_join(*interrupter_thread, nullptr);
    delete consumer_thread;
    delete producer_thread;
    delete interrupter_thread;
    delete sumProcess;
    delete interrupter_arg;
    int result = *((int*)(*result_pointer));
    delete (int*)(*result_pointer);
    delete result_pointer;
    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    std::cout.flush();
    return 0;
}