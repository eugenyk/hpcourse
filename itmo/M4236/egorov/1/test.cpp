
#include "task.h"
#include "cassert"

class arange_wrap : public myistream {
	int buf = 0;
public:
	bool operator >> (int& b) override {
		b = ++buf;
		return buf <= 1e6;
	}
};

int main() {
	arange_wrap loc_stream;
	myin = &loc_stream;

    int numThreads = 10000;
    maxSleepMs = 1000;

    int res = run_threads(numThreads);

    assert(res == 1784293664);
    std::cout << "result = " << res << " Test complete." << std::endl;
    return 0;
}
