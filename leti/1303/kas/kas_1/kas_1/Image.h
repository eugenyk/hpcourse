#pragma once
class Image
{
public:
	Image();
	Image(int w, int h);
	Image(int w, int h, int id);
	Image(int w, int h, int id, unsigned char *map);
	Image(const Image &img);
	~Image();

	int getWidth() const;
	int getHeight() const;
	unsigned char * getMap() const;

	void printMap() const;
	void updPix(int index, unsigned char value);

	std::vector<Image*> divByParts(int numParts);

private:
	int id;

	int width;
	int height;

	unsigned char *map;


	void fillPixels();
};

