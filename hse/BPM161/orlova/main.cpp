#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;
pthread_cond_t condition_value_change;
pthread_cond_t condition_value_change_finish;
pthread_mutex_t condition_value_change_mutext;
pthread_barrier_t threads_start_barrier;
int value;
bool is_finish_reading = false;

vector<pthread_t> threads;

void* producer_routine(void* arg) {
    string str;
    pthread_barrier_wait(&threads_start_barrier);
    getline(cin, str);
    stringstream stream;
    stream << str;
    int i;
    while (stream >> i) {
        pthread_mutex_unlock(&condition_value_change_mutext);
        pthread_mutex_lock(&condition_value_change_mutext);
        *((int*)arg) = i;
        pthread_cond_signal(&condition_value_change);
        pthread_cond_wait(&condition_value_change_finish, &condition_value_change_mutext);
    }
    is_finish_reading = true;
    pthread_mutex_unlock(&condition_value_change_mutext);
    pthread_cond_broadcast(&condition_value_change);
    pthread_exit(NULL);
}

thread_local int part_sum = 0;
void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_barrier_wait(&threads_start_barrier);
    pthread_mutex_lock(&condition_value_change_mutext);
    while (true) {
        pthread_cond_wait(&condition_value_change, &condition_value_change_mutext);
        if (is_finish_reading) {
            pthread_mutex_unlock(&condition_value_change_mutext);
            pthread_exit((void*)part_sum);
        } else {
            part_sum += *((int*)arg);
            *((int*)arg) = 0;
        }
        pthread_cond_signal(&condition_value_change_finish);
    }
}

void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&threads_start_barrier);
    while (true) {
        int index = rand() % (*(int*)arg);
        pthread_cancel(threads[index]);
        if (is_finish_reading) {
            pthread_exit(NULL);
        }
    }
}

int run_threads(int consumers_number) {
    vector<int> res;

    threads.resize(consumers_number);
    res.resize(consumers_number);

    pthread_mutex_init(&condition_value_change_mutext, NULL);
    pthread_cond_init(&condition_value_change, NULL);
    pthread_cond_init(&condition_value_change_finish, NULL);
    pthread_barrier_init(&threads_start_barrier, NULL, consumers_number + 2);

    pthread_t producer;
    pthread_t interruptor;

    pthread_create( &producer, NULL, producer_routine, &value);
    pthread_create( &interruptor, NULL, consumer_interruptor_routine, &consumers_number);
    for (int i = 0; i < consumers_number; i++) {
        pthread_create(&(threads[i]), NULL, consumer_routine, &value);
    }

    for (int i = 0; i < consumers_number; i++) {
        pthread_join(threads[i], (void**)&(res[i]));
    }

    pthread_mutex_destroy(&condition_value_change_mutext);
    pthread_cond_destroy(&condition_value_change);
    pthread_cond_destroy(&condition_value_change_finish);
    pthread_barrier_destroy(&threads_start_barrier);

    int sum = 0;
    for (auto el: res) {
        sum += el;
    }
    return sum;
}

int main(int argc, char **argv) {
    int N = stoi(argv[1]);
    cout << run_threads(N) << endl;
    return 0;
}