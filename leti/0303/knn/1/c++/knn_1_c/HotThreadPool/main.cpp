// HotThreadPool.cpp : Defines the entry point for the console application.
//

#include "defines.h"

void usage();

int main(int argc, char *argv[])
{
	/* Getopt */
	extern char *optarg;
	register int32_t opt;

	// хотим, чтобы пользователь указал количество горячих потоков и время перед тем как бездействующий поток будет удалён из пула
	// соответственно у нас два параметар и они должны быть заданы
	std::pair<std::pair<bool, int32_t>, std::pair<bool, int32_t>> required_options = { {false, 0}, {false, 0} };

	/* Get options */
	while ((opt = getopt(argc, argv, "c:t:h")) != EOF)
	{
		switch (opt)
		{
		/* Help */
		case 'h':
			usage();
			return(EXIT_SUCCESS);

		case 'c':
			required_options.first.first = true;
			required_options.first.second = std::atoi(optarg);
			break;

		case 't':
			required_options.second.first = true;
			required_options.second.second = std::atoi(optarg);
			break;
		}
	}
	if (std::get<0>(required_options.first) == false)
		std::cout << ">> Required option \"Count of hot threads\" not found." << std::endl;
	if (std::get<1>(required_options.first) == false)
		std::cout << ">> Required option \"Remove delay\" not found." << std::endl;
	if (!std::get<0>(required_options.first) || !std::get<1>(required_options.first))
	{
		std::cout << std::endl << "Catch error. Program can\'t be started." << std:: endl << "For more information please use \"-h\" key argument." << std::endl;
		getchar();
		exit(EXIT_FAILURE);
	}

	// 1 параметр - блокировка совместных операций ввода/вывода
	// 2 параметр - количество горячих потоков
	// 3 параметр - задержка перед удалением потока из пула
	thread_pool = new ThreadPool(&io_mutex, std::get<1>(required_options.first), std::get<1>(required_options.second));
	if (thread_pool) {

		signal(SIGINT, signalHandler);
		signal(SIGTERM, signalHandler);

		// переменная выбора пункта меню
		uint8_t choise;
		// время, необходимое для решения задачи
		int32_t time_to_perform;
		// уникальный идентификатор задачки
		int32_t task_uid;

		while (thread_pool && !thread_pool->thread_pool_is_stoped()) {
			{
				boost::mutex::scoped_lock scop_lock(io_mutex);
				std::cout << std::endl << "################" << std::endl
					<< "Choose action: " << std::endl
					<< "1) Add new task in queue" << std::endl
					<< "2) Interrupt task from queue" << std::endl
					<< "3) Show ThreadPool statistic" << std::endl
					<< "0) Quit" << std::endl
					<< "################" << std::endl;
			}

			choise = getOption(0, 3);
			switch (choise) {
			case 0:
				thread_pool->stop_thread_pool();
				delete thread_pool;
				thread_pool = nullptr;
				break;

			case 1: 
				{
					boost::mutex::scoped_lock scop_lock(io_mutex);
					std::cout << "Enter time (value >= 0) to perform your task: ";
					std::cin >> time_to_perform;
				}
				if (time_to_perform >= 0)
					thread_pool->add_task_in_queue(time_to_perform);
				break;

			case 2: 
				{
					boost::mutex::scoped_lock scop_lock(io_mutex);
					std::cout << "Enter task UID (value >= 0) for stoping: ";
					std::cin >> task_uid;
				}
				if (task_uid >= 0) 
					thread_pool->stop_task_with_uid(task_uid);
				break;

			case 3: 
				thread_pool->display_statistic();
				break;

			default: 
				continue;
			} 
		} 

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

void usage()
{
	std::cout << "Hot thread pool by Allow (lab #1)" << std::endl << std::endl;
	std::cout << "Usage options:" << std::endl;
	std::cout << " -c <number> \t - required option (count of hot threads in pool)" << std::endl;
	std::cout << " -t <number> \t - required option (delay before removing free thread from pool)" << std::endl;
	std::cout << " -h \t\t - show hint (help)" << std::endl;
}
