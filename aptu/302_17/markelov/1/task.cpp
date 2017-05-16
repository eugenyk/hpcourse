#include <pthread.h>
#include <iostream>
#include <string.h>
#include <string>
#include <errno.h>

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

pthread_mutex_t mtx;
pthread_cond_t producer_cnd, consumer_cnd, interruptor_cmd;



enum state {
   NOT_RUNNING,
   RUNNING,
   MSG_SENT,
   FINISHED
};

state producer_state = state::RUNNING;

state consumer_state = state::NOT_RUNNING;

static void wait_consumer_to_start(pthread_cond_t &cnd) {
    pthread_mutex_lock(&mtx);
    while(consumer_state == NOT_RUNNING)
      pthread_cond_wait(&cnd, &mtx);
    pthread_mutex_unlock(&mtx);
}

void* producer_routine(void* arg) {
    Value * value = static_cast<Value*>(arg);
    int x;

    wait_consumer_to_start(producer_cnd);

    while (std::cin >> x) {
        pthread_mutex_lock(&mtx);
        producer_state = MSG_SENT;
        value->update(x);
        pthread_cond_signal(&consumer_cnd);
        while(consumer_state != MSG_SENT)
            pthread_cond_wait(&producer_cnd, &mtx);
        consumer_state = RUNNING;
        pthread_mutex_unlock(&mtx);
    }
    pthread_mutex_lock(&mtx);
    producer_state = FINISHED;
    pthread_cond_signal(&consumer_cnd);
    pthread_cond_signal(&interruptor_cmd);
    pthread_mutex_unlock(&mtx);

    return 0;
  // Wait for consumer to start

  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

}

void* consumer_routine(void* arg) {
    Value * value = static_cast<Value*>(arg);
    int _;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &_);

    pthread_mutex_lock(&mtx);
    consumer_state = RUNNING;
    pthread_cond_signal(&producer_cnd);
    pthread_cond_signal(&interruptor_cmd);
    pthread_mutex_unlock(&mtx);

    Value * sum = new Value();
    bool exit_loop = false;
    while(!exit_loop) {
     pthread_mutex_lock(&mtx);
     if (producer_state == RUNNING)
         pthread_cond_wait(&consumer_cnd, &mtx);
      if (producer_state == FINISHED)
          exit_loop = true;
      else if (producer_state == MSG_SENT){
          sum->update((sum->get() + value->get()));
          consumer_state = MSG_SENT;
          producer_state = RUNNING;
          pthread_cond_signal(&producer_cnd);
      }
      pthread_mutex_unlock(&mtx);
    }

    return sum;
  // notify about start
  // allocate value for result
  // for every update issued by producer, read the value and add to sum
  // return pointer to result
}

void* consumer_interruptor_routine(void* arg) {
    pthread_t * pthread_arg =   static_cast<pthread_t*>(arg);


    wait_consumer_to_start(interruptor_cmd);

    bool exit_loop = false;
    while(!exit_loop) {
        pthread_cancel(*pthread_arg);
        pthread_mutex_lock(&mtx);
        if (producer_state != FINISHED)
            pthread_cond_wait(&interruptor_cmd, &mtx);
        if (producer_state == FINISHED)
          exit_loop = true;
        pthread_mutex_unlock(&mtx);
    }

    return 0;
  // wait for consumer to start

  // interrupt consumer while producer is running
}

int run_threads() {
    Value value;
    Value * ret;
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&producer_cnd, NULL);
    pthread_cond_init(&consumer_cnd, NULL);
    pthread_cond_init(&interruptor_cmd, NULL);

    pthread_t producer, consumer, interraptor;
    if (pthread_create(&producer, NULL, producer_routine, &value) != 0)
        throw std::runtime_error("producer creation fails");

    if (pthread_create(&consumer, NULL, consumer_routine, &value) != 0)
        throw std::runtime_error("consumer creation fails");

    if (pthread_create(&interraptor, NULL, consumer_interruptor_routine, &consumer) != 0)
        throw std::runtime_error("consumer creation fails");

    bool errored = false;
    if (pthread_join(producer, NULL) != 0)
        throw std::runtime_error("producer join failed");
    if (pthread_join(consumer, (void**)&ret) != 0)
        throw std::runtime_error("consumer join failed");
    if (pthread_join(interraptor, NULL) != 0)
        throw std::runtime_error("interraptor join failed");

    int ret_int = ret->get();
    delete ret;
    return ret_int;
}

int main() {
    try {
        std::cout << run_threads() << std::endl;
    } catch(std::exception &e) {
        std::cerr << "Errored " + std::string(e.what()) + " errno = " + std::string(strerror(errno));
    }
    return 0;
}
