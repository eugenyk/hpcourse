#include "stdafx.h"
#include <process.h>
#include "tbb/flow_graph.h"
#include <windows.h>
#include "Image.h"

using namespace tbb::flow;

struct max_brightness {
	int operator()(Image *img) {
		unsigned char max = CHAR_MIN;
		unsigned char* img_map = img->getMap();
		int h = img->getHeight();
		int w = img->getWidth();
		for (int i = 0; i < h * w; ++i)
		{
			if (img_map[i] > max) max = img_map[i];
		}
		printf("max %d\n", max);
		return max;
	}
};

struct min_brightness {
	int operator()(Image *img) {
		unsigned char min = CHAR_MAX;
		unsigned char* img_map = img->getMap();
		int h = img->getHeight();
		int w = img->getWidth();
		for (int i = 0; i < h * w; ++i)
		{
			if (img_map[i] < min) min = img_map[i];
		}
		printf("min %d\n", min);
		return min;
	}
};

class res {
	int &minb, &maxb;

public:
	res(int &minb, int &maxb) : minb(minb), maxb(maxb) {}
	int operator()(tuple<int, int> v) {
		printf("res %d and %d\n", get<0>(v), get<1>(v));
		minb += get<0>(v);
		maxb += get<1>(v);
		return 0;
	}
};

int main() {
	srand(time(NULL));

	graph g;
	int max = 0, min = 0;
	Image *img1 = new Image(3, 3);
	img1->printMap();

	broadcast_node<Image*> s(g);
	function_node<Image*, int> max_brightness_node(g, unlimited, max_brightness());
	function_node<Image*, int> min_brightness_node(g, unlimited, min_brightness());
	join_node<tuple<int, int>, queueing> join(g);
	function_node<tuple<int, int>, int> result_node(g, serial, res(max, min));

	make_edge(s, max_brightness_node);
	make_edge(s, min_brightness_node);
	make_edge(max_brightness_node, input_port<0>(join));
	make_edge(min_brightness_node, input_port<1>(join));
	make_edge(join, result_node);

	s.try_put(img1);
	g.wait_for_all();

	printf("Final result is max=%d min=%d\n", max, min);
	system("pause");
	return 0;
}