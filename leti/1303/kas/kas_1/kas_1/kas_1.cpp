#include "stdafx.h"
#include <process.h>
#include "tbb/flow_graph.h"
#include <windows.h>
#include "Image.h"

using namespace tbb::flow;
using namespace std;

struct max_brightness {
	vector<int> operator()(Image *img) {
		unsigned char* img_map = img->getMap();
		unsigned char max = CHAR_MIN;
		vector<int> arr_max_index;
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
		string str = "";
		for (vector<int>::iterator it = arr_max_index.begin(); it != arr_max_index.end(); ++it)
		{
			str += to_string(*it) + " ";
		}
		printf("Max value=%d, Array of index of max value: {%s}\n", max, str.c_str());
		return arr_max_index;
	}
};

struct min_brightness {
	vector<int> operator()(Image *img) {
		unsigned char* img_map = img->getMap();
		unsigned char min = CHAR_MAX;
		vector<int> arr_min_index;
		int h = img->getHeight();
		int w = img->getWidth();
		for (int i = 0; i < h * w; ++i)
		{
			if (img_map[i] < min)
			{
				min = img_map[i];

				//cleanup arrary of min index
				arr_min_index.clear();
				arr_min_index.push_back(i);
			}
			else if (img_map[i] == min)
			{
				arr_min_index.push_back(i);
			}
		}
		string str = "";
		for (vector<int>::iterator it = arr_min_index.begin(); it != arr_min_index.end(); ++it)
		{
			str += to_string(*it) + " ";
		}
		printf("Min value=%d, Array of index of min value: {%s}\n", min, str.c_str());
		return arr_min_index;
	}
};

class cnt_brightness {
	unsigned char input_brightness;

public:
	cnt_brightness(unsigned char input_brightness) : input_brightness(input_brightness) {}

	vector<int> operator()(Image *img) {
		unsigned char* img_map = img->getMap();
		vector<int> arr_eq_index;
		int h = img->getHeight();
		int w = img->getWidth();
		for (int i = 0; i < h * w; ++i)
		{
			if (img_map[i] == input_brightness)
			{
				arr_eq_index.push_back(i);
			}
		}
		
		string str = "";
		for (vector<int>::iterator it = arr_eq_index.begin(); it != arr_eq_index.end(); ++it)
		{
			str += to_string(*it) + " ";
		}
		printf("User brightness=%d, Array of vaulue's index equals user brightness: {%s}\n", input_brightness, str.c_str());
		return arr_eq_index;
	}
};

class highlight {
public:
	int operator()(tuple<Image*, vector<int>, vector<int>, vector<int>> v) {
		Image* img = get<0>(v);
		vector<int> arr_max_ind = get<1>(v);
		vector<int> arr_min_ind = get<2>(v);
		vector<int> arr_inp_ind = get<3>(v);

		/*printf("array of max index: (size=%d)\n", arr_max_ind.size());
		for (vector<int>::iterator it = arr_max_ind.begin(); it != arr_max_ind.end(); ++it)
		{
			printf("%d ", *it);
		}*/
		return 0;
	}
};


int main() {
	srand(time(NULL));

	//todo: get from args
	unsigned char input_brightness = 100;

	graph g;
	Image *img1 = new Image(10, 10);
	img1->printMap();

	broadcast_node<Image*> s(g);
	function_node<Image*, vector<int>> max_brightness_node(g, unlimited, max_brightness());
	function_node<Image*, vector<int>> min_brightness_node(g, unlimited, min_brightness());
	function_node<Image*, vector<int>> cnt_brightness_node(g, unlimited, cnt_brightness(input_brightness));
	join_node<tuple<Image*, vector<int>, vector<int>, vector<int>>, queueing> join(g);
	function_node<tuple<Image*, vector<int>, vector<int>, vector<int>>, int> highlight_node(g, unlimited, highlight());

	make_edge(s, max_brightness_node);
	make_edge(s, min_brightness_node);
	make_edge(s, cnt_brightness_node);
	make_edge(s, input_port<0>(join));
	make_edge(max_brightness_node, input_port<1>(join));
	make_edge(min_brightness_node, input_port<2>(join));
	make_edge(cnt_brightness_node, input_port<3>(join));
	make_edge(join, highlight_node);

	s.try_put(img1);
	g.wait_for_all();

	system("pause");
	return 0;
}