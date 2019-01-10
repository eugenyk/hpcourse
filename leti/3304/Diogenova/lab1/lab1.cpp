#include <iostream>
#include <pthread.h>
#include <sstream>
#include <unistd.h>
#include <cmath>

pthread_mutex_t get_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t update_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t read_cond = PTHREAD_COND_INITIALIZER;

bool toggle = true;
int time_to_sleep = 0;
int N;
bool end_of_read = false;

int all_sum = 0;

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

void* producer_routine(void* arg) {

    Value* var = reinterpret_cast<Value*>(arg);
    std::string input_line;
    std::getline(std::cin, input_line);
    int input_value = 0;
    std::istringstream is(input_line);

    while (is >> input_value) {
        pthread_mutex_lock(&get_mut);
        while (!toggle) {
            pthread_cond_wait(&read_cond, &get_mut);
        }
        var->update(input_value);
        toggle = !toggle;
        pthread_cond_signal(&update_cond);
        pthread_mutex_unlock(&get_mut);
    }
    pthread_mutex_lock(&get_mut);
    while (!toggle) {
        pthread_cond_wait(&read_cond, &get_mut);
    }
    var->update(0);
    toggle = !toggle;
    end_of_read = true;
    pthread_cond_broadcast(&update_cond);
    pthread_mutex_unlock(&get_mut);
    pthread_exit(NULL);
}

void add_val(Value* var) {
    pthread_mutex_lock(&get_mut);
    while (toggle) {
        pthread_cond_wait(&update_cond, &get_mut);
    }

    all_sum += var->get();
    pthread_cond_broadcast(&read_cond);

    if (!end_of_read) {
        toggle = !toggle;
    }

    pthread_mutex_unlock(&get_mut);
}

void* consumer_routine(void* arg) {

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    Value* var = reinterpret_cast<Value*>(arg);

    while (!end_of_read) {
        add_val(var);

        int sleep = time_to_sleep ? rand() % time_to_sleep : 0;
        usleep(sleep);
    }
    pthread_exit((void *)all_sum);
}

void* consumer_interruptor_routine(void* arg) {

    pthread_t *consumers = reinterpret_cast<pthread_t*>(arg);
    int new_rand;
    while (!end_of_read) {
        double r = ((double) rand() / (RAND_MAX));
        new_rand = ceil(r * (N-1));
        pthread_cancel(consumers[new_rand]);
    }
    pthread_exit(NULL);
}

int run_threads() {


    Value*  variable = new Value();

    pthread_t consumers[N];
    pthread_t producer;
    pthread_t interruptor;

    for (int i = 0; i < N; ++i) {
        pthread_create(&consumers[i], NULL, consumer_routine, variable);
    }

    pthread_create(&interruptor, NULL, consumer_interruptor_routine, consumers);
    pthread_create(&producer, NULL, producer_routine, variable);

    pthread_join(producer, NULL);
    pthread_join(interruptor, NULL);

    void* res;

    for (int i = 0; i < N; ++i) {
        pthread_join(consumers[i], &res);
    }
    delete(variable);

    return (long) res;
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Must 2 cmd-line arguments." << std::endl;
        return -1;
    }

    N = atoi(argv[1]);
    time_to_sleep = atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}