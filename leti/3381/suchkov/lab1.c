#include <pthread.h>
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <stdlib.h>

int consumersN;
int sleepLimit;

pthread_cond_t t_cond = PTHREAD_COND_INITIALIZER;
volatile bool t_ready = false;

pthread_mutex_t v_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t v_cond = PTHREAD_COND_INITIALIZER;
volatile bool v_ready = false;
volatile bool finish = false;

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
	
    std::vector<int> inputs;

    pthread_mutex_lock(&v_mutex);
    while (!t_ready) {
        pthread_cond_wait(&t_cond, &v_mutex);
    }
    pthread_mutex_unlock(&v_mutex);

	Value *value = reinterpret_cast<Value *>(arg);

    std::string buffer;
    std::getline(std::cin, buffer);
    std::istringstream inp(buffer);

    int input;
    while (inp >> input) {
        inputs.push_back(input);
    }

    for (int i : inputs) {
        // consumers are ready, update the value
        pthread_mutex_lock(&v_mutex);
        value->update(i);
        v_ready = true;
        pthread_cond_broadcast(&v_cond);

        // Wait for consumer to start
        do {
            pthread_cond_wait(&t_cond, &v_mutex);
        } while (!t_ready);

        t_ready = false;
        v_ready = false;
        pthread_mutex_unlock(&v_mutex);
    }

    pthread_mutex_lock(&v_mutex);
    finish = true;
    pthread_cond_broadcast(&v_cond);
    v_ready = true;
    pthread_mutex_unlock(&v_mutex);
    
    pthread_exit(NULL);
}

void* consumer_routine(void* arg) {
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    static int count = 0;
    Value *value = reinterpret_cast<Value *>(arg);

    Value *res = new Value();

    while (1) {
        pthread_mutex_lock(&v_mutex);
        count++;
        if (count >= consumersN) {
            v_ready = false;
            t_ready = true;
            pthread_cond_broadcast(&t_cond);
            count = 0;
        }

        do {
            pthread_cond_wait(&v_cond, &v_mutex);
        } while (!v_ready);

        t_ready = false;

        if (!finish) {
            res->update(value->get() + res->get());
        } else {
            pthread_mutex_unlock(&v_mutex);            
            pthread_exit(reinterpret_cast<void *>(res));
        }

        pthread_mutex_unlock(&v_mutex);

        unsigned sleepTime = 0;
		if (sleepLimit != 0) { sleepTime = rand() % sleepLimit; }
        
		timespec ts;
        ts.tv_sec = (float)sleepTime / 1000.0;
        ts.tv_nsec = 0;
        nanosleep(&ts, NULL);
    }
}

void* consumer_interruptor_routine(void* arg) {
	pthread_t *consumers = reinterpret_cast<pthread_t *>(arg);   
    while (!finish) {
        unsigned i = rand() % consumersN;
        pthread_cancel(consumers[i]);
    }

    pthread_exit(NULL);
}

int run_threads(int consumersNum) {

    Value *value = new Value();
    Value *result;

    consumersN = consumersNum;

    pthread_t producer;
    pthread_create(&producer, NULL, producer_routine, value);
    
    pthread_t *consumers = new pthread_t[consumersNum];
    for (size_t i = 0; i < consumersNum; i++) {
        pthread_create(&consumers[i], NULL, consumer_routine, value);
    }
    
    pthread_t interruptor;
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, consumers);

    for (size_t i = 0; i < consumersNum; i++) {
        pthread_join(consumers[i], reinterpret_cast<void **>(&result));
    }

    pthread_join(producer, NULL);
    pthread_join(interruptor, NULL);

    delete[] consumers;

    return result->get();
}

int main(int argc, const char *argv[]) {
	if (argc != 3) {
        std::cout << "Wrong arguments:\n\t1st argument - number of consumer-threads\n\t2nd argument - sleep limit in ms\n";
        return 1;
    }

    int consumersNum = atoi(argv[1]);
    sleepLimit = atoi(argv[2]);

	std::cout << run_threads(consumersNum) << std::endl;
	return 0;
}
