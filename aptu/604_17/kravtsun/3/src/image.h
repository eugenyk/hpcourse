#ifndef LAB03_IMAGE_H
#define LAB03_IMAGE_H

#include <functional>
#include <cassert>
#include <cstring>
#include <memory>

using ImageType = unsigned char;

struct Image;

using ImagePtr = std::shared_ptr<Image>;

using ImageConstPtr = std::shared_ptr<const Image>;

struct Image {
    typedef ImageType value_type;
    
    Image()
            : width_(0)
            , height_(0)
            , data_(nullptr)
    {}
    
    Image(size_t width, size_t height)
            : width_(width)
            , height_(height)
            , data_(nullptr)
    {
        data_ = new value_type[width * height];
        init();
    }
    
    Image(const Image &rhs) = delete;
    Image &operator=(const Image &rhs) = delete;
    
    Image(Image &&rhs) noexcept
            : width_(rhs.width_)
            , height_(rhs.height_)
            , data_(rhs.data_)
    {
        rhs.width_ = 0;
        rhs.height_ = 0;
        rhs.data_ = nullptr;
    }
    
    size_t width() const {
        return width_;
    }
    
    size_t height() const {
        return height_;
    }
    
    const value_type *data() const {
        return data_;
    }
    
    ImagePtr clone() const {
        auto result = new Image{width_, height_};
        memcpy(result->data_, data_, width_ * height_ * sizeof(value_type));
        return ImagePtr(result);
    }
    
    void init() {
        map([&](int x, int y) {
            set(x, y, static_cast<value_type>(rand() % 256));
        });
    }
    
    void map(const std::function<void(int, int)> &f) const {
        for (int i = 0; i < height_; ++i) {
            for (int j = 0; j < width_; ++j) {
                f(j, i);
            }
        }
    }
    
    value_type get(int x, int y) const {
        return data_[get_index(x, y)];
    }
    
    void set(int x, int y, value_type value) {
        data_[get_index(x, y)] = value;
    }
    
    virtual ~Image() {
        delete[] data_;
    }
    
    void debug(const std::string &name, bool wait = true) const;
    
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
    value_type *data_;
};

struct ImageHash {
    size_t operator()(const ImageConstPtr &ptr) const {
        size_t result = std::hash<ImageConstPtr>()(ptr);
        return result;
    }
};

#endif //LAB03_IMAGE_H
