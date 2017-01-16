#include "Arguments.h"
#include "Image.h"
#include "tbb/task_group.h"
#include "tbb/flow_graph.h"
#include "time.h"
#include <fstream>
Arguments Parser(int argc, char * argv[])
{
	int bright = -1;
	int countParaleImg = -1;
	int countImg = -1;
	std::string LogFileName ="";
	if (argc < 3)
	{
		std::cout << "Usage: -b <value> -l <value> -f <value> -n <value>" << std::endl
			<< "-b (brightness [0, 255])"
			<< std::endl << " -l (limit of parallel images)" << std::endl
			<< " -f (brightness log path)" << std::endl
			<< " -n (limit of images)" << std::endl;

	}
	for (int i = 0; i < argc; i++) {
		std::string currArg = argv[i];

		if (currArg == "-b")
		{
			if (i + 1 < argc) bright = atoi(argv[i + 1]);
		}
		else if (currArg == "-l") {
			if (i + 1 < argc) countParaleImg = atoi(argv[i + 1]);
		}
		else if (currArg == "-f")
		{
			if (i + 1 < argc) LogFileName = argv[i + 1];
		}
		else if (currArg == "-n")
		{
			if (i + 1 < argc) countImg = atoi(argv[i + 1]);
		}

	}
	if (bright == -1) { std::cout << "argument -b is null: using default -b = 123 " << std::endl; bright = 123; }
	if (countParaleImg == -1) { std::cout << "argument -l is null: using default -b = 4 " << std::endl; countParaleImg = 4; }
	if (countImg == -1) { std::cout << "argument -n is null: using default -b = 4 " << std::endl; countImg = 4; }

	
	return Arguments(bright, countParaleImg, LogFileName, countImg);

}

int main(int argc, char ** argv)
{
	srand(time(NULL));
	Arguments inputArg=Parser(argc, argv);
	std::ofstream fout(inputArg.LogFileName, std::fstream::out | std::fstream::trunc);
	using dataImg = std::pair<size_t, std::vector<size_t>>;
		using namespace tbb;
		flow::graph graph;
		int currStep=0;
		flow::source_node<Arguments> sourceNode(graph,
			[currStep, inputArg](Arguments &argm) mutable {
			if (currStep < argm.countImg)
			{	
				argm.setArg(inputArg);
				currStep++;
				return true;
			}else return false;
		});
		flow::limiter_node<Arguments> limiterNode(graph, inputArg.countParaleImg);
		flow::function_node<
			Arguments,
			std::shared_ptr<Image>> generateMatrixNode(graph,
				flow::unlimited,
				[](const Arguments& argm) {
			auto currImg = std::shared_ptr<Image>(new Image(50));
			return currImg;
		});
		flow::broadcast_node<std::shared_ptr<Image>> broadcastNodeFL(graph);
		flow::function_node<
			std::shared_ptr<Image>,
			dataImg> minFunctionNode(graph, flow::unlimited,
				[](std::shared_ptr<Image> cusrImg) {
			int min = 0;
			min= cusrImg->img[0] ;
			std::vector<size_t> indexes;
			for (int i = 0; i < cusrImg->size*cusrImg->size; i++)
			{
				if (cusrImg->img[i] <= min)
				{
					if (min != cusrImg->img[i])indexes.clear();
					min = cusrImg->img[i];
					indexes.emplace_back(i);
				}
			}
			
			return dataImg(cusrImg->getId(), indexes);
		});
		flow::function_node<
			std::shared_ptr<Image>,
			dataImg> maxFunctionNode(graph, flow::unlimited,
				[](std::shared_ptr<Image> cusrImg) {
			int max = 0;
			max = cusrImg->img[0];
			std::vector<size_t> indexes;
			for (int i = 0; i < cusrImg->size*cusrImg->size; i++)
			{
				if (cusrImg->img[i] >= max)
				{
					if (max!= cusrImg->img[i])indexes.clear();
					max = cusrImg->img[i];
					indexes.emplace_back(i);
				}
			}
			return dataImg(cusrImg->getId(), indexes);
		});



		flow::function_node<
			std::shared_ptr<Image>,
			dataImg> compareFunctionNode(graph, flow::unlimited,
				[inputArg](std::shared_ptr<Image> cusrImg) {
			std::vector<size_t> indexes;
			for (int i = 0; i < cusrImg->size*cusrImg->size; i++)
			{
				if (cusrImg->img[i] == inputArg.bright)
				{
					indexes.emplace_back(i);
				}
			}
			return dataImg(cusrImg->getId(), indexes);
		});

		using SelectionPixelsT = flow::tuple<
			std::shared_ptr<Image>,
			dataImg,
			dataImg,
			dataImg>;

		flow::join_node<SelectionPixelsT, tbb::flow::tag_matching> joinNode(graph, 
			[](std::shared_ptr<Image> image) -> size_t { return image->getId(); },
			[](const dataImg& pixels) -> size_t { return pixels.first; },
			[](const dataImg& pixels) -> size_t { return pixels.first; },
			[](const dataImg& pixels) -> size_t { return pixels.first; }
			
			);

		flow::function_node<
			SelectionPixelsT,
			std::shared_ptr<Image>> selectionPixelFunctionNode(graph, flow::unlimited, [](SelectionPixelsT selectPix) {

			auto highlighter = [](std::shared_ptr<Image> currImg, const std::vector<size_t>& indexes) {
				for (auto index : indexes)
				{
					currImg->highliting(index);
				}
			};

			auto matrix = flow::get<0>(selectPix);
			highlighter(matrix, flow::get<1>(selectPix).second);
			highlighter(matrix, flow::get<2>(selectPix).second);
			highlighter(matrix, flow::get<3>(selectPix).second);

			return matrix;
		});




		flow::broadcast_node<std::shared_ptr<Image>> broadcastNodeSP(graph);

		flow::function_node<
			std::shared_ptr<Image>,
			std::shared_ptr<Image>> inverseNode(graph,
				flow::unlimited,
				[](std::shared_ptr<Image> currImg) {
		
			std::shared_ptr<Image> inverse=std::make_shared<Image>(currImg->size);
			for (int i = 0; i < currImg->size*currImg->size; i++) {
				inverse->img[i] = 256 - currImg->img[i];
			}

			return inverse;
		});


		flow::function_node<
			std::shared_ptr<Image>,
			double> averageFunctinNode(graph,
				flow::unlimited,
				[](std::shared_ptr<Image> currImg) {
			
			double averageBrigh = 0.0;
			for (size_t i = 0; i < currImg->size*currImg->size; i++) {
				averageBrigh += currImg->img[i];
			}
			averageBrigh= averageBrigh / (currImg->size*currImg->size);
			return averageBrigh;
		});

		flow::function_node<double> logResultFunctionNode(graph,
			flow::serial,
			[&fout](double averageBrigh) {
			fout  << averageBrigh << "\n";
		});

		flow::function_node<std::shared_ptr<Image>> preLimitFunctionNode(graph, flow::serial, [](std::shared_ptr<Image> currImg) {});


  	flow::make_edge(sourceNode, limiterNode);
	flow::make_edge(limiterNode, generateMatrixNode);
	flow::make_edge(generateMatrixNode, broadcastNodeFL);
	flow::make_edge(broadcastNodeFL, minFunctionNode);
	flow::make_edge(broadcastNodeFL, maxFunctionNode);
	flow::make_edge(broadcastNodeFL, compareFunctionNode);


	flow::make_edge(broadcastNodeFL, flow::input_port<0>(joinNode));
	flow::make_edge(minFunctionNode, flow::input_port<1>(joinNode));
	flow::make_edge(maxFunctionNode, flow::input_port<2>(joinNode));
	flow::make_edge(compareFunctionNode, flow::input_port<3>(joinNode));

	flow::make_edge(joinNode, selectionPixelFunctionNode);

	flow::make_edge(selectionPixelFunctionNode, broadcastNodeSP);

	flow::make_edge(broadcastNodeSP, preLimitFunctionNode);
	flow::make_edge(preLimitFunctionNode, limiterNode.decrement);

	flow::make_edge(broadcastNodeSP, averageFunctinNode);
	flow::make_edge(broadcastNodeSP, inverseNode);

	flow::make_edge(averageFunctinNode, logResultFunctionNode);

	graph.wait_for_all();

	fout.close();

	system("pause");
	return 0;
}