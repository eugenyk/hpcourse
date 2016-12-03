#include <cstdio>
#include <image.h>
#include <iostream>
#include <vector>
#include "maxbrightness.h"
#include "minbrightness.h"
#include "fixedbrightness.h"
#include "pointhighlighter.h"
#include "joinkeymatcher.h"


int main() {
    unsigned char brightnessSearchValue = 128;
    tbb::flow::graph g;

    tbb::flow::broadcast_node< Image * > start(g);

    // Step 1 nodes
    tbb::flow::function_node< Image * , PointsWithImage > maxBrightnessNode(g,1,MaxBrightness());
    tbb::flow::function_node< Image * , PointsWithImage > minBrightnessNode(g,1,MinBrightness());
    tbb::flow::function_node< Image * , PointsWithImage > fixedBrightnessNode(g,1,FixedBrightness(brightnessSearchValue));

    // Step 2 nodes
    tbb::flow::join_node< ThreeMessages, tbb::flow::key_matching< Image* > >
                        myJoinNode(g, JoinKeyMatcher(), JoinKeyMatcher(), JoinKeyMatcher());
    tbb::flow::function_node< ThreeMessages, Image * > pointHighlighterNode(g,1, PointHighlighter());

    // Step 3 nodes





    tbb::flow::make_edge( start, maxBrightnessNode );
    tbb::flow::make_edge( start, minBrightnessNode );
    tbb::flow::make_edge( start, fixedBrightnessNode );

    tbb::flow::make_edge(maxBrightnessNode, tbb::flow::input_port<0>(myJoinNode));
    tbb::flow::make_edge(minBrightnessNode, tbb::flow::input_port<1>(myJoinNode));
    tbb::flow::make_edge(fixedBrightnessNode, tbb::flow::input_port<2>(myJoinNode));

    tbb::flow::make_edge(myJoinNode,pointHighlighterNode);

    for (int i = 0; i < 30; ++i ) {
        start.try_put( new Image());
    }
    g.wait_for_all();

    return 0;
}
