#include <iostream>
#include <pthread.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>

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

 bool isEnd = false;

pthread_barrier_t barrierStart;
pthread_barrier_t barrierReadyValue;
pthread_barrier_t barrierUpdateValue;

unsigned int sleepLimit;
unsigned long countConsumers;

void* producer_routine(void* arg) {
    Value* value = reinterpret_cast<Value*>(arg);
    // Wait for consumer to start

    pthread_barrier_wait(&barrierStart);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    //Read data
    string input, tmp;
    cout << "Enter numbers:\n\t";
    getline(cin, input);
    stringstream stream(input);
    //loop through each value and update the value
    int number;
    while (true) {
        if (!(stream >> number)) {
            isEnd = true;
            pthread_barrier_wait(&barrierReadyValue);
            break;
        }
        value->update(number);
        //notify consumer
        pthread_barrier_wait(&barrierReadyValue);
        //wait for consumer to process
        pthread_barrier_wait(&barrierUpdateValue);
    }
    return nullptr;

}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    Value* value = reinterpret_cast<Value*>(arg);

    pthread_barrier_wait(&barrierStart);

    Value * sum = new Value();

    while (true){
        pthread_barrier_wait(&barrierReadyValue);
        if (isEnd) break;
        sum->update(sum->get() + value->get());
        usleep((rand() % (sleepLimit + 1))*1000);
        pthread_barrier_wait(&barrierUpdateValue);

    }
    // return pointer to result (aggregated result for all consumers)
    return reinterpret_cast<void*>(sum);
}

void* consumer_interruptor_routine(void* arg) {
    vector<pthread_t>* consumers = reinterpret_cast<vector<pthread_t>*>(arg);

    pthread_barrier_wait(&barrierStart);

    while(!isEnd){
        unsigned long i = rand() % (consumers->size());
        pthread_cancel((consumers->at(i)));
    }
    return nullptr;
}

int run_threads() {

    int status = pthread_barrier_init(&barrierStart, nullptr, countConsumers + 2);
    if (status != 0) {
        printf("run_threads error: can't init barrier, status = %d\n", status);
        exit(-1);
    }
    status = pthread_barrier_init(&barrierReadyValue, nullptr, countConsumers + 1);
    if (status != 0) {
        printf("run_threads error: can't init barrier, status = %d\n", status);
        exit(-1);
    }
    status = pthread_barrier_init(&barrierUpdateValue, nullptr, countConsumers + 1);
    if (status != 0) {
        printf("run_threads error: can't init barrier, status = %d\n", status);
        exit(-1);
    }

    // start N threads and wait until they're done
    Value* value = new Value();

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, value);

    vector<pthread_t> consumers(countConsumers);
    for (int i = 0; i < countConsumers; i++)
        pthread_create(&consumers[i], nullptr, consumer_routine, value);

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
    void* sum;
    pthread_join(consumers[0], &sum);
    for (int i = 1; i < countConsumers; i++)
        pthread_join(consumers[i], nullptr);
    // return aggregated sum of values
    pthread_barrier_destroy(&barrierStart);
    pthread_barrier_destroy(&barrierReadyValue);
    pthread_barrier_destroy(&barrierUpdateValue);
    return (reinterpret_cast<Value*>(sum))->get();
}

int main(int argc, const char *argv[]) {
    srand(time(NULL));
    if (argc != 3) {
        cout << "Parameter error:\n\t1st argument - number of consumer-thread\n\t2st argument - max sleep limit\n";
        return -1;
    }
    countConsumers = atoi(argv[1]);
    sleepLimit = atoi(argv[2]);
    std::cout << "Result:\n\t" + std::to_string(run_threads()) << std::endl;
    return 0;
}
