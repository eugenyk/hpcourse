#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>

#include <pthread.h>



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t start_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition =  PTHREAD_COND_INITIALIZER;

bool value_updated =  true;
bool start = false;
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
  std::cout << "начинаем считать" << std::endl;
   pthread_mutex_lock(&mutex);
       
        (*v).update(numlist.back());
        value_updated = true;
        numlist.pop_back();   
      pthread_mutex_unlock(&mutex);
  while (!(numlist.empty())){
    
    pthread_mutex_lock(&mutex);

        pthread_cond_wait(&condition, &mutex); 
        std::cout << "producer_routine " << std::endl;
        (*v).update(numlist.back());
        value_updated = true;
        numlist.pop_back();   
      pthread_mutex_unlock(&mutex);
      
  }
  finish = true;
}
 
void* consumer_routine(void* arg) {
  // notify about start
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (aggregated result for all consumers)
  
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
std::cout << "запущен consumer_routine" << std::endl;
  Value* v =  static_cast<Value*>(arg);
  srand( time( 0 ) );
  while(value_updated||!finish){

    pthread_mutex_lock(&mutex);
    
      if(value_updated){
         sum+=(*v).get(); 
         std::cout <<pthread_self()<<" "<< (*v).get() << std::endl;
         value_updated = false;
         pthread_cond_broadcast( &condition);
   
      }
    pthread_mutex_unlock(&mutex);
    usleep((rand() % sleep_time +1 )*1000);
  }
  void* ptr = &sum;
  return ptr;
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
  // interrupt consumer while producer is running 
  pthread_t* threads = static_cast<pthread_t*>(arg);
  srand( time( 0 ) );
  int b = 1;
  while(!finish){
    int i = rand() % threads_num;
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&condition, &mutex); 
    pthread_cancel(threads[i]);
    pthread_mutex_unlock(&mutex);
    
    } 
  
}
 
int run_threads() {
  // start N threads and wait until they're done
  // return aggregated sum of values
  Value value;
  std::cout << "Запускаю потоки" << std::endl;
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
  //ожидаем выполнение всех потоков
  void *returnValue; 
  for(int i = 0; i < threads_num; i++)
      pthread_join(threads[i], &returnValue);
  pthread_join(interruptor_thread, NULL);
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