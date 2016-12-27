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
	int width; //ширина
	int height; //высота
	int ** imageR;
public:
	ImageRandom() {
		width = height = 0;
	};
	ImageRandom(int NEWwidth, int NEWheight) {
		width = NEWwidth;
		height = NEWheight;
		imageR = new int * [width];
		for (int i = 0; i < width; i++) imageR[i] = new int[height];
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) imageR[w][h] = rand() % 256;
		}
	};
	~ImageRandom() {
		/*for (unsigned i = width; i > 0; --i) {
			delete [] imageR[i - 1];
		} 
		delete [] imageR;*/
	};
	void OriginalImage() {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) std::cout << imageR[w][h] << " ";
			std::cout << std::endl;
		}
	};
	std::vector<Position> RfindMIN() const {
		std::vector<Position> num;
		Position pos;
		int minimum = 255;
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (imageR[w][h] < minimum) minimum = imageR[w][h];
			}
		}
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (imageR[w][h] == minimum) {
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
				if (imageR[w][h] > maximum) maximum = imageR[w][h];
			}
		}
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (imageR[w][h] == maximum) {
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
				if (imageR[w][h] == spec) {
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
				imageR[numPixels.at(i).w - 1][numPixels.at(i).h - 1] = numPixels.at(i).res;
			if ((numPixels.at(i).w - 1 >= 0) && (numPixels.at(i).w - 1 < width) && (numPixels.at(i).h >= 0) && (numPixels.at(i).h < height))
				imageR[numPixels.at(i).w - 1][numPixels.at(i).h] = numPixels.at(i).res;
			if ((numPixels.at(i).w - 1 >= 0) && (numPixels.at(i).w - 1 < width) && (numPixels.at(i).h + 1 >= 0) && (numPixels.at(i).h + 1 < height))
				imageR[numPixels.at(i).w - 1][numPixels.at(i).h + 1] = numPixels.at(i).res;

			if ((numPixels.at(i).w >= 0) && (numPixels.at(i).w < width) && (numPixels.at(i).h - 1 >= 0) && (numPixels.at(i).h - 1 < height))
				imageR[numPixels.at(i).w][numPixels.at(i).h - 1] = numPixels.at(i).res;
			if ((numPixels.at(i).w >= 0) && (numPixels.at(i).w < width) && (numPixels.at(i).h + 1 >= 0) && (numPixels.at(i).h + 1 < height))
				imageR[numPixels.at(i).w][numPixels.at(i).h + 1] = numPixels.at(i).res;

			if ((numPixels.at(i).w + 1 >= 0) && (numPixels.at(i).w + 1< width) && (numPixels.at(i).h - 1 >= 0) && (numPixels.at(i).h - 1 < height))
				imageR[numPixels.at(i).w + 1][numPixels.at(i).h - 1] = numPixels.at(i).res;
			if ((numPixels.at(i).w + 1 >= 0) && (numPixels.at(i).w + 1< width) && (numPixels.at(i).h >= 0) && (numPixels.at(i).h < height))
				imageR[numPixels.at(i).w + 1][numPixels.at(i).h] = numPixels.at(i).res;
			if ((numPixels.at(i).w + 1 >= 0) && (numPixels.at(i).w + 1< width) && (numPixels.at(i).h + 1 >= 0) && (numPixels.at(i).h + 1 < height))
				imageR[numPixels.at(i).w + 1][numPixels.at(i).h + 1] = numPixels.at(i).res;
		}
	};
	void inversion() {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) imageR[w][h] = 255 - imageR[w][h];
		}
	};
	double mean() const {
		double sum = 0; 
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) sum += imageR[w][h];
		}
		sum = sum / (height * width);
		return sum;
	};
};



void main() {
	std::srand(std::time(0));
	//¬ходные услови€ 
	int _b = 13; //интересующее значение €ркости, дл€ шага є 2
	int _l = 1; //предел одновременно обрабатываемых приложением изображений
	//У-f log.txtФ: им€ файла журнала €ркостей

	graph g;

	int currI = 0;
	source_node<ImageRandom> sourceNode(g, [&currI, &_l](ImageRandom &image){
		if (currI >= _l) {
			return false;
		}
		currI++;
		image = ImageRandom(4, 4);
		std::cout << "Start" << std::endl;
		image.OriginalImage();
		return true;
	}, false);
	
	limiter_node<ImageRandom> limiterNode(g, 4);

	function_node<ImageRandom, std::vector<Position> > MaxValueNode(g, tbb::flow::unlimited, [](const ImageRandom &image)
	{
		return image.RfindMAX();
	});

	function_node<ImageRandom, std::vector<Position> > MinValueNode(g, tbb::flow::unlimited, [](const ImageRandom &image)
	{
		return image.RfindMIN();
	});

	function_node<ImageRandom, std::vector<Position> > SpecValueNode(g, tbb::flow::unlimited, [&_b](const ImageRandom &image)
	{
		return image.RfindSPEC(_b);
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

	function_node<tuple<ImageRandom, double>, continue_msg> outputNode(g, serial, [](const tuple<ImageRandom, double> &info)
	{
		ImageRandom image = tbb::flow::get<0>(info);

		std::cout << "Finish" << std::endl;
		image.OriginalImage();
		std::cout << "Mean: " << tbb::flow::get<1>(info) << std::endl;
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

	system("pause");
}