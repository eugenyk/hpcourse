#include "ImageProcessor.h"

using namespace tbb::flow;

ImageProcessor::ImageProcessor(DummyImageInputStream & in, int numThreads, const std::string & logFilename)
	: in(in), logFilename(logFilename)
{
	if (!logFilename.empty())
	{
		loggerNode = new function_node<unsigned char, bool>(g, 1, LogWriter(logFilename));
		nodes.push_back(loggerNode);
	}

	for (int i = 0; i < numThreads; i++)
		buildProcessingBranch();
}

ImageProcessor::~ImageProcessor()
{
	for (auto node : nodes)
		delete node;
}

void ImageProcessor::start()
{
	std::shared_ptr<Image> image;

	for (int i = 0; in >> image; i++)
	{
		if (i == inputs.size())
		{
			i = 0;
			g.wait_for_all();
		}

		inputs[i]->try_put(image);
	}

	g.wait_for_all();
}

void ImageProcessor::buildProcessingBranch()
{
	// let's define some local types to simplify the statements below
	typedef std::shared_ptr<Image> Image;
	typedef std::vector<std::tuple<unsigned char, unsigned char>> L1Output;
	typedef std::tuple<L1Output, L1Output, L1Output, Image> L1Result;
	typedef std::tuple<unsigned char, unsigned char> MinMaxPair;

	// build the first level of G
	auto * input = new broadcast_node<Image>(g);
	auto * min = new function_node<Image, L1Output>(g, 1, MinBrightness());
	auto * max = new function_node<Image, L1Output>(g, 1, MaxBrightness());
	auto * equ = new function_node<Image, L1Output>(g, 1, EquBrightness(0));

	make_edge(*input, *min);
	make_edge(*input, *max);
	make_edge(*input, *equ);

	auto * join = new join_node<L1Result, queueing>(g);

	make_edge(*min, std::get<0>(join->input_ports()));
	make_edge(*max, std::get<1>(join->input_ports()));
	make_edge(*equ, std::get<2>(join->input_ports()));
	make_edge(*input, std::get<3>(join->input_ports()));

	// build the second level of G
	auto * inv = new function_node<Image, Image>(g, 1, Inverse());
	auto * avg = new function_node<MinMaxPair, unsigned char>(g, 1, Average());

	auto * j2i = new function_node<L1Result, Image>(g, 1, ToImageConverter());
	auto * j2a = new function_node<L1Result, MinMaxPair>(g, 1, ToMinMaxConverter());

	make_edge(*join, *j2i);
	make_edge(*join, *j2a);
	make_edge(*j2i, *inv);
	make_edge(*j2a, *avg);

	// if logging was enabled, connect the logger with the avg node
	if (!logFilename.empty())
		make_edge(*avg, *loggerNode);

	// only input nodes will be used in the start phase of processing, collect them
	inputs.push_back(input);

	// the branch is ready; save all created nodes for delete later
	nodes.push_back(input);
	nodes.push_back(min);
	nodes.push_back(max);
	nodes.push_back(equ);
	nodes.push_back(join);
	nodes.push_back(avg);
	nodes.push_back(inv);
	nodes.push_back(j2i);
	nodes.push_back(j2a);
}
