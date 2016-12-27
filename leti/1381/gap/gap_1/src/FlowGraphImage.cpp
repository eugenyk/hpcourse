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
#include "Arg_parser.h"

using namespace tbb::flow;
using namespace std;



int main(int argc, char *argv[]) {
	Arg_parser parser(argc, argv);
	srand(time(NULL));
	graph g;
		
	auto source = [&parser](Image& result)
	{
		static int counter = 0;
		if (counter >= parser.get_number_image())return false;
		result = Image();
		++counter;
		return true;
	};
	source_node<Image> input(g, source);
	limiter_node<Image> limit_node(g, parser.get_image_limit());
	broadcast_node<Image> broad_node(g);
	auto find_max_pixel = [](const Image& input){return input.max_pixel();};
	function_node<Image, Pixels> f_find_max(g, unlimited, find_max_pixel);
	auto find_min_pixel = [](const Image& input){return input.min_pixel();};
	function_node<Image, Pixels> f_find_min(g, unlimited, find_min_pixel);
	auto find_equil_pixel = [&parser](const Image& input){return input.find_pixel(parser.get_brightness()); };
	function_node<Image, Pixels> f_find_equil(g, unlimited, find_equil_pixel);
	join_node<tuple<Image, Pixels, Pixels, Pixels>> join_vector(g);
	auto distinguish_pixels = [](tuple<Image, Pixels, Pixels, Pixels> in){
		auto im = tbb::flow::get<0>(in);
		im.lead_point(tbb::flow::get<1>(in));
		im.lead_point(tbb::flow::get<2>(in));
		im.lead_point(tbb::flow::get<3>(in));
		return im;
	};
	function_node<tuple<Image, Pixels, Pixels, Pixels>, Image> f_dist_pixel(g, unlimited, distinguish_pixels);
	auto inverse_image = [](const Image& input)
	{
		Image new_im(input);
		new_im.inverse_image();
		return new_im;
	};
	function_node<Image> f_invers_image(g, unlimited, inverse_image);
	auto find_mean_brightness = [](const Image& input){	return input.mean_brightness();};
	function_node<Image, double> f_find_mean(g, unlimited, find_mean_brightness);
	ofstream out_file(parser.get_file_name());
	auto file_output = [&out_file, &parser](double b){if (parser.get_file_name() != "") out_file << b << endl;return continue_msg(); };
	function_node<double> f_file_output(g, serial, file_output);
	join_node<tuple<continue_msg, continue_msg>> finish_join(g);
	function_node<tuple<continue_msg, continue_msg>> finish_image(g, serial, [](tuple<continue_msg, continue_msg>){cout << "finish_image" << endl; });
	make_edge(input, limit_node);
	make_edge(limit_node, broad_node);
	make_edge(broad_node, f_find_max);
	make_edge(broad_node, f_find_min);
	make_edge(broad_node, f_find_equil);
	make_edge(f_find_max, input_port<1>(join_vector));
	make_edge(f_find_min, input_port<2>(join_vector));
	make_edge(f_find_equil, input_port<3>(join_vector));
	make_edge(broad_node, input_port<0>(join_vector));
	make_edge(join_vector, f_dist_pixel);
	make_edge(f_dist_pixel, f_invers_image);
	make_edge(f_dist_pixel, f_find_mean);
	make_edge(f_find_mean, f_file_output);
	make_edge(f_file_output, input_port<0>(finish_join));
	make_edge(f_invers_image, input_port<1>(finish_join));
	make_edge(finish_join, finish_image);
	make_edge(finish_image, limit_node.decrement);

	input.activate();
	g.wait_for_all();
	return 0;
}
