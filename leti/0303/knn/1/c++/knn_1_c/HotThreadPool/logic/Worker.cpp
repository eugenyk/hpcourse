#include "Worker.h"
#include "ThreadPool.h"

Worker::Worker(ThreadPool *_thread_pool,
			   boost::mutex *_io_mutex,
			   boost::condition_variable *_condition_v,
			   uint32_t _thread_timeout,
			   bool _worker_is_hot,
			   uint32_t _thread_uid
			  )
	: thread_pool(_thread_pool),
	  worker_task(nullptr),
	  io_mutex(_io_mutex),
	  condition_v(_condition_v),
	  is_stoped(false),
	  worker_thread(nullptr),
	  worker_is_free(true),
	  worker_is_hot(_worker_is_hot),
	  thread_uid(_thread_uid),
	  thread_timeout(_thread_timeout)  
{
}

Task* Worker::get_task() const
{
	return (worker_task) ? worker_task : nullptr;
}

int32_t Worker::get_task_result() const
{
	return (worker_task) ? worker_task->get_result() : -1;
}

int32_t Worker::get_task_uid() const
{
	return (worker_task) ? worker_task->get_uid() : -1;
}

uint32_t Worker::get_thread_uid() const
{
	return thread_uid;
}

boost::thread* Worker::get_worker_thread() const
{
	return (worker_thread) ? worker_thread : nullptr;
}

void Worker::operator()()
{
	// пока worker не остановлен ему можно присваивоить новую задачу
	while (!is_stoped) {
		boost::mutex::scoped_lock scop_lock(task_mutex);
		try {
			// если рабочий обладает горячим потоком
			if (worker_is_hot) {
				// если worker не работает ни над какой из задач
				if (!worker_task) {
					// пока внешне не запретят работу workerу
					while (!is_stoped) {
						// воруем задачи из пула или ждём, пока он нам что-то отдаст
						Task *stolen_task = thread_pool->get_task_from_queue();
						(stolen_task) ? set_worker_task(stolen_task) : condition_v->wait(scop_lock);
					}
				}
			}
			// у нас холодный поток на руках
			else {
				// если wokerу не назначен поток для работы
				if (!worker_task) {
					// пока внешне не запретят работу workerу
					while (!is_stoped) {
						// воруем одну задачу 
						Task *stolen_task = thread_pool->get_task_from_queue();
						if (stolen_task) {
							set_worker_task(stolen_task);
						}
						// задачу не удалось своровать и возможно поток будет уничтожен
						else {
							// посчитает время до уничтожения
							boost::system_time const timе_to_remove = boost::get_system_time() + boost::posix_time::seconds(thread_timeout);
							// если время ещё есть
							if (condition_v->timed_wait(scop_lock, timе_to_remove)) {
								// воруем задачу 
								stolen_task = thread_pool->get_task_from_queue();
								// если задача сворована
								if (stolen_task)
									set_worker_task(stolen_task);
							}
							// времени больше нет
							else {
								// останавливаем работу потока
								stop_worker_task();
								// удаляем рабочий поток из пула
								thread_pool->remove_thread_from_pool(this);
								return;
							}
						}
					}
				}
			}
		}
		// адекватное завершение потоков
		catch (boost::thread_interrupted) {
			if (worker_task) {
				// лочим поток ввода/вывода
				boost::mutex::scoped_lock scop_lock(*io_mutex);
				// бросаем туда наше сообщение
				std::cout << "Task[" << worker_task->get_uid() << "] with labor time: " 
					      << worker_task->get_time_to_perform() << " seconds interrupted in Thread["
						  << thread_uid << "]" << std::endl << ">> Task result: \"" << worker_task->get_result() 
						  << "\" and time elapsed: " << worker_task->get_elapsed_time() << " seconds."
						  << std::endl;
			}
			// если worker ещё работает над задачкой, то останавливаем работу
			if (!is_stoped) stop_working_thread();
		}
	}
}

void Worker::set_worker_task(Task *_worker_task)
{
	// у нас появилась новая задачка! Расскажем всем об этом!
	worker_task = _worker_task;
	{
		boost::mutex::scoped_lock scop_lock(*io_mutex);
		std::cout << std::endl << std::endl << "Task[" << worker_task->get_uid() << "] with labor time: " 
				  << worker_task->get_time_to_perform() << " seconds. >> Starting in Thread["
				  << thread_uid << "]" << std::endl;
	}

	worker_is_free = false;
	// в пуле на один поток больше
	thread_pool->inc_active_thread();
	// запускаем заадчку и ждём окончания её работы
	worker_task->run_task();

	{
		// как только задачка решится, расскажем всем об этом!
		boost::mutex::scoped_lock scop_lock(*io_mutex);
		std::cout << "Task[" << worker_task->get_uid() << "] with labor time:" 
				  << worker_task->get_time_to_perform() << "seconds. >> Finished in Thread["
				  << thread_uid << "] >> Task result: \"" << worker_task->get_result() << "\"" << std::endl;
	}

	stop_working_thread();
}

void Worker::set_working_thread(boost::thread *_worker_thread) {
	worker_thread = _worker_thread;
}

void Worker::stop_worker_task() {
	if (!is_stoped) {
		is_stoped = true;
		{
			boost::mutex::scoped_lock scop_lock(*io_mutex);
			std::cout << "Thread[" << thread_uid << "] was stoped" << std::endl;
		}
	}
}

void Worker::stop_working_thread() {
	thread_pool->dec_active_thread();
	if (worker_task) {
		delete worker_task;
		worker_task = nullptr;
	}
	worker_is_free = true;
}

bool Worker::worker_is_stoped() const
{
	return is_stoped;
}

bool Worker::worker_ready_for_task() const
{
	return (worker_is_free && !is_stoped) ? true : false;
}

Worker::~Worker() {
	if (worker_task != nullptr) {
		delete worker_task;
		worker_task = nullptr;
	}
}
