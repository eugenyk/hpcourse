//
// Automatically generated by Flow Graph Designer: C++ Code Generator Plugin version Alpha 4.14483
//

#pragma once
#define TBB_PREVIEW_GRAPH_NODES 1

#define NUM_OF_IMAGES 10
#define MAX_BRIGHTNESS 255
#define WIDTH 100
#define HEIGHT 100

#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "tbb/flow_graph.h"
#include "tbb/atomic.h"
#include "tbb/tick_count.h"

using namespace std;
using namespace tbb;
using namespace tbb::flow;

static void spin_for( double weight ) {
    tick_count t1, t0 = tick_count::now();
    const double weight_multiple = 10e-6;
    const double end_time = weight_multiple * weight;
    do {
        t1 = tick_count::now();
    } while ( (t1-t0).seconds() < end_time );
}

class brightness_max_body {
public:
    std::vector<std::pair<int, int>> operator()( const std::vector<std::vector<int>> & input ) {
		int brightness_max = 0;
		for (int i = 0; i < input.size(); i++) {
			for (int j = 0; j < input[i].size(); j++) {
				if (input[i][j] > brightness_max) {
					brightness_max = input[i][j];
				}
			}
		}

		std::vector<std::pair<int, int>> res(0);

		for (int i = 0; i < input.size(); i++) {
			for (int j = 0; j < input[i].size(); j++) {
				if (input[i][j] == brightness_max) {
					res.push_back(std::pair<int, int>(i, j));
				}
			}
		}
		return res;
    }
};

class brightness_min_body {
public:
    std::vector<std::pair<int, int>> operator()( const std::vector<std::vector<int>> & input ) {
		int brightness_min = MAX_BRIGHTNESS;
		for (int i = 0; i < input.size(); i++) {
			for (int j = 0; j < input[i].size(); j++) {
				if (input[i][j] < brightness_min) {
					brightness_min = input[i][j];
				}
			}
		}

		std::vector<std::pair<int, int>> res(0);

		for (int i = 0; i < input.size(); i++) {
			for (int j = 0; j < input[i].size(); j++) {
				if (input[i][j] == brightness_min) {
					res.push_back(std::pair<int, int>(i, j));
				}
			}
		}
		return res;
    }
};

class brightness_arg_body {
public:
	brightness_arg_body(int param)
		: param(param)
	{}
	std::vector<std::pair<int, int>> operator()(const std::vector<std::vector<int>> & input) {
		std::vector<std::pair<int, int>> res(0);

		for (int i = 0; i < input.size(); i++) {
			for (int j = 0; j < input[i].size(); j++) {
				if (input[i][j] == param) {
					res.push_back(std::pair<int, int>(i, j));
				}
			}
		}
		return res;
	}
private:
	int param;
};

class source_imgs_body {
public:
    bool operator()( std::vector<std::vector<int>> &output ) {
		static int k = 0;
		if (k++ < NUM_OF_IMAGES) {
			output = std::vector<std::vector<int>>(HEIGHT);
			for (int i = 0; i < HEIGHT; i++) {
				for (int j = 0; j < WIDTH; j++) {
					output[i].push_back(rand() % (MAX_BRIGHTNESS + 1));
				}
			}
			return true;
		}
		return false;
    }

};

class rectangle_drawer_body {
public:
    std::vector<std::vector<int>> operator()( const tbb::flow::tuple<std::vector<std::pair<int, int>>,std::vector<std::pair<int, int>>,std::vector<std::pair<int, int>>,std::vector<std::vector<int>>> & input ) {
		std::vector<std::vector<int>> res = get<3>(input);
		std::vector<std::pair<int, int>> p1 = get<0>(input);
		std::vector<std::pair<int, int>> p2 = get<1>(input);
		std::vector<std::pair<int, int>> p3 = get<2>(input);
		draw_rectangle(res, p1);
		draw_rectangle(res, p2);
		draw_rectangle(res, p3);
        return res;
    }
private:
	void draw_rectangle(std::vector<std::vector<int>> & src, std::vector<std::pair<int, int>> & points) {
		for (int i = 0; i < points.size(); i++) {
			if (points[i].first > 0 && points[i].second > 0) {
				src[points[i].first - 1][points[i].second - 1] = MAX_BRIGHTNESS;
			}
			if (points[i].first > 0) {
				src[points[i].first - 1][points[i].second] = MAX_BRIGHTNESS;
			}
			if (points[i].first > 0 && points[i].second < WIDTH - 1) {
				src[points[i].first - 1][points[i].second + 1] = MAX_BRIGHTNESS;
			}
			if (points[i].second < WIDTH - 1) {
				src[points[i].first][points[i].second + 1] = MAX_BRIGHTNESS;
			}
			if (points[i].first < HEIGHT - 1 && points[i].second < WIDTH - 1) {
				src[points[i].first + 1][points[i].second + 1] = MAX_BRIGHTNESS;
			}
			if (points[i].first < HEIGHT - 1) {
				src[points[i].first + 1][points[i].second] = MAX_BRIGHTNESS;
			}
			if (points[i].first > HEIGHT - 1 && points[i].second > 0) {
				src[points[i].first + 1][points[i].second - 1] = MAX_BRIGHTNESS;
			}
			if (points[i].second > 0) {
				src[points[i].first][points[i].second - 1] = MAX_BRIGHTNESS;
			}
		}
	}
};

class brightness_average_body {
public:
    int operator()( const std::vector<std::vector<int>> & input) {
		long sum = 0;
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				sum += input[i][j];
			}
		}
        return sum/(HEIGHT*WIDTH);
    }
};

class file_writer_body {
public:
	file_writer_body(std::string filename)
		: filename(filename)
	{}
    continue_msg operator()( const int & input) {
		std::ofstream out;
		out.open(filename, std::ios::app);
		out << input << endl;
		out.close();
        return continue_msg();
    }
private:
	std::string filename;
};

class inverse_img_body {
public:
    continue_msg operator()( const std::vector<std::vector<int>> & input ) {
		std::vector<std::vector<int>> res(input);
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				res[i][j] = MAX_BRIGHTNESS - input[i][j];
			}
		}
        return continue_msg();
    }
};

class limiter_decrementer_body {
public:
    continue_msg operator()( const tbb::flow::tuple<continue_msg,continue_msg> & input ) {
        return continue_msg();
    }
};
