#include "ImageGenerator.h"
#include <iostream>
#include "ThreadsInfo.h"
#include "ImageTransformer.h"


/*typedef tuple<unsigned char**, unsigned char**,
    unsigned char**, unsigned char**,
    unsigned char**, unsigned char**,
    unsigned char**, unsigned char**,
    unsigned char**, unsigned char**> image_tuple;

struct fill {
    unsigned char** operator()(unsigned char** v) {
        return NULL;
    }
};


template<std::size_t I = 0>
inline typename std::enable_if<I == JOIN_MAX, void>::type
connect(split_node<image_tuple>& t1, join_node<image_tuple>& t2)
{}

template<std::size_t I = 0>
inline typename std::enable_if<I < JOIN_MAX, void>::type
    connect(split_node<image_tuple>& t1, join_node<image_tuple>& t2)
{
    function_node<unsigned char**, unsigned char**> filler(imageGeneratorGraph, unlimited, fill());
    make_edge(output_port<I>(t1), filler);
    make_edge(filler, input_port<I>(t2));
    connect<I + 1>(t1, t2);
}

struct split {
    image_tuple operator()(Image v) {
        return image_tuple(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }
};

struct collect {
    Image operator()(image_tuple v) {
        return Image(1, 1);
    }
};*/
/*struct fillImageLine {
    unsigned char* operator() (unsigned char* imageLine) {
    }
};

class addToImage {
    Image &my_sum;
public:
    addToImage(Image &s) : my_sum(s) {}
    int operator()(std::tuple<int, int> v) {
        printf("adding %d and %d to %d\n", std::get<0>(v), std::get<1>(v), my_sum);
        my_sum += std::get<0>(v) + std::get<1>(v);
        return my_sum;
    }
};
struct fillImage {
    Image& operator() (Image& image) {
        graph lineImageGraph;
        unsigned int theradsNum = tbb::task_scheduler_init::automatic;
        broadcast_node<unsigned char*> input(lineImageGraph);
        limiter_node<unsigned char*> limiter(lineImageGraph, THREADS_INC * theradsNum);
        function_node<unsigned char*, unsigned char*> lineFiller(lineImageGraph, unlimited, fillImageLine());
        sequencer_node<unsigned char*> buffer(lineImageGraph, Sequencer());
        function_node<unsigned char*, continue_msg> output(lineImageGraph, serial, addToImage(filledImage));

        make_edge(input, limiter);
        make_edge(limiter, lineFiller);
        make_edge(lineFiller, buffer);
        make_edge(buffer, output);
        make_edge(output, limiter.decrement);
    }
};

void generateRandomImages(unsigned int imagesCount) {
    graph imageGeneratorGraph;
    broadcast_node<Image> input(imageGeneratorGraph);
    function_node<Image, image_tuple> splitter(imageGeneratorGraph, unlimited, split());
    split_node<image_tuple> splitNode(imageGeneratorGraph);
    join_node<image_tuple, queueing> join(imageGeneratorGraph);
    function_node<image_tuple, Image> output(imageGeneratorGraph, serial, collect());
    make_edge(input, splitter);
    make_edge(splitter, splitNode);
    connect(splitNode, join);
    make_edge(join, output);
    unsigned int theradsNum = tbb::task_scheduler_init::automatic;
    tbb::task_scheduler_init init(theradsNum);
    broadcast_node<Image&> input(imageGeneratorGraph);
    limiter_node<Image&> limiter(imageGeneratorGraph, THREADS_INC * theradsNum);
    function_node<Image&, Image&> imageFiller(imageGeneratorGraph, unlimited, fillImage());
}*/

unsigned char ThreadsInfo::currentBrightness = 0;
int main(int argc, char **argv)
{
    unsigned char brightnessValue = 0;
    unsigned int imagesLimit = ThreadsInfo::THREADS_INC_COEF * ThreadsInfo::THREADS_NUM;
    char *brightnessFile = nullptr;

    for (int i = 1; i < argc; i+=2) {
        if (i == argc - 1) {
            if (strcmp(argv[i], "-l") != 0 && strcmp(argv[i], "-b") != 0 &&
                strcmp(argv[i], "-f") != 0)
                std::cout << "Uknown option " << argv[i];
            else
                std::cout << "Option " << argv[i]  << " should have value.";
            return 1;
        }
        if (strcmp(argv[i], "-l") == 0) {
            imagesLimit = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-b") == 0) {
            brightnessValue = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-f") == 0) {
            brightnessFile = argv[i + 1];
        }
    }
    ThreadsInfo::currentBrightness = brightnessValue;
    /*while ((flag = getopt(argc, argv, "blf:")) != -1) {
        switch (flag)
        {
        case 'b':
            if (optarg != NULL)
            brightnessValue = atoi(optarg);
            break;
        case 'l':
            if (optarg != NULL)
            imagesLimit = atoi(optarg);
            break;
        case 'f':
            brightnessFile = optarg;
            break;
        case '?':
            break;
        default:
            break;
        }
    }*/
    // Constants for size of generated images.
    const unsigned int minWidth = 50;
    const unsigned int minHeight = 50;
    const unsigned int maxWidth = 256;
    const unsigned int maxHeight = 256;

    // Generate images.
    ImageGenerator generator(minWidth, minHeight, maxWidth, maxHeight);
    std::vector<Image*> generatedImages = generator.generate(100);
    ImageTransformer transformer;
    transformer.transform(generatedImages, imagesLimit, brightnessFile);
    
    return 0;
}