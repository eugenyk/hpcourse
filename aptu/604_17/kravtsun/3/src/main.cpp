#include <bits/stdc++.h>

#include <tbb/tbb.h>

#include "elements.h"
#include "process.h"

using namespace tbb::flow;

std::string journal_filename;
int brightness_interesting = -1;
int images_limit = 0;
int num_threads = 3;

#define STR(x) #x
#define XSTR(x) STR(x)

void parse_args(int argc, char **argv) {
//    “-b 123”: интересующее значение яркости, для шага № 2
//    “-l 5”: предел одновременно обрабатываемых приложением изображений
//    “-f log.txt”: имя файла журнала яркостей
    
    const std::vector<std::string> arg_names = {"-b", "-l", "-f"};
    
    for (int i = 0; i < argc; ++argv) {
        const std::string args = argv[i];
        if (std::count(arg_names.cbegin(), arg_names.cend(), args) && i + 1 == argc) {
            throw std::logic_error("Need to specify value for argument: " + args);
        }
        if (args == "-b") {
            brightness_interesting = atoi(argv[i + 1]);
        } else if (args == "-l") {
            images_limit = atoi(argv[i + 1]);
        } else if (args == "-f") {
            journal_filename = argv[i + 1];
        } else if (!args.empty() && args[0] == '-') {
            throw std::logic_error("Wrong argument: " + args);
        }
        ++i; // as arguments above require value.
    }
}



int main(int argc, char **argv) {
    parse_args(argc, argv);
    tbb::task_scheduler_init init(num_threads);
  
    graph g;
    broadcast_node<ImageConstPtr> image_source(g); // TODO several image sources.
    
    using ElementsFunctionNode = function_node<ImageConstPtr, ElementsResultType>;
    using ElementsFunctionNodePtr = std::shared_ptr<ElementsFunctionNode>;
    std::vector<ElementsFunctionNodePtr> elements_processors;
    
    elements_processors.push_back(std::make_shared<ElementsFunctionNode>(g, unlimited, MinimumElements()));
    elements_processors.push_back(std::make_shared<ElementsFunctionNode>(g, unlimited, MaximumElements()));
    
    if (brightness_interesting != -1) {
        elements_processors.push_back(std::make_shared<ElementsFunctionNode>(g, unlimited, ExactElements(
                static_cast<Image::value_type>(brightness_interesting))));
    }
    
    using ImagePositionsJoiner = join_node<ImageHighlighter::input_type, key_matching<size_t>>;
    
    function_node<ImageHighlighter::input_type, ImageConstPtr> highlighter(g, unlimited, ImageHighlighter());
    
    function_node<ImageConstPtr, MeanBrightnessCalculator::result_type> meaner(g, unlimited, MeanBrightnessCalculator("output.txt"));
    function_node<ImageConstPtr, ImagePtr> inverser(g, unlimited, ImageInverser());
    
    std::vector<std::shared_ptr<ImagePositionsJoiner>> joiners;
    
    auto process_element_function = [&](const ElementsFunctionNodePtr &node_ptr) {
        ElementsFunctionNode &node = *node_ptr;
        make_edge(image_source, node);
        auto image_positions_joiner = std::make_shared<ImagePositionsJoiner>(g, ImageHash(), PositionsListHash());
        auto &joiner_node = *image_positions_joiner.get();
    
        make_edge(image_source, input_port<0>(joiner_node));
        make_edge(node, input_port<1>(joiner_node));
    
        make_edge(joiner_node, highlighter);
        joiners.push_back(image_positions_joiner);
    };

    std::for_each(elements_processors.begin(), elements_processors.end(), process_element_function);
    
    make_edge(highlighter, meaner);
    
    using result_type = MeanBrightnessCalculator::result_type;
    result_type result;
    function_node<result_type> mean_result_printer(g, serial, [&](const result_type &input) {
        std::cout << "Result: " << input << std::endl;
    });
    make_edge(meaner, mean_result_printer);
    
    auto image = std::make_shared<Image>(640, 480);
    bool is_image_put = image_source.try_put(image);
    assert(is_image_put);
    g.wait_for_all();
    return 0;
}
