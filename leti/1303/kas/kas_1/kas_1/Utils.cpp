#include "stdafx.h"
#include "Utils.h"
#include <string>
#include <iostream>
#include <tuple>

Utils::Utils()
{
}


Utils::~Utils()
{
}

//-n 10 -l 5 -b 123 -f log.txt
std::tuple<int, int, int, std::string> Utils::argsProcessing(int argc, char * argv[])
{
	//default values.
	int n = 10, b = 123, l = 5;
	std::string f = "";

	std::vector<std::string> args(argv, argv + argc);
	for (size_t i = 1; i < args.size(); i++)
	{
		//printf("Argument %d : %s\n", i, args[i]);
		if (args[i] == "-n" && i + 1 < args.size())
		{
			try
			{
				b = std::stoi(args[i + 1]);
				if (n < 0 || n > 100) throw 1;
			}
			catch (const std::exception&)
			{
				std::cerr << "Exception while -n argument processing" << std::endl;
				system("pause");
				exit(0);
			}

		}
		else if (args[i] == "-b" && i + 1 < args.size())
		{
			try
			{
				b = std::stoi(args[i + 1]);
				if (b < 0 || b > 255) throw 2;
			}
			catch (const std::exception&)
			{
				std::cerr << "Exception while -b argument processing" << std::endl;
				system("pause");
				exit(0);
			}
			
		}
		else if (args[i] == "-l" && i + 1 < args.size())
		{
			try
			{
				l = std::stoi(args[i + 1]);
				if (l < 0 || l > 10) throw 3;
			}
			catch (const std::exception&)
			{
				std::cerr << "Exception while -l argument processing" << std::endl;
				system("pause");
				exit(0);
			}
			
		}
		else if (args[i] == "-f" && i + 1 < args.size())
		{
			try
			{
				f = args[i + 1];
			}
			catch (const std::exception&)
			{
				std::cerr << "Exception while -f argument processing" << std::endl;
				system("pause");
				exit(0);
			}
		}
	}

	printf("Result of args processing. -b:{%d}, -l:{%d}, -f:{%s}\n", b, l, f);
	return std::make_tuple(n, l, b, f);
}
