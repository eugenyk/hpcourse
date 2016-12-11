#include "random_image.h"
#include <sstream>
#include <iomanip>

using namespace random_image;

RandomImage::RandomImage(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
    , m_source(height, std::vector<int>(width))
{
    initSourceData();
}

RandomImage::RandomImage(const RandomImage& other)
    : m_width(other.m_width)
    , m_height(other.m_height)
    , m_source(other.m_source)
{}

RandomImage::~RandomImage() 
{}

std::string RandomImage::toString() const 
{
    std::stringstream ss;
    for (const auto& line : m_source)
    {
        for (const auto& item : line)
        {
            ss << std::setw(3) << item << " ";
        }
        ss << std::endl;
    }
    
    ss << "max = " << getMaxBrightness() << std::endl;
    ss << "min = " << getMinBrightness() << std::endl;
    
    return ss.str();
}

std::vector<Point> RandomImage::getMaxBrightPoints() const 
{
    int maxValue = getMaxBrightness();
    return getTargetBrightPoints(maxValue);
}

std::vector<Point> RandomImage::getMinBrightPoints() const 
{
    int minValue = getMinBrightness();
    return getTargetBrightPoints(minValue);
}

std::vector<Point> RandomImage::getTargetBrightPoints(int targetValue) const 
{
    std::vector<Point> result;
    for (size_t i = 0; i < m_height; ++i)
    {
        for (size_t j = 0; j < m_width; ++j)
        {
            if (m_source[i][j] == targetValue)
            {
                result.push_back({(int) i, (int) j});
            }
        }
    }
    
    return result;
}

void RandomImage::identifyPoints(const std::vector<Point>& points) 
{
    for (const auto& point : points)
    {
        int start_row = std::max(point.m_row - 1, 0);
        int start_col = std::max(point.m_col - 1, 0);
        for (int i = start_row; i < point.m_row + 1; ++i)
        {
            for (int j = start_col; i < point.m_col + 1; ++j)
            {
                m_source[i][j] = MAX_VALUE;
            }
        }
    }
}

double RandomImage::getMeanBrightness() const 
{
    std::function<int(int,int)> sum = [](int acc, int value) { return acc + value; };
    int globalSum = getBrightness(sum, 0);
    int itemAmount = m_width * m_height;
    
    return itemAmount > 0 ? globalSum / itemAmount : 0.0;
}

void RandomImage::invertBrightness() 
{
    for (auto& line : m_source)
    {
        for (auto& item : line)
        {
            item = MAX_VALUE - item;
        }
    }
}

void RandomImage::initSourceData() 
{
    for (auto& line : m_source)
    {
        for (auto& item : line)
        {
            item = rand() % (MAX_VALUE + 1);
        }
    }
}

int RandomImage::getMaxBrightness() const 
{
    std::function<int(int, int)> max = [](int fst, int snd) { return std::max(fst, snd); };
    return getBrightness(max, 0);
}

int RandomImage::getMinBrightness() const 
{
    std::function<int(int, int)> min = [](int fst, int snd) { return std::min(fst, snd); };
    return getBrightness(min, MAX_VALUE);
}

int RandomImage::getBrightness(std::function<int(int, int)> function, int initValue) const 
{
    int brightness = initValue;
    for (auto& line : m_source)
    {
        for (auto& item : line)
        {
            brightness = function(brightness, item);
        }
    }
    
    return brightness;
}








