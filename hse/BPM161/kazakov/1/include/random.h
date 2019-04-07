#ifndef UNTITLED_RANDOM_H
#define UNTITLED_RANDOM_H
#include <chrono>
#include <random>


class random {
    private:
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution;

    public:
        random(int min_value, int max_value);

        int next_int();
};

#endif //UNTITLED_RANDOM_H
