#include <vector>
#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <fstream>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

bool consumer_start = false;
bool next = false;
bool prod_not_finished = true;

std::vector<int>::size_type size;
std::vector<int> v;

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



void *producer_routine(void *arg) {

    pthread_mutex_lock(&mutex);
    if (!consumer_start){
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    Value *temp_data = (Value *) arg;
    // Read data, loop through each value
    for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it) {
        pthread_mutex_lock(&mutex);
        // Wait for consumer to start,wait for consumer to process
        while (!next)
            pthread_cond_wait(&cond, &mutex);
        //update the value
        temp_data->update(*it+1);
        next = false;
        // notify consumer
        pthread_cond_signal(&cond);

        if (it == std::prev(v.end(), 1)) {
            prod_not_finished = false;
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer_routine(void *arg) {

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    pthread_mutex_lock(&mutex);
    consumer_start = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    // allocate value for result
    int *sum = new int(0);
    Value *temp_data = (Value *) arg;

    for (std::vector<int>::size_type i = 0; i < size; ++i) {

        pthread_mutex_lock(&mutex);
        next = true;
        // for every update issued by producer
        // read the value and add to sum
        *sum += temp_data->get();
        // notify about start;
        pthread_cond_signal(&cond);

        while (next)
            pthread_cond_wait(&cond, &mutex);

        pthread_mutex_unlock(&mutex);
    }
    // return pointer to result
    return (void *) sum;
}

void *consumer_interruptor_routine(void *arg) {

    pthread_t* consumer_id = (pthread_t*) arg;

    // wait for consumer to start
    pthread_mutex_lock(&mutex);

    while (!consumer_start)
        pthread_cond_wait(&cond, &mutex);

    pthread_mutex_unlock(&mutex);

    // interrupt consumer while producer is running
    while (prod_not_finished) {
        //try to cancel consumer thread
        pthread_cancel(*consumer_id);
    }
}
int run_threads(char * path) {
    std::ifstream fin(path);
    int input_value;
    while (fin >> input_value) {
      v.push_back(input_value);
    }
    fin.close();

    size = v.size();

    void *result;
    pthread_t producer_id;
    pthread_t consumer_id;
    pthread_t interrupter_id;
    Value data;

    if (pthread_mutex_init(&mutex, NULL) != 0)
        printf("mutex init failed\n");

    if (pthread_create(&producer_id, NULL, &producer_routine, &data) != 0) {
        printf("Error creating producer\n");
        return -1;
    }
    if (pthread_create(&consumer_id, NULL, &consumer_routine, &data) != 0) {
        printf("Error creating consumer\n");
        return -1;
    }

    if (pthread_create(&interrupter_id, NULL, &consumer_interruptor_routine, &consumer_id) != 0) {
        printf("Error creating consumer\n");
        return -1;
    }

    // start 3 threads and wait until they're done
    pthread_join(producer_id, NULL);
    pthread_join(consumer_id, &result);
    pthread_join(interrupter_id, NULL);
    pthread_mutex_destroy(&mutex);
    // return sum of update values seen by consumer
    return *(int *) result;
}

int main(int argc, char *argv[]) {
    char * path;
    if (argc != 2) {
      std::cout << "You should type only one path to file with numbers as a parameter" << '\n';
      return 0;
    } else {
      path = argv[1];
    }

    std::cout << run_threads(path) << std::endl;

    return 0;
}
