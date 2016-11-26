#include <functional>
#include <vector>
#include "ImageLineWorker.h"

class Image;
typedef std::vector<std::pair<unsigned int, unsigned int>> ElementSet;
class FinderOnImage : ImageLineWorker<ElementSet, ElementSet> {
    std::function<unsigned char(Image*)> featureToFind;

public:
    FinderOnImage(std::function<unsigned char(Image*)>& featureToFind);
    ElementSet operator()(Image*);
};
