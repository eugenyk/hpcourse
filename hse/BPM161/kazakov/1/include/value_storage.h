#ifndef UNTITLED_VALUE_H
#define UNTITLED_VALUE_H

#include <pthread.h>


class value_storage {
    private:
        int value;
        pthread_mutex_t value_access = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t value_updated = PTHREAD_COND_INITIALIZER;
        pthread_cond_t value_queried = PTHREAD_COND_INITIALIZER;
        volatile bool has_value_ = false;
        volatile bool is_closed_ = false;

    public:
        void update(int value);

        int get();

        bool is_closed();

        bool has_value();

        void close();
};

#endif //UNTITLED_VALUE_H
