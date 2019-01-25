#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>

#include <pthread.h>



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_ready =  PTHREAD_COND_INITIALIZER;

bool value_updated =  false;
bool finish = false;

int sleep_time;
int threads_num;

int sum = 0;


std::vector<std::string> split(const std::string& s, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }
  return tokens;
}

std::vector<int> get_values_list(){
    std::string str = "";
    std::cout<<"Enter a list of numbers"<<std::endl;
    std::getline(std::cin, str);
    std::vector<std::string> nums_str = split(str,' ');
    std::vector<int> nums;
    for (std::string s : nums_str){
      nums.push_back(atoi(s.c_str()));
     }
     return nums;
}
 
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
// Wait for consumer to start
// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
  std::vector<int> numlist =  get_values_list();
  Value* v =  static_cast<Value*>(arg);
  do{
    pthread_mutex_lock(&mutex);
      if(!value_updated){
        (*v).update(numlist.back());
        value_updated = true;
        numlist.pop_back();   
      }
    pthread_mutex_unlock(&mutex);
  }while (!(numlist.empty()));
  pthread_mutex_lock(&mutex);
    finish = true;
  pthread_mutex_unlock(&mutex);
}
 
void* consumer_routine(void* arg) {
  // notify about start
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (aggregated result for all consumers)
  
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

// Get thread Id of calling thread
  pthread_t thId = pthread_self();
  Value* v =  static_cast<Value*>(arg);
    srand( time( 0 ) );
    
    while(true){
      pthread_mutex_lock(&mutex);
        if(!value_updated&&finish){
          pthread_mutex_unlock(&mutex);
          break;
        }
        if(value_updated){
             sum+=(*v).get(); 
             value_updated = false;
        }
      pthread_mutex_unlock(&mutex);
      usleep((rand() % (sleep_time +1) )*1000);
  }
    
  void* ptr = &sum;
  return ptr;
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
  // interrupt consumer while producer is running 
  pthread_t* threads = static_cast<pthread_t*>(arg);
  srand( time( 0 ) );
  //int b = 1;
  while(true){
    pthread_mutex_lock(&mutex);
      if(finish){
        pthread_mutex_unlock(&mutex);
          break;
      }
    pthread_mutex_unlock(&mutex);
    int i = rand() % threads_num;
    pthread_cancel(threads[i]);
  } 
  
}
 
int run_threads() {
  // start N threads and wait until they're done
  // return aggregated sum of values
  Value value;
  pthread_t produser_thread;
  pthread_create(&produser_thread, NULL, producer_routine,  &value);
  pthread_t* threads = (pthread_t*) malloc(threads_num * sizeof(pthread_t));
  for(int i = 0; i < threads_num; i++){ 
    //запускаем поток
    pthread_create(&(threads[i]), NULL, consumer_routine, &value);
    
  }
  pthread_t interruptor_thread;
  pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine,  threads);
  
  //ждем завершения потока
  pthread_join(produser_thread, NULL);
  pthread_join(interruptor_thread, NULL);
  //ожидаем выполнение всех потоков
  void *returnValue; 
  for(int i = 0; i < threads_num; i++)
      pthread_join(threads[i], &returnValue);
  
  int* res =  static_cast<int*>(returnValue);
  return *res;
}
 
int main(int argc, char *argv[]) {
  if(argc < 3){
    std::cout << "ERROR: must specify two arguments (N - number of consumers, T - max sleeping time)" << std::endl; 
    return 1;
  } 
  threads_num = atoi(argv[1]); 
  sleep_time = atoi(argv[2]);
  std::cout << run_threads() << std::endl;
  //run_threads();
  return 0;
}