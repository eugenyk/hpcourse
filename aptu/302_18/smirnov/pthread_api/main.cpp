#include <iostream>
#include <cassert>
#include <functional>
#include <pthread.h>

enum State {
    PRODUCED,  // value is assigned, but not processed
    CONSUMED,  // value is already processed and did not change
    FINISHED   // producer has finished operation
};

const int THREAD_COUNT = 3;

pthread_cond_t state_updated_condition;
pthread_mutex_t value_mutex;
volatile State state;

pthread_barrier_t consumer_started_barrier;

// update state and broadcast it to all threads
void update_state(State new_state) {
    state = new_state;
    pthread_cond_broadcast(&state_updated_condition);
}

// wait for state to become as expected (or finish)
void wait_for_state(State expected_state) {
    while (state != expected_state) {
        pthread_cond_wait(&state_updated_condition, &value_mutex);
        if (state == FINISHED) {
            return;
        }
    }
}

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
 
void* producer_routine(void* arg) {
    Value *value = (Value *) arg;

    // Wait for consumer to start
    pthread_barrier_wait(&consumer_started_barrier);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    int next_number;
    while (std::cin >> next_number) {
        pthread_mutex_lock(&value_mutex);
        
        value->update(next_number);
        update_state(PRODUCED);
        wait_for_state(CONSUMED);

        pthread_mutex_unlock(&value_mutex);
    }

    // update state to FINISHED
    // locks are to make sure we don't do it unexpectedly for consumer
    // (fixes deadlock of signal fired between consumer waiting for it 
    // and checking the actual state)
    pthread_mutex_lock(&value_mutex);
    update_state(FINISHED);
    pthread_mutex_unlock(&value_mutex);

    return NULL;
}

void* consumer_routine(void* arg) {
    Value *value = (Value *) arg;

    // defence against interruption
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    // notify about start
    pthread_barrier_wait(&consumer_started_barrier);

    // allocate value for result
    int *result = new int;

    // for every update issued by producer, read the value and add to sum
    while (state != FINISHED) {
        pthread_mutex_lock(&value_mutex);

        wait_for_state(PRODUCED);

        if (state != FINISHED) {
            *result += value->get();
            update_state(CONSUMED);
        }

        pthread_mutex_unlock(&value_mutex);
    }

    // return pointer to result
    return result;
}
 
void* consumer_interruptor_routine(void* arg) {
    pthread_t *consumer_thread = (pthread_t *) arg;
    
    // wait for consumer to start
    pthread_barrier_wait(&consumer_started_barrier);

    // interrupt consumer while producer is running
    while (state != FINISHED) {
        pthread_cancel(*consumer_thread);
    }

    return NULL;
}
 
int run_threads() {
    state_updated_condition = PTHREAD_COND_INITIALIZER;
    value_mutex = PTHREAD_MUTEX_INITIALIZER;
    state = CONSUMED;
    pthread_barrier_init(&consumer_started_barrier, NULL, THREAD_COUNT);

    pthread_t consumer;
    pthread_t producer;
    pthread_t consumer_interruptor;
    
    Value value;

    pthread_create(
        &consumer, 
        NULL, 
        consumer_routine,
        &value
    );
    
    pthread_create(
        &producer, 
        NULL, 
        producer_routine,
        &value
    );

    pthread_create(
        &consumer_interruptor, 
        NULL, 
        consumer_interruptor_routine,
        &consumer
    );

    int *result;

    pthread_join(producer, NULL);
    pthread_join(consumer, (void **) &result);
    pthread_join(consumer_interruptor, NULL);

    pthread_barrier_destroy(&consumer_started_barrier);
    
    return *result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}