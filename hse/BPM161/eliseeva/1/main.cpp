#include <pthread.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
 
int N;
int max_sleep_time;
 
__thread int sum = 0;
 
bool isFinished = false;
bool isRead = false;
 
pthread_mutex_t mutex;
 
pthread_cond_t written_cond;
pthread_cond_t read_cond;
 
pthread_barrier_t barrier;
 
void* producer_routine(void* arg) {
    pthread_barrier_wait(&barrier);
 
    int* value = (int*)arg;
    std::string input;
    std::getline(std::cin, input);
    std::stringstream sin(input);
    int x;
    while (sin >> x) {
        pthread_mutex_lock(&mutex);
        while (!isRead) {
            pthread_cond_wait(&read_cond, &mutex);
        }
        *value = x;
        isRead = false;
        pthread_cond_signal(&written_cond);
        pthread_mutex_unlock(&mutex);
    }
 
    pthread_mutex_lock(&mutex);
    while (!isRead) {
        pthread_cond_wait(&read_cond, &mutex);
    }
    isFinished = true;
    pthread_cond_broadcast(&written_cond);
    pthread_mutex_unlock(&mutex);
    return nullptr;
}
 
 
void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_barrier_wait(&barrier);
 
    int* value = (int*)arg;
    bool finished = false;;
    while (!finished) {
        pthread_mutex_lock(&mutex);
        while (!isFinished && isRead) {
            pthread_cond_wait(&written_cond, &mutex);
        }
        if (!isFinished) {
            sum += *value;
            isRead = true;
            pthread_cond_signal(&read_cond);
        }
        finished = isFinished;
        pthread_mutex_unlock(&mutex);
 
        if (!finished) {
            int sleep_time = rand() % (max_sleep_time + 1) * 1000;
            if(sleep_time != 0) {
                usleep(sleep_time);
            }
        }
    }
    return &sum;
}
 
void* consumer_interrupter_routine(void* arg) {
    pthread_barrier_wait(&barrier);
    std::vector<pthread_t> consumers = *((std::vector<pthread_t>*) arg);
    bool finished = false;
    while (!finished) {
        pthread_mutex_lock(&mutex);
        if (!isFinished) {
            pthread_cancel(consumers[rand() % consumers.size()]);
        }
        finished = isFinished;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
 
int run_threads() {
    srand(time(nullptr));
    pthread_barrier_init(&barrier, NULL, N + 2);
    int value = 0;
   
 
    pthread_t producer;
    pthread_create(&producer, NULL, producer_routine, &value);
 
    std::vector<pthread_t> consumers(N);
 
    for (int i = 0; i < N; i++) {
        pthread_create(&consumers[i], NULL, consumer_routine, &value);
    }
 
    pthread_t interrupter;
    pthread_create(&interrupter, NULL, consumer_interrupter_routine, &consumers);
 
    int total_sum = 0;
    int* consumer_res;
    for (int i = 0; i < N; i++) {
        pthread_join(consumers[i], (void**) &consumer_res);
        total_sum += *consumer_res;
    }
    pthread_join(interrupter, NULL);
    pthread_join(producer, NULL);
       
    return total_sum;
}
 
int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Pass N and max sleeep time" << std::endl;
        return 0;
    }
    N = std::stoi(argv[1]);
    max_sleep_time = std::stoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}