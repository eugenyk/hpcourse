#include <tbb/flow_graph.h>
#include <tbb/task_group.h>
#include <iostream>
#include <fstream>
#include <utility>
#include <mutex>
#include "program_arguments.hpp"
#include "pixmap.hpp"
#include "pixel.hpp"

using Result = std::pair<lex::Pixmap*, std::vector<hpcourse::Pixel>>;

static Result step2_find_min_brightness(lex::Pixmap* pixmap) {
    std::vector<hpcourse::Pixel> pixels;
    auto min_value = pixmap->at(0, 0);
    for(::uint32_t row = 0; row < pixmap->height(); row++) {
        for(::uint32_t column = 0; column < pixmap->width(); column++) {
            const auto value = pixmap->at(row, column);
            if (value < min_value) {
                min_value = value;
                pixels.clear();
                pixels.push_back(hpcourse::Pixel {row, column, value});
            }
            else if(value == min_value) {
                pixels.push_back(hpcourse::Pixel {row, column, value});
            }
        }
    }
//    pixels.shrink_to_fit();
    return std::pair<lex::Pixmap*, std::vector<hpcourse::Pixel>>(pixmap, pixels);
}

static Result step2_find_max_brightness(lex::Pixmap* pixmap) {
    std::vector<hpcourse::Pixel> pixels;
    auto max_value = pixmap->at(0, 0);
    for(::uint32_t row = 0; row < pixmap->height(); row++) {
        for(::uint32_t column = 0; column < pixmap->width(); column++) {
            const auto value = pixmap->at(row, column);
            if (value > max_value) {
                max_value = value;
                pixels.clear();
                pixels.push_back(hpcourse::Pixel {row, column, value});
            }
            else if(value == max_value) {
                pixels.push_back(hpcourse::Pixel {row, column, value});
            }
        }
    }
//    pixels.shrink_to_fit();
    return std::pair<lex::Pixmap*, std::vector<hpcourse::Pixel>>(pixmap, pixels);
}

static Result step2_find_user_brightness(hpcourse::ProgramArguments& program_arguments, lex::Pixmap* pixmap) {
    std::vector<hpcourse::Pixel> pixels;
    for(::uint32_t row = 0; row < pixmap->height(); row++) {
        for(::uint32_t column = 0; column < pixmap->width(); column++) {
            const auto value = pixmap->at(row, column);
            if(value == program_arguments.brightness()) {
                pixels.push_back(hpcourse::Pixel {row, column, value});
            }
        }
    }
//    pixels.shrink_to_fit();
    return std::pair<lex::Pixmap*, std::vector<hpcourse::Pixel>>(pixmap, pixels);
}

static tbb::flow::tuple<Result, Result, Result> step3(std::recursive_mutex& mutex, tbb::flow::tuple<Result, Result, Result> tuple) {
    std::lock_guard<std::recursive_mutex> lock {mutex};

    auto& min_brightness = std::get<0>(tuple).second;
    auto& max_brightness = std::get<1>(tuple).second;
    auto& user_brightness = std::get<2>(tuple).second;

    std::cout << "===== Step #3 =====\n\n";

    std::cout << "Pixmap #0: " << std::get<0>(tuple).first << "\n";
    std::cout << "Pixmap #1: " << std::get<1>(tuple).first << "\n";
    std::cout << "Pixmap #2: " << std::get<2>(tuple).first << "\n\n";

    std::cout << "Min brightness: " << "\n";
    for(auto& pixel : min_brightness) {
        std::cout << "(" << pixel.row << ", " << pixel.column << ") ";
    }
    std::cout << "\n\n";

    std::cout << "Max brightness: " << "\n";
    for(auto& pixel : max_brightness) {
        std::cout << "(" << pixel.row << ", " << pixel.column << ") ";
    }
    std::cout << "\n\n";

    std::cout << "User brightness: " << "\n";
    for(auto& pixel : user_brightness) {
        std::cout << "(" << pixel.row << ", " << pixel.column << ") ";
    }

    std::cout << "\n\n";

//    std::cout << "===================\n\n";

    return tuple;
}

static void step4_inversion_brightness(tbb::flow::tuple<Result, Result, Result> tuple) {
    auto& pixmap = std::get<0>(tuple).first;
    for(::uint32_t row = 0; row < pixmap->height(); row++) {
        for(::uint32_t column = 0; column < pixmap->width(); column++) {
            const auto value = pixmap->at(row, column);
            pixmap->at(row, column) = (255 - value);
        }
    }
}

static void step4_average_brightness(hpcourse::ProgramArguments& program_arguments, std::recursive_mutex& mutex, tbb::flow::tuple<Result, Result, Result> tuple) {
    std::lock_guard<std::recursive_mutex> lock {mutex};

    auto& pixmap = std::get<0>(tuple).first;
    ::uint32_t brightness = 0;
    for(::uint32_t row = 0; row < pixmap->height(); row++) {
        for(::uint32_t column = 0; column < pixmap->width(); column++) {
            brightness += pixmap->at(row, column);
        }
    }
    const auto average_brightness = brightness / (pixmap->width() * pixmap->height());

    std::cout << "===== Step #4 =====\n\n";

    std::cout << "Pixmap #0: " << std::get<0>(tuple).first << "\n";
    std::cout << "Pixmap #1: " << std::get<1>(tuple).first << "\n";
    std::cout << "Pixmap #2: " << std::get<2>(tuple).first << "\n\n";

    std::cout << "Average brightness: " << average_brightness;

    std::cout << "\n\n";

//    std::cout << "===================\n\n";

    std::ofstream file {program_arguments.file(), std::ios_base::app};
    file << average_brightness << "\n";
    file.close();
}

int main(int argc, const char** argv) {
    hpcourse::ProgramArguments program_arguments {argc, argv};
    std::recursive_mutex mutex;

    tbb::flow::graph graph;

    tbb::flow::broadcast_node<lex::Pixmap*> node_step1_broadcast {
        graph
    } ;

    tbb::flow::function_node<lex::Pixmap*, Result> node_step2_find_min_brightness {
        graph,
        tbb::flow::serial,
        step2_find_min_brightness
    };

    tbb::flow::function_node<lex::Pixmap*, Result> node_step2_find_max_brightness {
        graph,
        tbb::flow::serial,
        step2_find_max_brightness
    };

    tbb::flow::function_node<lex::Pixmap*, Result> node_step2_find_user_brightness {
        graph,
        tbb::flow::serial,
        [&program_arguments](lex::Pixmap* pixmap) -> Result {
            return step2_find_user_brightness(program_arguments, pixmap);
        }
    };

    tbb::flow::join_node<tbb::flow::tuple<Result, Result, Result>, tbb::flow::queueing> node_step2_join {
        graph
    };

    tbb::flow::function_node<tbb::flow::tuple<Result, Result, Result>, tbb::flow::tuple<Result, Result, Result>> node_step3 {
        graph,
        tbb::flow::serial,
        [&mutex](tbb::flow::tuple<Result, Result, Result> tuple) {
            return step3(mutex, tuple);
        }
    };

    tbb::flow::broadcast_node<tbb::flow::tuple<Result, Result, Result>> node_step4_broadcast {
        graph
    };

    tbb::flow::function_node<tbb::flow::tuple<Result, Result, Result>> node_step4_inversion_brightness {
        graph,
        tbb::flow::unlimited,
        step4_inversion_brightness
    };

    tbb::flow::function_node<tbb::flow::tuple<Result, Result, Result>> node_step4_average_brightness {
        graph,
        tbb::flow::serial,
        [&program_arguments, &mutex](tbb::flow::tuple<Result, Result, Result> tuple) {
            step4_average_brightness(program_arguments, mutex, tuple);
        }
    };

    tbb::flow::make_edge(node_step1_broadcast, node_step2_find_min_brightness);
    tbb::flow::make_edge(node_step1_broadcast, node_step2_find_max_brightness);
    tbb::flow::make_edge(node_step1_broadcast, node_step2_find_user_brightness);
    tbb::flow::make_edge(node_step2_find_min_brightness, tbb::flow::input_port<0>(node_step2_join));
    tbb::flow::make_edge(node_step2_find_max_brightness, tbb::flow::input_port<1>(node_step2_join));
    tbb::flow::make_edge(node_step2_find_user_brightness, tbb::flow::input_port<2>(node_step2_join));
    tbb::flow::make_edge(node_step2_join, node_step3);
    tbb::flow::make_edge(node_step3, node_step4_broadcast);
    tbb::flow::make_edge(node_step4_broadcast, node_step4_inversion_brightness);
    tbb::flow::make_edge(node_step4_broadcast, node_step4_average_brightness);

    std::remove(program_arguments.file().c_str());

    ::uint32_t threads = program_arguments.threads();
    std::vector<lex::Pixmap*> pixmaps;
    while(threads > 0) {
        for(::uint32_t i = 0; i < threads; i++) {
            auto pixmap = new(std::nothrow) lex::Pixmap;
            if(pixmap != nullptr) {
                pixmap->create(30, 30);
                pixmaps.push_back(pixmap);
                node_step1_broadcast.try_put(pixmap);
            }
        }
        graph.wait_for_all();
        for(auto pixmap : pixmaps) {
            delete pixmap;
        }
        pixmaps.clear();
        threads -= program_arguments.threads();
    }

    std::cout << "\n";
    return 0;
}
