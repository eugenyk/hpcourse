#include <stdio.h>
#include "Pixel.h"

namespace labutils {
    int getBrightness(Pixel* pixel) {
        return 0.3 * (double)pixel->getR() + 0.59 * (double)pixel->getG() + 0.11 * (double)pixel->getB();
    }
    
    void printPixelWithMetadata(std::pair<Pixmap*, std::pair<int, int>> pair) {
        Pixmap* pixmap = std::get<0>(pair);
        int i = std::get<0>(std::get<1>(pair));
        int j = std::get<1>(std::get<1>(pair));
        
        Pixel* pixel = pixmap->get(i, j);
        
        std::cout << "(" << i << ", " << j << ") = " << labutils::getBrightness(pixel) << std::endl;
    }
}

