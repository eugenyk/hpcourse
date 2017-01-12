#include <iostream>
#include <random>
#include <boost/program_options.hpp>
#include <tbb/flow_graph.h>
#include <fstream>
#include <iomanip>
#include <atomic>

using namespace std;
namespace po = boost::program_options;
namespace tf = tbb::flow;

// these are debug guys, to figure out how many times nodes were called
struct Image;

namespace debug
{
bool with_debug_prints;
vector<shared_ptr<const Image>> source_images;
std::atomic_int generate_called(0);
std::atomic_int border_called(0);
std::atomic_int intensity_max_called(0);
std::atomic_int intensity_min_called(0);
std::atomic_int intensity_exact_called(0);
std::atomic_int inverse_called(0);
std::atomic_int average_called(0);
std::atomic_int writer_called(0);

void print_stats() {
    cout << "Call statistics: \n";
    cout << "generated " << debug::generate_called << " images" << endl;
    cout << "border_called " << debug::border_called << endl;
    cout << "intensity_max_called " << debug::intensity_max_called << endl;
    cout << "intensity_min_called " << debug::intensity_min_called << endl;
    cout << "intensity_exact_called " << debug::intensity_exact_called << endl;
    cout << "inverse_called " << debug::inverse_called << endl;
    cout << "average_called " << debug::average_called << endl;
    cout << "writer_called " << debug::writer_called << endl;
}
}

struct Image {
    Image(size_t width, size_t height, size_t id) : _width(width), _height(height),
                                         _data(width * height, 0), _id(id) {
        if (width == 0 || height == 0) {
            throw logic_error("Image can't have zero size");
        }
    }

    uint8_t& operator() (size_t i, size_t j) {
        if (i >= _height || j >= _width) throw out_of_range("Subscript is out of range");
        size_t index = i * _width + j;
        return _data[index];
    }

    uint8_t operator() (size_t i, size_t j) const {
        if (i >= _height || j >= _width) throw out_of_range("Subscript is out of range");
        size_t index = i * _width + j;
        return _data[index];
    }

    vector<uint8_t> &data() {
        return _data;
    }

    const vector<uint8_t> &data() const {
        return _data;
    }

    size_t width() const {
        return _width;
    }

    size_t height() const {
        return _height;
    }

    size_t size() const {
        return _width * _height;
    }

    size_t id() const {
        return _id;
    }
private:
    const size_t _width;
    const size_t _height;
    const size_t _id;  // well, tagged messages could also be used; screw it
    vector<uint8_t> _data;
};

std::ostream& operator <<(std::ostream& out, const Image &img) {
    for (size_t i = 0; i < img.height(); i++) {
        for (size_t j = 0; j < img.width(); j++) {
            out << int(img(i, j)) << " ";
        }
        out << "\n";
    }

    return out;
}

struct ImageGenerator {
    ImageGenerator(size_t need_to_generate, size_t width, size_t height) :
            need_to_generate(need_to_generate), width(width), height(height) {}

    bool operator()(shared_ptr<const Image> &result) {
        if (generated_number == need_to_generate) {
            return false;
        }

        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<uint8_t> distribution(0, 255);

        auto dice = [&mt, &distribution]() {
            return distribution(mt);
        };


        shared_ptr<Image> temp(new Image(width, height, generated_number));
        auto& img_data = temp->data();
        for (auto it = img_data.begin(); it != img_data.end(); ++it) {
            *it = dice();
        }

        result = temp;
        generated_number++;
        debug::generate_called++;

        if (debug::with_debug_prints) {
            debug::source_images[result->id()] = result;
        }
        return true;
    }
private:
    size_t generated_number = 0;
    const size_t need_to_generate;
    const size_t width;
    const size_t height;
};

struct IntensityFinder {
    enum class PerformedTransform {
        MAX_INTENSITY_SEARCH,
        MIN_INTENSITY_SEARCH,
        EXACT_INTENSITY_SEARCH
    };
    struct result_t {
        result_t(shared_ptr<const Image> src, PerformedTransform tr)
            : source_img(src), transform_type(tr) {}
        // This constructor is used by flow; It's safe since after creation the result is
        // copied to an object cinstructed in such a way
        result_t() { }

        vector<pair<int, int>> pixels;
        shared_ptr<const Image> source_img;
        PerformedTransform transform_type;
    };
};

struct MatchingIntensityFinder : IntensityFinder {
    MatchingIntensityFinder(uint8_t intensity, PerformedTransform tr =
        PerformedTransform::EXACT_INTENSITY_SEARCH) : intensity(intensity), tr(tr) { }

    result_t operator() (shared_ptr<const Image> img) const {
        size_t rows_number = img->height();
        size_t cols_number = img->width();

        switch (tr) {
            case PerformedTransform::MAX_INTENSITY_SEARCH:
                debug::intensity_max_called++;
                break;
            case PerformedTransform::MIN_INTENSITY_SEARCH:
                debug::intensity_min_called++;
                break;
            case PerformedTransform::EXACT_INTENSITY_SEARCH:
                debug::intensity_exact_called++;
                break;
        }

        result_t result(img, tr);
        for (size_t i = 0; i < rows_number; i++) {
            for (size_t j = 0; j < cols_number; j++) {
                if ((*img)(i, j) == intensity) {
                    result.pixels.push_back({i, j});
                }
            }
        }

        return result;
    }

private:
    const uint8_t intensity;
    const PerformedTransform tr;
};

namespace {
bool is_less(...) { return false; }

template <class T>
bool is_less(std::less<T>) { return true; }
}

template<class Compare>
struct MinMaxIntensityFinder : IntensityFinder {
    result_t operator() (shared_ptr<const Image> img) const {
        auto max_elem = max_element(img->data().begin(), img->data().end(), Compare());

        PerformedTransform tr = is_less(Compare()) ? PerformedTransform::MIN_INTENSITY_SEARCH :
            PerformedTransform::MAX_INTENSITY_SEARCH;

        MatchingIntensityFinder finder(*max_elem, tr);
        return finder(img);
    }
};


struct BorderMaker {
    typedef shared_ptr<const Image> result_t;
    typedef tf::tuple<IntensityFinder::result_t, IntensityFinder::result_t, IntensityFinder::result_t> input_t;

    result_t operator() (const input_t &source) const {
        const uint8_t border_intensity = 0;
        shared_ptr<Image> result(new Image(*get<0>(source).source_img));

        assert(get<0>(source).source_img->id() == get<2>(source).source_img->id());
        draw_borders(get<0>(source), result);
        draw_borders(get<1>(source), result);
        draw_borders(get<2>(source), result);

        debug::border_called++;
        return result;
    }

private:
    void draw_borders(const IntensityFinder::result_t &source,
                      shared_ptr <Image> &res) const {
        const uint8_t border_intensity = 0;

        for (auto &pixel: source.pixels) {
            int max_i = res->height() > pixel.first + 1 ? 2 : 1;
            int max_j = res->width() > pixel.second + 1 ? 2 : 1;
            int min_i = pixel.first == 0 ? 0 : -1;
            int min_j = pixel.second == 0 ? 0 : -1;

            for (int i = min_i; i < max_i; i++) {
                for (int j = min_j; j < max_j; j++) {
                    if (!i && !j) continue;
                    (*res)(pixel.first + i, pixel.second + j) = border_intensity;
                }
            }
        }
    }

};

int main(int argc, char** argv) {
    const size_t DEFAULT_WIDTH = 50;
    const size_t DEFAULT_HEIGHT = 50;
    const size_t MAX_IMAGE_DIMENSION_SIZE = 2000;
    const size_t SHOW_DEBUG_EVERY = 10;

    uint8_t intensity;
    size_t parallel_images_limit;
    string log_file;
    size_t total_images_number;
    size_t width, height;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "print this help")
            (",b", po::value(&intensity)->default_value(127), "intensity which is searched")
            (",l", po::value(&parallel_images_limit)->default_value(4), "max amount of images processed in parallel")
            (",f", po::value(&log_file)->default_value(""), "file name to store log")
            (",d", po::bool_switch(&debug::with_debug_prints)->default_value(false), "show debug prints once in a while")
            ("total-images,i", po::value(&total_images_number)->default_value(100), "total amount of images to process")
            ("width", po::value(&width)->default_value(DEFAULT_WIDTH), "image width")
            ("height", po::value(&height)->default_value(DEFAULT_HEIGHT), "image height")
            ;

    po::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    vm.notify();
    debug::source_images.resize(total_images_number);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 0;
    }

    if (width == 0 || height == 0 || width > MAX_IMAGE_DIMENSION_SIZE || height > MAX_IMAGE_DIMENSION_SIZE) {
        cerr << "Image sizes should contain exactly two positive integer values less than " <<
                MAX_IMAGE_DIMENSION_SIZE << endl;
        return 1;
    }

    /**
     * Graph model:
     *                Source                        (Input: None, Output: Image)
     *                   ↓
     *                Limiter                       (so that no more than limit + 1 images can exist [if only not DEBUG_PRINTS defined])
     *            ↙      ↓       ↘
     * Search Exact  Search Min   Search Max        (Input: Image, Output: IntensityFinder::result_t)
     *            ↘      ↓       ↙
     *            Joining by Image ID
     *                   ↓
     *              Draw Borders                    (Input: IntensityFinder::result_t, Output: Image with borders)
     *            ↙              ↘
     * Average Intensity     Invert Image           (Input: Image, Output: pair<average_intensity, id> for Average Intensity node and
     *            ↘              ↙                      Image (inverted) for the Invert Image node
     *           Joining by Image ID                (joining result is pair<pair<average_intensity, id>, Image>
     *                   ↓
     *                 Buffer             Needed since writing to file is serial; Input = Output = pair<pair<average_intensity, id>, Image>
     *                   ↓
     *      Write Average Intensity to File    Input: pair<pair<average_intensity, id>, Image>, Output: continue_msg
     *
     */

    tf::graph g;

    tf::source_node<shared_ptr<const Image>> image_provider(g, ImageGenerator(total_images_number, width, height), false);
    tf::limiter_node<shared_ptr<const Image>> limiter(g, parallel_images_limit);
    tf::make_edge(image_provider, limiter);

    // step 2: 3 different intensity search algorithms
    tf::function_node<shared_ptr<const Image>, IntensityFinder::result_t> search_exact(
            g, tf::unlimited, MatchingIntensityFinder(intensity));
    tf::function_node<shared_ptr<const Image>, IntensityFinder::result_t> search_min_intensity(
            g, tf::unlimited, MinMaxIntensityFinder<std::less<uint8_t>>{});
    tf::function_node<shared_ptr<const Image>, IntensityFinder::result_t> search_max_intensity(
            g, tf::unlimited, MinMaxIntensityFinder<std::greater<uint8_t>>{});


    tf::make_edge(limiter, search_exact);
    tf::make_edge(limiter, search_min_intensity);
    tf::make_edge(limiter, search_max_intensity);

    // step 3: draw borders around found pixels
    auto id_by_intensity_result = [](const IntensityFinder::result_t &res) { return res.source_img->id(); };

    typedef tf::tuple<IntensityFinder::result_t, IntensityFinder::result_t, IntensityFinder::result_t> border_input_t;
    tf::join_node<border_input_t, tf::key_matching<size_t>> pre_border_joiner(g, id_by_intensity_result,
                                                                              id_by_intensity_result,
                                                                              id_by_intensity_result);

    tf::make_edge(search_exact, tf::input_port<0>(pre_border_joiner));
    tf::make_edge(search_min_intensity, tf::input_port<1>(pre_border_joiner));
    tf::make_edge(search_max_intensity, tf::input_port<2>(pre_border_joiner));

    tf::function_node<border_input_t, shared_ptr<const Image>> draw_borders(
            g, tf::unlimited, BorderMaker{});



    tf::make_edge(pre_border_joiner, draw_borders);
    // step 4: average intensity and inverse image evaluation

    tf::function_node<BorderMaker::result_t, shared_ptr<const Image>> inverse_image(
            g, tf::unlimited, [](BorderMaker::result_t img) {
                shared_ptr<Image> result(new Image(*img));
                debug::inverse_called++;
                for (auto it = result->data().begin(); it != result->data().end(); ++it) {
                    *it = 255 - *it;
                }

                return result;
            });

    // this node returns pair, because we want to know not only the intensity (second element of pair),
    // but also id of the image to which it corresponds
    tf::function_node<BorderMaker::result_t, pair<double, size_t>> find_average_intensity(
            g, tf::unlimited, [](BorderMaker::result_t img) {
                debug::average_called++;
                uint64_t sum = 0;
                for_each (img->data().begin(), img->data().end(), [&sum](uint8_t i) {
                    sum += i;
                });
                return pair<double, size_t>{double(sum) / img->size(), img->id()};
            });

    tf::make_edge(draw_borders, find_average_intensity);
    tf::make_edge(draw_borders, inverse_image);


    typedef tf::tuple<pair<double, size_t>, shared_ptr<const Image>> sink_input_t;

    // we want to pair results for the same image! So, we provide
    // key matching which uses image id as key
    tf::join_node<sink_input_t, tf::key_matching<size_t>> sink(
            g, [](const pair<double, size_t>&p) {return p.second; },
                [](const shared_ptr<const Image> &img) {
                    return img->id();
                });
    tf::make_edge(find_average_intensity, tf::input_port<0>(sink));
    tf::make_edge(inverse_image, tf::input_port<1>(sink));

    // print average intensity
    ofstream out(log_file);
    if (!out.bad()) {
        out << "Image id\tAverage Intensity\n";
        out << setprecision(4) << fixed;
    }


    // since we have serial output and async processing and we don't want to think a lot,
    // we just add this buffer so that it stores the results that can't be processed
    // by output node right now
    tf::buffer_node<sink_input_t> output_buffer(g);
    tf::make_edge(sink, output_buffer);


    int processed_images = 0;
    tf::function_node<sink_input_t, tf::continue_msg> on_something_processed(g, tf::serial,
        [&](const sink_input_t& results) {
            debug::writer_called++;
            limiter.decrement.try_put({});
            processed_images++;
            assert(get<0>(results).second == get<1>(results)->id());

            if (!out.bad()) {
                out << get<0>(results).second << "\t\t" << get<0>(results).first << "\n";
            }

            if (!debug::with_debug_prints) {
                return tf::continue_msg{};
            }

            if (processed_images % SHOW_DEBUG_EVERY == 0) {
                cout << "processed " << processed_images << " images" <<
                endl;
                cout << "Source image #" << processed_images << ":\n";
                cout << *debug::source_images[get<1>(results)->id()];
                cout << "-----" << "\n";

                auto result_img = get<1>(results);
                cout << "result image #" << processed_images << ":\n";

                cout << *result_img;
                cout << "=====" << "\n";
            }
            assert(get<0>(results).second ==
                   debug::source_images[get<1>(results)->id()]->id());
            debug::source_images[get<1>(results)->id()].reset();
            return tf::continue_msg{};
        });
    tf::make_edge(output_buffer, on_something_processed);

    image_provider.activate();
    g.wait_for_all();

    debug::print_stats();

    return 0;
}