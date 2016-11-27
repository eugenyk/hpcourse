#include "FinderOnImage.h"

/// Struct for finding special elements in line.
struct FinderElementsInLine {
    /// Value to find.
    unsigned char compareValue;

    FinderElementsInLine(unsigned char compareValue) : compareValue(compareValue) {}

    /// Find all elements with set value.
    ElementSet operator()(tuple<unsigned char*, unsigned int, unsigned int> lineInfo)
    {
        ElementSet elements;
        unsigned char * line = get<0>(lineInfo);
        // Check for all pixels if they have such color value.
        for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
            if (line[i] == compareValue) {
                elements.emplace_back(get<2>(lineInfo), i);
            }
        }
        return elements;
    }
};

/// Struct for collecting found elements.
struct ElementsCollector {
    /// Set for collection.
    ElementSet& resultSet;

    ElementsCollector(ElementSet& set) : resultSet(set) {}

    /// Put found set into result.
    continue_msg operator()(ElementSet set) {
        resultSet.insert(resultSet.begin(), set.begin(), set.end());
        return continue_msg();
    }
};

FinderOnImage::FinderOnImage(std::function<unsigned char(Image*)>& featureToFind) :
    featureToFind(featureToFind) {}

ElementSetTuple FinderOnImage::operator()(Image *image) {
    ElementSet result;
    // Calculate value of color to find on image.
    unsigned int valueToFind = featureToFind(image);
    // Find pixels with such value using graph.
    runTaskOnGraph(image, FinderElementsInLine(valueToFind), ElementsCollector(result));
    return ElementSetTuple(result, image->getId());
}
