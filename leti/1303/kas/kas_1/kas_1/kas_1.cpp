#include "stdafx.h"
#include <process.h>
#include "tbb/flow_graph.h"
#include <windows.h>
#include "Image.h"
#include "Utils.h"

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
		unsigned char min = UCHAR_MAX;
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
	Image* operator()(tuple<Image*, vector<int>, vector<int>, vector<int>> v) {
		Image* img = get<0>(v);
		vector<int> arr_max_ind = get<1>(v);
		vector<int> arr_min_ind = get<2>(v);
		vector<int> arr_inp_ind = get<3>(v);

		//to highlight the adj of max values.
		for (vector<int>::iterator it = arr_max_ind.begin(); it != arr_max_ind.end(); ++it)
		{
			toHighlight(img, *it);
		}

		//to highlight the adj of min values.
		for (vector<int>::iterator it = arr_min_ind.begin(); it != arr_min_ind.end(); ++it)
		{
			toHighlight(img, *it);
		}

		//to highlight the adj of input values.
		for (vector<int>::iterator it = arr_inp_ind.begin(); it != arr_inp_ind.end(); ++it)
		{
			toHighlight(img, *it);
		}
		return img;
	}

	//to highlight the adj pixels.
	void toHighlight(Image* img, int pixelIndex)
	{
		int w = img->getWidth();
		int h = img->getHeight();
		
		int l = pixelIndex - 1;
		int r = pixelIndex + 1;
		int d = pixelIndex + w;
		int dl = pixelIndex + w - 1;
		int dr = pixelIndex + w + 1;
		int u = pixelIndex - w;
		int ul = pixelIndex - w - 1;
		int ur = pixelIndex - w + 1;

		vector<int> adj_pixs = { u, d };
		//left border.
		if (pixelIndex % w != 0)
		{
			adj_pixs.push_back(ul);
			adj_pixs.push_back(l);
			adj_pixs.push_back(dl);
		}

		//right border.
		if ((pixelIndex + 1) % w != 0)
		{
			adj_pixs.push_back(ur);
			adj_pixs.push_back(r);
			adj_pixs.push_back(dr);
		}

		for (vector<int>::iterator it = adj_pixs.begin(); it != adj_pixs.end(); ++it)
		{
			int cur_pix = *it;
			if (cur_pix >= 0 && cur_pix < w * h)
			{

				img->updPix(cur_pix, 0);
			}
		}
	}
};

struct process_inv {
	Image* operator()(Image *img) {
		Image inv_img = *img;

		unsigned char* img_map = inv_img.getMap();
		int h = inv_img.getHeight();
		int w = inv_img.getWidth();
		for (int i = 0; i < h * w; ++i)
		{
			img_map[i] = UCHAR_MAX - img_map[i];
		}

		printf("Print the inversion image\n");
		inv_img.printMap();

		return &inv_img;
	}
};

struct process_avg {
	long operator()(Image *img) {
		unsigned char* img_map = img->getMap();
		int h = img->getHeight();
		int w = img->getWidth();
		long sum = 0;
		for (int i = 0; i < h * w; ++i)
		{
			sum += img_map[i];
		}
		long avg = sum / (h * w);
		printf("AVG=%d\n", avg);
		return avg;
	}
};


int main(int argc, char* argv[]) {
	tuple<int, int, int, string> params = Utils::argsProcessing(argc, argv);
	int n = get<0>(params);
	srand(time(NULL));

	//todo: get from args
	unsigned char input_brightness = 100;

	graph g;
	vector<Image> imgs = Utils::generateImages(10, 10, n);
	Image *img1 = &imgs.front();
	img1->printMap();

	broadcast_node<Image*> s(g);
	function_node<Image*, vector<int>> max_brightness_node(g, unlimited, max_brightness());
	function_node<Image*, vector<int>> min_brightness_node(g, unlimited, min_brightness());
	function_node<Image*, vector<int>> cnt_brightness_node(g, unlimited, cnt_brightness(input_brightness));
	join_node<tuple<Image*, vector<int>, vector<int>, vector<int>>, queueing> join(g);
	function_node<tuple<Image*, vector<int>, vector<int>, vector<int>>, Image*> highlight_node(g, unlimited, highlight());
	function_node<Image*, Image*> process_inv_node(g, unlimited, process_inv());
	function_node<Image*, long> process_avg_node(g, unlimited, process_avg());

	make_edge(s, max_brightness_node);
	make_edge(s, min_brightness_node);
	make_edge(s, cnt_brightness_node);
	make_edge(s, input_port<0>(join));
	make_edge(max_brightness_node, input_port<1>(join));
	make_edge(min_brightness_node, input_port<2>(join));
	make_edge(cnt_brightness_node, input_port<3>(join));
	make_edge(join, highlight_node);
	make_edge(highlight_node, process_inv_node);
	make_edge(highlight_node, process_avg_node);

	s.try_put(img1);
	g.wait_for_all();

	printf("\nPrint after update image\n");
	img1->printMap();

	system("pause");
	return 0;
}

//TODO: 3. fork task (use composite_node + split_node)
//TODO: 5. refactoring
//TODO: 6. output in file
//TODO: 7. add source node
//TODO: 8. message buffer