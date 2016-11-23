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
    
    /// Collection of generated images.
    tbb::concurrent_vector<Image*> imagesCollection;

    /// Add generated image to collection.
    continue_msg addImage(Image* image);
public:
    ImageGenerator(unsigned int minWidth, unsigned int minHeight,
                   unsigned int maxWidth, unsigned int maxHeight);

    /// Generate current number of random images.
    tbb::concurrent_vector<Image*> generate(unsigned int count);

};
