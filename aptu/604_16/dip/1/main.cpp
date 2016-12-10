#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <vector>

#include <tbb/flow_graph.h>

using namespace tbb::flow;
using namespace std;

/// Image class that couples data representation and functions specified in the task.

class image {
public:

    // Pixel positions are returned from a handful of functions.

    using pixel_positions = vector<pair<size_t, size_t>>;

    image(size_t width, size_t height) : width_(width),
                                         height_(height),
                                         pixels_(height, vector<size_t>(width)) {
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

    image &invert() {
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
    size_t width_;
    size_t height_;
    vector<vector<size_t>> pixels_;

    const static size_t max_image_value;

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

const size_t image::max_image_value = 255;

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
