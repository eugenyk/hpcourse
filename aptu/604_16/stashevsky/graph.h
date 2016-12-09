#pragma once

#include <tbb/concurrent_vector.h>

tbb::concurrent_vector<double>
Measure(size_t height, size_t width, size_t matrix_count, size_t equals_value, size_t concurrency = 4);
