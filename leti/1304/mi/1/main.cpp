#include "stdafx.h"
#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <memory>
#include <algorithm>
#include "tbb\flow_graph.h"
#include "tbb\task_group.h"
#include "Matrix.h"

using namespace tbb;
using CommandLineArguments = std::tuple<uint8_t, size_t, size_t, std::string>;
void showCommandOptions(std::ostream& out);
CommandLineArguments parseCommandLineArgs(int argc, const char** argv);
template<class Number> class RndNumGen {
public:
	Number operator()() {
		return rndDistribution(rndEngine);
	}
private:
	std::default_random_engine rndEngine = std::default_random_engine((unsigned int)std::chrono::high_resolution_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<Number> rndDistribution;
};

int main(int argc, const char** argv)
{
	flow::graph graph;
	uint8_t brightnessValue;
	size_t flowLimit;
	size_t matrixCount;
	std::string pathOfLogFile;
	size_t currentTaskId = 0;
	std::tie(brightnessValue, flowLimit, matrixCount, pathOfLogFile) = parseCommandLineArgs(argc, argv);
	auto outStream = std::fstream(pathOfLogFile, std::fstream::out | std::fstream::trunc);

		flow::source_node<Matrix::FormCard> sourceNode(graph, [&currentTaskId, matrixCount](Matrix::FormCard& formCard) mutable {
		formCard.first = 100;
		formCard.second = 100;
		return currentTaskId++ < matrixCount;
	});

	flow::limiter_node<Matrix::FormCard> limiterNode(graph, flowLimit);
	flow::function_node<Matrix::FormCard, std::shared_ptr<Matrix>> generateMatrixNode(graph,flow::unlimited, [](const Matrix::FormCard& formCard) {
		RndNumGen<std::uint8_t> generator;
		auto matrix = std::shared_ptr<Matrix>(new Matrix(formCard));
		*matrix << generator;
		return matrix;
	});
	flow::broadcast_node<std::shared_ptr<Matrix>> broadcastNode(graph);
	flow::function_node<std::shared_ptr<Matrix>, std::vector<size_t>> maxValue(graph, flow::unlimited, [](std::shared_ptr<Matrix> inputMatrix) {
		const auto& data = inputMatrix->getMatrix();
		uint8_t max = data[0];

		std::vector<size_t> indx;
		for (size_t i = 1; i < inputMatrix->getLength(); i++) {
			auto curValue = data[i];
			if (curValue > max) {
				max = curValue;
				indx.clear();
				indx.emplace_back(i);
				continue;
			}
			else if (curValue == max) 
					indx.emplace_back(i);
			
		}

		return indx;
	});
	flow::function_node<std::shared_ptr<Matrix>, std::vector<size_t>> minValue(graph, flow::unlimited, [](std::shared_ptr<Matrix> inputMatrix) {
		const auto& data = inputMatrix->getMatrix();
		uint8_t min = data[0];
		std::vector<size_t> indx;
		for (size_t i = 1; i < inputMatrix->getLength(); i++) {
			auto curValue = data[i];
			if (curValue < min) {
				min = curValue;
				indx.clear();
				indx.emplace_back(i);
				continue;
			}
			else if (curValue == min)
				indx.emplace_back(i);

		}
		return indx;
	});
	flow::function_node<std::shared_ptr<Matrix>, std::vector<size_t>> equalToValue(graph, flow::unlimited, [brightnessValue](std::shared_ptr<Matrix> inputMatrix) {
		const auto& data = inputMatrix->getMatrix();
		uint8_t max = data[0];

		std::vector<size_t> indx;
		for (size_t i = 1; i < inputMatrix->getLength(); i++) {
			auto curValue = data[i];
			if (curValue == brightnessValue) {
				indx.emplace_back(i);
			}
		}

		return indx;
	});
	using HighlightInput = flow::tuple<std::shared_ptr<Matrix>,	std::vector<size_t>, std::vector<size_t>, std::vector<size_t>>;
	flow::join_node<HighlightInput> joinNode(graph);
	flow::function_node<HighlightInput,	std::shared_ptr<Matrix>> highlight_node(graph, flow::unlimited, [](HighlightInput highlightInput) {
		auto highlighter = [](std::shared_ptr<Matrix> matrix, const std::vector<size_t>& indxs) {
			auto& data = matrix->getMatrix();
			auto formCard = matrix->getFormCard();
			auto length = matrix->getLength();
			size_t rows = formCard.first;
			size_t cols = formCard.second;

			for (auto indx : indxs) {
				auto row = indx / cols;
				auto col = indx % cols;

				if (row + 1 != rows) {
					if (col != 0) {
						data[(row + 1) * cols + col - 1] = 0;
					}
					data[(row + 1) * cols + col] = 0;
					if (col + 1 != cols) {
						data[(row + 1) * cols + col + 1] = 0;
					}
				}

				if (col != 0)data[row * cols + col - 1] = 0;
				data[row * cols + col] = 0;
				if (col + 1 != cols)data[row * cols + col + 1] = 0;

				if (row != 0) {
					if (col != 0)data[(row - 1) * rows + col - 1] = 0;
					data[(row - 1) * cols + col] = 0;
					if (col + 1 != cols)data[(row - 1) * cols + col + 1] = 0;
				}
			}
		};

		auto matrix = flow::get<0>(highlightInput);
		highlighter(matrix, flow::get<1>(highlightInput));
		highlighter(matrix, flow::get<2>(highlightInput));
		highlighter(matrix, flow::get<3>(highlightInput));
		return matrix;
	});

	flow::broadcast_node<std::shared_ptr<Matrix>> broadcastNode_new(graph);
	flow::function_node<std::shared_ptr<Matrix>, std::shared_ptr<Matrix>> inverseNode(graph, flow::unlimited, [](std::shared_ptr<Matrix> matrix) {
		std::shared_ptr<Matrix> inverseMatrix = std::make_shared<Matrix>(matrix->getFormCard());
		auto& input = matrix->getMatrix();
		auto& output = inverseMatrix->getMatrix();
		for (size_t i = 0; i < matrix->getLength(); ++i) {
			output[i] = std::numeric_limits<uint8_t>::max() - input[i];
		}
		return inverseMatrix;
	});
	flow::function_node<std::shared_ptr<Matrix>, double> meanNode(graph, flow::unlimited, [](std::shared_ptr<Matrix> matrix) {
		const auto& data = matrix->getMatrix();
		auto length = matrix->getLength();

		double mean = 0.0;
		for (size_t i = 0; i < length; ++i) 
			mean += data[i];
		
		return mean / length;
	});
	flow::function_node<std::shared_ptr<Matrix>> matrixDebugOutput(graph, flow::serial,	[](std::shared_ptr<Matrix> matrix) {
		*matrix >> std::cout;
		std::cout << "\n";
	});
	flow::function_node<double> doubleOutputNode(graph,	flow::serial, [&outStream](double mean) {
		outStream << std::setprecision(10) << mean << "\n";
	});
	flow::function_node<std::shared_ptr<Matrix>> decrementer(graph, flow::serial, [](std::shared_ptr<Matrix> val) {});

	flow::make_edge(sourceNode, limiterNode);
	flow::make_edge(limiterNode, generateMatrixNode);
	flow::make_edge(generateMatrixNode, broadcastNode);
	flow::make_edge(broadcastNode, minValue);
	flow::make_edge(broadcastNode, maxValue);
	flow::make_edge(broadcastNode, equalToValue);

	flow::make_edge(broadcastNode, flow::input_port<0>(joinNode));
	flow::make_edge(minValue, flow::input_port<1>(joinNode));
	flow::make_edge(maxValue, flow::input_port<2>(joinNode));
	flow::make_edge(equalToValue, flow::input_port<3>(joinNode));

	flow::make_edge(joinNode, highlight_node);
	flow::make_edge(highlight_node, broadcastNode_new);

	flow::make_edge(broadcastNode_new, decrementer);
	flow::make_edge(decrementer, limiterNode.decrement);

	flow::make_edge(broadcastNode_new, meanNode);
	flow::make_edge(broadcastNode_new, inverseNode);

	flow::make_edge(inverseNode, matrixDebugOutput);
	flow::make_edge(meanNode, doubleOutputNode);

	graph.wait_for_all();

	outStream.close();

	return 0;
}

CommandLineArguments parseCommandLineArgs(int argc, const char** argv) {
	uint8_t brightnessValue = 0;
	size_t flowLimit = 10;
	size_t matrixCount = 50;
	std::string pathOfLogFile = "./result.log";
	bool error = false;

	for (size_t i = 1; i < argc; i += 2) {
		std::cout << std::string(argv[i]) << " " << std::string(argv[i + 1]) << std::endl;
		if (std::strcmp(argv[i], "-b") == 0) {
			brightnessValue = (uint8_t)std::stoi(argv[i + 1]);
		}
		else if (std::strcmp(argv[i], "-n") == 0) {
			matrixCount = (size_t)std::stoi(argv[i + 1]);
		}
		else if (std::strcmp(argv[i], "-l") == 0) {
			flowLimit = (size_t)std::stoi(argv[i + 1]);
		}
		else if (std::strcmp(argv[i], "-f") == 0) {
			pathOfLogFile = argv[i + 1];
		}
		else {
			error = true;
		}
	}

	if (error) {
		showCommandOptions(std::cout);
		std::terminate();
	}

	return CommandLineArguments{ brightnessValue, flowLimit, matrixCount, pathOfLogFile };
}

void showCommandOptions(std::ostream& out) {
	out << " Options: \n";
	out << " -b searched brightness value \n";
	out << " -l flow limit \n";
	out << " -f log file name brightnesses \n";
	out << " -n count of images \n";
}
