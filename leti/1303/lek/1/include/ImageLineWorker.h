#ifndef IMAGE_LINE_WORKER_H
#define IMAGE_LINE_WORKER_H

#include <tbb/flow_graph.h>
#include "ExtraInfo.h"
#include "Image.h"

using namespace tbb::flow;

/// Abstract class for all tasks on image which can be run in parallel for each line.
template <typename ResultOutput, typename FunctionOutput, typename Input = Image*>
class ImageLineWorker {
protected:
    /// Run task on flow graph.
    template<typename FunctionBody, typename OutputBody>
    void runTaskOnGraph(Image* image, FunctionBody functionBody,
                        OutputBody outputBody) {
        graph imageGraph;
        // Limit number of created working nodes.
        limiter_node<tuple<unsigned char*, unsigned int, unsigned int>> limiter(imageGraph,
                                                                                ExtraInfo::THREADS_NUM);
        // Main task node.
        function_node<tuple<unsigned char*, unsigned int, unsigned int>, FunctionOutput> taskNode(
            imageGraph, unlimited, functionBody);
        queue_node<FunctionOutput> buffer(imageGraph);
        // Output node.
        function_node<FunctionOutput, continue_msg> output(imageGraph, serial, outputBody);

        // Add edges to graph.
        make_edge(limiter, taskNode);
        make_edge(taskNode, buffer);
        make_edge(buffer, output);
        make_edge(output, limiter.decrement);

        // Put image lines to graph input.
        for (unsigned int i = 0; i < image->getHeight(); i++) {
            while(!limiter.try_put(tuple<unsigned char*, unsigned int, unsigned int>(
                image->getImageLine(i), image->getWidth(), i)));
        }
        imageGraph.wait_for_all();
    }

public:
    /// Operator to run task.
    virtual ResultOutput operator()(Input input) = 0;
};

#endif
