#include <pthread.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <unistd.h>

using namespace std;

static  int N;
static  int sleepLimit ;
pthread_mutex_t mutex;
pthread_cond_t cond_c, cond_p;
pthread_barrier_t barrier;
bool done = 0;
int status = 0; // 0 Not started
// 1 Started
// 2 Producer's time to work
// 3 Consumer's time to work
// 4 Finished

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

    if(status == 0) {
        pthread_mutex_lock(&mutex);
        while (status != 1) {
            pthread_cond_wait(&cond_p, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    //read data
    string input;
    cout<<"Enter numbers:\n\t";
    getline(cin,input);
    stringstream stream(input);
    int number;
    while (stream >> number)
    {
        pthread_mutex_lock(&mutex);
        // update the value
        value->update(number);
        // notify consumer
        status = 2;
        pthread_cond_signal(&cond_c);
        // wait time to work
        while (status != 3)
        {
            pthread_cond_wait(&cond_p, &mutex);
        }

        pthread_mutex_unlock(&mutex);
    }

    // notify about the end
    pthread_mutex_lock(&mutex);
    // notify that we finished
    status = 4;
    pthread_cond_broadcast(&cond_c);
    pthread_mutex_unlock(&mutex);

    return nullptr;
}

void* consumer_routine(void* arg) {
//	The task of consumer flows is to respond to each change in the 
//	data variable and type the sum of the values ​​obtained.
	
// temporarily prevent other threads from terminating the thread
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_barrier_wait(&barrier);

    Value* value = (Value*)arg;
    static  int result = 0;

    if(status == 0) {
        pthread_mutex_lock(&mutex);
        status = 1;
        pthread_cond_broadcast(&cond_p);
        pthread_mutex_unlock(&mutex);
    }


    while (status != 4) {
        pthread_mutex_lock(&mutex);
        while (status != 4 && status != 2) {
            pthread_cond_wait(&cond_c, &mutex);
        }
        if(status == 4) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        result += value->get();
        status = 3;
        pthread_cond_broadcast(&cond_p);
        pthread_mutex_unlock(&mutex);
        int time = rand() % sleepLimit;
        usleep(static_cast<__useconds_t>(time * 1000));
    }
    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE,  NULL);
    return &result;
}




void* consumer_interruptor_routine(void* arg) {

    pthread_t* consumer_threads = (pthread_t*)arg;
    int randIdC;

    if(status == 0) {
        //Waiting
        pthread_mutex_lock(&mutex);
        while(status != 1)
        {
            pthread_cond_wait(&cond_p, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    // Try to interrupt consumer while producer is running
    while (status != 4) {
        pthread_mutex_lock(&mutex);
        while (status != 3 && status != 4 ) {
            pthread_cond_wait(&cond_p, &mutex);
        }
        if(status == 4) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        randIdC = rand()%N;

        pthread_cancel(consumer_threads[randIdC]);
        pthread_mutex_unlock(&mutex);
    }

    return nullptr;

}

int run_threads() {
    //Init 3 type of threads
    pthread_t producer_thread;
    pthread_t consumers_thread[N];
    pthread_t interruptor_thread;

    Value val ;
    int *answer ;
    //Init
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_c, NULL);
    pthread_cond_init(&cond_p, NULL);

    //Create threads
    if (pthread_create(&producer_thread, NULL, producer_routine, &val))
    {
        perror("Cannot create producer_thread!");
    }
    pthread_barrier_init(&barrier, NULL, N + 1);
    for (int i = 0; i < N; ++i){
        pthread_create(&consumers_thread[i], NULL, consumer_routine, &val);
    }
    
    pthread_barrier_wait(&barrier);
    pthread_barrier_destroy(&barrier);
    if (pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine, consumers_thread))
    {
        perror("Cannot create interrupter_thread!");
    }

    //Waiting
    pthread_join(producer_thread, NULL);
    pthread_join(interruptor_thread, NULL);
    for (int i = 0; i < N; ++i) {
        pthread_join(consumers_thread[i],(void**)&answer);
    }
    //Destroy
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_c);
    pthread_cond_destroy(&cond_p);
    return *answer;
}

int main(int argc, const char *argv[]) {

    if (argc != 3)
    {
        std::cout << "Wrong arguments:\n\t1st argument - number of consumer-threads\n\t2nd argument - sleep limit in ms\n";
        return 1;
    }
    N = atoi(argv[1]);
    sleepLimit = atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}
