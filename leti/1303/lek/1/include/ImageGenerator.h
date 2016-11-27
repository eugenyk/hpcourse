#include <tbb/concurrent_vector.h>
#include <tbb/flow_graph.h>
#include <random>
class Image;

using namespace tbb::flow;

/// Class for generating random images.
class ImageGenerator {

    unsigned int minWidth;
    unsigned int minHeight;
    unsigned int maxWidth;
    unsigned int maxHeight;
public:
    ImageGenerator(unsigned int minWidth, unsigned int minHeight,
                   unsigned int maxWidth, unsigned int maxHeight);

    /// Generate random image.
    Image* generate() const;

};
