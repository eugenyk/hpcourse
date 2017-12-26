#include <iostream>
#include <mutex>
#include "image.h"



#if HAVE_OPENCV
std::mutex show_mutex;
#endif

void Image::debug(const std::string &name, bool wait) const {
#if HAVE_OPENCV
    std::unique_lock<std::mutex> show_lock{show_mutex};
    auto mat = cv::Mat(static_cast<int>(height_), static_cast<int>(width_), CV_8UC1, data_);
    cv::imshow(name, mat);
    if (wait) {
        int key = cv::waitKey(0) % 256;
        if (key == 27) {
            exit(0);
        }
    }
#else
    std::cout << name << ": " << std::endl;
    for (int i = 0; i < height_; ++i) {
        for (int j = 0; j < width_; ++j) {
            std::cout << get(j, i) << ' ';
        }
        std::cout << '\n';
    }
    std::cout << std::endl;
#endif
}
