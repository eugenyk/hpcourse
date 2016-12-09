#include <cassert>
#include <fstream>
#include <iostream>
#include <map>

#include <tbb/flow_graph.h>

using namespace tbb::flow;

int main(int argc, char *argv[]) {

    // Program can't recover if it receives an odd number of args.

    assert((argc - 1) % 2 == 0);

    ///
    /// Parse program's arguments.
    ///

    std::map<std::string, std::string> args;

    for (std::size_t i = 0; i < (argc - 1) / 2; ++i) {
        args[argv[(i * 2) + 1]] = argv[(i * 2) + 1 + 1];
    }

    int brightness = std::stoi(args["-b"]);
    int limit = std::stoi(args["-l"]);
    std::ofstream log_file;

    log_file.open(args["-f"]);

    // Program can't recover if the log file can't be opened.

    if (!log_file.is_open()) {
        std::cout << "Failed to open the log file.\n";
        return 1;
    }

    std::cout << "Received arguments:\n";
    std::cout << " Brightness value: " << brightness << '\n';
    std::cout << " Concurrency limit: " << limit << '\n';
    std::cout << " Log file path: " << args["-f"] << '\n';

    return 0;
}
