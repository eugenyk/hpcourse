#include "ThreadPool.h"
#include "Worker.h"

uint32_t ThreadPool::count_uid_threads = 0;

ThreadPool::ThreadPool(boost::mutex *_io_mutex, uint32_t _hot_threads_count, uint32_t _delay_to_remove_thread)
	: io_mutex(_io_mutex),
	  active_threads_count(0),
	  hot_threads_count(_hot_threads_count),
	  inactive_threads_count(0),
	  all_threads_count(0),
	  delay_to_remove_thread(_delay_to_remove_thread),
	  is_stoped(false)
{
	for (uint32_t i = 0; i < hot_threads_count; i++)
		add_thread_in_pool();
}

void ThreadPool::add_task_in_queue(const uint32_t time_to_perform) {
	// если наш пул работает
	if (!is_stoped) {
		// создаём новую задачу и бросаем уведомление о данном событии
		Task *my_task = new Task(time_to_perform);
		{
			boost::mutex::scoped_lock scop_lock(task_mutex);
			queue_tasks.push_back(my_task);
			condition_v.notify_one();
		}

		// как обычно болтаем лишнего
		{
			boost::mutex::scoped_lock scop_lock(*io_mutex);
			std::cout << "Task[" << my_task->get_uid() << "] with labor time: " << time_to_perform 
					  << " seconds was added in ThreadPool queue" << std::endl;
		}

		// дальше гениальные строки
		bool free_thread_is_hear = false;
		{
			boost::mutex::scoped_lock scop_lock(worker_mutex);
			// если количество активных на данный момент потоков >= количеству горячих потоков
			// и потоков в состоянии ожидания нету
			if ( (active_threads_count >= hot_threads_count) && (inactive_threads_count == 0) )
				// тогда нам нужно выделить новый поток под поступившую задачу
				free_thread_is_hear = true;
		}
		// ну и в зависимости от результата предыдущего шага добавляем поток в пул или не добавляем
		if (free_thread_is_hear) add_thread_in_pool();
	}
}

void ThreadPool::add_thread_in_pool() {
	// можем создать горячий поток?
	bool thread_is_hot = (count_uid_threads < hot_threads_count) ? true : false;

	// оборачиваем поток в рабочего
	Worker *worker = new Worker(this, io_mutex, &condition_v, delay_to_remove_thread, thread_is_hot, count_uid_threads++);
	// передаём объект по ссылке для конструктора
	boost::thread *worker_thread = new boost::thread(boost::ref(*worker));
	// устанавливаем worker'у рабочий поток
	worker->set_working_thread(worker_thread);

	{
		// добавляем свежеиспечённого workera в нашу мнмую очередь рабочих
		boost::mutex::scoped_lock scop_lock(worker_mutex);
		queue_workers.push_back(worker);
		all_threads_count++;
		inactive_threads_count++;
	}

	{
		// раскажем миру о том, что ряды рабочих пополнились
		boost::mutex::scoped_lock scop_lock(*io_mutex);
		std::cout << "Add in ThreadPool " << ( (thread_is_hot) ? " hottable " : "") << " Thread[" << count_uid_threads - 1 << "]" << std::endl;
	}
}

void ThreadPool::dec_active_thread() {
	boost::mutex::scoped_lock scop_lock(worker_mutex);
	active_threads_count--;
	inactive_threads_count++;
}

void ThreadPool::display_statistic()
{
	boost::mutex::scoped_lock scop_lock(*io_mutex);

	{
		boost::mutex::scoped_lock scop_scop_lock(worker_mutex);
		std::cout << std::endl << " >> ThreadPool stats << " << std::endl;
		std::cout << "Exist threads: " << all_threads_count << std::endl;
		std::cout << "Active threads: " << active_threads_count << std::endl;
		std::cout << "Inactive threads: " << inactive_threads_count << std::endl << std::endl;
	}

	{
		boost::mutex::scoped_lock scop_scop_lock(task_mutex);
		for (std::vector<Worker *>::iterator it = queue_workers.begin(); it != queue_workers.end(); ++it) {
			// если рабочий не сидит и курит, а работает (не находится в ожидании работы)
			if ( ( !(*it)->worker_is_stoped() ) && ( !(*it)->worker_ready_for_task() ) )
				std::cout << "Task[" << (*it)->get_task_uid() << "] is achieved >> Elapsed time: " 
						  << (*it)->get_task()->get_elapsed_time() << " seconds >> Perform time: "
						  << (*it)->get_task()->get_time_to_perform() << " seconds. " << std::endl;
		}
	}
}

Task *ThreadPool::get_task_from_queue() {
	Task *stolen_task = nullptr;
	{
		// хватает из очереди первую попавшуюся задачу
		boost::mutex::scoped_lock scop_lock(task_mutex);
		// а в очереди задачи то есть?
		if (!queue_tasks.empty()) {
			stolen_task = queue_tasks.front();
			queue_tasks.erase(queue_tasks.begin());
		}
	}

	return stolen_task;
}

void ThreadPool::inc_active_thread() {
	boost::mutex::scoped_lock scop_lock(worker_mutex);
	active_threads_count++;
	inactive_threads_count--;
}

void ThreadPool::remove_thread_from_pool(Worker *worker)
{
	int32_t worker_uid = (worker) ? worker->get_thread_uid() : -1;
	std::vector<Worker*>::iterator erase_it;
	{
		boost::mutex::scoped_lock scop_lock(worker_mutex);
		for (std::vector<Worker*>::iterator it = queue_workers.begin(); it != queue_workers.end(); ++it) {
			if ((*it) == worker) {
				erase_it = it;
				break;
			}
		}

		inactive_threads_count--;
		all_threads_count--;
		queue_workers.erase(erase_it);
	}

	{
		boost::mutex::scoped_lock scop_lock(*io_mutex);
		std::cout << std::endl <<  "Thread[" << worker_uid << "] was removed from ThreadPool" << std::endl;
	}
}

void ThreadPool::remove_thread_from_pool(std::vector<Worker*>::iterator it) {
	{
		boost::mutex::scoped_lock scop_lock(*io_mutex);
		std::cout << "Thread[" << (*it)->get_thread_uid() << "] was removed from pool." << std::endl;
	}

	// ну тут всё просто, даже по очереди в поисках ходить не пришлось
	all_threads_count--;
	queue_workers.erase(it);
}

void ThreadPool::stop_thread_pool()
{
	is_stoped = true;
	{
		boost::mutex::scoped_lock scop_lock(worker_mutex);
		// снова очень сложная операция
		for (std::vector<Worker *>::iterator it = queue_workers.begin(); it != queue_workers.end(); ++it) {
			// останавливаем исполняемую worker'ом задачку
			(*it)->stop_worker_task();
			// кажись это можно записать используя сокращённый оператор, но ... так понятнее и переписывать не буду
			if ((*it)->worker_ready_for_task())
				inactive_threads_count--;
			else
				active_threads_count--;
			// ранее мы остановили "полезную" работу потока для обёртки worker
			// теперь прерываем исполнение потока в обёртке
			(*it)->get_worker_thread()->interrupt();
			// дожидаемся нормального завершения потока
			(*it)->get_worker_thread()->join();
		}
		// уничтожаем всю очередь
		while (!queue_workers.empty())
			// оформил в виде отдельной процедуры
			remove_thread_from_pool(queue_workers.begin());
	}

	{
		boost::mutex::scoped_lock scop_lock(task_mutex);
		// а теперь пришло время и для задач
		while (!queue_tasks.empty())
			queue_tasks.erase(queue_tasks.begin());
	}

	{
		// всё хорошее однажды заканчивается
		boost::mutex::scoped_lock scop_lock(*io_mutex);
		std::cout << "HotThreadPool was stoped" << std::endl;
	}
}

void ThreadPool::stop_task_with_uid(const uint32_t uid) {
	{
		boost::mutex::scoped_lock scop_lock(worker_mutex);
		// дальше очень сложно и с кучей задач отнимет кучу времени
		for (std::vector<Worker *>::iterator it = queue_workers.begin(); it != queue_workers.end(); ++it) {
			// процесс требует оптимизации :)
			if ( ( !(*it)->worker_ready_for_task() ) && ( (*it)->get_task_uid() == uid) ) {
				// вроде отыскали задачу с таким идентификатором
				boost::thread *thread = (*it)->get_worker_thread();
				// приостанавливаем работу данного потока
				// на той стороне должно вывалиться исключение и известить народ об этом
				thread->interrupt();
				return;
			}
		}
	}

	// если исключение не вывалилось, то это плохо. Не знаю что делать :) Пусть пользователь попробует ещё раз
	{
		boost::mutex::scoped_lock scop_lock(*io_mutex);
		std::cout << "Attention! Task[" << uid << "] can not be interrupted!" << std::endl;
	}
}

bool ThreadPool::thread_pool_is_stoped() const
{
	return is_stoped;
}

ThreadPool::~ThreadPool() {
}
