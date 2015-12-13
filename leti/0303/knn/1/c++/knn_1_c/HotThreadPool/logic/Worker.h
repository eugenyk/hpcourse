#ifndef _WORKER_H_
#define _WORKER_H_

#pragma once

#include "Task.h"

class ThreadPool;

class Worker
{
	// владелец объекта, которому принадлежит наша задачка
	ThreadPool *thread_pool;
	// указатель на задачку, которая сейчас обрабатывается в данном потоке
	Task *worker_task;

	boost::mutex *io_mutex;
	boost::mutex task_mutex;
	// условная переменная для уведомления worker'а о появлении новой задачки
	boost::condition_variable *condition_v;
	
	// флажок для проверки, работает ли сейчас worker
	volatile bool is_stoped;
	// указатель на рабочий поток, в котором обрабатывается наша задачка
	boost::thread *worker_thread;
	// рабочий поток свободен?
	bool worker_is_free;
	// поток по совместительству горяч?
	bool worker_is_hot;

	// уникальный идентификатор потока
	uint32_t thread_uid;
	// задержка перед освобождением потока от работы (применимо к холодным потокам)
	uint32_t thread_timeout;

public:
	explicit Worker(ThreadPool *_thread_pool = nullptr, 
				    boost::mutex *_io_mutex	= nullptr,
					boost::condition_variable *_condition_v = nullptr,
					uint32_t _thread_timeout = 0,
					bool _worker_is_hot = false,
					uint32_t _thread_uid = 0
				   );

	// первый опыт переопределения операторов! (:
	void operator()();

	// вернуть указатель на исполняющуюся задачку
	Task* get_task() const;
	// получить результат выполнения задачи
	int32_t get_task_result() const;
	// получить уникальный идентификатор задачки
	int32_t get_task_uid() const;
	// установить worker'y задачку
	void set_worker_task(Task *_worker_task);				// кажется это private
	// остановить выполняющуюся задачу
	void stop_worker_task();

	// получить уникальный идентификатор исполняющегося потока
	uint32_t get_thread_uid() const;
	// вернуть указатель на поток исполняющий задачки 
	boost::thread *get_worker_thread() const;
	// задать worker'у рабочий поток
	void set_working_thread(boost::thread *_worker_thread);
	// приостановить выполнение потока
	void stop_working_thread();
	// worker остановлен?
	bool worker_is_stoped() const;
	// worker в состоянии работы и свободен для решения задачи
	bool worker_ready_for_task() const;

	virtual ~Worker();
};

#endif