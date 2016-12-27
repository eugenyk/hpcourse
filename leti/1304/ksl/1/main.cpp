#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <tbb/flow_graph.h>

#include "Utils.h"
#include "Matrix.h"

int main(int argc, const char* argv[]) 
{
	srand(time(nullptr));

	unsigned char brightnessValue = 0;
	int	maxParallel = 10,
		width = 320,
		height = 240,
		matrixCount = 20;
	int currCount = 0;
	std::string logFile = "./log.txt";

	std::fstream fs = std::fstream(logFile, std::fstream::out | std::fstream::trunc);

	std::tie(brightnessValue, maxParallel, matrixCount, logFile, width, height) = parse(argc, argv);

	tbb::flow::graph g;

	tbb::flow::source_node<CharMatrix*> sourceNode(g, 
		[=, &currCount](CharMatrix* &matrix) {
		matrix = new CharMatrix(width, height);
		return currCount++ < matrixCount;
	});

	tbb::flow::limiter_node<CharMatrix*> limeterNode(g, maxParallel);
	
	tbb::flow::function_node<CharMatrix*, CharMatrix*> generetaMatrixNode(g, tbb::flow::unlimited,
		[](CharMatrix* matrix) {
		matrix->randomFilling();
		return matrix;
	});

	tbb::flow::function_node<CharMatrix*, std::vector<int>> searchMinNode(g, tbb::flow::unlimited,
		[](CharMatrix* matrix) {

		std::vector<int> vec;
		unsigned char min = 255;

		matrix->processingCell([&vec, &min](unsigned char& elem, int position){
			if (elem < min) {
				min = elem;
				vec.clear();
				vec.push_back(position);
			}
			else if (elem == min) {
				vec.push_back(position);
			}
		});
		return vec;
	});

	tbb::flow::function_node<CharMatrix*, std::vector<int>> searchMaxNode(g, tbb::flow::unlimited,
		[](CharMatrix* matrix) {

		std::vector<int> vec;
		unsigned char max = 0;

		matrix->processingCell([&vec, &max](unsigned char& elem, int position){
			if (elem > max) {
				max = elem;
				vec.clear();
				vec.push_back(position);
			}
			else if (elem == max) {
				vec.push_back(position);
			}
		});
		return vec;
	});

	tbb::flow::function_node<CharMatrix*, std::vector<int>> searchEqualNode(g, tbb::flow::unlimited,
		[brightnessValue](CharMatrix* matrix) {
		
		std::vector<int> vec;

		matrix->processingCell([&vec, brightnessValue](unsigned char& elem, int position){
			if (elem == brightnessValue) {
				vec.push_back(position);
			}
		});
		
		return vec;
	});


	tbb::flow::join_node<std::tuple<CharMatrix*, std::vector<int>, std::vector<int>, std::vector<int>>> joinNode(g);

	tbb::flow::function_node<std::tuple<CharMatrix*, std::vector<int>, std::vector<int>, std::vector<int>>, CharMatrix*> highlightNode(g, tbb::flow::unlimited,
		[](std::tuple<CharMatrix*, std::vector<int>, std::vector<int>, std::vector<int>> param) {
		
		CharMatrix* matrix = std::get<0>(param);
		std::vector<int> all = std::get<1>(param);
		all.insert(all.end(), std::get<2>(param).begin(), std::get<2>(param).end());
		all.insert(all.end(), std::get<3>(param).begin(), std::get<3>(param).end());

		for each (int pos in all) {
			matrix->pixel(pos, 255);
		}
		
		return matrix;
	});

	tbb::flow::function_node<CharMatrix*, CharMatrix*> inverstNode(g, tbb::flow::unlimited,
		[](CharMatrix* matrix) {

		CharMatrix* invertMatrix = new CharMatrix(*matrix);

		invertMatrix->processingCell([](unsigned char& elem, int position){
			elem = 255 - elem;
		});

		return invertMatrix;
	});

	tbb::flow::function_node<CharMatrix*, float> averageBrightnessNode(g, tbb::flow::unlimited,
		[](CharMatrix* matrix) {

		unsigned long long avr = 0;

		matrix->processingCell([&avr](unsigned char& elem, int position) {
			avr += elem;
		});

		return (float) avr / matrix->getSize();
	});

	tbb::flow::join_node<std::tuple<CharMatrix*, float>> waitCalculateNode(g);

	tbb::flow::function_node<std::tuple<CharMatrix*, float>> debugNode(g, tbb::flow::serial,
		[&fs](std::tuple<CharMatrix*, float> res) {
		delete std::get<0>(res);
		fs << std::get<1>(res) << std::endl;
	});


	tbb::flow::make_edge(sourceNode, limeterNode);
	tbb::flow::make_edge(limeterNode, generetaMatrixNode);

	tbb::flow::make_edge(generetaMatrixNode, searchMinNode);
	tbb::flow::make_edge(generetaMatrixNode, searchMaxNode);
	tbb::flow::make_edge(generetaMatrixNode, searchEqualNode);

	tbb::flow::make_edge(generetaMatrixNode, tbb::flow::input_port<0>(joinNode));
	tbb::flow::make_edge(searchMinNode, tbb::flow::input_port<1>(joinNode));
	tbb::flow::make_edge(searchMaxNode, tbb::flow::input_port<2>(joinNode));
	tbb::flow::make_edge(searchEqualNode, tbb::flow::input_port<3>(joinNode));

	tbb::flow::make_edge(joinNode, highlightNode);

	tbb::flow::make_edge(highlightNode, inverstNode);
	tbb::flow::make_edge(highlightNode, averageBrightnessNode);

	tbb::flow::make_edge(inverstNode, tbb::flow::input_port<0>(waitCalculateNode));
	tbb::flow::make_edge(averageBrightnessNode, tbb::flow::input_port<1>(waitCalculateNode));

	tbb::flow::make_edge(waitCalculateNode, debugNode);

	tbb::flow::make_edge(debugNode, limeterNode.decrement);

	g.wait_for_all();

	fs.close();

	return 0;
}