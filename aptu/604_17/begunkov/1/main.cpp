#include <pthread.h>
#include <stdlib.h>
#include <iostream>

#include "state.h"

#define STATE_INIT 0
#define STATE_WAIT_UPD 1
#define STATE_WAIT_CONS 2
#define STATE_END 3

class Value {
public:
    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }

private:
    int _value;
};

state_sync_t sync;

void* producer_routine(void* arg)
{
    Value *val = (Value *)arg;
    int input = 0;

    /* wait start */
    wait_state_change_lock(&sync, STATE_INIT);
    pthread_mutex_unlock(&sync.mutex);

    /* producing */
    while (std::cin >> input) {
        wait_state_lock(&sync, STATE_WAIT_UPD);
        val->update(input);
        set_state_unlock(&sync, STATE_WAIT_CONS);
    }

    /* finalize */
    wait_state_lock(&sync, STATE_WAIT_UPD);
    set_state_unlock(&sync, STATE_END);
    return NULL;
}

void* consumer_routine(void* arg)
{
    Value *val = (Value *)arg;
    int *answer = new int(0);

    /* notify start */
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    (void)get_state_lock(&sync);
    set_state_unlock(&sync, STATE_WAIT_UPD);

    /* consuming */
    while (true) {
        pthread_mutex_lock(&sync.mutex);
        static_assert(STATE_WAIT_UPD < STATE_WAIT_CONS && STATE_WAIT_CONS < STATE_END);
        while (sync.state < STATE_WAIT_CONS) /* wait STATE_WAIT_CONS or STATE_END */
            pthread_cond_wait(&sync.cond, &sync.mutex);

        if (sync.state == STATE_END) {
            pthread_mutex_unlock(&sync.mutex);
            break;
        }
        *answer += val->get();
        set_state_unlock(&sync, STATE_WAIT_UPD);
    }

    return answer;
}

void* consumer_interrupter_routine(void *arg)
{
    pthread_t *th = (pthread_t *)arg;
    int state;

    /* wait start */
    wait_state_change_lock(&sync, STATE_INIT);
    pthread_mutex_unlock(&sync.mutex);

    /* cancel loop */
    do {
        state = get_state_lock(&sync);
        pthread_cancel(*th);
        /* once @state became STATE_END it won't change, thus it's safe to unlock preemptively */
        pthread_mutex_unlock(&sync.mutex);
    } while (state != STATE_END);

    return NULL;
}

int run_threads()
{
    pthread_t th_producer, th_consumer, th_interrupter;
    Value value;
    int *sum, res, err;

    err = init_state(&sync, STATE_INIT);
    if (err) {
        perror("Can't initialize state\n");
        exit(1);
    }

    pthread_create(&th_interrupter, NULL, consumer_interrupter_routine, &th_consumer);
    pthread_create(&th_producer, NULL, producer_routine, &value);
    pthread_create(&th_consumer, NULL, consumer_routine, &value);

    pthread_join(th_consumer, (void**)&sum);
    pthread_join(th_producer, NULL);
    pthread_join(th_interrupter, NULL);

    release_state(&sync);

    res = *sum;
    delete sum;
    return res;
}

int main()
{
    const int res = run_threads();
    std::cout << "sum = " << res << std::endl;
    return 0;
}
