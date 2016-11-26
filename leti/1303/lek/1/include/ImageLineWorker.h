#include <tbb/flow_graph.h>
#include "ThreadsInfo.h"
#include "Image.h"
using namespace tbb::flow;

template <typename ResultOutput, typename FunctionOutput>
class ImageLineWorker {
protected:
    // Structure for generating sequence for sequencer_buffer.
    struct Sequencer {
        size_t operator()(FunctionOutput line) {
            static unsigned int counter = 0;
            counter++;
            return (counter - 1);
        }
    };

    template<typename FunctionBody, typename OutputBody>
    void runTaskOnGraph(Image* image, FunctionBody functionBody,
                        OutputBody outputBody) {
        graph imageGraph;
        broadcast_node<tuple<unsigned char*, unsigned int, unsigned int>> input(imageGraph);
        // Limit number of created working nodes.
        limiter_node<tuple<unsigned char*, unsigned int, unsigned int>> limiter(imageGraph,
            ThreadsInfo::THREADS_INC_COEF * ThreadsInfo::THREADS_NUM);
        // Node for filling.
        function_node<tuple<unsigned char*, unsigned int, unsigned int>, FunctionOutput> lineFiller(imageGraph, unlimited,
            functionBody);
        queue_node<FunctionOutput> buffer(imageGraph);
        // Output node.
        function_node<FunctionOutput, continue_msg> output(imageGraph, serial, outputBody);

        // Add edges to graph.
        make_edge(input, limiter);
        make_edge(limiter, lineFiller);
        make_edge(lineFiller, buffer);
        make_edge(buffer, output);
        make_edge(output, limiter.decrement);

        // Put image lines to graph input.
        for (unsigned int i = 0; i < image->getHeight(); i++) {
            input.try_put(tuple<unsigned char*, unsigned int, unsigned int>(image->getImageLine(i), image->getWidth(), i));
        }
        imageGraph.wait_for_all();
    }

public:
    virtual ResultOutput operator()(Image* image) = 0;
};
