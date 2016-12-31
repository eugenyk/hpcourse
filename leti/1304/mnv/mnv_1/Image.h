#ifndef IMAGE_H
#define IMAGE_H

using Pixel = unsigned char;

class Image
{
public:
    Image();
    Image(int width, int height);
    Image(int width, int height, int id);
    Image(int width, int height, int id, Pixel* picture);
    Image(const Image &img);
    ~Image();

    int getId() const;
    int getWidth() const;
    int getHeight() const;
    Pixel* getPixelMap() const;

    void printPixelMap() const;
    void update(int index, Pixel value);

private:
    static int counter;
    int id;
    int width;
    int height;
    Pixel* picture;
    void fillPixels();
};

#endif // IMAGE_H
