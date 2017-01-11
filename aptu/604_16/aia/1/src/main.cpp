#include "tbb/flow_graph.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <math.h>

#include "random_image.h"
#include "input_parser.h"

using namespace std;
using namespace random_image;
using namespace input_parser;
using namespace tbb::flow;

typedef std::vector<Point> Points; 

const int DEFAULT_IMAGE_AMOUNT  = 10000;

int main(int argc, char** argv) {
    srand(time(0));
    InputParser parser(argc, argv);
    if (parser.isValid())
    {
        std::cout << parser.toString() << std::endl;
    }
    else
    {
        std::cout << __func__ << ": input parameters are invalid!" << std::endl;
        std::cout << "HOWTO: -b <brightness [0, 255]>  -l <task limit> -f <output file path>" << endl;
        return -1;
    }
    
    ofstream output(parser.m_logName);
    
    auto generateMessage = [&](RandomImage& result)
    {
        static int counter = 0;
        if (counter >= DEFAULT_IMAGE_AMOUNT)
        {
            return false;
        }
        result = RandomImage();
        ++counter;

        return true;
    };
    auto maxBrightPoints = [] (const RandomImage& input)
    {
        return input.getMaxBrightPoints();
    };
    auto minBrightPoints = [] (const RandomImage& input)
    {
        return input.getMinBrightPoints();
    };
    auto targetBrightPoints = [&parser] (const RandomImage& input)
    {
        return input.getTargetBrightPoints(parser.m_targetBrightness);
    };
    auto identify = [] (tuple<RandomImage, Points, Points, Points> input)
    {
        RandomImage image = get<0>(input);
        image.identifyPoints(get<1>(input));
        image.identifyPoints(get<2>(input));
        image.identifyPoints(get<3>(input)); 
        
        return image;
    };
    auto invert = [] (const RandomImage& input)
    {
        RandomImage help(input);
        help.invertBrightness();
        return help;
    };
    auto mean = [] (const RandomImage& input)
    {
        return input.getMeanBrightness();
    };
    auto printer = [&output](double meanValue)
    {
        output << "Mean value = " << meanValue << std::endl;
        return continue_msg();
    };
    
    graph graph;
    
    source_node<RandomImage> sourceNode(graph, generateMessage);
    limiter_node<RandomImage> limitNode(graph, parser.m_taskLimit);
    broadcast_node<RandomImage> broadcastFst(graph);
    
    function_node<RandomImage, Points, rejecting> maxBrightNode(graph, serial, maxBrightPoints);
    function_node<RandomImage, Points, rejecting> minBrightNode(graph, serial, minBrightPoints);
    function_node<RandomImage, Points, rejecting> targetBrightNode(graph, serial, targetBrightPoints);
    
    
    join_node<tuple<RandomImage, Points, Points, Points>, queueing> joinNode(graph);
    function_node<tuple<RandomImage, Points, Points, Points>, RandomImage, rejecting> identifyNode(graph, unlimited, identify);
    
    broadcast_node<RandomImage> broadcastSnd(graph);
    function_node<RandomImage, RandomImage, rejecting> invertNode(graph, serial, invert);
    function_node<RandomImage, double, rejecting> meanNode(graph, serial, mean);
    function_node<double, continue_msg, rejecting> printNode(graph, serial, printer);
    
    make_edge(sourceNode, limitNode);
    make_edge(limitNode, broadcastFst);
    
    make_edge(broadcastFst, maxBrightNode);
    make_edge(broadcastFst, minBrightNode);
    make_edge(broadcastFst, targetBrightNode);
    
    make_edge(broadcastFst, input_port<0>(joinNode));
    make_edge(maxBrightNode, input_port<1>(joinNode));
    make_edge(minBrightNode, input_port<2>(joinNode));
    make_edge(targetBrightNode, input_port<3>(joinNode));
    
    make_edge(joinNode, identifyNode);
    make_edge(identifyNode, broadcastSnd);
    make_edge(broadcastSnd, meanNode);
    make_edge(broadcastSnd, invertNode);
    
    make_edge(meanNode, printNode);
    make_edge(printNode, limitNode.decrement);
    
    sourceNode.activate();
    graph.wait_for_all();
    
    cout << "Done!" << endl;
    
    return 0;
}

