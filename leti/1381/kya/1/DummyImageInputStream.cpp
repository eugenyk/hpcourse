#include "DummyImageInputStream.h"
#include "Image.h"

DummyImageInputStream::DummyImageInputStream(int numGenerations)
{
	this->numGenerations = numGenerations;
	currGeneration = 0;
}

DummyImageInputStream::operator bool()
{
	return currGeneration <= numGenerations;
}

DummyImageInputStream & operator >> (DummyImageInputStream & in, std::shared_ptr<Image> & image)
{
	in.currGeneration++;

	if (!in)
		return in;

	image = std::shared_ptr<Image>( new Image() );
	return in;
}
