#include "ImageHighlighter.h"
#include <functional>

/// Struct for changing color of pixels in line.
struct ColorChanger {
    /// Map with pixels colors.
    ColorMap pixelColors;

    ColorChanger(const ColorMap& pixelColors) : pixelColors(pixelColors) {}

    /// Change color.
    unsigned char *operator()(tuple<unsigned char*, unsigned int, unsigned int> lineInfo) {
        unsigned char * line = get<0>(lineInfo);
        for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
            // Change color if it's nessecary.
            auto it = pixelColors.find(std::make_pair(get<2>(lineInfo), i));
            if (it != pixelColors.end()) {
                line[i] = it->second;
            }
        }
        return line;
    }
};

Image *ImageHighlighter::operator()(tuple<ColorMap, Image*> imageAndChanges) {
    ColorMap pixelColors = get<0>(imageAndChanges);
    Image *image = get<1>(imageAndChanges);

    // Highlight found pixels with graph.
    runTaskOnGraph(image, ColorChanger(pixelColors),
        [](unsigned char*)->continue_msg { return continue_msg(); });

    return image;
}
