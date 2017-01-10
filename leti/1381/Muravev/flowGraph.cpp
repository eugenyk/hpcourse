#include "flowGraph.h"
using namespace tbb;
void flowGraph(base args)
{
	flow::graph g;
	flow::broadcast_node<image8u> inputNode(g);
	flow::function_node<image8u, foundValue8u> findMinValNode(g, args.imagSizeMax, [](image8u img) {
		foundValue8u fV;
		fV.imageFV = img;
		fV.val = findMinMean(img);
		return fV;
	});
	flow::function_node<foundValue8u, foundPixels8u> findMinPixelsNode(g, args.imagSizeMax, [](foundValue8u fv) {
		foundPixels8u fP;
		fP.imageFP = fv.imageFV;
		fP.vals = findValue(fP.imageFP, fv.val);
		return fP;
	});
	flow::function_node<foundPixels8u, image8u> markMinPixelsNode(g, args.imagSizeMax, [](foundPixels8u img) {
		parallel_for(0, int(img.vals.size()), 1, [&img](size_t i) {
			markElement(img.imageFP, img.vals[i]);
		});
		return img.imageFP;
	});
	flow::function_node<image8u, foundValue8u> findMaxValNode(g, args.imagSizeMax, [](image8u img) {
		foundValue8u fV;
		fV.imageFV = img;
		fV.val = findMaxValue(img);
		return fV;
	});
	flow::function_node<foundValue8u, foundPixels8u> findMaxPixelsNode(g, args.imagSizeMax, [](foundValue8u fv) {
		foundPixels8u fP;
		fP.imageFP = fv.imageFV;
		fP.vals = findValue(fP.imageFP, fv.val);
		return fP;
	});
	flow::function_node<foundPixels8u, image8u> markMaxPixelsNode(g, args.imagSizeMax, [](foundPixels8u fPImage) {
		parallel_for(0, int(fPImage.vals.size()), 1, [&fPImage](size_t i) {
			markElement(fPImage.imageFP, fPImage.vals[i]);
		});
		return fPImage.imageFP;
	});
	flow::function_node<image8u, foundPixels8u> findPixelsNode(g, args.imagSizeMax, [args](image8u img) {
		foundPixels8u fP;
		fP.imageFP = img;
		fP.vals = findValue(img, static_cast<unsigned char>(args.spec));
		return fP;
	});
	flow::function_node<foundPixels8u, image8u> markSpecPixelsNode(g, args.imagSizeMax, [](foundPixels8u fPImage) {
		parallel_for(0, int(fPImage.vals.size()), 1, [&fPImage](size_t i) {
			markElement(fPImage.imageFP, fPImage.vals[i]);
		});
		return fPImage.imageFP;
	});
	
	flow::join_node<jointype, flow::tag_matching> joinNode(g,
         [](const image8u& img)->int{return img.id;},
         [](const image8u& img)->int{return img.id;},
         [](const image8u& img)->int{return img.id;}
     );
	 
	flow::function_node<jointype, image8u> inverseNode(g, args.imagSizeMax, [](jointype t) {
		image8u img = std::get<0>(t);
		parallel_for(0, img.height, 1, [img](size_t i) {
			for (int j = 0; j < img.width; j++)
				img.data[i*img.width + j] = ~img.data[i*img.width + j];
		});
		return img;
	});
	flow::function_node<image8u, avgBrightness8u> avgBrightnessNode(g, args.imagSizeMax, [](image8u img) {
		avgBrightness8u aB;
		aB.imageAB = img;
		aB.avg = 0;
		double* buffer = new double[img.height];
		parallel_for(0, img.height, 1, [buffer, img](size_t i) {
			buffer[i] = 0;
			for (int j = 0; j < img.width; j++)
				buffer[i] += img.data[i * img.width + j];
			buffer[i] /= img.width;
		});
		for (int i = 0; i < img.height; i++)
			aB.avg += buffer[i];
		aB.avg /= img.height;
		delete[] buffer;
		return aB;
	});
	flow::buffer_node<avgBrightness8u> avgResultNode(g);

	flow::make_edge(inputNode, findMinValNode);
	flow::make_edge(findMinValNode, findMinPixelsNode);
	flow::make_edge(findMinPixelsNode, markMinPixelsNode);
	flow::make_edge(inputNode, findMaxValNode);
	flow::make_edge(findMaxValNode, findMaxPixelsNode);
	flow::make_edge(findMaxPixelsNode, markMaxPixelsNode);
	flow::make_edge(inputNode, findPixelsNode);
	flow::make_edge(findPixelsNode, markSpecPixelsNode);
	flow::make_edge(markMinPixelsNode, flow::input_port<0>(joinNode));
	flow::make_edge(markMaxPixelsNode, flow::input_port<1>(joinNode));
	flow::make_edge(markSpecPixelsNode, flow::input_port<2>(joinNode));
	flow::make_edge(joinNode, inverseNode);
	flow::make_edge(inverseNode, avgBrightnessNode);
	flow::make_edge(avgBrightnessNode, avgResultNode);

	for (int i = 0; i < args.imagSizeMax; i++) {
		image8u img;
		img.id = i;
		img.createImage(args.width, args.height);
		inputNode.try_put(img);
	}

	g.wait_for_all();

	std::vector<avgBrightness8u> vec;
	for (int i = 0; i < args.imagSizeMax; i++) {
		avgBrightness8u v;
		avgResultNode.try_get(v);
		vec.push_back(v);
	}

	if (args.logData)
		logAvgBrightness(vec, args.fileName);
}
