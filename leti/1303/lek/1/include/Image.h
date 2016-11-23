#ifndef IMAGE_H
#define IMAGE_H

/// Class of image.
class Image {
    // Width of image.
    unsigned int width;
    // Height of image.
    unsigned int height;
    // Matrix with pixels.
    unsigned char **pixelMatrix;
public:
    Image(unsigned int width, unsigned int height);
    ~Image();

    unsigned int getWidth();
    unsigned int getHeight();
    /// Get separate line of image by its number.
    unsigned char* getImageLine(unsigned int lineNumber);
};

#endif
