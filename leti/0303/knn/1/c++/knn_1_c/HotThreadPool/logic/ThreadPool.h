#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#pragma once

#include <boost\thread.hpp>
#include <vector>

#include "Task.h"

class Worker;

class ThreadPool
{
	// как обычно, нам нужен блок операций ввода/вывода
	boost::mutex *io_mutex;
	// блок для захвата задачек
	boost::mutex task_mutex;
	// блок для захвата worker'ов
	boost::mutex worker_mutex;
	// условная переменная для уведомления о появлении новой задачки
	boost::condition_variable condition_v;

	// задачи, которые находятся в пуле
	std::vector<Task *> queue_tasks;
	// набор worker'ов решателей задач
	std::vector<Worker *> queue_workers;
	
	// небольшая статистика по тредам
	// верим, что всё адекватно будет работать и не произойдёт выхода за предел unsigned
	uint32_t active_threads_count;
	uint32_t hot_threads_count;
	uint32_t inactive_threads_count;
	uint32_t all_threads_count;

	// генератор уникальных идентификаторов для потоков
	static uint32_t count_uid_threads;
	// задержка перед удалением потока из пула
	uint32_t delay_to_remove_thread;
	// исполнение деятельности приостановлено?
	bool is_stoped;

public:
	// конструктор как заказывали
	explicit ThreadPool(boost::mutex *_io_mutex, uint32_t _hot_threads_count = 0, uint32_t _delay_to_remove_thread = 0);

	// добавить поток в пул
	void add_thread_in_pool();
	// удалить рабочего из пула, но сначала найти его в списке
	void remove_thread_from_pool(Worker *worker);
	// удалить рабочего из пула
	void remove_thread_from_pool(std::vector<Worker *>::iterator it);
	// увеличить количество активных потоков
	void inc_active_thread();
	// увеличить количество активных потоков
	void dec_active_thread();
	// отобразить на экране информацию о работе пула
	void display_statistic();

	// работа пула приостановлена?
	bool thread_pool_is_stoped() const;
	// приостановить работу пула
	void stop_thread_pool();

	// добавить задачку в пул. Необходимо указать время её решения
	void add_task_in_queue(const uint32_t time_to_perform);
	Task* get_task_from_queue();
	// остановить задачу с указанным идентификатором
	void stop_task_with_uid(const uint32_t uid);

	virtual ~ThreadPool();
};

#endif