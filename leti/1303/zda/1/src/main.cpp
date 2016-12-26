#include <cstdio>
#include <cstdlib>
#include <image.h>
#include <iostream>
#include <vector>
#include "maxbrightness.h"
#include "minbrightness.h"
#include "fixedbrightness.h"
#include "pointhighlighter.h"
#include "joinkeymatcher.h"
#include "brightnessinverter.h"
#include "meanbrightness.h"
#include "fileoutput.h"
#include "imagegenerator.h"
#include "imagecleanup.h"

int main(int argc, char * argv[])
{
    std::cout << "===============================" << std::endl;
    std::cout << "    Flow graph task (Lab 1)    " << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "  Author: Zykov D.A. Gr: 1303  " << std::endl;
    std::cout << "===============================" << std::endl;

    unsigned char brightnessSearchValue = 128;
    std::string filename;
    unsigned int imageLimit = 5;

    int opt = 0;

    while ((opt = getopt (argc, argv, "b:l:f:h")) != -1) {
        switch(opt) {
        case 'b':
            brightnessSearchValue = std::atoi(optarg);
            break;
        case 'l':
            imageLimit = std::atoi(optarg);
            break;
        case 'f':
            filename = optarg;
            break;
        case 'h':
            std::cout << "Usage:" <<std::endl;
            std::cout << "-b    Brightness value to find" << std::endl;
            std::cout << "-l    Image flow limit" << std::endl;
            std::cout << "-f    Brightness output file" << std::endl;
            std::cout << "===============================" << std::endl;
            break;
        }
    }

    std::cout << "            Settings           " <<std::endl;
    std::cout << "Brightness value:           " << static_cast<int>(brightnessSearchValue) << std::endl;
    std::cout << "Image flow limit:           " << imageLimit << std::endl;
    std::cout << "Output file: " << ((filename.size()>0) ? filename : "none") << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "Running..." << std::endl;

    std::ofstream fileOutput;
    fileOutput.open(filename.c_str(), std::ios::out | std::ios::app);

    tbb::flow::graph g;

    // Generator node
    tbb::flow::source_node< Image * > generatorNode(g, ImageGenerator());

    // Limiter node
    tbb::flow::limiter_node< Image * > inputLimiter(g, imageLimit);

    // Broadcaster node
    tbb::flow::broadcast_node< Image * > inputBroadcaster(g);

    // Step 1 nodes
    tbb::flow::function_node< Image * , PointsWithImage > maxBrightnessNode(g, tbb::flow::unlimited, MaxBrightness());
    tbb::flow::function_node< Image * , PointsWithImage > minBrightnessNode(g, tbb::flow::unlimited, MinBrightness());
    tbb::flow::function_node< Image * , PointsWithImage > fixedBrightnessNode(g, tbb::flow::unlimited, FixedBrightness(brightnessSearchValue));

    // Step 2 nodes
    tbb::flow::join_node< MinMaxFixedResults, tbb::flow::key_matching< Image* > >
    minMaxFixedJoinNode(g, JoinKeyMatcher(), JoinKeyMatcher(), JoinKeyMatcher());
    tbb::flow::function_node< MinMaxFixedResults, Image * > pointHighlighterNode(g, tbb::flow::unlimited, PointHighlighter());
    tbb::flow::broadcast_node< Image * > highlitedBroadcaster(g);

    // Step 3 nodes
    tbb::flow::function_node< Image * , ImageWithInverted> brightnessInverterNode(g, tbb::flow::unlimited, BrightnessInverter());
    tbb::flow::function_node< Image * , ImageWithMean> meanBrightnessNode(g, tbb::flow::unlimited, MeanBrightness());

    // Cleanup
    tbb::flow::join_node< MeanAndInvertedResults, tbb::flow::key_matching< Image* > >
    meanAndInvertedJoinNode(g, JoinKeyMatcher(), JoinKeyMatcher());
    tbb::flow::function_node<MeanAndInvertedResults, double> cleanupNode(g, tbb::flow::unlimited, ImageCleanup());

    // Output node
    tbb::flow::function_node<double, tbb::flow::continue_msg> fileOutputNode(g, 1, FileOutput(fileOutput));

    // Limiting input flow
    tbb::flow::make_edge(generatorNode, inputLimiter);
    tbb::flow::make_edge(inputLimiter, inputBroadcaster);

    // Broadcasting images to 3 nodes
    tbb::flow::make_edge(inputBroadcaster, maxBrightnessNode );
    tbb::flow::make_edge(inputBroadcaster, minBrightnessNode );
    tbb::flow::make_edge(inputBroadcaster, fixedBrightnessNode );

    // Joining results
    tbb::flow::make_edge(maxBrightnessNode, tbb::flow::input_port<0>(minMaxFixedJoinNode));
    tbb::flow::make_edge(minBrightnessNode, tbb::flow::input_port<1>(minMaxFixedJoinNode));
    tbb::flow::make_edge(fixedBrightnessNode, tbb::flow::input_port<2>(minMaxFixedJoinNode));

    // Passing joined results to highlighter
    tbb::flow::make_edge(minMaxFixedJoinNode, pointHighlighterNode);
    tbb::flow::make_edge(pointHighlighterNode, highlitedBroadcaster);

    // Broadcasting modified images
    tbb::flow::make_edge(highlitedBroadcaster, brightnessInverterNode);
    tbb::flow::make_edge(highlitedBroadcaster, meanBrightnessNode);

    // Joining results
    tbb::flow::make_edge(meanBrightnessNode, tbb::flow::input_port<0>(meanAndInvertedJoinNode));
    tbb::flow::make_edge(brightnessInverterNode, tbb::flow::input_port<1>(meanAndInvertedJoinNode));

    // Image cleanup & passing mean
    tbb::flow::make_edge(meanAndInvertedJoinNode,cleanupNode);

    // Passing mean to file output node
    tbb::flow::make_edge(cleanupNode, fileOutputNode);

    // Informing limiter
    tbb::flow::make_edge(fileOutputNode, inputLimiter.decrement);

    g.wait_for_all();

    return 0;
}
