#include "FinderOnImage.h"


struct FinderElementsInLine {
    unsigned char compareValue;
    FinderElementsInLine(unsigned char compareValue) : compareValue(compareValue) {}

    ElementSet operator()(tuple<unsigned char*, unsigned int, unsigned int> lineInfo)
    {
        ElementSet elements;
        unsigned char * line = get<0>(lineInfo);
        for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
            if (line[i] == compareValue) {
                elements.emplace_back(get<2>(lineInfo), i);
            }
        }
        return elements;
    }
};

struct ElementsCollector {
    ElementSet& resultSet;
    ElementsCollector(ElementSet& set) : resultSet(set) {}
    continue_msg operator()(ElementSet set) {
        resultSet.insert(resultSet.begin(), set.begin(), set.end());
        return continue_msg();
    }
};


FinderOnImage::FinderOnImage(std::function<unsigned char(Image*)>& featureToFind) :
    featureToFind(featureToFind) {}

ElementSet FinderOnImage::operator()(Image *image) {
    ElementSet result;
    unsigned int valueToFind = featureToFind(image);
    runTaskOnGraph(image, FinderElementsInLine(valueToFind), ElementsCollector(result));
    return result;
}
