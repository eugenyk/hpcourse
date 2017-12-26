#include <bits/stdc++.h>

#include <tbb/tbb.h>

#include "elements.h"
#include "process.h"

using namespace tbb::flow;

std::string journal_filename = "";
int brightness_interesting = -1;
int images_limit = 0;
int num_threads = 3;

void parse_args(int argc, char **argv) {
//    “-b 123”: интересующее значение яркости, для шага № 2
//    “-l 5”: предел одновременно обрабатываемых приложением изображений
//    “-f log.txt”: имя файла журнала яркостей
    
    const std::vector<std::string> arg_names = {"-b", "-l", "-f"};
    
    for (int i = 0; i < argc; ++i) {
        const std::string args = argv[i];
        if (std::count(arg_names.cbegin(), arg_names.cend(), args) && i + 1 == argc) {
            throw std::logic_error("Need to specify value for argument: " + args);
        }
        if (!args.empty() && args[0] == '-') {
            if (args == "-b") {
                brightness_interesting = atoi(argv[i + 1]);
            } else if (args == "-l") {
                images_limit = atoi(argv[i + 1]);
            } else if (args == "-f") {
                journal_filename = argv[i + 1];
            } else if (!args.empty()) {
                throw std::logic_error("Wrong argument: " + args);
            }
            ++i; // as arguments above require value.
        }
    }
}

int main(int argc, char **argv) {
    parse_args(argc, argv);
    tbb::task_scheduler_init init(num_threads);
  
    graph g;
    source_node<ImagePtr> image_sourcer(g, [](ImagePtr &image) -> bool {
        image = std::make_shared<Image>(640, 480);
        return true;
    }, false); // TODO several image sources.
    broadcast_node<ImagePtr> image_source_broadcaster(g);
    make_edge(image_sourcer, image_source_broadcaster);
    
    using ElementsFunctionNode = function_node<ImagePtr, ElementsResultType>;
    using ElementsFunctionNodePtr = std::shared_ptr<ElementsFunctionNode>;
    
    auto miner = std::make_shared<ElementsFunctionNode>(g, unlimited, MinimumElements());
    auto maxer = std::make_shared<ElementsFunctionNode>(g, unlimited, MaximumElements());
    ElementsFunctionNodePtr exacter;

    if (brightness_interesting != -1) {
        exacter = std::make_shared<ElementsFunctionNode>(g, unlimited, ExactElements(
                static_cast<Image::value_type>(brightness_interesting)));
    } else {
        exacter = std::make_shared<ElementsFunctionNode>(g, unlimited, NoneElements());
    }
    
    using ImagePositionsJoiner = join_node<ImageHighlighter::input_type>;
    
    ImagePositionsJoiner elements_joiner(g);
    
    make_edge(image_source_broadcaster, input_port<0>(elements_joiner));
    
#define PROCESS_ELEMENT_NODE(node_ptr, port_number)          \
    do {                                                     \
        ElementsFunctionNode &node = *(node_ptr);            \
        make_edge(image_source_broadcaster, node);           \
        make_edge(node, input_port<port_number>(elements_joiner)); \
    } while (0);
    
    PROCESS_ELEMENT_NODE(miner, static_cast<const int>(ImageElementFinderType::MIN));
    PROCESS_ELEMENT_NODE(maxer, static_cast<const int>(ImageElementFinderType::MAX));
    PROCESS_ELEMENT_NODE(exacter, static_cast<const int>(ImageElementFinderType::EXACT));
    
    function_node<ImageHighlighter::input_type, ImagePtr> highlighter(g, unlimited, ImageHighlighter());
    make_edge(elements_joiner, highlighter);
    
    broadcast_node<ImagePtr> highlighter_broadcast(g);
    make_edge(highlighter, highlighter_broadcast);

    function_node<ImagePtr, MeanBrightnessCalculator::result_type> meaner(g, unlimited, MeanBrightnessCalculator(journal_filename));
    function_node<ImagePtr, ImagePtr> inverser(g, unlimited, ImageInverser());
    
    make_edge(highlighter_broadcast, meaner);
    make_edge(highlighter_broadcast, inverser);

    using result_type = MeanBrightnessCalculator::result_type;
    result_type result;
    function_node<result_type> mean_result_printer(g, serial, [&](const result_type &input) {
        std::cout << "Mean result: " << input << std::endl;
    });
    make_edge(meaner, mean_result_printer);

    image_sourcer.activate();
    g.wait_for_all();
    return 0;
}
