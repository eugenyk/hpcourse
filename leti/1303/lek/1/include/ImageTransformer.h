#include <map>
#include "FinderOnImage.h"

typedef tuple<ElementSetTuple, ElementSetTuple, ElementSetTuple, Image*> FoundElementsTuple;
typedef std::multimap<std::pair<unsigned int, unsigned int>, unsigned char> ColorMultiMap;
typedef std::map<std::pair<unsigned int, unsigned int>, unsigned char> ColorMap;

/// Class for image transformation.
class ImageTransformer {
    /// Count colors for border pixels.
    tuple<ColorMap, Image*> countColor(FoundElementsTuple elementsSet);
    /// Get image id the found set connected with.
    unsigned long getId(ElementSetTuple setTuple);
    /// Calculate right borders colors.
    void setBordersColor(const ElementSet& elementSet, unsigned char color,
                         ColorMultiMap& pixelColors, const Image* image);
public:
    /// Transform image.
    void transform(std::vector<Image*> images, size_t imagesLimit,
                   const char* fileName);
};
