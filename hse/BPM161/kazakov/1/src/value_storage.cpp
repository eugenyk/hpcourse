#include "../include/value_storage.h"
#include "../include/no_value_exception.h"


void value_storage::update(int value) {
    pthread_mutex_lock(&value_access);
    while (has_value_) {
        pthread_cond_wait(&value_queried, &value_access);
    }
    this->value = value;
    has_value_ = true;
    pthread_cond_signal(&value_updated);
    pthread_mutex_unlock(&value_access);
}

int value_storage::get() {
    pthread_mutex_lock(&value_access);
    while (!has_value_) {
        pthread_cond_wait(&value_updated, &value_access);
        if (is_closed_ && !has_value_) {
            pthread_mutex_unlock(&value_access);
            throw no_value_exception();
        }
    }
    int current_value = this->value;
    has_value_ = false;
    pthread_cond_signal(&value_queried);
    pthread_mutex_unlock(&value_access);
    return current_value;
}

void value_storage::close() {
    pthread_mutex_lock(&value_access);
    is_closed_ = true;
    pthread_cond_broadcast(&value_updated);
    pthread_mutex_unlock(&value_access);
}

bool value_storage::is_closed() {
    return is_closed_;
}


bool value_storage::has_value() {
    return has_value_;
}
