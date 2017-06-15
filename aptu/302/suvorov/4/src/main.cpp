#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <cstdint>
#include "tbb/flow_graph.h"
#include "tbb/task_scheduler_init.h"

using namespace tbb::flow;

typedef std::uint8_t Pixel;
typedef std::vector<std::vector<Pixel>> ImageData;
typedef std::shared_ptr<ImageData> Image;

struct ImageGenerator {
	ImageGenerator(int h, int w) : h_(h), w_(w) {
	}
	bool operator()(Image &result) {
		result = std::make_shared<ImageData>(h_, std::vector<Pixel>(w_));
		for (int y = 0; y < h_; y++)
			for (int x = 0; x < w_; x++) {
				(*result)[y][x] = gen();
			}
		std::cout << "New\n";
		return true;
	}
private:
	int h_, w_;
	std::mt19937 gen;
};

struct ImageInvert {
	Image operator()(Image source) {
		const int h = source->size();
		const int w = (*source)[0].size();
		Image result = std::make_shared<ImageData>(h, std::vector<Pixel>(w));
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++) {
				(*result)[y][x] = -(*source)[y][x];
			}
		return result;
	}
};

struct ImageAverage {
	double operator()(Image source) {
		double sum = 0, cnt = 0;
		for (const auto &row : *source)
			for (const auto &pixel : row) {
				sum += pixel;
				cnt++;
			}
		return sum / cnt;
	}
};

struct Printer {
	Printer(std::ostream &os) : os_(&os) {}
	template<typename T>
	continue_msg operator()(T value) {
		std::cout << value << std::endl;
		return continue_msg();
	}
private:
	std::ostream *os_;
};

const int HEIGHT = 1000;
const int WIDTH = 1000;

int main() {
	graph g;
	tbb::task_scheduler_init init(10);

	source_node<Image> generator(g, ImageGenerator(HEIGHT, WIDTH));
	limiter_node<Image> limiter(g, 5);
	function_node<Image, double> averager(g, unlimited, ImageAverage());
	function_node<double> printer(g, serial, Printer(std::cout));

	make_edge(generator, limiter);
	make_edge(limiter, averager);
	make_edge(averager, printer);

	generator.activate();
	g.wait_for_all();
	return 0;
}
