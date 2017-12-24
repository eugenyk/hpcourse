
 #include <pthread.h>
 #include <iostream>
 
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
 
 pthread_mutex_t mutex;
 pthread_cond_t cond_start, cond_prod_running;
 pthread_t producer, consumer, interruptor;
 
 enum state {INITIAL, PRODUCER_RUNNING, CONSUMER_RUNNING, PRODUCER_STOPED};
 state global_state = INITIAL;
 
 
 void *producer_routine(void *arg) {
 
     pthread_mutex_lock(&mutex);
     while (global_state == INITIAL)
         pthread_cond_wait(&cond_start, &mutex);
     pthread_mutex_unlock(&mutex);
 
     Value *v = reinterpret_cast<Value *>(arg);
 
     pthread_mutex_lock(&mutex);
     int x;
     while (std::cin >> x) {
 
         while (global_state == CONSUMER_RUNNING)
             pthread_cond_wait(&cond_prod_running, &mutex);
 
         v->update(x);
         global_state = CONSUMER_RUNNING;
 
         if (x == -1) break;
     }
 
     while (global_state == CONSUMER_RUNNING)
         pthread_cond_wait(&cond_prod_running, &mutex);
 
     global_state = PRODUCER_STOPED;
     pthread_mutex_unlock(&mutex);
 
     return NULL;
 }
 
 
 void *consumer_routine(void *arg) {
     pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
 
     pthread_mutex_lock(&mutex);
     global_state = CONSUMER_RUNNING;
     pthread_cond_broadcast(&cond_start);
     pthread_mutex_unlock(&mutex);
 
     Value *v = reinterpret_cast<Value *>(arg);
     int *result = new int(0);
 
     state cur_state = PRODUCER_RUNNING;
     while (cur_state != PRODUCER_STOPED) {
         pthread_mutex_lock(&mutex);
         if (cur_state == CONSUMER_RUNNING) {
             *result += v->get();
             global_state = PRODUCER_RUNNING;
             pthread_cond_broadcast(&cond_prod_running);
         }
         cur_state = global_state;
         pthread_mutex_unlock(&mutex);
     }
 
     pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
     return result;
 }
 
 
 void *consumer_interruptor_routine(void *arg) {
 
     pthread_mutex_lock(&mutex);
     while (global_state == INITIAL)
         pthread_cond_wait(&cond_start, &mutex);
     pthread_mutex_unlock(&mutex);
 
     state cur_state = global_state;
     while (cur_state != PRODUCER_STOPED) {
         pthread_cancel(consumer);
 
         pthread_mutex_lock(&mutex);
         cur_state = global_state;
         pthread_mutex_unlock(&mutex);
     }
 
     return NULL;
 }
 
 
 int run_threads() {
     Value v;
 
     pthread_create(&consumer, NULL, consumer_routine, reinterpret_cast<void *>(&v));
     pthread_create(&producer, NULL, producer_routine, reinterpret_cast<void *>(&v));
     pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);
 
     int *sum;
     pthread_join(producer, NULL);
     pthread_join(consumer, reinterpret_cast<void **>(&sum));
     pthread_join(interruptor, NULL);
 
     return *sum;
 }
 int main() {
     std::cout << run_threads() << std::endl;
     return 0;
} 