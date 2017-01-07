#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include "DummyImageInputStream.h"
#include "Parameters.h"
#include "Algs.h"
#include "tbb/flow_graph.h"

class ImageProcessor
{
	std::vector<tbb::flow::graph_node*> nodes;
	std::vector<tbb::flow::receiver< std::shared_ptr<Image> >*> inputs;
	std::string logFilename;
	tbb::flow::graph g;
	tbb::flow::function_node<unsigned char, bool> * loggerNode;
	DummyImageInputStream & in;

	void buildProcessingBranch();

public:
	ImageProcessor(DummyImageInputStream & in, int numThreads = 1, const std::string & logFilename = "");
	~ImageProcessor();

	void start();
};

#endif 
