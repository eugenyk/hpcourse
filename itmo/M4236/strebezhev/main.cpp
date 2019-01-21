#include <pthread.h>
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <cassert>
#include <ctime>
#include <cstdlib>

using namespace std;

int CONSUMERS_COUNT;
int SLEEP_MILLISECONDS;

pthread_mutex_t mutex;
pthread_cond_t is_generated_cond, is_consumed_cond;
pthread_barrier_t barrier;

int total_sum;
bool is_produced, producer_is_active = true;


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


void* producer_routine(Value* value) {
  string input;
  getline(cin, input);
  stringstream numbers(input);

  pthread_barrier_wait(&barrier);

  int num;
  while (numbers >> num) {
    pthread_mutex_lock(&mutex);
    {
      value->update(num);

      is_produced = true;
      pthread_cond_broadcast(&is_generated_cond);

      while (is_produced)
        pthread_cond_wait(&is_consumed_cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
  }

  producer_is_active = false;

  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&is_generated_cond);
  pthread_mutex_unlock(&mutex);

  return nullptr;
}

void* consumer_routine(Value* value) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
  pthread_barrier_wait(&barrier);

  while (producer_is_active) {
    pthread_mutex_lock(&mutex);
    {
      while (producer_is_active && !is_produced)
        pthread_cond_wait(&is_generated_cond, &mutex);

      if (!producer_is_active) {
        pthread_mutex_unlock(&mutex);
        break;
      }

      total_sum += value->get();
      is_produced = false;
      pthread_cond_signal(&is_consumed_cond);
    }
    pthread_mutex_unlock(&mutex);

    usleep(rand() % SLEEP_MILLISECONDS);
  }

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
  return &total_sum;
}

void* consumer_interruptor_routine(pthread_t* consumers) {
  pthread_barrier_wait(&barrier);

  while (producer_is_active)
    pthread_cancel(consumers[rand() % CONSUMERS_COUNT]);

  return nullptr;
}

int run_threads() {
  Value value;
  int* sum = nullptr;

  pthread_mutex_init(&mutex, nullptr);
  pthread_cond_init(&is_consumed_cond, nullptr);
  pthread_cond_init(&is_generated_cond, nullptr);
  pthread_barrier_init(&barrier, nullptr, 1 + CONSUMERS_COUNT + 1);


  pthread_t producer;
  pthread_create(&producer, nullptr, (void* (*)(void*)) producer_routine, &value);

  pthread_t consumers[CONSUMERS_COUNT];
  for (pthread_t &thread : consumers)
    pthread_create(&thread, nullptr, (void* (*)(void*)) consumer_routine, &value);

  pthread_t interrupter;
  pthread_create(&interrupter, nullptr, (void* (*)(void*)) consumer_interruptor_routine, consumers);

  pthread_join(producer, nullptr);

  for (pthread_t &thread : consumers)
    pthread_join(thread, (void**) (&sum));

  pthread_join(interrupter, nullptr);

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&is_consumed_cond);
  pthread_cond_destroy(&is_generated_cond);

  return *sum;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Two arguments are expected" << std::endl;
    exit(1);
  }

  CONSUMERS_COUNT = stoi(argv[1]);
  SLEEP_MILLISECONDS = stoi(argv[2]);
  assert(CONSUMERS_COUNT > 0 && SLEEP_MILLISECONDS > 0);

  srand(time(nullptr));
  std::cout << run_threads() << std::endl;
  return 0;
}
