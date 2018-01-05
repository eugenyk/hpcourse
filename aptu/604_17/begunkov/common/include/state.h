#ifndef STATE_H_
#define STATE_H_

#include <errno.h>
#include <pthread.h>

typedef struct state_sync {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    volatile int state;
} state_sync_t;

#ifdef __cplusplus
    #define EVENT_INITIALIZER(state) {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, (state)}
#else
    #define TASK_WAIT_INITIALIZER(state) { \
        .mutex = PTHREAD_MUTEX_INITIALIZER,\
        .cond = PTHREAD_COND_INITIALIZER,\
        .started = (state)\
    }
#endif

inline int init_state(state_sync_t *state_sync, int state)
{
    int err;

    if (pthread_mutex_init(&state_sync->mutex, NULL))
        return errno;

    if (pthread_cond_init(&state_sync->cond, NULL)) {
        err = errno;
        pthread_mutex_destroy(&state_sync->mutex);
        return err;
    }
    state_sync->state = state;

    return 0;
}

inline void release_state(state_sync_t *state_sync)
{
    pthread_mutex_destroy(&state_sync->mutex);
    pthread_cond_destroy(&state_sync->cond);
}

inline int wait_state_lock(state_sync_t *state_sync, int state)
{
    pthread_mutex_lock(&state_sync->mutex);
    while (state_sync->state != state)
        pthread_cond_wait(&state_sync->cond, &state_sync->mutex);
    return state_sync->state;
}

inline int wait_state_change_lock(state_sync_t *state_sync, int state)
{
    pthread_mutex_lock(&state_sync->mutex);
    while (state_sync->state == state)
        pthread_cond_wait(&state_sync->cond, &state_sync->mutex);
    return state_sync->state;
}

inline void set_state(state_sync_t *state_sync, int state)
{
    state_sync->state = state;
    pthread_cond_broadcast(&state_sync->cond);
}

inline void set_state_unlock(state_sync_t *state_sync, int state)
{
    set_state(state_sync, state);
    pthread_mutex_unlock(&state_sync->mutex);
}

inline int get_state_lock(state_sync_t *state_sync)
{
    pthread_mutex_lock(&state_sync->mutex);
    return state_sync->state;
}

#endif //STATE_H_
