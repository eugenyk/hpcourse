#include <pthread.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
 
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

bool produces = true;
bool consumes = false;
bool updated = false;

pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void fail(const char *msg, int err);

void* producer_routine(void* arg)
{
    Value *val = (Value *)arg;
    int x;

    // wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!consumes)
        pthread_cond_wait(&can_consume, &mutex);
    pthread_mutex_unlock(&mutex);

    // produce values
    while (std::cin >> x) {
        pthread_mutex_lock(&mutex);
        val->update(x);
        updated = true;
        pthread_cond_signal(&can_consume); // notify consumer to consume
        while (updated)
            pthread_cond_wait(&can_consume, &mutex); // wait consumer to actually consume
        pthread_mutex_unlock(&mutex);
    }

    // notify consumer that there is nothing to produce anymore
    pthread_mutex_lock(&mutex);
    produces = false;
    pthread_cond_signal(&can_consume);
    pthread_mutex_unlock(&mutex);

    return NULL;
}
 
void* consumer_routine(void* arg)
{
    Value *val = (Value *)arg;

    // prevent cancellations from interruptor
    int oldstate;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate); // this can never fail, no need to check return value

    // notify that consumer started
    pthread_mutex_lock(&mutex);
    consumes = true;
    pthread_cond_broadcast(&can_consume);
    pthread_mutex_unlock(&mutex);

    // consume values
    int result = 0;
    pthread_mutex_lock(&mutex);
    while (true) {
        if (!updated)
            pthread_cond_wait(&can_consume, &mutex); // wait until we can consume
        if (!produces)
            break;
        if (updated) {
            result += val->get();
            updated = false;
            pthread_cond_signal(&can_consume); // notify that we successfully consumed
        }
    }
    pthread_mutex_unlock(&mutex);

    // allocate memory for result and store it there 
    int *res = (int *)malloc(sizeof(int));
    if (res == NULL) {
        fail("No mem for consumer result", 0);
        return NULL;
    }
    *(res) = result;
    return res;
}
 
void* consumer_interruptor_routine(void* arg)
{
    // wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!consumes)
        pthread_cond_wait(&can_consume, &mutex);
    pthread_mutex_unlock(&mutex);

    // get consumer id
    pthread_t consumer_thread_id = *((pthread_t *)arg);

    /*
     * As I guess, this thread just models some
     * outer cancellations of consumer thread.
     * Thus I dont acquire any mutexes here, just
     * send cancel if I've been given a chance to.
     */
    while (produces) {
        pthread_cancel(consumer_thread_id);
        // should I pthread_yield() here?
        // I would do it but I think the formal task imposes me not to.
    }

    return NULL;
}

int run_threads()
{
    int ret;
    Value data;
    pthread_t consumer_thread_id;
    pthread_t producer_thread_id;
    pthread_t interruptor_thread_id;

    if ((ret = pthread_create(&producer_thread_id, NULL, producer_routine, &data)) < 0) {
        fail("Failed to create producer thread", ret);
        return -1;
    }
    if ((ret = pthread_create(&consumer_thread_id, NULL, consumer_routine, &data)) < 0) {
        fail("Failed to create consumer thread", ret);
        return -1;
    }
    if ((ret = pthread_create(&interruptor_thread_id, NULL, consumer_interruptor_routine, &consumer_thread_id)) < 0) {
        fail("Failed to create interruptor thread", ret);
        return -1;
    }

    void *result;
    if ((ret = pthread_join(consumer_thread_id, &result)) < 0) {
        fail("Failed to join consumer thread", ret);
        return -1;
    }
    if (result == PTHREAD_CANCELED) {
        fail("Consumer thread was cancelled", 0);
        return -1;
    }

    ret = *((int *)result);
    free(result);

    return ret;
}

static void fail(const char *msg, int err)
{
    std::cerr << msg << std::endl;
    if (err < 0)
        std::cerr << strerror(err) << std::endl;
}
 
int main()
{
    std::cout << run_threads() << std::endl;
    return 0;
}
