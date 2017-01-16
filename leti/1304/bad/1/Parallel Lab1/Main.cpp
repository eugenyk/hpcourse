#include <iostream>
#include <tbb/flow_graph.h>
#include "ApplicationArguments.hpp"
#include "Pixmap.h"
#include "Pixel.h"
#include "LabUtils.h"
#include <fstream>

typedef std::pair<Pixmap*, std::pair<int, int>> NodeResult;
typedef tbb::flow::tuple<NodeResult, NodeResult, NodeResult> NodeTupleResult;

void print_application_arguments(ApplicationArguments* arguments) {
    std::cout << "Application Arguments: " << std::endl;
    std::cout << "Brightness: " << arguments->getBrightness() << std::endl;
    std::cout << "Image number: " << arguments->getImageNumbers() << std::endl;
    std::cout << "Log file name: " << arguments->getLogFileName() << std::endl;
}

int main(int argc, const char * argv[]) {
    ApplicationArguments* arguments = new ApplicationArguments();
    arguments->extract(argc, argv);
    
    print_application_arguments(arguments);
    
    tbb::flow::graph graph;
    tbb::flow::broadcast_node<Pixmap*> node_step1_broadcast { graph } ;
    
    tbb::flow::function_node<Pixmap*, NodeResult> node_step2_find_min_brightness {
        graph,
        tbb::flow::unlimited,
        [](Pixmap* pixmap) {
            Pixel* min_value = pixmap->get(0, 0);
            int min_brightness = labutils::getBrightness(min_value);
            int pixel_i = 0, pixel_j = 0;
            
            for(int i = 0; i < pixmap->getWidth(); i++) {
                for(int j = 0; j < pixmap->getHeight(); j++) {
                    Pixel* pixel = pixmap->get(i, j);
                    int current_min = labutils::getBrightness(pixel);
                    if(current_min < min_brightness) {
                        min_value = pixel;
                        min_brightness = current_min;
                        pixel_i = i;
                        pixel_j = j;
                    }
                }
            }
            return NodeResult(pixmap, std::pair<int,int>(pixel_i, pixel_j));
        }
    };
    
    tbb::flow::function_node<Pixmap*, NodeResult> node_step2_find_max_brightness {
        graph, tbb::flow::unlimited, [](Pixmap* pixmap) {
            Pixel* max_pixel = pixmap->get(0, 0);
            int max_brightness = labutils::getBrightness(max_pixel);
            int pixel_i = 0, pixel_j = 0;
            
            for(int i = 0; i < pixmap->getWidth(); i++) {
                for(int j = 0; j < pixmap->getHeight(); j++) {
                    Pixel* pixel = pixmap->get(i, j);
                    int current_max = labutils::getBrightness(pixel);
                    if(current_max > max_brightness) {
                        max_pixel = pixel;
                        max_brightness = current_max;
                        pixel_i = i;
                        pixel_j = j;
                    }
                }
            }
            return NodeResult(pixmap, std::pair<int,int>(pixel_i, pixel_j));
        }
    };
    
    tbb::flow::function_node<Pixmap*, NodeResult> node_step2_find_user_value {
        graph, tbb::flow::unlimited, [&arguments](Pixmap* pixmap) {
            std::pair<int,int>* pixel_coords = NULL;
            
            for(int i = 0; i < pixmap->getWidth(); i++) {
                for(int j = 0; j < pixmap->getHeight(); j++) {
                    Pixel* pixel = pixmap->get(i, j);
                    int pixel_brightness = labutils::getBrightness(pixel);
                    if(pixel_brightness == arguments->getBrightness()) {
                        if(pixel_coords != NULL) delete pixel_coords;
                        pixel_coords = new std::pair<int,int>(i, j);
                    }
                }
            }
            return NodeResult(pixmap, *pixel_coords);
        }
    };
    
    tbb::flow::join_node<NodeTupleResult> node_step2_join { graph };
    
    
    tbb::flow::function_node<NodeTupleResult, NodeTupleResult> node_step3 {
        graph, tbb::flow::unlimited, [](NodeTupleResult tuple) {
            std::cout << "Min brightness: " << "\n";
            labutils::printPixelWithMetadata(std::get<0>(tuple));
            
            std::cout << "Max brightness: " << "\n";
            labutils::printPixelWithMetadata(std::get<1>(tuple));
            
            std::cout << "User brightness: " << "\n";
            labutils::printPixelWithMetadata(std::get<2>(tuple));
            
            return tuple;
        }
    };
    
    tbb::flow::function_node<NodeTupleResult> node_step4_average_brightness {
        graph, tbb::flow::unlimited, [&arguments](NodeTupleResult tuple) {
            auto& pixmap = std::get<0>(tuple).first;
            ::uint32_t brightness = 0;
            for(::uint32_t row = 0; row < pixmap->getHeight(); row++) {
                for(::uint32_t column = 0; column < pixmap->getWidth(); column++) {
                    brightness += labutils::getBrightness(pixmap->get(row, column));
                }
            }
            const auto average_brightness = brightness / (pixmap->getWidth() * pixmap->getHeight());
            
//            std::ofstream file { arguments->getLogFileName(), std::ios_base::app };
//            file << average_brightness << "\n";
//            file.close();
//            file.open(filepath_average_brigthness, std::ios_base::app);
//            print(file, pixmap);
//            file.close();
        }
    };
    
    tbb::flow::broadcast_node<NodeTupleResult> node_step4_broadcast { graph };
    
    tbb::flow::function_node<NodeTupleResult> node_step4_inversion_brightness {
        graph, tbb::flow::unlimited, [](NodeTupleResult tuple) {
                auto& pixmap = std::get<0>(tuple).first;
                for(::uint32_t row = 0; row < pixmap->getHeight(); row++) {
                        for(::uint32_t column = 0; column < pixmap->getWidth(); column++) {
                            const auto value = labutils::getBrightness(pixmap->get(row, column));
//                               pixmap->get(row, column) = (255 - value);
                           }
                    }
//                std::ofstream file {filepath_inversion_brigthness, std::ios_base::app};
//                print(file, pixmap);
//                file.close();
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
    
//    std::remove(program_arguments.file().c_str());
//    std::remove(filepath_min_brigthness);
//    std::remove(filepath_max_brigthness);
//    std::remove(filepath_user_brigthness);
//    std::remove(filepath_inversion_brigthness);
//    std::remove(filepath_average_brigthness);
    
    for(int i = 0; i < arguments->getImageNumbers(); i++) {
        Pixmap* pixmap = new Pixmap(50, 50);
        node_step1_broadcast.try_put(pixmap);
    }
    graph.wait_for_all();
    
    std::cout << "\n";
    return 0;
}
