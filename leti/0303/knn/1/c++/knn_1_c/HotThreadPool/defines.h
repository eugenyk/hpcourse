#ifndef _DEFINES_H_
#define _DEFINES_H_

#pragma once

#include <boost\thread\mutex.hpp>

#include <stdint.h>
#include <iostream>
#include <signal.h>
#include <conio.h>

#include "utils\getopt.h"
#include "logic\ThreadPool.h"

// блокировка совместных операций ввода/выводы
boost::mutex io_mutex;

// указатель на пул гор€чих потоков
ThreadPool *thread_pool;

uint8_t getOption(uint8_t from_index, uint8_t to_index)
{
	std::cout << "Yours choice: ";
	uint8_t choise;
	do {
		choise = uint8_t(getch()) - 48;
	} while (choise < from_index || choise > to_index);
	std::cout << std::to_string(choise) << std::endl << std::endl;
	return choise;
}

//
void signalHandler(int32_t signal)
{
	{
		boost::mutex::scoped_lock scop_lock(io_mutex);
		std::cout << "Signal " << signal << " was sended" << std::endl;
	}

	if (thread_pool) {
		thread_pool->stop_thread_pool();
		delete thread_pool;
		thread_pool = nullptr;
	}

	exit(signal);
}

#endif
