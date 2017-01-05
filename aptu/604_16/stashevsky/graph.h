#pragma once

#include <tbb/concurrent_vector.h>

tbb::concurrent_vector<double>
Measure(size_t height, size_t width, size_t matrix_count, int equals_value, size_t max_images = 4);
