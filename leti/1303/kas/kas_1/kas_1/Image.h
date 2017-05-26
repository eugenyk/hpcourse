#ifndef IMAGE_H
#define IMAGE_H


class Image
{
public:
    Image();
    Image(int w, int h);
    Image(int w, int h, int id);
    Image(int w, int h, int id, unsigned char *map);
    Image(const Image &img);
    ~Image();

    int getId() const;
    int getWidth() const;
    int getHeight() const;
    unsigned char * getMap() const;

    void printMap() const;
    void updPix(int index, unsigned char value);

private:
    static int counter;
    int id;

    int width;
    int height;

    unsigned char *map;


    void fillPixels();
};

#endif // IMAGE_H
