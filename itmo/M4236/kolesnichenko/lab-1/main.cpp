#include <pthread.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <string>
#include <vector>


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


struct producer_arg_t
{
    pthread_mutex_t* mutex_ptr;
    pthread_cond_t* cond_producer_ptr;
    pthread_cond_t* cond_consumer_ptr;
    Value* value_ptr;
    bool* values_left_ptr;
    bool* ready_ptr;
    pthread_t* interruptor_ptr;
};
struct consumer_arg_t
{
    pthread_mutex_t* mutex_ptr;
    pthread_cond_t* cond_producer_ptr;
    pthread_cond_t* cond_consumer_ptr;
    Value* value_ptr;
    bool* values_left_ptr;
    bool* ready_ptr;
    int* consumers_started_ptr;
    int* result_ptr;
};
struct interruptor_arg_t
{
    pthread_mutex_t* mutex_ptr;
    int* consumers_started_ptr;
    std::vector<pthread_t>* consumers_ptr;
};

int N;
int SLEEPSECONDS;



int random_int(int min, int max) {
    return min + (rand() % static_cast<int>(max - min + 1));
}


void* producer_routine(void* arg) {
    producer_arg_t* real_arg = (producer_arg_t*) arg;
    auto& mutex = *real_arg->mutex_ptr;
    auto& cond_producer = *real_arg->cond_producer_ptr;
    auto& cond_consumer = *real_arg->cond_consumer_ptr;
    auto& value = *real_arg->value_ptr;
    auto& values_left = *real_arg->values_left_ptr;
    auto& ready = *real_arg->ready_ptr;
    pthread_t& interruptor = *real_arg->interruptor_ptr;

    std::string inputstring;
    std::getline(std::cin,inputstring);
    std::stringstream ss;
    ss << inputstring;
    std::vector<char> ints;
    int v;
    while (ss >> v) {
        ints.push_back(v);
    }
    for (int i = 0; i < ints.size(); ++i) {
        pthread_mutex_lock(&mutex);
        if (ready) {
            pthread_cond_wait(&cond_producer, &mutex);
        }
        value.update(ints[i]);
        ready = true;
        std::cout << "PRODUCED" << std::endl;
        if (i == ints.size() - 1) {
            pthread_cancel(interruptor);
        }
        pthread_cond_signal(&cond_consumer);
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_lock(&mutex);
    values_left = false;
    if (ready) {
        pthread_cond_wait(&cond_producer, &mutex);
    }
    pthread_cond_broadcast(&cond_consumer);
    pthread_mutex_unlock(&mutex);
}

void* consumer_routine(void* arg) {
    consumer_arg_t* real_arg = (consumer_arg_t*) arg;
    auto& mutex = *real_arg->mutex_ptr;
    auto& cond_producer = *real_arg->cond_producer_ptr;
    auto& cond_consumer = *real_arg->cond_consumer_ptr;
    auto& value = *real_arg->value_ptr;
    auto& values_left = *real_arg->values_left_ptr;
    auto& ready = *real_arg->ready_ptr;
    int& consumers_started = *real_arg->consumers_started_ptr;
    int& result = *real_arg->result_ptr;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    pthread_mutex_lock(&mutex);
    ++consumers_started;
    pthread_mutex_unlock(&mutex);

    while (values_left || ready) {
        pthread_mutex_lock(&mutex);
        if (values_left && !ready) {
            pthread_cond_wait(&cond_consumer, &mutex);
        }
        if (values_left && ! ready) { // spurious wakeup
            pthread_mutex_unlock(&mutex);
            continue;
        }
        if (!values_left && !ready) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        int l = value.get();
        ready = false;
        result += l;
        std::cout << "CONSUMED " << l << std::endl;
        pthread_cond_signal(&cond_producer);
        pthread_mutex_unlock(&mutex);
        sleep(random_int(0, SLEEPSECONDS));
    }
    return arg;
}
 
void* consumer_interruptor_routine(void* arg) {
    interruptor_arg_t* real_arg = (interruptor_arg_t*) arg;
    auto& mutex = *real_arg->mutex_ptr;
    int& consumers_started = *real_arg->consumers_started_ptr;
    std::vector<pthread_t>& consumers = *real_arg->consumers_ptr;

    while(1) {
        pthread_mutex_lock(&mutex);
        if (consumers_started == N) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
    }

    while (1) {    
        int i = random_int(0, N-1);
        pthread_cancel(consumers[i]);
        pthread_testcancel();
    }                                                                                 
}
 
int run_threads() {
    pthread_cond_t cond_consumer = PTHREAD_COND_INITIALIZER;
    pthread_cond_t cond_producer = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
    pthread_t producer;
    pthread_t interruptor;
    std::vector<pthread_t> consumers(N);

    Value value;
    int result = 0; 
    bool ready = false;
    bool values_left = true;
    int consumers_started = 0;
    
    producer_arg_t producer_arg = {&mutex,
                                   &cond_producer,
                                   &cond_consumer,
                                   &value,
                                   &values_left,
                                   &ready,
                                   &interruptor};    
    consumer_arg_t consumer_arg = {&mutex,
                                   &cond_producer,
                                   &cond_consumer,
                                   &value,
                                   &values_left,
                                   &ready,
                                   &consumers_started,
                                   &result};
    interruptor_arg_t interruptor_arg = {&mutex,
                                   &consumers_started,
                                   &consumers};

    pthread_create(&producer, NULL, producer_routine, &producer_arg);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, &interruptor_arg);
    for (auto& consumer : consumers) {
        pthread_create(&consumer, NULL, consumer_routine, &consumer_arg);
    }

    pthread_join(producer, NULL);
    pthread_join(interruptor, NULL);
    for (auto& consumer : consumers) {
        pthread_join(consumer, NULL);
    }

    pthread_mutex_destroy(&mutex); /* Free up the_mutex */
    pthread_cond_destroy(&cond_consumer); /* Free up consumer condition variable */
    pthread_cond_destroy(&cond_producer); /* Free up producer condition variable */

    return result;
}
 
int main(int argc, char* argv[]) {
    srand(time(0));

    if (argc != 3) {
        std::cout << "got " << argc - 1 << " arguments, need 2" << std::endl;
        return 0;
    }
    std::stringstream ss;
    ss << argv[1];
    ss >> N;
    ss.clear();
    ss << argv[2];
    ss >> SLEEPSECONDS; 
    
    int sum = run_threads();
    std::cout << sum << std::endl;
    return 0;
}

