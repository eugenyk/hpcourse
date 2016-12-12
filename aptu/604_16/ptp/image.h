
#include <vector>
#include <utility>
#include <memory>


struct image {

    image(size_t height, size_t width, std::unique_ptr<int[]> data, size_t id = 0);

    int at(size_t i, size_t j) const;
    int& at(size_t i, size_t j);
    void set(size_t i, size_t j, int value);

    std::vector<std::pair<size_t, size_t>> min_positions() const;
    std::vector<std::pair<size_t, size_t>> max_positions() const;
    std::vector<std::pair<size_t, size_t>> positions(size_t value) const;
    float mean() const;
    size_t get_id() const;

    void square_around(size_t i, size_t j, int value, size_t shift = 2);

    std::shared_ptr<image> inverse() const;

private:

    std::vector<std::pair<size_t, size_t>> min_positions(int multiplier) const;

    size_t id_;
    size_t height_;
    size_t width_;
    std::unique_ptr<int[]> data_;

};
