#include <cstdio>
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

int main() {
    unsigned char brightnessSearchValue = 128;
    std::string filename = "File.txt";
    unsigned int imageLimit = 5;

    tbb::flow::graph g;

    // Generator node
    tbb::flow::source_node< Image * > generatorNode(g, ImageGenerator());

    // Limiter node
    tbb::flow::limiter_node< Image * > inputLimiter(g, imageLimit);

    // Step 1 nodes
    tbb::flow::function_node< Image * , PointsWithImage > maxBrightnessNode(g, tbb::flow::unlimited, MaxBrightness());
    tbb::flow::function_node< Image * , PointsWithImage > minBrightnessNode(g, tbb::flow::unlimited, MinBrightness());
    tbb::flow::function_node< Image * , PointsWithImage > fixedBrightnessNode(g, tbb::flow::unlimited, FixedBrightness(brightnessSearchValue));

    // Step 2 nodes
    tbb::flow::join_node< MinMaxFixedResults, tbb::flow::key_matching< Image* > >
                        minMaxFixedJoinNode(g, JoinKeyMatcher(), JoinKeyMatcher(), JoinKeyMatcher());
    tbb::flow::function_node< MinMaxFixedResults, Image * > pointHighlighterNode(g, tbb::flow::unlimited, PointHighlighter());

    // Step 3 nodes
    tbb::flow::function_node< Image * , ImageWithInverted> brightnessInverterNode(g, tbb::flow::unlimited, BrightnessInverter());
    tbb::flow::function_node< Image * , ImageWithMean> meanBrightnessNode(g, tbb::flow::unlimited, MeanBrightness());

    // Cleanup
    tbb::flow::join_node< MeanAndInvertedResults, tbb::flow::key_matching< Image* > >
                        meanAndInvertedJoinNode(g, JoinKeyMatcher(), JoinKeyMatcher());
    tbb::flow::function_node<MeanAndInvertedResults, double> cleanupNode(g, tbb::flow::unlimited, ImageCleanup());

    // Output node
    tbb::flow::function_node<double, tbb::flow::continue_msg> fileOutputNode(g, 1, FileOutput(filename));

    // Limiting input flow
    tbb::flow::make_edge(generatorNode, inputLimiter);

    // Broadcasting images to 3 nodes
    tbb::flow::make_edge(inputLimiter, maxBrightnessNode );
    tbb::flow::make_edge(inputLimiter, minBrightnessNode );
    tbb::flow::make_edge(inputLimiter, fixedBrightnessNode );

    // Joining results
    tbb::flow::make_edge(maxBrightnessNode, tbb::flow::input_port<0>(minMaxFixedJoinNode));
    tbb::flow::make_edge(minBrightnessNode, tbb::flow::input_port<1>(minMaxFixedJoinNode));
    tbb::flow::make_edge(fixedBrightnessNode, tbb::flow::input_port<2>(minMaxFixedJoinNode));

    // Passing joined results to highlighter
    tbb::flow::make_edge(minMaxFixedJoinNode, pointHighlighterNode);

    // Broadcasting modified images
    tbb::flow::make_edge(pointHighlighterNode, brightnessInverterNode);
    tbb::flow::make_edge(pointHighlighterNode, meanBrightnessNode);

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
