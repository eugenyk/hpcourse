#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include <random>

 using namespace std;

enum Status {
    BEFORE_START, START, PRODUCER_IS_READY, CONSUMER_IS_READY, FINISHED
};

int num_of_consumers = 1;
int max_sleep_time = 1000;

default_random_engine generator;
uniform_int_distribution<int> distribution(1,10000);

Status status = BEFORE_START;
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

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

class ConsumerArg {
public:
    ConsumerArg(Value* value, int number) { _value = value; _number = number; }
    void update(Value* value, int number) {
        _value = value; _number = number;
    }
    ConsumerArg() {_value = 0; _number = 0;}
     Value* getValue() const {
        return _value;
    }

    int getNumber() const {
        return _number;
    }

 private:
    Value* _value;
    int _number;
};
 void* producer_routine(void* arg) {
    cout << "Producer started" << endl;
    if(status == BEFORE_START) {
        pthread_mutex_lock(&my_mutex);
        while (status != START) {
            pthread_cond_wait(&producer_cond, &my_mutex);
        }
        pthread_mutex_unlock(&my_mutex);
    }
    cout << "Producer waited when consumers is started" << endl;

    Value* value = static_cast<Value *>(arg);
    int num;
    cout << "Enter number" << endl;
    while (cin >> num) {
        cout << "Producer read number=" + to_string(num) << endl;

        pthread_mutex_lock(&my_mutex);
        value->update(num);
        status = PRODUCER_IS_READY;
        cout << "Producer updated value" << endl;
        pthread_cond_broadcast(&consumer_cond);

        cout << "Producer is waiting consumer" << endl;
        while (status != CONSUMER_IS_READY) {
            pthread_cond_wait(&producer_cond, &my_mutex);
        }
        pthread_mutex_unlock(&my_mutex);
        cout << "Producer waited consumer" << endl;
        cout << "Enter number" << endl;
    }
    cout << "Producer finished" << endl;
    status = FINISHED;
    pthread_mutex_lock(&my_mutex);
    pthread_cond_broadcast(&consumer_cond);
    pthread_mutex_unlock(&my_mutex);

    return nullptr;
}
 void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    ConsumerArg* consumerArg = static_cast<ConsumerArg *>(arg);

    Value* value = consumerArg->getValue();
    int number = consumerArg->getNumber();
    cout << "Consumer №" + to_string(number) + " started" << endl;

    if(status == BEFORE_START) {
        pthread_mutex_lock(&my_mutex);
        status = START;
        pthread_cond_broadcast(&producer_cond);
        pthread_mutex_unlock(&my_mutex);
    }

    int sum = 0;
    while (status != FINISHED) {
        pthread_mutex_lock(&my_mutex);
        cout << "Consumer №" + to_string(number) + " is waiting producer or finish" << endl;
        while (status != PRODUCER_IS_READY && status != FINISHED) {
            pthread_cond_wait(&consumer_cond, &my_mutex);
        }
        if(status == PRODUCER_IS_READY) {
            cout << "Consumer №" + to_string(number) + " waited producer" << endl;
            int v = value->get();
            sum += v;
            status = CONSUMER_IS_READY;
            cout << "Consumer №" + to_string(number) + " toted " + to_string(v) + " sum=" + to_string(sum) << endl;
            pthread_cond_broadcast(&producer_cond);
            pthread_mutex_unlock(&my_mutex);
            //usleep(static_cast<useconds_t>((distribution(generator) % max_sleep_time) * 1000));
        } else {
            pthread_mutex_unlock(&my_mutex);   
        }
    }
    cout << "Consumer №" + to_string(number) + " finished" << endl;
    return new int(sum);
}
void* consumer_interruptor_routine(void* arg) {
    auto consumers = static_cast<const std::vector<pthread_t> *>(arg);
    std::cout << "Start interruptor" << std::endl;
    if(status == BEFORE_START) {
        pthread_mutex_lock(&my_mutex);
        while (status != START) {
            pthread_cond_wait(&producer_cond, &my_mutex);
        }
        pthread_mutex_unlock(&my_mutex);
    }
    std::cout << "Interruptor waited started producer" << std::endl;
    while (status != FINISHED) {
        pthread_mutex_lock(&my_mutex);
        while (status != CONSUMER_IS_READY && status != FINISHED ) {
            pthread_cond_wait(&producer_cond, &my_mutex);
        }
        if(status == FINISHED) {
            pthread_mutex_unlock(&my_mutex);
            break;
        }
        auto i = static_cast<size_t>(distribution(generator) % num_of_consumers);
        pthread_cancel(reinterpret_cast<pthread_t>(consumers->at(i)));
        pthread_mutex_unlock(&my_mutex);
    }
    std::cout << "Interruptor finished" << std::endl;
    return nullptr;
}
 int run_threads() {
    Value * value = new Value();

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, value);
    cout << "Createed producer" << endl;
    ConsumerArg* args = new ConsumerArg[num_of_consumers];
    pthread_t* consumers = new pthread_t[num_of_consumers];
    for (int i = 0; i<num_of_consumers; i++) {
        ConsumerArg arg = args[i];
        arg.update(value, i);
        pthread_create(&consumers[i], nullptr, consumer_routine, &arg);
        cout << "Created cosumer №" + to_string(i) << endl;
    }
    cout << "Createed " + to_string(num_of_consumers) + " consumers" << endl;

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);
    cout << "Created interruptor" << endl;

    pthread_join(producer, nullptr);
    cout << "Joined producer" << endl;
    pthread_join(interruptor, nullptr);
    cout << "Joined interruptor" << endl;

    int res = 0;
    for (int i = 0; i<num_of_consumers; i++) {
        int* v;
        pthread_join(consumers[i], reinterpret_cast<void**>(&v));
        cout << "Joined consumer №" + to_string(i) << endl;
        res += (*v);
        delete v;
    }

    delete[] args;
    delete[] consumers;
    return res;
}
 int main(int argc, char* argv[]) {
     if(argc < 3)
     {
        cout << "Not enough input arguments: you must enter the number of consumer threads and the upper sleep limit of the consumer in milliseconds." << endl;
        return 1;
     } else {
         try {
            num_of_consumers = stoi(argv[1]);
            max_sleep_time = stoi(argv[2]);
         } catch(exception e){
             cout << "Incorrect arguments:" + string(e.what()) << endl;
             return 1;
         }
     }
    std::cout << run_threads() << std::endl;
    pthread_cond_destroy(&consumer_cond);
    pthread_cond_destroy(&producer_cond);
    pthread_mutex_destroy(&my_mutex);
    return 0;
}