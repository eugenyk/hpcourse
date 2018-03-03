#include <iostream>
#include <vector>

#include <pthread.h>

class Value {
    public:
        Value() : _value(0), isNewValue(false), willBeUpdated(true) {
            pthread_mutex_init(&mutex, NULL);
            pthread_cond_init(&cond, NULL);
        }
        
        ~Value() {
            pthread_cond_destroy(&cond);
            pthread_mutex_destroy(&mutex);
        }
     
        void update(int value) {
            pthread_mutex_lock(&mutex);
        
            while (isNewValue) {
                pthread_cond_wait(&cond, &mutex);
            }
        
            updateImpl(value);
            
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
        
        void lastUpdate(int value) {
            pthread_mutex_lock(&mutex);
        
            while (isNewValue) {
                pthread_cond_wait(&cond, &mutex);
            }
        
            lastUpdateImpl(value);
            
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
        
        bool isEmptyForever() {
            pthread_mutex_lock(&mutex);
        
            bool result = !isNewValue && !willBeUpdated;
            
            pthread_mutex_unlock(&mutex);

            return result;
        }
     
        int get() {
            pthread_mutex_lock(&mutex);
            
            while (!isNewValue) {
                pthread_cond_wait(&cond, &mutex);
            }
            
            int localCopy = _value;
            isNewValue = false;
        
            pthread_cond_signal(&cond);    
            pthread_mutex_unlock(&mutex);
            
            return localCopy;
        }
     
    private:
        void updateImpl(int value) {
            _value = value;
            isNewValue = true;
        }
        
        void lastUpdateImpl(int value) {
            updateImpl(value);
            willBeUpdated = false;
        }
    
        int _value;
        bool isNewValue;
        bool willBeUpdated;
        
        pthread_mutex_t mutex;
        pthread_cond_t cond;
};

std::vector<int> readNumbers() {
    std::vector<int> numbers;
    
    while (true) {
        int n;
        std::cin >> n;
        
        if (std::cin.eof()) {
            break;
        }
        
        numbers.push_back(n);
    }
    
    return numbers;
}

void* producer_routine(void* arg) {
    Value *value = static_cast<Value*>(arg);
    std::vector<int> ints = readNumbers();
    
    int lastInt = ints.back();
    ints.pop_back();
    
    for (int i : ints) {
        value->update(i);
    }
    
    value->lastUpdate(lastInt);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    Value *value = static_cast<Value*>(arg);
    int *data = new int;
    *data = 0;
    
    while (!value->isEmptyForever()) {
        *data += value->get();
    }
  
    return data;
}

struct InterruptorArg {
    Value *value;
    pthread_t consumer;
};

void* consumer_interruptor_routine(void* arg) {
    InterruptorArg *realArg = static_cast<InterruptorArg*>(arg);
    
    Value *value = realArg->value;
    pthread_t consumer = realArg->consumer;
    
    while (!value->isEmptyForever()) {
        pthread_cancel(consumer);
    }                                       
}

int run_threads() {
    pthread_t producer, consumer, interruptor;
    Value value;
    
    pthread_create(&producer, NULL, producer_routine, &value);
    pthread_create(&consumer, NULL, consumer_routine, &value);
    
    InterruptorArg arg = {&value, consumer};
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, &arg);
    
    void *ptr_result = NULL;
    pthread_join(producer, NULL);
    pthread_join(consumer, &ptr_result);
    pthread_join(interruptor, NULL);
    
    int *int_result = static_cast<int*>(ptr_result);
    int result = *int_result;
    delete int_result;
    
    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    
    return 0;
}

