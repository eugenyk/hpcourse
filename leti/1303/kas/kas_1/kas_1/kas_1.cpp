#include "stdafx.h"
#include <process.h>
#include "tbb/flow_graph.h"
#include <windows.h>
#include "Image.h"

using namespace tbb::flow;

struct max_brightness {
	int operator()(Image *img) {
		unsigned char* img_map = img->getMap();
		unsigned char max = img_map[0];
		int max_index = 0;
		int h = img->getHeight();
		int w = img->getWidth();
		for (int i = 1; i < h * w; ++i)
		{
			if (img_map[i] > max)
			{
				max = img_map[i];
				max_index = i;
			}
		}
		printf("max=%d, index=%d\n", max, max_index);
		return max_index;
	}
};

struct min_brightness {
	int operator()(Image *img) {
		unsigned char* img_map = img->getMap();
		unsigned char min = img_map[0];
		int min_index = 0;
		int h = img->getHeight();
		int w = img->getWidth();
		for (int i = 1; i < h * w; ++i)
		{
			if (img_map[i] < min)
			{
				min = img_map[i];
				min_index = i;
			}
		}
		printf("min=%d, index=%d\n", min, min_index);
		return min_index;
	}
};

class cnt_brightness {
	unsigned char input_brightness;

public:
	cnt_brightness(unsigned char input_brightness) : input_brightness(input_brightness) {}

	int operator()(Image *img) {
		unsigned char* img_map = img->getMap();
		int cnt = 0;
		int h = img->getHeight();
		int w = img->getWidth();
		for (int i = 0; i < h * w; ++i)
		{
			if (img_map[i] == input_brightness) cnt++;
		}
		printf("cnt %d\n", cnt);
		return cnt;
	}
};

class highlight {
	Image &img;
public:
	//highlight(int &minb, int &maxb, int &cnt) : minb(minb), maxb(maxb), cnt(cnt) {}
	highlight(Image &img) : img(img) {}
	int operator()(tuple<int, int, int> v) {
		int max_ind = get<0>(v);
		int min_ind = get<1>(v);
		int inp_ind = get<2>(v);
		printf("highlight max_ind=%d min_ind=%d inp_ind=%d\n", max_ind, min_ind, inp_ind);
		//minb += get<0>(v);
		//maxb += get<1>(v);
		//cnt = get<2>(v);
		return 0;
	}
};

//class res {
//	int &minb, &maxb, &cnt;
//
//public:
//	res(int &minb, int &maxb, int &cnt) : minb(minb), maxb(maxb), cnt(cnt) {}
//	int operator()(tuple<int, int, int> v) {
//		printf("res %d and %d and %d\n", get<0>(v), get<1>(v), get<2>(v));
//		minb += get<0>(v);
//		maxb += get<1>(v);
//		cnt = get<2>(v);
//		return 0;
//	}
//};

int main() {
	srand(time(NULL));

	unsigned char input_brightness = 100;

	graph g;
	//int max = 0, min = 0;
	//int cnt_input = 0;
	Image *img1 = new Image(10, 10);
	img1->printMap();

	broadcast_node<Image*> s(g);
	function_node<Image*, int> max_brightness_node(g, unlimited, max_brightness());
	function_node<Image*, int> min_brightness_node(g, unlimited, min_brightness());
	function_node<Image*, int> cnt_brightness_node(g, unlimited, cnt_brightness(input_brightness));
	join_node<tuple<int, int, int>, queueing> join(g);
	//todo: delete afte this node.
	//function_node<tuple<int, int, int>, int> result_node(g, serial, res(max, min, cnt_input));

	function_node<tuple<int, int, int>, int> highlight_node(g, unlimited, highlight(*img1));

	make_edge(s, max_brightness_node);
	make_edge(s, min_brightness_node);
	make_edge(s, cnt_brightness_node);
	make_edge(max_brightness_node, input_port<0>(join));
	make_edge(min_brightness_node, input_port<1>(join));
	make_edge(cnt_brightness_node, input_port<2>(join));
	//make_edge(join, result_node);
	make_edge(join, highlight_node);

	s.try_put(img1);
	g.wait_for_all();

	//printf("Final result is max=%d min=%d cnt=%d\n", max, min, cnt_input);
	system("pause");
	return 0;
}