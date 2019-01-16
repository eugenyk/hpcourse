#include <pthread.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <unistd.h>

using namespace std;

static  int N;
static  int sleepLimit ;
pthread_barrier_t barrier_wait, barrier_new_value, barrier_update_value;
bool done = false;

class Value {
public:
    Value() : _value(0) {}

    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }
private:
    int _value;
};


void* producer_routine(void* arg) {
//The producer - flow task is to get a list of numbers as input,
//and use each value in turn in turn to update a variable shared between threads.
    // read data
    int data_val;
    Value* value = (Value*)arg;

    pthread_barrier_wait(&barrier_wait);

    //read data
    string input;
    cout<<"Enter numbers:\n\t";
    getline(cin,input);
    stringstream stream(input);
    int number;
    while (stream >> number)
    {
        // update the value
        value->update(number);
        // notify consumer
        pthread_barrier_wait(&barrier_new_value);
        pthread_barrier_wait(&barrier_update_value);
    }
    // notify that we finished
    done = true;
    pthread_barrier_wait(&barrier_new_value);
    return nullptr;
}

void* consumer_routine(void* arg) {
//	The task of consumer flows is to respond to each change in the
//	data variable and type the sum of the values ​​obtained.

// temporarily prevent other threads from terminating the thread
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    pthread_barrier_wait(&barrier_wait);

    Value* value = (Value*)arg;
    int* result = new int;
    while (true) {
        pthread_barrier_wait(&barrier_new_value);
        if(done) {
            break;
        }
        *result += value->get();
        int time = rand() % sleepLimit;
        usleep(static_cast<__useconds_t>(time * 1000));
        pthread_barrier_wait(&barrier_update_value);
    }
    //pthread_setcancelstate (PTHREAD_CANCEL_ENABLE,  NULL);
    return (void *)result;
}




void* consumer_interruptor_routine(void* arg) {

    pthread_t* consumer_threads = (pthread_t*)arg;
    int randIdC;

    pthread_barrier_wait(&barrier_wait);

    // Try to interrupt consumer while producer is running
    while (!done) {
        randIdC = rand()%N;
        pthread_cancel(consumer_threads[randIdC]);
    }

    return nullptr;

}

int run_threads() {
    //Init 3 type of threads
    pthread_t producer_thread;
    pthread_t consumers_thread[N];
    pthread_t interruptor_thread;

    Value val ;
    void* answer ;
    //Init
    pthread_barrier_init(&barrier_wait, nullptr, N + 2);
    pthread_barrier_init(&barrier_new_value, nullptr, N + 1);
    pthread_barrier_init(&barrier_update_value, nullptr, N + 1);


        //Create threads
    if (pthread_create(&producer_thread, NULL, producer_routine, &val))
    {
        perror("Cannot create producer_thread!");
    }
    for (int i = 0; i < N; ++i){
        pthread_create(&consumers_thread[i], NULL, consumer_routine, &val);
    }

    if (pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine, consumers_thread))
    {
        perror("Cannot create interrupter_thread!");
    }

    //Waiting
    pthread_join(producer_thread, NULL);
    pthread_join(interruptor_thread, NULL);
    for (int i = 0; i < N; ++i) {
        pthread_join(consumers_thread[i], &answer);
    }
    //Destroy
    pthread_barrier_destroy(&barrier_wait);
    pthread_barrier_destroy(&barrier_new_value);
    pthread_barrier_destroy(&barrier_update_value);
    return *(int*)answer;
}

int main(int argc, const char *argv[]) {

    if (argc != 3)
    {
        std::cout << "Wrong arguments:\n\t1st argument - number of consumer-threads\n\t2nd argument - sleep limit in ms\n";
        return 1;
    }
    N = atoi(argv[1]);
    sleepLimit = atoi(argv[2]);
    if (sleepLimit <= 0){
        sleepLimit = 1;
    }
    std::cout << run_threads() << std::endl;
    return 0;
}
