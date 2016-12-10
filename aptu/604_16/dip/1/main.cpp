#include <cassert>
#include <fstream>
#include <iostream>
#include <map>

#include <tbb/flow_graph.h>

using namespace tbb::flow;
using namespace std;

int main(int argc, char *argv[]) {

    // Program can't recover if it receives an odd number of args.

    assert((argc - 1) % 2 == 0);

    ///
    /// Parse program's arguments.
    ///

    map<string, string> args;

    for (size_t i = 0; i < (argc - 1) / 2; ++i) {
        args[argv[(i * 2) + 1]] = argv[(i * 2) + 1 + 1];
    }

    int brightness = stoi(args["-b"]);
    int limit = stoi(args["-l"]);
    ofstream log_file;

    log_file.open(args["-f"]);

    // Program can't recover if the log file can't be opened.

    if (!log_file.is_open()) {
        cout << "Failed to open the log file.\n";
        return 1;
    }

    cout << "Received arguments:\n";
    cout << " Brightness value: " << brightness << '\n';
    cout << " Concurrency limit: " << limit << '\n';
    cout << " Log file path: " << args["-f"] << '\n';

    return 0;
}
