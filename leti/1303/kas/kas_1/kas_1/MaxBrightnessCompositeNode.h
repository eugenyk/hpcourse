#include "stdafx.h"
#include "tbb/flow_graph.h"
#include <iostream>
#include <tuple>
#include "Image.h"
#include <vector>

using namespace tbb::flow;

#pragma once
class MaxBrightnessCompositeNode : public composite_node<  tuple< Image* >, tuple< std::vector<int> > >
{
private:
	function_node<Image*, tuple<Image*, Image*, Image*, Image* > > fork_node;
	split_node<tuple<Image*, Image*, Image*, Image* > > s;
	function_node<Image*, std::vector<int > > fnode1;
	function_node<Image*, std::vector<int > > fnode2;
	function_node<Image*, std::vector<int > > fnode3;
	function_node<Image*, std::vector<int > > fnode4;
	join_node < tuple<std::vector<int >, std::vector<int >, std::vector<int >, std::vector<int> > > j;
	function_node< tuple<std::vector<int >, std::vector<int >, std::vector<int >, std::vector<int> >, std::vector<int> > merge_node;

	typedef composite_node < tuple< Image* >, tuple< std::vector<int> > > base_type;

	struct fnode_body
	{
		std::vector<int> operator() (const Image* img);
	};

	struct fork_body
	{
		tuple<Image*, Image*, Image*, Image* > operator() (tuple<Image*> tuple_img);
	};

	struct merge_body
	{
		std::vector<int> operator() (tuple<std::vector<int >, std::vector<int >, std::vector<int >, std::vector<int> > image_parts);
	};

public:
	MaxBrightnessCompositeNode(graph &g);
	~MaxBrightnessCompositeNode();
};

