#include <pthread.h>
#include <iostream>
#include <vector>
#include <unistd.h>
//#include <thread> Windows
//#include <chrono> 
//http://wiki.osll.ru/doku.php/courses:high_performance_computing:producer_consumer

pthread_mutex_t mutex;
pthread_cond_t condprod;
pthread_cond_t condcons;
pthread_cond_t condstart;


//program arguments
int max_delay = 0;
int consumer_number = 0;
 
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

enum class Status {
    STARTED,
    FINISHED
};

enum class ValueStatus {
    NEEDSPRODUCER,
    NEEDSCONSUMER
};


namespace shared {
   Status status;
   ValueStatus value_status;
   int result = 0;
   Value value;
}

 
void* producer_routine(void* arg) {
// Wait for consumer to start
    if (shared::status != Status::STARTED) {
        pthread_mutex_lock(&mutex);
    }
    while (shared::status != Status::STARTED) {
        pthread_cond_wait(&condstart, &mutex);
    }
    pthread_mutex_unlock(&mutex);
 
// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    Value* value = static_cast<Value*>(arg);
    std::vector<int> values;
    int number;
    
    while (std::cin >> number) {
        values.push_back(number);
    }
    
    for (std::vector<int>::iterator it=values.begin(); it != values.end(); ++it) {
        pthread_mutex_lock(&mutex);
        value->update(*it);
        shared::value_status = ValueStatus::NEEDSCONSUMER;
        pthread_cond_signal(&condprod);
        
        while (shared::value_status != ValueStatus::NEEDSPRODUCER) {
             pthread_cond_wait(&condcons, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_lock(&mutex);
    shared::status = Status::FINISHED;
    pthread_cond_broadcast(&condprod);
    pthread_mutex_unlock(&mutex);
    
    return nullptr;
}
 
void* consumer_routine(void* arg) {
   //disable thread cancellation
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

  // notify about start
    pthread_mutex_lock(&mutex);
    shared::status = Status::STARTED;
    pthread_cond_broadcast(&condstart);
    pthread_mutex_unlock(&mutex);
    
  // for every update issued by producer, read the value and add to sum
    Value* value = static_cast<Value*>(arg);
    
    while (true) {
        pthread_mutex_lock(&mutex);
        while (shared::value_status != ValueStatus::NEEDSCONSUMER &&
                shared::status != Status::FINISHED) {
            pthread_cond_wait(&condprod, &mutex);
        }
        
        if (shared::status == Status::FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        shared::result += value->get();
        shared::value_status = ValueStatus::NEEDSPRODUCER;
        pthread_cond_signal(&condcons);
        pthread_mutex_unlock(&mutex);
        
        // После суммирования переменной поток должен заснуть на случайное 
        //количество миллисекунд, верхний предел будет передан на вход приложения.
        int random_delay = rand() % max_delay;
        usleep(random_delay); 
        //std::this_thread::sleep_for(std::chrono::microseconds(random_delay)); //Windows
    }
  // return pointer to result (aggregated result for all consumers)
    return &shared::result;
    
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
    if (shared::status != Status::STARTED) {
        pthread_mutex_lock(&mutex);
    }
    std::vector<pthread_t>* threads = static_cast<std::vector<pthread_t>*>(arg);
    while (shared::status != Status::STARTED) {
        pthread_cond_wait(&condstart, &mutex);
    }
    pthread_mutex_unlock(&mutex);
 
  // interrupt consumer while producer is running      
    while (shared::status != Status::FINISHED) {
        int thread_to_cancel = rand() % threads->size();
        pthread_cancel(threads->at(thread_to_cancel));
        
    }
    return nullptr; 
}
 
int run_threads() {
  // start N threads and wait until they're done
    pthread_cond_init(&condstart, nullptr);
    pthread_cond_init(&condprod, nullptr);
    pthread_cond_init(&condcons, nullptr);
    pthread_mutex_init(&mutex, nullptr);
  
    pthread_t producer;
    pthread_t interruptor;
    std::vector<pthread_t> consumers(consumer_number);
    
    pthread_create(&producer, nullptr, producer_routine, &shared::value);
    
    
    for (auto& cons : consumers) {
        pthread_create(&cons, nullptr, consumer_routine, &shared::value);
    }
 
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers); //bug fix, interuptor should start after consumer threads
    

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
    for (auto& cons : consumers) {
        pthread_join(cons, nullptr);
    }    
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condcons);
    pthread_cond_destroy(&condprod);
    pthread_cond_destroy(&condstart);
      
  // return aggregated sum of values
 
  return shared::result;
}
 
int main(int argc, char* argv[]) {
    consumer_number = std::atoi(argv[1]);
    max_delay = std::atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}
