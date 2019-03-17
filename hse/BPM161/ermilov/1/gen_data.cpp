#include <iostream>
#include <cassert>
#include <random>

int main(int argc, const char** argv) {
    assert(argc == 2);
    int N = atoi(argv[1]);
    assert(N >= 0);

    int64_t sum = 0;

    std::random_device rd;
    std::mt19937 rnd(rd());
    std::uniform_int_distribution<> distr(-100, 100);
    for (int i = 0; i < N; i++) {
        int val = distr(rnd);
        sum += val;
        std::cout << val << ' ';
    }
    std::cout << '\n';
    std::cerr << sum << '\n';

    return 0;
}
