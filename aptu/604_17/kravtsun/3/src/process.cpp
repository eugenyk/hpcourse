#include <fstream>
#include <mutex>

#include "process.h"
#include "common.h"

#define DEBUG_INVERSE 0
#define DEBUG_HIGHLIGHT 0

ImageHighlighter::ImageHighlighter(size_t radius)
        : radius_(radius)
{}

#if DEBUG_HIGHLIGHT
static std::mutex highlight_mutex;
#endif

ImagePtr ImageHighlighter::operator()(ImageHighlighter::input_type image_and_positions) {
    auto image = std::get<0>(image_and_positions);
    LOG("enter highlighter for image with hash: " + std::to_string(ImageHash()(image)));
    auto &min_positions = std::get<static_cast<int>(ImageElementFinderType::MIN)>(image_and_positions);
    auto &max_positions = std::get<static_cast<int>(ImageElementFinderType::MAX)>(image_and_positions);
    auto &exact_positions = std::get<static_cast<int>(ImageElementFinderType::EXACT)>(image_and_positions);
    
    if (image->width() * image->height() == 0) {
        return image;
    }

#if DEBUG_HIGHLIGHT
    std::unique_lock<std::mutex> highlight_lock(highlight_mutex);
    image->debug("input", false);
#endif
    
    PositionsList positions;
    for (auto const &v : {min_positions, max_positions, exact_positions}) {
        std::copy(v.cbegin(), v.cend(), std::back_inserter(positions));
    }
    
    std::for_each(positions.cbegin(), positions.cend(), [&](const Position &pos) {
        const int posx = pos.first;
        const int posy = pos.second;
        const int width_start = std::max(posx - static_cast<int>(radius_), 0);
        const int width_limit = static_cast<const int>(std::min(posx + radius_ + 1, image->width()));
    
        const int height_start = std::max(posy - static_cast<int>(radius_), 0);
        const int height_limit = static_cast<const int>(std::min(posy + radius_ + 1, image->height()));
    
        const ImageType hightlight_value = std::numeric_limits<ImageType>::max();
        for (int y = height_start; y < height_limit; ++y) {
            for (int x = width_start; x < width_limit; ++x) {
                image->set(x, y, hightlight_value);
            }
        }
    });

#if DEBUG_HIGHLIGHT
    image->debug("output", true);
#endif
    
    return image;
}

#if DEBUG_INVERSE
static std::mutex inverse_mutex;
#endif
ImagePtr ImageInverser::operator()(const ImageConstPtr &image) {
#if DEBUG_INVERSE
    std::unique_lock<std::mutex> show_pair_lock(inverse_mutex);
    image->debug("input", false);
#endif
    auto image_ptr = image->clone();
    image_ptr->map([&](int x, int y) {
        auto value = image_ptr->get(x, y);
        image_ptr->set(x, y, std::numeric_limits<Image::value_type>::max() - value);
    });
#if DEBUG_INVERSE
    image_ptr->debug("output", true);
#endif
    return image_ptr;
}

MeanBrightnessCalculator::MeanBrightnessCalculator(const std::string &filename)
        : filename_(filename)
{}

MeanBrightnessCalculator::result_type MeanBrightnessCalculator::operator()(ImageConstPtr image) {
    uint64_t s = 0;
    image->map([&](int x, int y) {
        s += image->get(x, y);
    });
    
    auto result = static_cast<result_type>(s) / (image->width() * image->height());
    write(result);
    
    LOG("Mean result: " + std::to_string(result)
//                + "for image with hash: " + std::to_string(ImageHash()(image))
    );
    return result;
}

static std::mutex mean_brightness_write_mutex;
void MeanBrightnessCalculator::write(MeanBrightnessCalculator::result_type s) {
    std::unique_lock<std::mutex> write_lock{mean_brightness_write_mutex};
    if (filename_.empty()) {
        return;
    }
    std::ofstream fout(filename_, std::ios_base::app);
    fout << s << std::endl;
}
