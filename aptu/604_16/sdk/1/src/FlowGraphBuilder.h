//
// Created by dsavvinov on 09.12.16.
//

#ifndef TBB_FLOWGRAPHBUILDER_H
#define TBB_FLOWGRAPHBUILDER_H


#include <tbb/flow_graph.h>
#include "typedefs.h"

namespace FlowGraphBuilder {

void buildFlowGraph(std::vector<Image> images, size_t limit, uint8_t brightness, std::string logfile);

void highlight(Image & image, std::vector<std::pair<size_t, size_t>> positions);

}

#endif //TBB_FLOWGRAPHBUILDER_H
