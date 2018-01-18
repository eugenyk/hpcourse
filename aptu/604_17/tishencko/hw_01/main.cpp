#include <pthread.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include<iterator>

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

pthread_mutex_t the_mutex;
pthread_cond_t condcons, condprod, condrun;
volatile bool buffer_flag = false;
volatile bool started_flag = false;
volatile bool finished_flag = false;

std::vector<int> list;

void* producer_routine(void* arg) {
    pthread_mutex_lock(&the_mutex);
    while (!started_flag) pthread_cond_wait(&condrun, &the_mutex);
    pthread_mutex_unlock(&the_mutex);

    for (int i = 0; i <= list.size(); ++i) {
          pthread_mutex_lock(&the_mutex);
          while (buffer_flag) pthread_cond_wait(&condprod, &the_mutex);
          ((Value*)arg)->update(list[i]);
          buffer_flag = true;
          pthread_cond_signal(&condcons);
          if (i == list.size() - 1) {
              finished_flag = true;
              pthread_cond_signal(&condrun);
          }
          pthread_mutex_unlock(&the_mutex);
    }
    return nullptr;
}

void* consumer_routine(void* arg) {
    int* sum = new int(0);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
    pthread_mutex_lock(&the_mutex);
    started_flag = true;
    pthread_cond_broadcast(&condrun);
    pthread_mutex_unlock(&the_mutex);

    for (int i = 0; i < list.size(); ++i) {
        pthread_mutex_lock(&the_mutex);
        while (!buffer_flag) pthread_cond_wait(&condcons, &the_mutex);
        *sum += ((Value*)arg)->get();
        buffer_flag = false;
        pthread_cond_signal(&condprod);
        pthread_mutex_unlock(&the_mutex);
    }
    return sum;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_mutex_lock(&the_mutex);
    while (!started_flag) pthread_cond_wait(&condrun, &the_mutex);
    pthread_mutex_unlock(&the_mutex);
    while (!finished_flag) pthread_cancel(*((pthread_t*)arg));
    return nullptr;
}

int run_threads() {
    copy(std::istream_iterator<int>(std::cin), std::istream_iterator<int>(), back_inserter(list));
    Value val = Value();
    void* res;
    pthread_t con, prod, inter;
    pthread_mutex_init(&the_mutex, 0);
    pthread_cond_init(&condcons, 0);
    pthread_cond_init(&condprod, 0);
    pthread_cond_init(&condrun, 0);
    pthread_create(&con, 0, consumer_routine, &val);
    pthread_create(&prod, 0, producer_routine, &val);
    pthread_create(&inter, 0, consumer_interruptor_routine, &con);
    pthread_join(prod, 0);
    pthread_join(con, &res);
    pthread_join(inter, 0);
    pthread_cond_destroy(&condcons);
    pthread_cond_destroy(&condprod);
    pthread_cond_destroy(&condrun);
    pthread_mutex_destroy(&the_mutex);
    return *((int*)res);
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
