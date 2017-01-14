#include <tbb/flow_graph.h>
#include <tbb/task_group.h>
#include <iostream>
#include <fstream>
#include <utility>
#include "program_arguments.hpp"
#include "pixmap.hpp"
#include "pixel.hpp"

constexpr const char* const filepath_min_brigthness = "hpcourse_lab1_step2_min_brigthness.txt";
constexpr const char* const filepath_max_brigthness = "hpcourse_lab1_step2_max_brigthness.txt";
constexpr const char* const filepath_user_brigthness = "hpcourse_lab1_step2_user_brigthness.txt";
constexpr const char* const filepath_inversion_brigthness = "hpcourse_lab1_step4_inversion_brigthness.txt";
constexpr const char* const filepath_average_brigthness = "hpcourse_lab1_step4_average_brigthness.txt";

void print(std::ofstream& out, lex::Pixmap* pixmap) {
    out << "Image:\n";
    for(::uint32_t row = 0; row < pixmap->height(); row++) {
        for(::uint32_t column = 0; column < pixmap->width(); column++) {
            out << pixmap->at(row, column) << " ";
        }
        out << "\n";
    }
}

int main(int argc, const char** argv) {
    hpcourse::ProgramArguments program_arguments {argc, argv};

    tbb::flow::graph graph;

    tbb::flow::broadcast_node<lex::Pixmap*> node_step1_broadcast { graph } ;

    using Result = std::pair<lex::Pixmap*, std::vector<hpcourse::Pixel>>;

    tbb::flow::function_node<lex::Pixmap*, Result> node_step2_find_min_brightness {
        graph,
        tbb::flow::unlimited,
        [](lex::Pixmap* pixmap) {
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
            std::ofstream file {filepath_min_brigthness, std::ios_base::app};
            print(file, pixmap);
            file.close();
            return std::pair<lex::Pixmap*, std::vector<hpcourse::Pixel>>(pixmap, pixels);
        }
    };

    tbb::flow::function_node<lex::Pixmap*, Result> node_step2_find_max_brightness {
        graph,
        tbb::flow::unlimited,
        [](lex::Pixmap* pixmap) {
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
            std::ofstream file {filepath_max_brigthness, std::ios_base::app};
            print(file, pixmap);
            file.close();
            return std::pair<lex::Pixmap*, std::vector<hpcourse::Pixel>>(pixmap, pixels);
        }
    };

    tbb::flow::function_node<lex::Pixmap*, Result> node_step2_find_user_value {
        graph,
        tbb::flow::unlimited,
            [&program_arguments](lex::Pixmap* pixmap) {
            std::vector<hpcourse::Pixel> pixels;
            for(::uint32_t row = 0; row < pixmap->height(); row++) {
                for(::uint32_t column = 0; column < pixmap->width(); column++) {
                    const auto value = pixmap->at(row, column);
                    if(value == program_arguments.brightness()) {
                        pixels.push_back(hpcourse::Pixel {row, column, value});
                    }
                }
            }
            std::ofstream file {filepath_user_brigthness, std::ios_base::app};
            print(file, pixmap);
            file.close();
            return std::pair<lex::Pixmap*, std::vector<hpcourse::Pixel>>(pixmap, pixels);
        }
    };

    tbb::flow::join_node<tbb::flow::tuple<Result, Result, Result>> node_step2_join { graph };

    tbb::flow::function_node<tbb::flow::tuple<Result, Result, Result>, tbb::flow::tuple<Result, Result, Result>> node_step3 {
        graph,
        tbb::flow::unlimited,
        [](tbb::flow::tuple<Result, Result, Result> tuple) {
            auto& min_brightness = std::get<0>(tuple).second;
            auto& max_brightness = std::get<1>(tuple).second;
            auto& user_brightness = std::get<2>(tuple).second;

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

            return tuple;
        }
    };

    tbb::flow::broadcast_node<tbb::flow::tuple<Result, Result, Result>> node_step4_broadcast { graph } ;

    tbb::flow::function_node<tbb::flow::tuple<Result, Result, Result>> node_step4_inversion_brightness {
        graph,
        tbb::flow::unlimited,
        [](tbb::flow::tuple<Result, Result, Result> tuple) {
            auto& pixmap = std::get<0>(tuple).first;
            for(::uint32_t row = 0; row < pixmap->height(); row++) {
                for(::uint32_t column = 0; column < pixmap->width(); column++) {
                    const auto value = pixmap->at(row, column);
                    pixmap->at(row, column) = (255 - value);
                }
            }
            std::ofstream file {filepath_inversion_brigthness, std::ios_base::app};
            print(file, pixmap);
            file.close();
        }
    };

    tbb::flow::function_node<tbb::flow::tuple<Result, Result, Result>> node_step4_average_brightness {
        graph,
        tbb::flow::unlimited,
        [&program_arguments](tbb::flow::tuple<Result, Result, Result> tuple) {
            auto& pixmap = std::get<0>(tuple).first;
            ::uint32_t brightness = 0;
            for(::uint32_t row = 0; row < pixmap->height(); row++) {
                for(::uint32_t column = 0; column < pixmap->width(); column++) {
                    brightness += pixmap->at(row, column);
                }
            }
            const auto average_brightness = brightness / (pixmap->width() * pixmap->height());

            std::ofstream file {program_arguments.file(), std::ios_base::app};
            file << average_brightness << "\n";
            file.close();
            file.open(filepath_average_brigthness, std::ios_base::app);
            print(file, pixmap);
            file.close();
        }
    };

    tbb::flow::make_edge(node_step1_broadcast, node_step2_find_min_brightness);
    tbb::flow::make_edge(node_step1_broadcast, node_step2_find_max_brightness);
    tbb::flow::make_edge(node_step1_broadcast, node_step2_find_user_value);
    tbb::flow::make_edge(node_step2_find_min_brightness, tbb::flow::input_port<0>(node_step2_join));
    tbb::flow::make_edge(node_step2_find_max_brightness, tbb::flow::input_port<1>(node_step2_join));
    tbb::flow::make_edge(node_step2_find_user_value, tbb::flow::input_port<2>(node_step2_join));
    tbb::flow::make_edge(node_step2_join, node_step3);
    tbb::flow::make_edge(node_step3, node_step4_broadcast);
    tbb::flow::make_edge(node_step4_broadcast, node_step4_inversion_brightness);
    tbb::flow::make_edge(node_step4_broadcast, node_step4_average_brightness);

    std::remove(program_arguments.file().c_str());
    std::remove(filepath_min_brigthness);
    std::remove(filepath_max_brigthness);
    std::remove(filepath_user_brigthness);
    std::remove(filepath_inversion_brigthness);
    std::remove(filepath_average_brigthness);

    ::uint32_t images = program_arguments.threads();
    while (images > 0) {
        for(::uint32_t i = 0; i < program_arguments.threads(); i++) {
            lex::Pixmap* pixmap = new lex::Pixmap;
            pixmap->create(30, 30);
            node_step1_broadcast.try_put(pixmap);
        }
        images -= program_arguments.threads();
    }

    graph.wait_for_all();

    std::cout << "\n";
    return 0;
}
