#ifndef _TASK_H_
#define _TASK_H_

#pragma once

#include <stdint.h>

#include <boost\thread\thread.hpp>

class Task {
	// счЄтчик количества созданных за врем€ работы объектов
	static uint32_t all_objects_count;
	// врем€, которое прошло с момента запуска задачи в работу
	uint32_t elapsed_time;
	// результат работы
	uint32_t result;
	// врем€, которое требуетс€ дл€ решени€ задачи
	uint32_t time_to_perform;
	// уникальный идентификатор текущей задачи
	uint32_t uid;

public:
	explicit Task(uint32_t _time_to_perform = 0);

	// вернуть врем€, прошедшее с начала работы задачи
	uint32_t get_elapsed_time() const;
	// вернуть текущий результат работы задачи
	uint32_t get_result() const;
	// вернуть врем€, которое требуетс€ дл€ решени€ данной задачи
	uint32_t get_time_to_perform() const;
	// вернуть уникальный идентификатор задачи
	uint32_t get_uid() const;
	// запустить задачу
	uint32_t run_task();

	virtual ~Task();
};

#endif