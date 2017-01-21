#include <iostream>
#include <string>
#include <algorithm>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/flow_graph.h>
#include "ArgumentsParser.h"
#include "ParseException.h"
#include "Generator.h"
#include "FlowGraphBuilder.h"

using namespace tbb;
using namespace std;

static size_t IMAGES_COUNT = 100;
static uint32_t SEED = 42;

int main(int argc, char ** argv) {
    ArgumentsParser parser {};
    try {
        parser.parse(argc, argv);
    } catch (ParseException & e) {
        cerr << e.what() << endl;
        return -1;
    }

    vector <Image> input = Generator::generateImages(IMAGES_COUNT, SEED);

    FlowGraphBuilder::buildFlowGraph(
            input,
            (size_t) parser.getParallelismLimit(),
            (uint8_t) parser.getBrightnessOption(),
            parser.getLogFileName()
    );
    return 0;
}
