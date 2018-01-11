#include <pthread.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <tuple>
#include <zconf.h>


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


typedef std::pair<pthread_mutex_t*, pthread_cond_t*> mut_cond_pair;

typedef std::tuple<pthread_t, bool*, bool*> thread_tup;
typedef std::tuple<pthread_mutex_t*,
        pthread_cond_t*, pthread_cond_t*,
        Value*, bool*, bool*> tup;

void* producer_routine(void* arg) {
    tup* tupl = (tup*) arg;

    pthread_mutex_t* mutex_data = std::get<0>(*tupl);
    pthread_cond_t* buffer_full = std::get<1>(*tupl);
    pthread_cond_t* buffer_empty = std::get<2>(*tupl);
    Value* val = std::get<3>(*tupl);
    bool* not_finished = std::get<4>(*tupl);
    bool* buffer_ready_flag = std::get<5>(*tupl);

    std::vector<int64_t> values{std::istream_iterator<int64_t>(std::cin),
                                std::istream_iterator<int64_t>()};

    for (size_t i = 0; i < values.size(); ++i) {
        pthread_mutex_lock(mutex_data);
        while (*buffer_ready_flag) {
            pthread_cond_wait(buffer_empty, mutex_data);
        }
        val->update(values[i]);
        *buffer_ready_flag = true;

        pthread_mutex_unlock(mutex_data);
        pthread_cond_signal(buffer_full);
    }

    pthread_mutex_lock(mutex_data);
    while (*buffer_ready_flag) {
        pthread_cond_wait(buffer_empty, mutex_data);
    }
    *not_finished = false;
    pthread_mutex_unlock(mutex_data);
    pthread_cond_signal(buffer_full);

    return 0;
}

void* consumer_routine(void* arg) {
    tup* tupl = (tup*) arg;

    pthread_mutex_t* mutex_data = std::get<0>(*tupl);
    pthread_cond_t* buffer_full = std::get<1>(*tupl);
    pthread_cond_t* buffer_empty = std::get<2>(*tupl);
    Value* val = std::get<3>(*tupl);
    bool* not_finished = std::get<4>(*tupl);
    bool* buffer_ready_flag = std::get<5>(*tupl);

    int64_t* result = new int64_t(0);

    while (*not_finished) {
        pthread_mutex_lock(mutex_data);
        while (!*buffer_ready_flag) {
            pthread_cond_wait(buffer_full, mutex_data);

            if (!*not_finished && !*buffer_ready_flag) break;
        }
        if (!*not_finished && !*buffer_ready_flag) break;

        *result += val->get();

        *buffer_ready_flag = false;
        pthread_mutex_unlock(mutex_data);
        pthread_cond_broadcast(buffer_empty);
    }
    pthread_exit(result);
}

void* consumer_interruptor_routine(void* arg) {

    thread_tup* tupl = (thread_tup*) arg;

    pthread_t thread = std::get<0>(*tupl);
    bool* not_finished = std::get<1>(*tupl);
    bool* buffer_ready_flag = std::get<2>(*tupl);

    while (true) {
        while (!*buffer_ready_flag && *not_finished) {
            pthread_cancel(thread);
        }
        if (!*not_finished) break;
    }



    return 0;
}

int run_threads() {
    pthread_t prod_thread;
    pthread_t con_thread;
    pthread_t inter_thread;

    pthread_mutex_t mutex_data;
    pthread_cond_t buffer_full;
    pthread_cond_t buffer_empty;

    pthread_mutex_init(&mutex_data, NULL);
    pthread_cond_init(&buffer_full, NULL);
    pthread_cond_init(&buffer_empty, NULL);

    Value value;
    bool not_finished = true;
    bool ready = false;

    tup tupl = std::make_tuple(&mutex_data, &buffer_full,
                               &buffer_empty, &value, &not_finished, &ready);

    thread_tup for_cancel = std::make_tuple(con_thread, &not_finished, &ready);

    assert(pthread_create(&prod_thread, NULL,
                          producer_routine, &tupl) == 0);
    assert(pthread_create(&con_thread, NULL,
                          consumer_routine, &tupl) == 0);
    assert(pthread_create(&inter_thread, NULL,
                          consumer_interruptor_routine, &for_cancel) == 0);

    void* result = NULL;

    assert(pthread_join(prod_thread, NULL) == 0);
    assert(pthread_join(con_thread, &result) == 0);
    assert(pthread_join(inter_thread, NULL) == 0);

    pthread_mutex_destroy(&mutex_data);
    pthread_cond_destroy(&buffer_full);
    pthread_cond_destroy(&buffer_empty);


  return *(int64_t*)result;
}

int main() {

    std::cout << run_threads() << std::endl;

    return 0;
}