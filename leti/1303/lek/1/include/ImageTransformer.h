#include <map>
#include "FinderOnImage.h"

typedef tuple<ElementSetTuple, ElementSetTuple, ElementSetTuple, Image*> FoundElementsTuple;
typedef std::multimap<std::pair<unsigned int, unsigned int>, unsigned char> ColorMultiMap;
typedef std::map<std::pair<unsigned int, unsigned int>, unsigned char> ColorMap;
class ImageTransformer {
    
    tuple<ColorMap, Image*> countColor(FoundElementsTuple elementsSet);
    unsigned long getId(ElementSetTuple setTuple);
    void setBordersColor(const ElementSet& elementSet, unsigned char color,
        ColorMultiMap& pixelColors, Image* image);
public:
    void transform(std::vector<Image*> images, unsigned int imagesLimit, const char* fileName);
};
