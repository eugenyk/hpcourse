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

std::tuple<int, int, int, std::string> Utils::parseArguments(int argc, char * argv[])
{
    int n = 10, b = 123, l = 5;
    std::string f = "";

    std::vector<std::string> args(argv, argv + argc);
    for (size_t i = 1; i < args.size(); i++)
    {
        if (args[i] == "-n" && i + 1 < args.size())
        {
            try
            {
                b = std::stoi(args[i + 1]);
                if (n < 0 || n > 100) throw 1;
            }
            catch (const std::exception&)
            {
                std::cerr << "Exception while -n argument parse" << std::endl;
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
                std::cerr << "Exception while -b argument pare" << std::endl;
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
                std::cerr << "Exception while -l argument parse" << std::endl;
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
                std::cerr << "Exception while -f argument parse" << std::endl;
                system("pause");
                exit(0);
            }
        }
    }

    printf("Result of arguments processing. -n:{%d}, -b:{%d}, -l:{%d}, -f:{%s}\n", n, b, l, f.c_str());
    return std::make_tuple(n, l, b, f);
}
