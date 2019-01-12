
#include "task.h"
#include <unistd.h>
#include <cstdlib>

class cinwrap : public myistream {
public:
	bool operator >> (int& b) override {
		if (std::cin >> b) {
			return true;
		}
		return false;
	}
};




int main(int argc, char* argv[]) {
	cinwrap loc_stream;
	myin = &loc_stream;

    int numThreads = std::atoi(argv[1]);

    maxSleepMs = std::atoi(argv[2]);
    std::cout << run_threads(numThreads) << std::endl;
    return 0;
}
