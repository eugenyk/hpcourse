#include "../include/random.h"


random::random(int min_value, int max_value) {
    long seed = std::chrono::system_clock::now().time_since_epoch().count();
    generator = std::default_random_engine(seed);
    distribution = std::uniform_int_distribution<int>(min_value, max_value);
}

int random::next_int() {
    return distribution(generator);
}


