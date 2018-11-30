#include <pthread.h>
#include <iostream>

//Volatile — ключевое слово языков C/C++,
// которое информирует компилятор о том, 
// что значение переменной может меняться 
// из вне и что компилятор не будет оптимизировать эту переменную.

volatile bool isReady = false;//готов к использованию 
volatile bool isFin = false;// заканчиваем выполнение

pthread_mutex_t producer_mutex;//produser_muter типа pthread_mutex_t(глобальный мьютекс, доступный всем потокам)
pthread_mutex_t consumer_mutex;//consumer_muter типа pthread_mutex_t(глобальный мьютекс, доступный всем потокам)
pthread_cond_t producer_signal;//produser_signal типа pthread_mutex_t
pthread_cond_t consumer_signal;//consumer_muter типа pthread_mutex_t

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

// Задача producer-потока - получить на вход список чисел, 
// и по очереди использовать каждое значение из этого списка
// для обновления переменной разделяемой между потоками. После
// этого поток должен дождаться реакции одного из consumer-потоков, 
// и продолжить обновление значений только после того как поток-consumer
// принял это изменение. Функция исполняющая код этого потока 
// producer_routine должна принимать указатель на объект типа Value,
// и использовать его для обновления.

  // Wait for consumer to start
 
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

 int count_numerous;//нужна для считывания строки (сюда читаем)
 
 Value *value = (Value*)arg; // указатель на объект типа Value, 

  while(std::cin >> count_numerous) { //Считываем строку в цикле в переменную count_numerous (перебираем значения)
      
          value->update(count_numerous);//обновляем значения переменной разделяемой между потоками
          
          //Блокировка: теперь к ресурсам имеет доступ только один
          //поток, который владеет мьютексом. Он же единственный, 
          //кто может его разблокировать
          pthread_mutex_lock(&producer_mutex);
          
          isReady = true; //меняем значение глобальной переменной на true

          //Блокированный поток пробуждается с помощью вызовов pthread_cond_signal()

          pthread_cond_signal(&producer_signal);//другой поток устанавливает условие  producer-signal (уведомляем потребителя)
          pthread_mutex_unlock(&producer_mutex);//разблокировали producer-поток

          pthread_mutex_lock(&consumer_mutex); //захватываем мьютекс consumer-потока
          
          while (isReady) { //если готов к использованию
            //pthread_cond_wait() всегда возвращает запертый мьютекс, который принадлежит вызывающему потоку, даже если возникла ошибка.
              pthread_cond_wait(&consumer_signal, &consumer_mutex);//также перед возвратом повторно захватывает мьютекс (ждем, пока потребитель обработает)
          }
          
          pthread_mutex_unlock(&consumer_mutex); //разблокировали поток
        }

        isFin = true; //флаг конца использования
        
        pthread_cond_signal(&producer_signal); //другой поток устанавливает условие  producer-signal (уведомили потребителя)

}
 
void* consumer_routine(void* arg) {
  // Задача consumer-потоков отреагировать на каждое изменение 
  // переменной data и набирать сумму полученных значений. 
  // После того как достигнуто последнее обновление, функция потока 
  // должна вернуть результирующую сумму. Также этот поток должен
  // защититься от попыток потока-interruptor его остановить.
  // Функция исполняющая код этого потока consumer_routine должна
  // принимать указатель на тот же объект типа Value, и читать из
  // него обновления. После суммирования переменной поток должен
  // заснуть на случайное количество миллисекунд, верхний предел
  // будет передан на вход приложения. Во время сна поток не должен
  // мешать другим потокам consumer выполнять свои задачи, если они есть
  // notify about start
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (aggregated result for all consumers)
  //изменение статуса восприимчивости к удалению потока управления 
  // PTHREAD_CANCEL_DISABLE - новое значение статуса (допустимость удаления);
  // NULL - старое значение статуса или NULL. 
  //При успешном завершении функция возвращает 0, в противном случае - код ошибки.
  
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    
    Value *value = (Value*)arg; // указатель на объект типа Value, 
    
    int *summa = new int; 
    *summa = 0; //сначала сумма = 0

    while(!isFin){
      pthread_mutex_lock(&producer_mutex);
      while (!isReady && !isFin) {
        pthread_cond_wait(&producer_signal, &producer_mutex);
      }
      pthread_mutex_unlock(&producer_mutex);
      if (isFin) {
          break;
      }

      *summa += value->get();

      pthread_mutex_lock(&consumer_mutex);
      isReady = false;
      pthread_cond_signal(&consumer_signal);
      pthread_mutex_unlock(&consumer_mutex);  
    }
    return summa;
}
 
void* consumer_interruptor_routine(void* arg) {

  // wait for consumer to start
 
  // interrupt consumer while producer is running

  // Задача потока-interruptor проста: пока происходит
  // процесс обновления значений, он должен постоянно
  // пытаться остановить случайный поток consumer
  // (вычисление случайного потока происходит перед
  // каждой попыткой остановки). Как только поток producer 
  // произвел последнее обновление, этот поток завершается.
  // wait for consumer to start
 
  // interrupt consumer while producer is running                                          

  while (!isFin) {
    pthread_cancel(*(pthread_t*)arg);
  }                                          
}
 
int run_threads() {
  // start N threads and wait until they're done
  // return aggregated sum of values
  
  pthread_t producer_thread;
  pthread_t consumer_thread;
  pthread_t consumer_interruptor_thread;
  pthread_cond_init(&producer_signal, NULL);
  pthread_cond_init(&consumer_signal, NULL);
  Value value;
  int *summa;

  pthread_create(&producer_thread, NULL, producer_routine, &value);
  pthread_create(&consumer_thread, NULL, consumer_routine, &value);
  pthread_create(&consumer_interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread);

  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, (void**)&summa);

  int res = *summa;
  delete summa;
  return res;
  return 0;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}