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

pthread_mutex_t valueMutex;
pthread_cond_t valueCondition;
pthread_barrier_t startBarrier;
volatile bool producerOnline = false;
volatile bool consumerOnline = false;
volatile bool valueConsumed = true;

pthread_t consumerThread;

void* producer_routine(void* arg) {
    Value* value = (Value*)(arg);
    int v;

    producerOnline = true;

    //Wait for start
    pthread_barrier_wait(&startBarrier);

    pthread_mutex_lock(&valueMutex);
    
    while (cin >> v) {
        while (!valueConsumed) {
            pthread_cond_wait(&valueCondition, &valueMutex);
        }
        value->update(v);
        valueConsumed = false;
        pthread_cond_broadcast(&valueCondition);
    }
    producerOnline = false;
    pthread_cond_broadcast(&valueCondition);
    pthread_mutex_unlock(&valueMutex);
    return nullptr;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
	consumerOnline = true;
	
    
    pthread_mutex_lock(&valueMutex);
    pthread_barrier_wait(&startBarrier);
	
	Value* value = (Value*)(arg);
    int* sumptr = new int;
    *sumptr = 0;

    while (producerOnline) {
        pthread_cond_wait(&valueCondition, &valueMutex);
        if (!valueConsumed) {
            *sumptr += value->get();
            valueConsumed = true;
            pthread_cond_broadcast(&valueCondition);
        }
    }
    consumerOnline = false;
    pthread_mutex_unlock(&valueMutex);
    return sumptr;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&startBarrier);

	while (consumerOnline) {
        pthread_cancel(consumerThread);
    }
    return nullptr;
}

int run_threads() {
    Value value = Value();
    int* sumptr;
    pthread_t producerThread;
    pthread_t consumerInterruptorThread;

    pthread_mutex_init(&valueMutex, nullptr);
    pthread_cond_init(&valueCondition, nullptr);
	pthread_barrier_init(&startBarrier, nullptr, 3);

    pthread_create(&producerThread, nullptr, producer_routine, &value);
    pthread_create(&consumerInterruptorThread, nullptr, consumer_interruptor_routine, nullptr);
    pthread_create(&consumerThread, nullptr, consumer_routine, &value);

    pthread_join(producerThread, nullptr);
    pthread_join(consumerInterruptorThread, nullptr);
    pthread_join(consumerThread, (void**)(&sumptr));

    int sum = *sumptr;
    delete sumptr;

    return sum;
}

int main() {
    cout << run_threads() << std::endl;
    return 0;
}
