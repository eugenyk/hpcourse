#include <iostream>
#include <vector>
#include <sstream>
#include <pthread.h>

class Value {
private:
    int value = 0;
    volatile bool flag_value = false;
    volatile bool flag_processed = false;
    volatile bool flag_finished = false;

    pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t value_cond = PTHREAD_COND_INITIALIZER;

    pthread_mutex_t process_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t process_cond = PTHREAD_COND_INITIALIZER;

public:
    Value() = default;

    ~Value() 
    {
        pthread_mutex_destroy(&value_mutex);
        pthread_cond_destroy(&value_cond);

        pthread_mutex_destroy(&process_mutex);
        pthread_cond_destroy(&process_cond);
    }

    void set_value(int x) 
    {
        pthread_mutex_lock(&value_mutex);

        value = x;
        flag_value = true;

        pthread_cond_signal(&value_cond);
        pthread_mutex_unlock(&value_mutex);
    }

    int get_value() 
    {
        int res;

        pthread_mutex_lock(&value_mutex);
        while (!flag_finished && !flag_value) {
            pthread_cond_wait(&value_cond, &value_mutex);
        }
        if (flag_finished)
        {
            pthread_mutex_unlock(&value_mutex);
            throw -1;
        }
        res = value;
        flag_value = false;
        pthread_mutex_unlock(&value_mutex);

        return res;
    }

    void wait_processed()
    {
        pthread_mutex_lock(&process_mutex);
        while (!flag_processed) {
            pthread_cond_wait(&process_cond, &process_mutex);
        }
        flag_processed = false;
        pthread_mutex_unlock(&process_mutex);
    }

    void set_processed()
    {
        pthread_mutex_lock(&process_mutex);
        flag_processed = true;
        pthread_cond_signal(&process_cond);
        pthread_mutex_unlock(&process_mutex);
    }

    bool is_finished()
    {
        return flag_finished;
    }

    void set_finished()
    {
        pthread_mutex_lock(&value_mutex);
        flag_finished = true;
        pthread_cond_broadcast(&value_cond);
        pthread_cond_broadcast(&value_cond);
        pthread_mutex_unlock(&value_mutex);
    }
};

size_t consumers_count;
unsigned int max_sleep_time;
std::vector<pthread_t> consumers;

pthread_barrier_t start_barrier;

__thread int sum = 0;

std::vector<int> read_input_sequense()
{
    std::vector<int> sequense;

    std::string line;
    std::getline(std::cin, line);
    std::istringstream stream(line);
    
    int x;
    while (stream >> x) {
        sequense.push_back(x);
    }

    return sequense;
}

void* producer_routine(void* arg)
{
    pthread_barrier_wait(&start_barrier);

    std::vector<int> sequense = read_input_sequense();
    Value* value = static_cast<Value*>(arg);
    for (int x : sequense) {
        value->set_value(x);
        value->wait_processed();
    }

    value->set_finished();

    return nullptr;
}

void sleep_random_time() 
{
    long t = (rand() % (max_sleep_time + 1)) * 1000000;
    timespec ts = {0, t};
    nanosleep(&ts, nullptr);
}

void* consumer_routine(void* arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&start_barrier);

    Value* value = static_cast<Value*>(arg);
    while(!value->is_finished()) {
        try {
            sum += value->get_value();
        } catch (...) {
            break;
        }
        value->set_processed();

        sleep_random_time();
    }

    return &sum;
}

void* consumer_interruptor_routine(void* arg)
{
    pthread_barrier_wait(&start_barrier);

    Value* value = static_cast<Value*>(arg);
    while(!value->is_finished()) {
        pthread_cancel(consumers[rand() % consumers_count]);
    }

    return nullptr;
}

int run_threads()
{
    pthread_barrier_init(&start_barrier, nullptr, consumers_count + 2);

    Value value;

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, &value);

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &value);

    consumers.resize(consumers_count);
    for (auto& consumer : consumers) {
        pthread_create(&consumer, nullptr, consumer_routine, &value);
    }

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    int total_sum = 0;
    for (auto& consumer : consumers) {
        int* res = nullptr;
        pthread_join(consumer, (void**)(&res));
        total_sum += *res;
    }

    return total_sum;
}

void read_input(int argc, char** argv)
{
    consumers_count = static_cast<size_t>(std::stoi(argv[1]));
    max_sleep_time = static_cast<unsigned int>(std::stoi(argv[2]));
}

int main(int argc, char** argv)
{
    srand(static_cast<unsigned int>(time(nullptr)));

    read_input(argc, argv);

    std::cout << run_threads() << std::endl;

    return 0;
}