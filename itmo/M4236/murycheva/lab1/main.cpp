#include <iostream>
#include <string>
#include <pthread.h>
#include <sstream>
#include <fstream>
#include <vector>
#include <time.h>

using namespace std;
class Value;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var_to_cons_signal   = PTHREAD_COND_INITIALIZER;
pthread_cond_t  condition_var_to_prod_signal   = PTHREAD_COND_INITIALIZER;
pthread_cond_t  condition_var_cons_starts      = PTHREAD_COND_INITIALIZER;

static struct timespec time_to_sleep = {0, 0};
static bool is_finished = false;
int num_threads;

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

static Value* accumulator = new Value();
bool is_cons_started = false;

/**
 * wait for consumer to start;
 * read data, loop through each value and update the value, notify consumer, wait for consumer to process
 * @param arg - shared Value
 * @return - nothing
 */
void* producer_routine(void* arg) {

    // input variable
    Value* modifiedValue = (Value *)arg;

    // local variables
    string input;
    int input_in_int;
    vector<int> values;

    // read from input
//    ifstream infile;
//    infile.open("/home/q1/CLionProjects/hpc_hw1/numbers.txt");
//    getline(infile, input);
//    infile.close();

    getline(cin, input);

    istringstream iss( input );

    while(iss >> input_in_int) {
        values.push_back(input_in_int);
    }

    pthread_mutex_lock( &mutex );

    if (!is_cons_started) {
        pthread_cond_wait( &condition_var_cons_starts, &mutex );
    }

    // change global modified Value with values from values
    for (int value: values) {

        modifiedValue->update(value);
        pthread_cond_signal( &condition_var_to_cons_signal );

        pthread_cond_wait( &condition_var_to_prod_signal, &mutex );
    }

    is_finished = true;
    pthread_cond_broadcast(&condition_var_to_cons_signal);

    pthread_mutex_unlock( &mutex );
}


/**
 * notify about start
 * for every update issued by producer, read the value and add to sum
 * @param arg - shared Value
 * @return return pointer to result (aggregated result for all consumers)
 */
void* consumer_routine(void* arg) {
    // save from interrupter
    int last_state;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

    //
    Value* modifiedValue = (Value *) arg;

    pthread_mutex_lock( &mutex );
    is_cons_started = true;
    pthread_cond_signal( &condition_var_cons_starts );

    for (;;) {
        pthread_cond_wait( &condition_var_to_cons_signal, &mutex );

        // all values have ended
        if (is_finished) {
            break;
        }

        accumulator->update(accumulator->get() +
                            modifiedValue->get());

        struct timespec time_res = {0, 0};
        nanosleep( &time_to_sleep, &time_res);

        pthread_cond_signal( &condition_var_to_prod_signal );

    }
    pthread_mutex_unlock( &mutex );

    return new int(accumulator->get());
 }

/*
 * wait for consumer to start
 * interrupt consumer while producer is running
 */
void* consumer_interruptor_routine(void* arg) {
    pthread_t* consumers = (pthread_t *)arg;

    if (!is_cons_started) {
        pthread_cond_wait( &condition_var_cons_starts, &mutex );
    }

    for (;;) {

        pthread_mutex_lock( &mutex );
        // all values have ended
        if (is_finished) {
            break;
        }
        pthread_mutex_unlock( &mutex );

        int cons_to_inter = rand() % num_threads;
        pthread_cancel(consumers[cons_to_inter]);
    }
}

/**
 * start N threads and wait until they're done
 * @return return aggregated sum of values
 */
int run_threads() {
    pthread_t producer;
    pthread_t consumers[num_threads];
    pthread_t interrupter;
    Value* global = new Value();

    int pthr = pthread_create(&producer, NULL, producer_routine, (void *)global);
    if (pthr) {
        cout << "Error: unable to create producer" << endl;
        exit(-1);
    }

    for( int i = 0; i < num_threads; i++ ) {
        int rc = pthread_create(&consumers[i], NULL, consumer_routine, (void *)global);

        if (rc) {
            cout << "Error: unable to create consumer," << rc << endl;
            exit(-1);
        }
    }

    int inter_thread = pthread_create( &interrupter, NULL, consumer_interruptor_routine, (void *)consumers );
    if (inter_thread) {
        cout << "Error: unable to create interrupter" << endl;
        exit(-1);
    }

    pthread_join( producer, NULL );
    int* m = NULL;
    for (int i = 0; i < num_threads; ++i) {
        pthread_join( consumers[i], reinterpret_cast<void **>(&m) );
    }
    pthread_join( interrupter, NULL );

    return *(int *)m;
}

int main (int argc, char* argv[]) {
    if (argc == 3) {
        num_threads = atoi(argv[1]);
        time_to_sleep.tv_nsec= time(NULL) + atoi(argv[2]);
    } else {
        num_threads = 1;
        time_to_sleep.tv_nsec = time(NULL) + 10000;
    }

    std::cout << run_threads() << std::endl;

    return 0;
}