#include "Task.h"

uint32_t Task::all_objects_count = 0;

Task::Task(uint32_t _time_to_perform) : uid(all_objects_count++), elapsed_time(0), result(0), time_to_perform(_time_to_perform) {
}

// вернуть время, прошедшее с начала работы задачи
uint32_t Task::get_elapsed_time() const {
	return elapsed_time;
}

// вернуть текущий результат работы задачи
uint32_t Task::get_result() const {
	return result;
}

// вернуть время, которое требуется для решения данной задачи
uint32_t Task::get_time_to_perform() const {
	return time_to_perform;
}

// вернуть уникальный идентификатор задачи
uint32_t Task::get_uid() const {
	return uid;
}

// запустить задачу
uint32_t Task::run_task() {
	try {
		// подождём пока наша задачка считается
		while (elapsed_time < time_to_perform) {
			// кладём поток каждую секунду, имитируем процесс активной работы
			// возможно следовало спать 999 миллисекунд
			boost::this_thread::sleep(boost::posix_time::millisec(1000));
			// увеличиваем счётчик прошедших секунд
			elapsed_time++;
			// имитируем сложный рассчёт
			result++;
		}
	}
	catch (boost::thread_interrupted) {
		// бросаем исключение в случае прерывания исполнения задачки
		throw boost::thread_interrupted();
	}
	return result;
}

Task::~Task() {
}
