#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <vector>

#include <tbb/flow_graph.h>

using namespace tbb::flow;
using namespace std;

// The maximum value that must be stored in an image.

const size_t max_image_value = 255;

// Pixel positions are returned from a handful of functions.

using pixel_positions = vector<pair<size_t, size_t>>;

/// Create a random generator for integers.

std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<size_t> uni(0, max_image_value);

/// rnd_image class that couples data representation and functions specified in the task.

class rnd_image {
public:

    // Fill random image with random values (duh).

    rnd_image(size_t height, size_t width) : height_(height),
                                             width_(width),
                                             pixels_(height, vector<size_t>(width)) {
        for (size_t i = 0; i < height_; ++i) {
            for (size_t k = 0; k < width_; ++k) {
                pixels_[i][k] = uni(rng);
            }
        }
    }

    // Can't use reduce_values_ here because pixels' positions are memorized.

    pixel_positions pixels_with_value(size_t value) const {
        pixel_positions positions;
        for (size_t i = 0; i < height_; ++i) {
            for (size_t k = 0; k < width_; ++k) {
                if (pixels_[i][k] == value) {
                    positions.push_back(make_pair(i, k));
                }
            }
        }
        return positions;
    }

    pixel_positions pixels_with_min_value() const {
        return pixels_with_value(reduce_values_(max_image_value, [](size_t x, size_t y) {
            return min(x, y);
        }));
    }

    pixel_positions pixels_with_max_value() const {
        return pixels_with_value(reduce_values_(0, [](size_t x, size_t y) {
            return max(x, y);
        }));
    }

    rnd_image &invert() {
        for (size_t i = 0; i < height_; ++i) {
            for (size_t k = 0; k < width_; ++k) {
                pixels_[i][k] = max_image_value - pixels_[i][k];
            }
        }
        return *this;
    }

    float mean_value() const {
        return reduce_values_(0, [](size_t sum, size_t value) {
            return sum + value;
        }) / static_cast<float>(height_ * width_);
    }

    void println() const {
        for (size_t i = 0; i < height_; ++i) {
            for (size_t k = 0; k < width_; ++k) {
                cout << pixels_[i][k] << ' ';
            }
            cout << '\n';
        }
    }

private:
    size_t height_;
    size_t width_;
    vector<vector<size_t>> pixels_;

    size_t reduce_values_(size_t start, function<size_t(size_t, size_t)> fn) const {
        size_t reduced_value = start;
        for (size_t i = 0; i < height_; ++i) {
            for (size_t k = 0; k < width_; ++k) {
                reduced_value = fn(reduced_value, pixels_[i][k]);
            }
        }
        return reduced_value;
    }
};

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

    size_t brightness = stoul(args["-b"]);
    size_t limit = stoul(args["-l"]);
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
