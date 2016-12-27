// FlowGraphImage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <tbb/flow_graph.h>
#include "tbb/tbb.h"
#include "tbb/blocked_range2d.h"
#include "image.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace tbb::flow;
using namespace std;



int main(int argc, char *argv[]) {
	int result = 0;
	byte bright_val = 100;
	int image_limit = 5;
	srand(time(NULL));
	graph g;

	
	auto find_max_pixel = [](const Image& input){
		cout << "find max" << endl;
		Sleep(1000);
		return input.max_pixel();
	
	};
	auto find_min_pixel = [](const Image& input){
		cout << "find min" << endl;
		Sleep(1000);
		return input.min_pixel();
		
	};
	auto find_equil_pixel = [&bright_val](const Image& input){
		cout << "find" << endl;
		Sleep(1000);
		return input.find_pixel(bright_val);
		
	};
	auto source = [&](Image& result)
		{
		static int counter = 0;
		if (counter >= 5)return false;
		result = Image();
		result.print_image();
		++counter;
		return true;
		};
	auto distinguish_pixels = [](tuple<Image, Pixels, Pixels, Pixels> in){
		auto im = tbb::flow::get<0>(in);
		im.lead_point(tbb::flow::get<1>(in));
		im.lead_point(tbb::flow::get<2>(in));
		im.lead_point(tbb::flow::get<3>(in));
		cout << "dist" << endl;
		Sleep(1000);
		return im;
	};
	auto inverse_image = [](const Image& input)
	{
		Image new_im(input);
		new_im.inverse_image();
		cout << "inverse" << endl;
		Sleep(1000);
		return new_im;
	};
	auto find_mean_brightness = [](const Image& input)
	{
		cout << "brightness" << endl;
		return input.mean_brightness();
	};
	ofstream out_file("output.txt");
	auto file_output = [&out_file](double b)
	{
		out_file << b << endl;
		return continue_msg();
	};

	source_node<Image> input(g, source);
	limiter_node<Image> limit_node(g, image_limit);
	broadcast_node<Image> broad_node(g);
	function_node<Image, Pixels> f_find_max(g, unlimited, find_max_pixel);
	function_node<Image, Pixels> f_find_min(g, unlimited, find_min_pixel);
	function_node<Image, Pixels> f_find_equil(g, unlimited, find_equil_pixel);
	join_node<tuple<Image, Pixels, Pixels, Pixels>> join_node(g);
	function_node<tuple<Image, Pixels, Pixels, Pixels>, Image> f_dist_pixel(g, unlimited, distinguish_pixels);
	function_node<Image> f_invers_image(g, unlimited, inverse_image);
	function_node<Image, double> f_find_mean(g, unlimited, find_mean_brightness);
	function_node<double> f_file_output(g, unlimited, file_output);
	make_edge(input, limit_node);
	make_edge(limit_node, broad_node);
	make_edge(broad_node, f_find_max);
	make_edge(broad_node, f_find_min);
	make_edge(broad_node, f_find_equil);
	make_edge(f_find_max, input_port<1>(join_node));
	make_edge(f_find_min, input_port<2>(join_node));
	make_edge(f_find_equil, input_port<3>(join_node));
	make_edge(broad_node, input_port<0>(join_node));
	make_edge(join_node, f_dist_pixel);
	make_edge(f_dist_pixel, f_invers_image);
	make_edge(f_dist_pixel, f_find_mean);
	make_edge(f_find_mean, f_file_output);
	input.activate();
	g.wait_for_all();


	return 0;
}
