//
// Created by antonpp on 16.01.17.
//

#include "ImageProcessor.h"

using namespace tbb::flow;
using std::make_shared;
using std::vector;

namespace {

    typedef std::pair<Image, vector<size_t> > f_node_result_t;
    typedef tuple<f_node_result_t, f_node_result_t, f_node_result_t> first_stage_tuple;
    typedef tuple<bool, bool> second_stage_tuple;

    vector<size_t> get_indices(const Image &image, pixel_t pixel_value) {
        vector<size_t> result;
        for (size_t i = 0; i < image.get_pixels().size(); ++i) {
            if (image.get_pixels()[i] == pixel_value) {
                result.push_back(i);
            }
        }

        return result;
    }

    void invert_border(const Image &image, size_t selected_pixel) {
        const auto &border = image.get_border(selected_pixel);
        for (auto index: border) {
            Image::invert_pixel(image.get_pixel(index));
            // some useful code here
        }
    }

    void invert_border(f_node_result_t image_pixels) {
        for (auto index: image_pixels.second) {
            invert_border(image_pixels.first, index);
        }
    }

    vector<pixel_t> get_selected_pixels(f_node_result_t holder) {
        vector<pixel_t> result;
        for (auto x : holder.second) {
            result.push_back(holder.first.get_pixel(x));
        }
        return result;
    }

    size_t get_job_id(f_node_result_t node_result) {
        return node_result.first.get_id();
    }
}

void ImageProcessor::process() {
    source_generation_node->activate();
    flow_graph.wait_for_all();
}

ImageProcessor::ImageProcessor(const vector<Image> &images,
                               pixel_t pixel_value,
                               size_t image_parallel,
                               std::string log_fname)
        : pixel_to_search(pixel_value), images(images), average_pixel_log(log_fname) {

    auto source_f = [&](Image &image) {
        image = this->images[generated_images++];
        return generated_images < this->images.size();
    };

    auto max_pixel_f = [](const Image &image) {
        auto max = *std::max_element(image.get_pixels().begin(), image.get_pixels().end());
        return std::make_pair(image, get_indices(image, max));
    };
    auto min_pixel_f = [](const Image &image) {
        auto min = *std::min_element(image.get_pixels().begin(), image.get_pixels().end());
        return std::make_pair(image, get_indices(image, min));
    };

    auto search_pixel_f = [&](const Image &image) {
        return std::make_pair(image, get_indices(image, pixel_to_search));
    };

    auto invert_selected_f = [](const first_stage_tuple &tuple) {
        invert_border(get<0>(tuple));
        invert_border(get<1>(tuple));
        invert_border(get<2>(tuple));

        return true;
    };

    auto average_selected_f = [&](first_stage_tuple const &t) {

        vector<pixel_t> selected_pixels;
        auto image_with_selected_pixels = get<0>(t);
        auto r = get_selected_pixels(image_with_selected_pixels);
        std::copy(r.begin(), r.end(), std::back_inserter(selected_pixels));

        image_with_selected_pixels = get<1>(t);
        r = get_selected_pixels(image_with_selected_pixels);
        std::copy(r.begin(), r.end(), std::back_inserter(selected_pixels));

        image_with_selected_pixels = get<2>(t);
        r = get_selected_pixels(image_with_selected_pixels);
        std::copy(r.begin(), r.end(), std::back_inserter(selected_pixels));

        size_t value = std::accumulate<vector<pixel_t>::iterator, size_t>(selected_pixels.begin(),
                                                                          selected_pixels.end(), 0);

        average_pixel_log << (value / selected_pixels.size()) << std::endl;

        return true;
    };

    auto stub_continue = [](const second_stage_tuple &) { return continue_msg(); };

    /* vertices */

    // setup
    source_generation_node = make_shared<source_node<Image> >(flow_graph, source_f, false);
    auto limiter = make_shared<limiter_node<Image> >(flow_graph, image_parallel);
    auto input_broadcast_node = make_shared<broadcast_node<Image> >(flow_graph);

    // stage 1
    auto max_node = make_shared<function_node<Image, f_node_result_t> >(flow_graph, serial, max_pixel_f);
    auto min_node = make_shared<function_node<Image, f_node_result_t> >(flow_graph, serial, min_pixel_f);
    auto search_node = make_shared<function_node<Image, f_node_result_t> >(flow_graph, serial, search_pixel_f);
    auto first_stage_joiner_node = make_shared<join_node<first_stage_tuple, key_matching<size_t> > >(flow_graph,
                                                                                                     get_job_id,
                                                                                                     get_job_id,
                                                                                                     get_job_id);
    auto first_stage_broadcast_node = make_shared<broadcast_node<first_stage_tuple> >(flow_graph);

    // stage 2
    auto invert_border_node = make_shared<function_node<first_stage_tuple, bool> >(flow_graph, unlimited,
                                                                                   invert_selected_f);
    auto calc_average_node = make_shared<function_node<first_stage_tuple, bool> >(flow_graph, unlimited,
                                                                                  average_selected_f);
    auto second_stage_joiner_node = make_shared<join_node<second_stage_tuple> >(flow_graph);

    // update limiter
    auto decrement_limiter_node = make_shared<function_node<second_stage_tuple, continue_msg> >(flow_graph, unlimited,
                                                                                                stub_continue);

    /* edges */

    // setup
    make_edge(*source_generation_node, *limiter);
    make_edge(*limiter, *input_broadcast_node);

    // stage 1
    make_edge(*input_broadcast_node, *max_node);
    make_edge(*input_broadcast_node, *search_node);
    make_edge(*input_broadcast_node, *min_node);
    make_edge(*max_node, input_port<0>(*first_stage_joiner_node));
    make_edge(*search_node, input_port<1>(*first_stage_joiner_node));
    make_edge(*min_node, input_port<2>(*first_stage_joiner_node));
    make_edge(*first_stage_joiner_node, *first_stage_broadcast_node);

    // stage 2
    make_edge(*first_stage_broadcast_node, *invert_border_node);
    make_edge(*first_stage_broadcast_node, *calc_average_node);
    make_edge(*invert_border_node, input_port<0>(*second_stage_joiner_node));
    make_edge(*calc_average_node, input_port<1>(*second_stage_joiner_node));

    // decrement
    make_edge(*second_stage_joiner_node, *decrement_limiter_node);
    make_edge(*decrement_limiter_node, limiter->decrement);

    nodes = {
            source_generation_node, limiter, input_broadcast_node, max_node, min_node, search_node,
            first_stage_joiner_node, first_stage_broadcast_node, invert_border_node, calc_average_node,
            second_stage_joiner_node, decrement_limiter_node
    };
}

