#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
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

enum class JobStatus {
    none,
    started,
    finished,
};

enum class ValueStatus {
    none,
    updated,
    consumed
};

int sum = 0;
int consumer_threads = 0;
unsigned max_consumer_delay = 0; // in milliseconds
JobStatus job_status;
ValueStatus value_status;
pthread_mutex_t mutex;
pthread_cond_t cond;

void wait_for_consumer_tostart() {
    pthread_mutex_lock(&mutex);
    while (job_status != JobStatus::started) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void set_job_status(JobStatus status) {
    pthread_mutex_lock(&mutex);
    job_status = status;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

void* producer_routine(void* arg) {
    wait_for_consumer_tostart();

    int input = 0;
    Value &value = *static_cast<Value*>(arg);

    while (std::cin >> input) {
        pthread_mutex_lock(&mutex);
        value.update(input);
        value_status = ValueStatus::updated;
        pthread_cond_broadcast(&cond);
        while (value_status != ValueStatus::consumed) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    set_job_status(JobStatus::finished);

    return NULL;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    set_job_status(JobStatus::started);

    Value &value = *static_cast<Value*>(arg);

    while (true) {
        pthread_mutex_lock(&mutex);
        while (value_status != ValueStatus::updated &&
               job_status != JobStatus::finished) {
            pthread_cond_wait(&cond, &mutex);
        }

        if (job_status == JobStatus::finished) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        sum += value.get();
        value_status = ValueStatus::consumed;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);

        // usleep takes microseconds, so the
        // delay should be multiplied by 10^3
        unsigned delay = (rand() % max_consumer_delay) * 1000;
        usleep(delay);
    }

    return &sum;
}

void* consumer_interruptor_routine(void* arg) {
    wait_for_consumer_tostart();

    pthread_t *th_consumers = static_cast<pthread_t*>(arg);

    while (job_status == JobStatus::finished) {
        pthread_cancel(th_consumers[rand() % consumer_threads]);
    }

    return NULL;
}

int run_threads() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_t th_producer;
    pthread_t th_interruptor;
    pthread_t *th_consumers = new pthread_t[consumer_threads];
    Value value;

    pthread_create(&th_producer, NULL, producer_routine, &value);
    pthread_create(&th_interruptor, NULL, consumer_interruptor_routine,
                   th_consumers);
    for (int i = 0; i < consumer_threads; ++i) {
        pthread_create(&th_consumers[i], NULL, consumer_routine, &value);
    }

    pthread_join(th_producer, NULL);
    pthread_join(th_interruptor, NULL);
    for (int i = 0; i < consumer_threads; ++i) {
        pthread_join(th_consumers[i], NULL);
    }

    delete [] th_consumers;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return sum;
}

int main(int argc, char* argv[]) {
    consumer_threads = atoi(argv[1]);
    max_consumer_delay = atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}
