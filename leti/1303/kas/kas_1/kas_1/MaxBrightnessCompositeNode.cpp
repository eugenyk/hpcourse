#include "stdafx.h"
#include "MaxBrightnessCompositeNode.h"


MaxBrightnessCompositeNode::MaxBrightnessCompositeNode(graph &g) : base_type(g), j(g), s(g),
	fork_node(g, unlimited, fork_body()),
	merge_node(g, unlimited, merge_body()),
	fnode1(g, unlimited, fnode_body()),
	fnode2(g, unlimited, fnode_body()),
	fnode3(g, unlimited, fnode_body()),
	fnode4(g, unlimited, fnode_body())
{
	make_edge(fork_node, s);
	make_edge(s, fnode1);
	make_edge(s, fnode2);
	make_edge(s, fnode3);
	make_edge(s, fnode4);

	make_edge(fnode1, input_port<0>(j));
	make_edge(fnode2, input_port<1>(j));
	make_edge(fnode3, input_port<2>(j));
	make_edge(fnode4, input_port<3>(j));

	make_edge(j, merge_node);

	base_type::input_ports_type input_tuple(fork_node);
	base_type::output_ports_type output_tuple(merge_node);
	base_type::set_external_ports(input_tuple, output_tuple);
}


MaxBrightnessCompositeNode::~MaxBrightnessCompositeNode()
{
}

std::vector<int> MaxBrightnessCompositeNode::fnode_body::operator()(const Image* img)
{
	unsigned char* img_map = img->getMap();
	unsigned char max = CHAR_MIN;
	std::vector<int> arr_max_index;
	int h = img->getHeight();
	int w = img->getWidth();
	for (int i = 0; i < h * w; ++i)
	{
		if (img_map[i] > max)
		{
			max = img_map[i];

			//cleanup arrary of max index
			arr_max_index.clear();
			arr_max_index.push_back(i);
		}
		else if (img_map[i] == max)
		{
			arr_max_index.push_back(i);
		}
	}
	std::string str = "";
	for (std::vector<int>::iterator it = arr_max_index.begin(); it != arr_max_index.end(); ++it)
	{
		str += std::to_string(*it) + " ";
	}
	printf("Max value=%d, Array of index of max value: {%s}\n", max, str.c_str());
	return arr_max_index;
}

tuple<Image*, Image*, Image*, Image*> MaxBrightnessCompositeNode::fork_body::operator()(tuple<Image*> tuple_img)
{
	Image* img = get<0>(tuple_img);
	std::vector<Image*> imgs = img->divByParts(4);
	return std::make_tuple(imgs.at(0), imgs.at(1), imgs.at(2), imgs.at(3));
}

std::vector<int> MaxBrightnessCompositeNode::merge_body::operator()(tuple<std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>> image_parts)
{
	std::vector<int> indMax0 = get<0>(image_parts);
	std::vector<int> indMax1 = get<1>(image_parts);
	std::vector<int> indMax2 = get<2>(image_parts);
	std::vector<int> indMax3 = get<3>(image_parts);

	MaxBrightnessCompositeNode::im->divByParts(4);
	//unsigned char maxPart0 = [indMax0.at(0)];

	//TODO: implement...
	return std::vector<int>();
}
