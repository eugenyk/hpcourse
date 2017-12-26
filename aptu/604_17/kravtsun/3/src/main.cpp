#include <bits/stdc++.h>

#include <tbb/tbb.h>

#include "elements.h"
#include "process.h"

static std::string journal_filename = "";
static int brightness_interesting = -1;
static int images_limit = 0;
static int num_threads = 3;

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

#if DEBUG_COUNT
static int image_count = 0;
static std::mutex image_count_mutex;
int increment_image_count() {
    std::unique_lock<std::mutex> image_count_lock(image_count_mutex);
    image_count++;
    std::cout << "image_count = " << image_count << std::endl;
}

int decrement_image_count() {
    std::unique_lock<std::mutex> image_count_lock(image_count_mutex);
    image_count--;
    std::cout << "image_count = " << image_count << std::endl;
}
#endif

int main(int argc, char **argv) {
    parse_args(argc, argv);
    
    using namespace tbb::flow;
    tbb::task_scheduler_init init(num_threads);
  
    graph g;
    source_node<ImagePtr> image_sourcer(g, [](ImagePtr &image) -> bool {
        image = std::make_shared<Image>(640, 480);
#if DEBUG_COUNT
        increment_image_count();
#endif
        return true;
    }, false); // TODO several image sources.
    broadcast_node<ImagePtr> image_source_broadcaster(g);
    
    limiter_node<ImagePtr> limiter(g, images_limit);
    
    if (images_limit > 0) {
        make_edge(image_sourcer, limiter);
        make_edge(limiter, image_source_broadcaster);
    } else {
        make_edge(image_sourcer, image_source_broadcaster);
    }
    
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
    
    function_node< double, continue_msg > meaner_pit( g, unlimited,
                                                   []( double v ) -> continue_msg {
#if DEBUG_COUNT
                                                       decrement_image_count();
#endif
                                                       return continue_msg();
                                                   } );
    
    make_edge(meaner, meaner_pit);
    
    if (images_limit > 0) {
        make_edge(meaner_pit, limiter.decrement);
    }

    image_sourcer.activate();
    g.wait_for_all();
    return 0;
}
