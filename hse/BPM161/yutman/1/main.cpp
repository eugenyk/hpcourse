#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <random>

struct holder {
  int value;
  volatile bool updated = false;
  pthread_mutex_t mutex;
  pthread_cond_t cond_read;
  pthread_cond_t cond_write;

  holder() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_read, NULL);
    pthread_cond_init(&cond_write, NULL);
  }

  ~holder() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_read);
    pthread_cond_destroy(&cond_write);
  }
};

int consumer_threads_number;
int consumer_sleep_limit;
pthread_barrier_t barrier;
std::vector<pthread_t> consumer_threads;
volatile bool is_over;
std::mt19937 rnd(1289546); 

void* producer_routine(void* arg) {
  holder *hld = (holder*) arg;
  is_over = false;
  pthread_barrier_wait(&barrier);
  std::string line;
  std::getline(std::cin, line);
  std::stringstream line_stream;
  line_stream << line;
  int val;

  while (line_stream >> val) {
    //std::cerr << val << "\n";
    pthread_mutex_lock(&(hld->mutex));
    //std::cerr << "In\n";
    while (hld->updated) {
      pthread_cond_wait(&(hld->cond_write), &(hld->mutex));
    }
    hld->updated = true;
    hld->value = val;
    //std::cerr << "Out\n";
    pthread_mutex_unlock(&(hld->mutex));
    pthread_cond_signal(&(hld->cond_read));
  }

  is_over = true;

  pthread_cond_broadcast(&(hld->cond_read));
}
 
void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  pthread_barrier_wait(&barrier);
  holder *hld = (holder*) arg;

  int sum = 0;

  while (true) {
    pthread_mutex_lock(&(hld->mutex));
    while (!hld->updated && !is_over) {
      pthread_cond_wait(&(hld->cond_read), &(hld->mutex));
    }
    if (hld->updated) {
      sum += hld->value;
      hld->updated = false;
    }

    pthread_mutex_unlock(&(hld->mutex));

    if (is_over) {
      break;
    }
    pthread_cond_signal(&(hld->cond_write));

    usleep(rnd() % (consumer_sleep_limit + 1));
  }

  int *b = new int[1];
  b[0] = sum;
  return b;
}

void* consumer_interruptor_routine(void* arg) {
  pthread_barrier_wait(&barrier);
  while (!is_over) {
    int i = rnd() % consumer_threads_number;
    pthread_cancel(consumer_threads[i]);
  }
}
 
int run_threads() {
  pthread_t producer_thread;
  pthread_t interruptor_thread;
  consumer_threads = std::vector<pthread_t>(consumer_threads_number);

  pthread_barrier_init(&barrier, NULL, consumer_threads_number + 2);

  holder hld;

  pthread_create(&producer_thread, NULL, producer_routine, &hld);
  pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine, &consumer_threads);
  for (int i = 0; i < consumer_threads_number; i++) {
    pthread_create(&(consumer_threads[i]), NULL, consumer_routine, &hld);
  }

  pthread_join(interruptor_thread, NULL);
  pthread_join(producer_thread, NULL);
  pthread_barrier_destroy(&barrier);

  int sum = 0;
  for (int i = 0; i < consumer_threads_number; i++) {
    void **a = new void*[1];
    pthread_join(consumer_threads[i], a);
    //std::cerr << ((int*)*a)[0] << "\n";
    sum += ((int*)*a)[0];
  }

  return sum;
}
 
int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage " << argv[0] << " consumer_threads_number consumer_sleep_limit" << std::endl;
    return 0;
  }
  consumer_threads_number = atoi(argv[1]);
  consumer_sleep_limit = atoi(argv[2]);
  std::cout << run_threads() << std::endl;
  return 0;
}