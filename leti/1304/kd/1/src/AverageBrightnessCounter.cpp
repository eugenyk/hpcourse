#include "AverageBrightnessCounter.h"
#include <functional>

/// Struct for getting sum.
struct Summator {
    /// Result sum.
    unsigned long long& sum;

    Summator(unsigned long long& sum) : sum(sum) {}

    /// Sum new value.
    continue_msg operator()(unsigned long long partOfSum) {
        sum += partOfSum;
        return continue_msg();
    }
};

unsigned long long AverageBrightnessCounter::summaryBrightness(
                tuple<unsigned char*, unsigned int, unsigned int> lineInfo) {
    unsigned char * line = get<0>(lineInfo);
    unsigned long long sum = 0;
    // Sum all pixels values in line.
    for (unsigned int i = 0; i < get<1>(lineInfo); i++) {
        sum += line[i];
    }
    return sum;
}

tuple<Image*, double> AverageBrightnessCounter::operator()(Image * image)
{
    unsigned long long sum = 0;
    runTaskOnGraph(image, std::bind(&AverageBrightnessCounter::summaryBrightness, this, std::placeholders::_1),
        Summator(sum));

    return tuple<Image*, double>(image, (double)sum / (image->getHeight() * image->getWidth()));
}
