#ifndef RANDOM_IMAGE_H
#define RANDOM_IMAGE_H

#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <functional>

namespace random_image
{
    const int MAX_VALUE = 255;
    
    struct Point
    {
        int m_row;
        int m_col;
    };
    
    class RandomImage 
    {

    public:
        RandomImage(size_t width, size_t height);
        RandomImage(const RandomImage& other);
        virtual ~RandomImage();
        
    public:
        std::vector<Point> getMaxBrightPoints() const;
        std::vector<Point> getMinBrightPoints() const;
        std::vector<Point> getTargetBrightPoints(int targetValue) const;
        void identifyPoints(const std::vector<Point>& points);
        double getMeanBrightness() const;
        void invertBrightness();
        
        std::string toString() const;
        
    private:
        void initSourceData();
        int getMaxBrightness() const;
        int getMinBrightness() const;
        int getBrightness(std::function<int(int, int)> function, int initValue) const;

    private:
        size_t                        m_width;
        size_t                        m_height;
        std::vector<std::vector<int>> m_source;
    };

}

#endif /* RANDOM_IMAGE_H */

