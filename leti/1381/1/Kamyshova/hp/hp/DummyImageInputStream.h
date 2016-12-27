#ifndef DUMMY_IMAGE_INPUT_STREAM_H
#define DUMMY_IMAGE_INPUT_STREAM_H

#include <memory>

class Image;

class DummyImageInputStream
{
	friend DummyImageInputStream & operator >> (DummyImageInputStream & in, std::shared_ptr<Image> & image);

	int numGenerations;
	int currGeneration;

public:
	DummyImageInputStream(int numGenerations);

	operator bool();
};

DummyImageInputStream & operator >> (DummyImageInputStream & in, std::shared_ptr<Image> & image);

#endif