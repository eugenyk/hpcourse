#include <functional>
#include <vector>
#include "ImageLineWorker.h"

class Image;

typedef std::vector<std::pair<unsigned int, unsigned int>> ElementSet;
typedef tuple<ElementSet, unsigned long> ElementSetTuple;

/// Class to find special pixels on image.
class FinderOnImage : ImageLineWorker<ElementSetTuple, ElementSet> {

    /// Feature to detect pixels to find.
    std::function<unsigned char(Image*)> featureToFind;

public:
    FinderOnImage(std::function<unsigned char(Image*)>& featureToFind);
    ElementSetTuple operator()(Image*);
};
