#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <cstdint>
#include <cassert>
#include <fstream>
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
		std::cerr << "New image\n";
		return true;
	}

private:
	int h_, w_;
	std::mt19937 gen;
};

struct Coordinate {
	int x, y;
};

typedef std::vector<Coordinate> Coordinates;
typedef std::pair<Image, Coordinates> TaggedCoordinates;

struct FindMaxPoints {
	TaggedCoordinates operator()(Image source) {
		const int h = source->size();
		const int w = (*source)[0].size();
		Pixel max_val = std::numeric_limits<Pixel>::min();
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				max_val = std::max(max_val, (*source)[y][x]);
		Coordinates result;
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				if ((*source)[y][x] == max_val) {
					result.push_back({ x, y });
				}
		return std::make_pair(source, result);
	}
};

struct FindMinPoints {
	TaggedCoordinates operator()(Image source) {
		const int h = source->size();
		const int w = (*source)[0].size();
		Pixel min_val = std::numeric_limits<Pixel>::max();
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				min_val = std::min(min_val, (*source)[y][x]);
		Coordinates result;
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				if ((*source)[y][x] == min_val) {
					result.push_back({ x, y });
				}
		return std::make_pair(source, result);
	}
};

struct FindExactPoints {
	FindExactPoints(Pixel value) : value_(value) {}
	TaggedCoordinates operator()(Image source) {
		const int h = source->size();
		const int w = (*source)[0].size();
		Coordinates result;
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
				if ((*source)[y][x] == value_) {
					result.push_back({ x, y });
				}
		return std::make_pair(source, result);
	}
private:
	Pixel value_;
};

struct HighlightImage {
	HighlightImage(int window_width, int window_height) : window_width_(window_width), window_height_(window_height) {
		assert(window_width_ % 2 == 1);
		assert(window_height_ % 2 == 1);
	}
	Image operator()(std::tuple<Image, TaggedCoordinates, TaggedCoordinates, TaggedCoordinates> source) {
		highlight(std::get<0>(source), std::get<1>(source));
		highlight(std::get<0>(source), std::get<2>(source));
		highlight(std::get<0>(source), std::get<3>(source));
		return std::get<0>(source);
	}
	void highlight(Image image, const TaggedCoordinates &coordinates) {
		assert(image == coordinates.first);
		const int h = image->size();
		const int w = (*image)[0].size();
		for (Coordinate pt : coordinates.second) {
			int x1 = std::max(0, pt.x - window_width_ / 2);
			int x2 = std::min(w - 1, pt.x + window_width_ / 2);
			int y1 = std::max(0, pt.y - window_height_ / 2);
			int y2 = std::min(h - 1, pt.y + window_height_ / 2);
			for (int y = y1; y <= y2; y++)
				for (int x = x1; x <= x2; x++)
					(*image)[y][x] = 0;
		}
	}
private:
	int window_width_, window_height_;
};

struct ImageInvert {
	std::tuple<Image, Image> operator()(Image source) {
		const int h = source->size();
		const int w = (*source)[0].size();
		Image result = std::make_shared<ImageData>(h, std::vector<Pixel>(w));
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++) {
				(*result)[y][x] = -(*source)[y][x];
			}
		return std::make_tuple(source, result);
	}
};

struct ImageAverage {
	std::tuple<Image, double> operator()(Image source) {
		double sum = 0, cnt = 0;
		for (const auto &row : *source)
			for (const auto &pixel : row) {
				sum += pixel;
				cnt++;
			}
		return std::make_tuple(source, sum / cnt);
	}
};

struct Printer {
	Printer(std::ostream &os) : os_(&os) {}
	template<typename T>
	std::tuple<Image, T> operator()(std::tuple<Image, T> value) {
		*os_ << std::get<1>(value) << std::endl;
		return value;
	}
private:
	std::ostream *os_;
};

const int HEIGHT = 4000;
const int WIDTH = 3000;
const int WINDOW_WIDTH = 5;
const int WINDOW_HEIGHT = 5;

int main(int argc, char* argv[]) {
	Pixel exact_value = 0;
	int limit = 1;

	std::ofstream avg_log;
	for (int i = 1; i < argc;) {
		if (!strcmp(argv[i], "-b")) {
			i++;
			assert(i < argc);
			exact_value = atoi(argv[i]);
			i++;
		}
		else if (!strcmp(argv[i], "-l")) {
			i++;
			assert(i < argc);
			limit = atoi(argv[i]);
			i++;
		}
		else if (!strcmp(argv[i], "-f")) {
			i++;
			assert(i < argc);
			avg_log.open(argv[i], std::ios_base::out);
			assert(avg_log);
			i++;
		}
		else {
			std::cerr << "Invalid argument: " << argv[i] << "\n";
			return 1;
		}
	}

	graph g;

	source_node<Image> generator(g, ImageGenerator(HEIGHT, WIDTH));
	limiter_node<Image> limiter(g, limit);
	function_node<Image, Image> start_node(g, serial, [](Image img) {
		std::cerr << "Image processing started...\n";
		return img;
	});
	function_node<Image, TaggedCoordinates> min_finder(g, unlimited, FindMinPoints());
	function_node<Image, TaggedCoordinates> max_finder(g, unlimited, FindMaxPoints());
	function_node<Image, TaggedCoordinates> exact_finder(g, unlimited, FindExactPoints(exact_value));
	join_node<std::tuple<Image, TaggedCoordinates, TaggedCoordinates, TaggedCoordinates>, key_matching<Image>> coord_joiner(
		g,
		[](const Image &img) { return img; },
		[](const TaggedCoordinates &c) { return c.first; },
		[](const TaggedCoordinates &c) { return c.first; },
		[](const TaggedCoordinates &c) { return c.first; }
		);

	function_node<std::tuple<Image, TaggedCoordinates, TaggedCoordinates, TaggedCoordinates>, Image> highlighter(g, unlimited, HighlightImage(WINDOW_WIDTH, WINDOW_HEIGHT));

	function_node<Image, std::tuple<Image, Image>> inverter(g, unlimited, ImageInvert());
	function_node<Image, std::tuple<Image, double>> averager(g, unlimited, ImageAverage());
	function_node<std::tuple<Image, double>, std::tuple<Image, double>> printer(g, serial, Printer(avg_log.is_open() ? avg_log : std::cout));

	join_node<std::tuple<std::tuple<Image, Image>, std::tuple<Image, double>>, key_matching<Image>> finish_joiner(
		g,
		[](const std::tuple<Image, Image> &img) { return std::get<0>(img); },
		[](const std::tuple<Image, double> &c) { return std::get<0>(c);  }
		);
	function_node<std::tuple<std::tuple<Image, Image>, std::tuple<Image, double>>> finish_node(g, serial, [](const std::tuple<std::tuple<Image, Image>, std::tuple<Image, double>> &arg) {
		std::cerr << "Image processed\n";
	});

	// Step 1. Generate images
	make_edge(generator, limiter);
	make_edge(limiter, start_node);
	make_edge(start_node, input_port<0>(coord_joiner));
	// Step 2. Find points
	make_edge(limiter, min_finder);
	make_edge(min_finder, input_port<1>(coord_joiner));

	make_edge(limiter, max_finder);
	make_edge(max_finder, input_port<2>(coord_joiner));

	make_edge(limiter, exact_finder);
	make_edge(exact_finder, input_port<3>(coord_joiner));

	// Step 3. Highlight area around points
	make_edge(coord_joiner, highlighter);
	// Step 4. Calculate average and invert the result
	make_edge(highlighter, averager);
	make_edge(highlighter, inverter);
	// Step 5. Print average
	make_edge(averager, printer);
	make_edge(printer, input_port<1>(finish_joiner));
	// Step 6. Do nothing with the inversion
	make_edge(inverter, input_port<0>(finish_joiner));

	// Join everything together
	make_edge(finish_joiner, finish_node);
	make_edge(finish_node, limiter.decrement);

	generator.activate();
	g.wait_for_all();
	return 0;
}
