#include "ImageProcessor.h"
#include "DummyImageInputStream.h"
#include "Image.h"
#include "Parameters.h"
#include "ParmParser.h"


int main(int argc, char ** argv)
{
	srand(time(0));

	ParmParser parser;
	Parameters parms = parser.parse(argc, argv);
	if (!parms)
		return 1;

	// will produce a specified number of random images
	DummyImageInputStream in(parms.reqImageCount);

	ImageProcessor processor(in, parms.parallelLimit, parms.logFilename);
	processor.start();

	return 0;
}