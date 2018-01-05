#include <pthread.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <ctime>
 
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

#define ASSERT_TRUE(x, err) do { \
                              if (!(x)) { \
                                ::std::cerr << err << ::std::endl; \
                                exit(-1); \
                              } \
                            } while (false)

struct SharedData {
  Value data;
  Value data_done;
  Value snapshot_done;
  pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t condition = PTHREAD_COND_INITIALIZER;;
};

struct ProducerData {
  std::vector<int> values;
  SharedData shared_data;
};

 
void* producer_routine(void* arg) {
  // Wait for consumer to start
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

  ProducerData& producer_data = *(ProducerData*)arg;
  for (const auto& value : producer_data.values) {
    pthread_mutex_lock(&producer_data.shared_data.lock);

    while (producer_data.shared_data.data_done.get())
      pthread_cond_wait(&producer_data.shared_data.condition, &producer_data.shared_data.lock);
   
    producer_data.shared_data.data.update(value);
    asm volatile ("":::"memory");
    producer_data.shared_data.data_done.update(1);
    pthread_cond_signal(&producer_data.shared_data.condition);

    pthread_mutex_unlock(&producer_data.shared_data.lock);
  }

  pthread_mutex_lock(&producer_data.shared_data.lock);

  while (producer_data.shared_data.data_done.get())
    pthread_cond_wait(&producer_data.shared_data.condition, &producer_data.shared_data.lock);
 
  producer_data.shared_data.snapshot_done.update(1);
  pthread_cond_signal(&producer_data.shared_data.condition);

  pthread_mutex_unlock(&producer_data.shared_data.lock);
     
  return nullptr; 
}
 
void* consumer_routine(void* arg) {
  // notify about start
  // allocate value for result
  // for every update issued by producer, read the value and add to sum
  // return pointer to result

  ASSERT_TRUE(!pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr),
              "disabe cancellation error");

  SharedData& shared_data = *(SharedData*)arg;
  int *sump = new int(0);
  while (!shared_data.snapshot_done.get()) {
    pthread_mutex_lock(&shared_data.lock);

    while (!shared_data.data_done.get())
    {
      pthread_cond_wait(&shared_data.condition, &shared_data.lock);
      if (shared_data.snapshot_done.get() && !shared_data.data_done.get())
      {
        pthread_mutex_unlock(&shared_data.lock);
        return sump;
      }
    }
    
    *sump += shared_data.data.get();
    shared_data.data_done.update(0);
    pthread_cond_signal(&shared_data.condition);

    pthread_mutex_unlock(&shared_data.lock);
  }

  ASSERT_TRUE(!pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr),
              "disabe cancellation error");
  return sump;
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
  // interrupt consumer while producer is running                                          
  pthread_t& consumer_thread = *(pthread_t*)arg;
  while (!pthread_cancel(consumer_thread))
    pthread_yield();
  return nullptr;
}

int run_threads(const std::vector<int>& data)
{
  using namespace std;
  pthread_t producer_thread;
  ProducerData producer_data = { data, SharedData() };
  ASSERT_TRUE(!pthread_create(&producer_thread, nullptr, producer_routine, &producer_data),
              "Error creating producer thread");

  pthread_t consumer_thread;
  ASSERT_TRUE(!pthread_create(&consumer_thread, nullptr, consumer_routine, &producer_data.shared_data),
              "Error creating consumer thread");

  pthread_t consumer_interruptor_thread;
  ASSERT_TRUE(!pthread_create(&consumer_interruptor_thread, nullptr, consumer_interruptor_routine, &consumer_thread),
              "Error creating consumer interruptor thread");

  ASSERT_TRUE(!pthread_join(producer_thread, nullptr), "Error joining producer thread");
  ASSERT_TRUE(!pthread_join(consumer_interruptor_thread, nullptr), "Error joining consumer interruptor thread");

  int *sump = nullptr;
  ASSERT_TRUE(!pthread_join(consumer_thread, (void**)&sump), "Error joining consumer thread");
  const int sum = *sump;
  delete sump;

  return sum;
}
 
int run_threads() {
  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer
  return run_threads({ 2, 4, 4, 5, 6, 1 });
}

void crash_test() {
  srand(time(nullptr));
  std::vector<int> data(100000);
  for (auto& value : data) {
    value = rand() % 100;
  }
  int etalon = std::accumulate(data.begin(), data.end(), 0);
  ASSERT_TRUE(etalon == run_threads(data), "test error");
}
 
int main() {
  crash_test();  
  std::cout << run_threads() << std::endl;
  return 0;
}
