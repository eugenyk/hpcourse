#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

pthread_mutex_t value_lock;
pthread_cond_t value_ready_cond;
pthread_cond_t value_processed_cond;
pthread_cond_t consumers_ready_cond;
pthread_barrier_t consumers_barrier;
pthread_barrier_t consumers_done_barrier;
int value_ready = 0;
int eof = 0;
int sum = 0;
int sleep_msec;
int n;

static void producer_routine_impl(int *value) {
    while (scanf("%d", value) > 0) {
        pthread_mutex_lock(&value_lock);
        value_ready = 1;
        pthread_cond_signal(&value_ready_cond);
        while (value_ready)
            pthread_cond_wait(&value_processed_cond, &value_lock);
        pthread_mutex_unlock(&value_lock);
    }
    eof = 1;
    pthread_cond_broadcast(&value_ready_cond);
}

static int consumer_routine_impl(int *value) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_barrier_wait(&consumers_barrier);
    while (!eof) {
        pthread_mutex_lock(&value_lock);
        while (!value_ready && !eof)
            pthread_cond_wait(&value_ready_cond, &value_lock);
        if (!eof) {
            sum += *value;
            value_ready = 0;
            pthread_mutex_unlock(&value_lock);
            pthread_cond_signal(&value_processed_cond);
            int msec = rand() % sleep_msec;
            usleep(1000 * msec);
        } else {
            pthread_mutex_unlock(&value_lock);
            break;
        }
    }
    pthread_barrier_wait(&consumers_done_barrier);
    return sum;
}

static void interruptor_routine_impl(pthread_t *cons) {
    pthread_barrier_wait(&consumers_barrier);
    while (!eof) {
        int i = rand() % n;
        pthread_cancel(cons[i]);
    }
    pthread_barrier_wait(&consumers_done_barrier);
}

static void *producer_routine(void *arg) {
    producer_routine_impl((int *)arg);
    return NULL;
}

static void *consumer_routine(void *arg) {
    return (void *)consumer_routine_impl((int *)arg);
}

static void *interruptor_routine(void *arg) {
    interruptor_routine_impl((pthread_t *)arg);
    return NULL;
}

int run_threads() {
    pthread_mutex_init(&value_lock, NULL);
    pthread_cond_init(&value_ready_cond, NULL);
    pthread_cond_init(&value_processed_cond, NULL);
    pthread_cond_init(&consumers_ready_cond, NULL);
    pthread_barrier_init(&consumers_barrier, NULL, n + 1);
    pthread_barrier_init(&consumers_done_barrier, NULL, n + 1);

    time_t t;
    srand((unsigned int)time(&t));

    int value;
    pthread_t prod;
    pthread_t *cons = (pthread_t *)malloc(n * sizeof(pthread_t));
    pthread_t intr;
    pthread_create(&intr, NULL, interruptor_routine, cons);
    for (int i = 0; i < n; ++i) {
        pthread_create(cons + i, NULL, consumer_routine, &value);
    }
    pthread_create(&prod, NULL, producer_routine, &value);

    int result;
    pthread_join(intr, NULL);
    pthread_join(prod, NULL);
    for (int i = 0; i < n; ++i) {
        pthread_join(cons[i], (void **)(&result));
    }
    free(cons);

    pthread_barrier_destroy(&consumers_done_barrier);
    pthread_barrier_destroy(&consumers_barrier);
    pthread_cond_destroy(&consumers_ready_cond);
    pthread_cond_destroy(&value_processed_cond);
    pthread_cond_destroy(&value_ready_cond);
    pthread_mutex_destroy(&value_lock);
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        exit(1);
    }
    n = atoi(argv[1]);
    sleep_msec = atoi(argv[2]);
    if (n <= 0 || sleep_msec <= 0) {
        exit(1);
    }
    printf("%d\n", run_threads());
}
