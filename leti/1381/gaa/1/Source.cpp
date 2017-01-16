#include <iostream>
#include <vector>
#include <fstream>
#include <windows.h>
#include <ctime>

#include "tbb/flow_graph.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

using namespace std;
using namespace tbb::flow;

struct Position
{
	int w;
	int h;
	int res;
};

class ImageRandom {
public:
	std::vector<int> imageP;
	int width; //ширина
	int height; //высота

	ImageRandom() {
		width = height = 0;
	};
	ImageRandom(int NEWwidth, int NEWheight) {
		//std::srand(std::time(0));
		Position newPosition;
		width = NEWwidth;
		height = NEWheight;
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				imageP.push_back(rand() % 256);
			}
		}
	};
	ImageRandom(ImageRandom* image) {
		Position newPosition;
		width = image->width;
		height = image->height;
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				imageP.push_back(image->imageP[h*width + w]);
			}
		}
	};
	ImageRandom(const ImageRandom & image) {
		Position newPosition;
		width = image.width;
		height = image.height;
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				imageP.push_back(image.imageP[h*width + w]);
			}
		}
	};

	~ImageRandom() {
	};

	void OriginalImage() {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) std::cout << imageP[width*h + w] << " ";
			std::cout << std::endl;
		}
	};
	std::vector<Position> RfindMIN() const {
		std::vector<Position> num;
		Position pos;
		int minimum = 255;
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (imageP[width*h + w] < minimum) minimum = imageP[width*h + w];
			}
		}
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (imageP[width*h + w] == minimum) {
					pos.w = w;
					pos.h = h;
					pos.res = minimum;
					num.push_back(pos);
				}
			}
		}

		return num;
	};
	std::vector<Position> RfindMAX() const {
		std::vector<Position> num;
		Position pos;
		int maximum = 0;
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (imageP[width*h + w] > maximum) maximum = imageP[width*h + w];
			}
		}
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (imageP[width*h + w] == maximum) {
					pos.w = w;
					pos.h = h;
					pos.res = maximum;
					num.push_back(pos);
				}
			}
		}

		return num;
	};
	std::vector<Position> RfindSPEC(int spec) const {
		std::vector<Position> num;
		Position pos;
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (imageP[width*h + w] == spec) {
					pos.w = w;
					pos.h = h;
					pos.res = spec;
					num.push_back(pos);
				}
			}
		}
		return num;
	};

	void selectPixels(std::vector<Position> numPixels) {
		for (int i = 0; i < numPixels.size(); i++) {
			if ((numPixels.at(i).w - 1 >= 0) && (numPixels.at(i).w - 1 < width) && (numPixels.at(i).h - 1 >= 0) && (numPixels.at(i).h - 1 < height))
				imageP[numPixels.at(i).w - 1 + width*(numPixels.at(i).h - 1)] = numPixels.at(i).res;
			if ((numPixels.at(i).w - 1 >= 0) && (numPixels.at(i).w - 1 < width) && (numPixels.at(i).h >= 0) && (numPixels.at(i).h < height))
				imageP[numPixels.at(i).w - 1 + width*(numPixels.at(i).h)] = numPixels.at(i).res;
			if ((numPixels.at(i).w - 1 >= 0) && (numPixels.at(i).w - 1 < width) && (numPixels.at(i).h + 1 >= 0) && (numPixels.at(i).h + 1 < height))
				imageP[numPixels.at(i).w - 1 + width*(numPixels.at(i).h + 1)] = numPixels.at(i).res;

			if ((numPixels.at(i).w >= 0) && (numPixels.at(i).w < width) && (numPixels.at(i).h - 1 >= 0) && (numPixels.at(i).h - 1 < height))
				imageP[numPixels.at(i).w + width*(numPixels.at(i).h - 1)] = numPixels.at(i).res;
			if ((numPixels.at(i).w >= 0) && (numPixels.at(i).w < width) && (numPixels.at(i).h + 1 >= 0) && (numPixels.at(i).h + 1 < height))
				imageP[numPixels.at(i).w + width*(numPixels.at(i).h + 1)] = numPixels.at(i).res;

			if ((numPixels.at(i).w + 1 >= 0) && (numPixels.at(i).w + 1< width) && (numPixels.at(i).h - 1 >= 0) && (numPixels.at(i).h - 1 < height))
				imageP[numPixels.at(i).w + 1 + width*(numPixels.at(i).h - 1)] = numPixels.at(i).res;
			if ((numPixels.at(i).w + 1 >= 0) && (numPixels.at(i).w + 1< width) && (numPixels.at(i).h >= 0) && (numPixels.at(i).h < height))
				imageP[numPixels.at(i).w + 1 + width*(numPixels.at(i).h)] = numPixels.at(i).res;
			if ((numPixels.at(i).w + 1 >= 0) && (numPixels.at(i).w + 1< width) && (numPixels.at(i).h + 1 >= 0) && (numPixels.at(i).h + 1 < height))
				imageP[numPixels.at(i).w + 1 + width*(numPixels.at(i).h + 1)] = numPixels.at(i).res;
		}
	};
	void inversion() {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) imageP[width*h + w] = 255 - imageP[width*h + w];
		}
	};
	double mean() const {
		double sum = 0;
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) sum += imageP[width*h + w];
		}
		sum = sum / (height * width);
		return sum;
	};
};

class Parameters {
public:
	int b;
	int l;
	std::string f;
	int n;
	int w;
	int h;
	Parameters() {
		b = 13;
		l = 4;
		f = "./log.txt";
		n = 4;
		w = 4;
		h = 4;
	};
	void setParameters(int argc, char *argv[]) {
		for (int i = 1; i < argc; i++)
		{
			if (!std::strcmp(argv[i], "-b") && i + 1 < argc)
			{
				b = std::atoi(argv[++i]);
			}
			else if (!std::strcmp(argv[i], "-l") && i + 1 < argc)
			{
				l = std::atoi(argv[++i]);
			}
			else if (!std::strcmp(argv[i], "-f") && i + 1 < argc)
			{
				f = argv[++i];
			}
			else if (!std::strcmp(argv[i], "-n") && i + 1 < argc)
			{
				n = std::atoi(argv[++i]);
			}
			else if (!std::strcmp(argv[i], "-h") && i + 1 < argc)
			{
				h = std::atoi(argv[++i]);
			}
			else if (!std::strcmp(argv[i], "-w") && i + 1 < argc)
			{
				w = std::atoi(argv[++i]);
			}
		}
		if (b > 255) b = 255;
		if (b < 0) b = 0;
		if (l > 4) l = 4;
		if (l < 1) l = 1;
		if (n < 1) n = 1;
		if (w < 1) w = 1;
		if (h < 1) h = 1;
	};
};

void main(int argc, char *argv[]) {
	std::srand(std::time(0));
	Parameters par;
	if (argc != 1) {
		par.setParameters(argc, argv);
	}
	std::ofstream fileout(par.f);
	graph g;

	int currI = 0;
	source_node<ImageRandom> sourceNode(g, [&currI, &par](ImageRandom &image){
		if (currI >= par.n) {
			return false;
		}
		currI++;
		image = ImageRandom(par.w, par.h);
		std::cout << "Start" << std::endl;
		image.OriginalImage();
		return true;
	}, false);
	
	limiter_node<ImageRandom> limiterNode(g, par.l);

	function_node<ImageRandom, std::vector<Position> > MaxValueNode(g, tbb::flow::unlimited, [](const ImageRandom &image)
	{
		return image.RfindMAX();
	});

	function_node<ImageRandom, std::vector<Position> > MinValueNode(g, tbb::flow::unlimited, [](const ImageRandom &image)
	{
		return image.RfindMIN();
	});

	function_node<ImageRandom, std::vector<Position> > SpecValueNode(g, tbb::flow::unlimited, [&par](const ImageRandom &image)
	{
		return image.RfindSPEC(par.b);
	});

	using HighlightArgs = tuple<ImageRandom, std::vector<Position>, std::vector<Position>, std::vector<Position> >;
	join_node<HighlightArgs> joinNode(g);

	function_node<HighlightArgs, ImageRandom> highlightNode(g, tbb::flow::unlimited, [](HighlightArgs highlightArgs)
	{
		ImageRandom image = tbb::flow::get<0>(highlightArgs);
		image.selectPixels(tbb::flow::get<1>(highlightArgs));
		image.selectPixels(tbb::flow::get<2>(highlightArgs));
		image.selectPixels(tbb::flow::get<3>(highlightArgs));
		return image;
	});

	function_node<ImageRandom, ImageRandom> inversionNode(g, unlimited, [](ImageRandom image)
	{
		image.inversion();
		return image;
	});

	function_node<ImageRandom, double> MeanNode(g, unlimited, [](const ImageRandom &image)
	{
		return image.mean();
	});

	join_node<tuple<ImageRandom, double> > finalJoinNode(g);

	function_node<tuple<ImageRandom, double>, continue_msg> outputNode(g, serial, [&fileout](const tuple<ImageRandom, double> &info)
	{
		fileout << "Mean: " << tbb::flow::get<1>(info) << std::endl;
		ImageRandom image = tbb::flow::get<0>(info);
		std::cout << "Finish" << std::endl;
		//image.OriginalImage();
		//std::cout << "Mean: " << tbb::flow::get<1>(info) << std::endl;
		return continue_msg();
	});

	make_edge(sourceNode, limiterNode);
	make_edge(limiterNode, MaxValueNode);
	make_edge(limiterNode, MinValueNode);
	make_edge(limiterNode, SpecValueNode);
	make_edge(limiterNode, tbb::flow::input_port<0>(joinNode));
	make_edge(MaxValueNode, tbb::flow::input_port<1>(joinNode));
	make_edge(MinValueNode, tbb::flow::input_port<2>(joinNode));
	make_edge(SpecValueNode, tbb::flow::input_port<3>(joinNode));
	make_edge(joinNode, highlightNode);
	make_edge(highlightNode, inversionNode);
	make_edge(highlightNode, MeanNode);
	make_edge(inversionNode, tbb::flow::input_port<0>(finalJoinNode));
	make_edge(MeanNode, tbb::flow::input_port<1>(finalJoinNode));
	make_edge(finalJoinNode, outputNode);
	make_edge(outputNode, limiterNode.decrement);

	sourceNode.activate();

	g.wait_for_all();

	fileout.close();
	system("pause");
}