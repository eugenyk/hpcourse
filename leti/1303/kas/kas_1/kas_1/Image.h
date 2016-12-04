#pragma once
class Image
{
public:
	Image();
	Image(int w, int h);
	Image(int w, int h, int id);
	Image(const Image &img);
	~Image();

	int getWidth() const;
	int getHeight() const;
	unsigned char * getMap() const;

	void printMap() const;
	void updPix(int index, unsigned char value);

private:
	int id;

	int width;
	int height;

	unsigned char *map;


	void fillPixels();
};

