#include <iostream>
#include <cstring>
#include <pthread.h>

using namespace std;

pthread_mutex_t mutex, mutex_end;
pthread_cond_t condition_next_update, condition_next_sum, condition_start_consumer;
pthread_t _ThreadIdConsumer, _ThreadIdProducer, _ThreadIdInterruptor;
bool end_consumer;

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

Value* value = new Value();


void* producer_routine(void* arg) {
  // Wait for consumer to start
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    Value* value = (Value*) arg;


    string str;
    std::getline(std::cin, str);

    char *cstr = new char[str.length() + 1];
    std::strcpy(cstr, str.c_str());

    char *pch = strtok(cstr ," "); // во втором параметре указаны разделитель (пробел, запятая, точка, тире)

    while(pch != NULL) {
        pthread_mutex_lock(&mutex);
            value->update(atoi(pch));
            pthread_cond_signal(&condition_next_sum);
            pthread_cond_wait(&condition_next_update, &mutex);
            pch = strtok (NULL, " ");
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);
        value->update(NULL);
        pthread_cond_signal(&condition_next_sum);
    pthread_mutex_unlock(&mutex);

    return 0;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cond_broadcast(&condition_start_consumer);

    Value* value = (Value*) arg;
    int sum = 0;

    pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condition_next_sum, &mutex);
        sum += value->get();
        pthread_cond_signal(&condition_next_update);
    pthread_mutex_unlock(&mutex);

    while (true) {
        pthread_mutex_lock(&mutex);
            pthread_cond_wait(&condition_next_sum, &mutex);
            if (value->get() == NULL) {
                pthread_mutex_unlock(&mutex);
                break;
            }
            sum += value->get();
            pthread_cond_signal(&condition_next_update);
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex_end);
        end_consumer = false;
    pthread_mutex_unlock(&mutex_end);

    pthread_exit((void*)sum);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
}

void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
  // interrupt consumer while producer is running
    pthread_mutex_lock(&mutex_end);
        pthread_cond_wait(&condition_start_consumer, &mutex_end);
        end_consumer = true;
    pthread_mutex_unlock(&mutex_end);

    while (true) {
        pthread_mutex_lock(&mutex_end);
            if (!end_consumer) break;
        pthread_mutex_unlock(&mutex_end);
        pthread_cancel(_ThreadIdConsumer);
    }


    return 0;
}

int run_threads() {
  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer
    void* sum = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_end, NULL);

    pthread_cond_init(&condition_next_sum, NULL);
    pthread_cond_init(&condition_next_update, NULL);
    pthread_cond_init(&condition_start_consumer, NULL);

    pthread_create( &_ThreadIdProducer, NULL,
                               producer_routine, (void*)value);
    pthread_create( &_ThreadIdConsumer, NULL,
                               consumer_routine, (void*)value);
    pthread_create( &_ThreadIdInterruptor, NULL,
                               consumer_interruptor_routine, NULL);

    pthread_join(_ThreadIdProducer, NULL);
    pthread_join(_ThreadIdConsumer, &sum);
    pthread_join(_ThreadIdInterruptor, NULL);
    return (int) sum;
}

int main(int argc, char *argv[])
{
    std::cout << run_threads() << std::endl;
    return 0;
}
