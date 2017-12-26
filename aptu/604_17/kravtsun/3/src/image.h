#ifndef LAB03_IMAGE_H
#define LAB03_IMAGE_H

#include <functional>
#include <cassert>
#include <cstring>

template<typename T=unsigned char>
struct ImageT {
    typedef T value_type;
    
    ImageT()
            : width_(0)
            , height_(0)
            , data_(nullptr)
    {}
    
    ImageT(size_t width, size_t height)
            : width_(width)
            , height_(height)
            , data_(nullptr)
    {
        data_ = new T[width * height];
        init();
    }
    
    ImageT(const ImageT &rhs) = delete;
    ImageT &operator=(const ImageT &rhs) = delete;
    
    ImageT(ImageT &&rhs) noexcept
            : width_(rhs.width_)
            , height_(rhs.height_)
            , data_(rhs.data_)
    {
        rhs.width_ = 0;
        rhs.height_ = 0;
        rhs.data_ = nullptr;
    }
    
    ImageT clone() const {
        ImageT result{width_, height_};
        memcpy(result.data_, data_, width_ * height_ * sizeof(value_type));
        return result;
    }
    
    void init() {
        map([&](int x, int y) {
            set(x, y, static_cast<char>(rand() % 256));
        });
    }
    
    void map(const std::function<void(int, int)> &f) const {
        for (int i = 0; i < height_; ++i) {
            for (int j = 0; j < width_; ++j) {
                f(j, i);
            }
        }
    }
    
    T get(int x, int y) const {
        return data_[get_index(x, y)];
    }
    
    void set(int x, int y, T value) {
        data_[get_index(x, y)] = value;
    }
    
    virtual ~ImageT() {
        delete[] data_;
    }

private:
    void check_index(int x, int y) const {
        assert(x >= 0 && x < width_);
        assert(y >= 0 && y < height_);
    }
    
    inline size_t get_index(int x, int y) const {
        check_index(x, y);
        return y * width_ + x;
    }
    
    size_t width_, height_;
    T *data_;
};

using Image = ImageT<unsigned char>;

#endif //LAB03_IMAGE_H
