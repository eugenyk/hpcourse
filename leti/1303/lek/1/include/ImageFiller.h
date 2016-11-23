#include <tbb/flow_graph.h>
class Image;

using namespace tbb::flow;

/// Class for filling one image with random colors.
class ImageFiller {
    /// Fill one line of image.
    unsigned char* fillImageLine(tuple<unsigned char*, unsigned int> lineInfo);
public:
    /// Main action - fill image with different colors.
    Image *operator()(Image* image);
};
