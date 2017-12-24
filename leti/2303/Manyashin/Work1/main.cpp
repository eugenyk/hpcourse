#include <pthread.h>
#include <iostream>
#include <vector>

using namespace std;

bool debug = true;

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

pthread_mutex_t m;
pthread_cond_t upd;
bool is_new_val = true;
bool is_end = false;

pthread_t producer, consumer, interruptor;

void* producer_routine(void* arg) {
    vector<int> data;
    int num;
    cout << "input in producer section\n";
    data.push_back(1);
    data.push_back(2);
    data.push_back(4);
    data.push_back(8);
    data.push_back(16);
    data.push_back(32);
    data.push_back(64);
    data.push_back(128);
    data.push_back(256);
    cout<<"produser run "<< endl;
    for (vector<int>::iterator it = data.begin(); it != data.end(); ++it) {
        pthread_mutex_lock(&m);
        while (is_new_val) pthread_cond_wait(&upd, &m);

        ((Value*)arg)->update(*it);
        is_new_val = true;
        if (debug) cout << "producer: next value = " << *it << endl;
        pthread_cond_signal(&upd);
        pthread_mutex_unlock(&m);
    }

    pthread_mutex_lock(&m);
    is_end = true;
    pthread_cond_signal(&upd);
    pthread_mutex_unlock(&m);

    if (debug) cout << "producer: finished\n";
}

void* consumer_routine(void* arg) {

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    int* result = new int;
    *result = 0;
    cout << "input in consumer section\n";
    pthread_mutex_lock(&m);
    is_new_val = false;
    if (debug) cout << "consumer: started\n";
    pthread_cond_signal(&upd);
    pthread_mutex_unlock(&m);

    while (!is_end) {
        pthread_mutex_lock(&m);
        while (!is_new_val && !is_end) pthread_cond_wait(&upd, &m);

        if (is_new_val) {
            *result += ((Value*)arg)->get();
            is_new_val = false;
            pthread_cond_signal(&upd);
        }

        pthread_mutex_unlock(&m);
    }

    if (debug)  cout << "consumer: finished\n";

    return (void*)result;
}

void* consumer_interruptor_routine(void* arg) {
    cout << "input in interruptor section\n";
    pthread_mutex_lock(&m);
    while (is_new_val) pthread_cond_wait(&upd, &m);
    if (debug) cout << "interruptor: started\n";
    pthread_mutex_unlock(&m);

    while (!is_end) pthread_cancel(consumer);

    if (debug) cout << "interruptor: finished\n";
}

int run_threads() {
    Value* val = new Value();
    int* sum = new int;

    pthread_create(&consumer, NULL, consumer_routine, (void*)val);
    cout << "Consumer started" << endl;
    pthread_create(&producer, NULL, producer_routine, (void*)val);
    cout << "Producer started" << endl;
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);
    cout << "Interrupter started" << endl;

    pthread_join(producer, NULL);
    pthread_join(consumer, (void**)&sum);
    pthread_join(interruptor, NULL);
    return *sum;
}

int main() {
    cout << "Final result:" << run_threads() << endl;
    return 0;
}
