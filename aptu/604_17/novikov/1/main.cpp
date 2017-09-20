#include <iostream>
#include <pthread.h>

using namespace std;
 
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

bool consumer_started = false;
bool consumer_ready = false;
bool producer_ready = false;
bool producer_finished = false;

void* producer_routine(void* arg) {
    Value& value = *static_cast<Value*>(arg);
  
    while (!consumer_started);

    int val;
    while (cin >> val) {
        value.update(val);

        consumer_ready = false;
        producer_ready = true;

        while (!consumer_ready);
    }
 
    producer_finished = true;
    return nullptr;
}
 
void* consumer_routine(void* arg) {
    Value& value = *static_cast<Value*>(arg);

    consumer_started = true;
        
    int* sum_ptr = new int();

    while (true) {
        while (!producer_ready && !producer_finished);
        if (producer_finished)
            break;
        
        *sum_ptr += value.get();
        producer_ready = false;
        consumer_ready = true;
    }

    return sum_ptr;
}

 
void* consumer_interruptor_routine(void* arg) {
    pthread_t consumer = *static_cast<pthread_t*>(arg);

    while (!producer_finished) {
        pthread_cancel(consumer) == 0;
    }

    return nullptr;
}
 
int run_threads() {
    pthread_t producer;
    pthread_t consumer;
    pthread_t interruptor;

    Value value;

    pthread_create(&producer, nullptr, producer_routine, &value);
    pthread_create(&consumer, nullptr, consumer_routine, &value);
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumer);

    void* sum_ptr;
    pthread_join(consumer, &sum_ptr);
    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
 
    return *static_cast<int*>(sum_ptr);
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
