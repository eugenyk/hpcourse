#include <iostream>

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

int numc, *nums;
pthread_mutex_t value_mutex;
pthread_cond_t value_cond;
enum {
    NOTHING, PRODUCED, FINISHED, CONSUMED
} status;

void *producer_routine(void *arg) {
    Value *value = (Value *) arg;

    for (int i = 0; i < numc; i++) {
        pthread_mutex_lock(&value_mutex);

        while (status != CONSUMED) {
            pthread_cond_wait(&value_cond, &value_mutex);
        }
        value->update(nums[i]);
        status = (i == numc - 1 ? FINISHED : PRODUCED);
        pthread_cond_signal(&value_cond);

        pthread_mutex_unlock(&value_mutex);
    }

    pthread_exit(0);
}

void *consumer_routine(void *arg) {
    Value *value = (Value *) arg;
    int *acc = new int(0);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    status = CONSUMED;
    pthread_cond_broadcast(&value_cond);

    pthread_mutex_lock(&value_mutex);
    while (1) {
        while (status != PRODUCED && status != FINISHED) {
            pthread_cond_wait(&value_cond, &value_mutex);
        }
        *acc += value->get();
        if (status == FINISHED) {
            break;
        }
        status = CONSUMED;
        pthread_cond_signal(&value_cond);
    }
    pthread_mutex_unlock(&value_mutex);

    pthread_exit((void *) acc);
}

void *consumer_interruptor_routine(void *arg) {
    pthread_mutex_lock(&value_mutex);
    while (status == NOTHING) {
        pthread_cond_wait(&value_cond, &value_mutex);
    }
    pthread_mutex_unlock(&value_mutex);

    pthread_t consumer = *((pthread_t *) arg);
    while (status != FINISHED) {
        pthread_cancel(consumer);
    }

    pthread_exit(0);
}

int run_threads() {
    pthread_t producer, consumer, interruptor;
    pthread_attr_t attr;
    status = NOTHING;

    pthread_mutex_init(&value_mutex, NULL);
    pthread_cond_init(&value_cond, NULL);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    Value *value = new Value();
    pthread_create(&producer, &attr, producer_routine, (void *) value);
    pthread_create(&consumer, &attr, consumer_routine, (void *) value);
    pthread_create(&interruptor, &attr, consumer_interruptor_routine, (void *) consumer);

    void *status;
    pthread_join(producer, &status);
    pthread_join(consumer, &status);
    int return_value = *((int *) status);
    delete status;
    pthread_join(interruptor, &status);

    delete value;
    pthread_attr_destroy(&attr);
    pthread_cond_destroy(&value_cond);
    pthread_mutex_destroy(&value_mutex);

    return return_value;
}

void parse_nums(int argc, char *args[]) {
    numc = argc - 1;
    nums = new int[numc];
    for (int i = 0; i < numc; i++) {
        nums[i] = atoi(args[i + 1]);
    }
}

void delete_nums() {
    delete nums;
}

int main(int argc, char *args[]) {
    parse_nums(argc, args);
    cout << run_threads() << endl;
    delete_nums();
    pthread_exit(0);
}