//
// Created by wimag on 12.12.16.
//

#ifndef TBB_IMAGE_H
#define TBB_IMAGE_H

#include <vector>

namespace std{
    typedef pair<size_t, size_t> point;

    class image {
    private:
        std::size_t n, m, window_size;
        vector<char> data;
        void tryset(int x, int y, char value);
    public:
        image(size_t n, size_t m);
        image(size_t n, size_t m, size_t window_size);
        image();
        image(const image& other);
        
        char& get(size_t x, size_t y);
        char& get(point p);
        void set(size_t x, size_t y, char value);
        void set(point p, char value);

        vector<point> max_points() const;
        vector<point> min_points() const;
        vector<point> exact_points(char v) const;
        double average() const;
        void invert();
        void highlight_point(point p);
    };
}


#endif //TBB_IMAGE_H
