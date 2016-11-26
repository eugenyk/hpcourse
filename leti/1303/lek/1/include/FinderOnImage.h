#include <functional>
#include <vector>
#include "ImageLineWorker.h"

class Image;
typedef std::vector<std::pair<unsigned int, unsigned int>> ElementSet;
typedef tuple<ElementSet, unsigned long> ElementSetTuple;
class FinderOnImage : ImageLineWorker<ElementSetTuple, ElementSet> {
    std::function<unsigned char(Image*)> featureToFind;

public:
    FinderOnImage(std::function<unsigned char(Image*)>& featureToFind);
    ElementSetTuple operator()(Image*);
};
