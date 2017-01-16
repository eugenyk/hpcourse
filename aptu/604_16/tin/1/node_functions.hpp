#pragma once

#include "image.hpp"
#include <iostream>
#include <algorithm>
#include <utility>
#include <memory>
#include <fstream>

using std::shared_ptr;
using namespace tbb::flow;

struct ImageWithValue {
    shared_ptr<Image> image_;
    int value_;
    ImageWithValue(): image_(), value_() {}
    ImageWithValue(shared_ptr<Image> image, int value): image_(image), value_(value) {}
};

struct ImageWithPoints {
    shared_ptr<Image> image_;
    std::vector<std::pair<size_t, size_t>> indices_;
    ImageWithPoints(): image_(), indices_() {}
    ImageWithPoints(shared_ptr<Image> image, const vector<std::pair<size_t, size_t>>& indices_):
            image_(image),
            indices_(indices_) {}
};

struct image_with_points_to_key {
    int operator()(const ImageWithPoints& imageWithPoints) {
        return imageWithPoints.image_->GetId();
    }
};

struct image_to_key {
    int operator()(shared_ptr<Image> image) {
        return image->GetId();
    }
};

template <class T>
struct tuple_to_key {
    int operator()(const std::tuple<T, int>& tuple) {
        return std::get<1>(tuple);
    }
};

typedef std::tuple<double, int> float_with_id;

namespace node_functions {
    struct image_generator {
    private:
        size_t counter_;
        const size_t number_of_images_;
        const size_t image_rows_;
        const size_t image_cols_;
    public:
        image_generator(size_t number_of_images, size_t image_rows, size_t image_cols): counter_(0),
                                                 number_of_images_(number_of_images),
                                                 image_rows_(image_rows),
                                                 image_cols_(image_cols)
        {};

        bool operator()(std::shared_ptr<Image>& image) {
            if (counter_ > number_of_images_) {
                return false;
            }
            image = std::make_shared<Image>(image_rows_, image_cols_, counter_);
            ++counter_;
            return true;
        }
    };

    struct max {
        ImageWithValue operator()(shared_ptr<Image> image) {
            std::vector<int> row_max;
            for (auto &row: image->GetData()) {
                row_max.push_back(*std::max_element(row.begin(), row.end()));
            }
            int max_el = *std::max_element(row_max.begin(), row_max.end());
            auto image_with_value = ImageWithValue(image, max_el);
            return image_with_value;

        }
    };

    struct min {
        ImageWithValue operator()(shared_ptr<Image> image) {
            std::vector<int> row_min;
            for (auto &row: image->GetData()) {
                row_min.push_back(*std::min_element(row.begin(), row.end()));
            }
            int max_el = *std::max_element(row_min.begin(), row_min.end());
            auto image_with_value = ImageWithValue(image, max_el);
            return image_with_value;
        }
    };

    struct add_value {
    private:
        int value_;
    public:
        add_value(int value): value_(value) {}

        ImageWithValue operator() (shared_ptr<Image> image) {
            auto image_with_value = ImageWithValue(image, value_);
            return image_with_value;
        }
    };

    struct get_values {
        ImageWithPoints operator()(const ImageWithValue &image_with_value) {
            int value = image_with_value.value_;
            auto image = image_with_value.image_;
            size_t nrows = image->GetRows();
            size_t ncols = image->GetCols();
            const auto& data = image->GetData();
            vector<std::pair<size_t, size_t>> indices;
            for (size_t i = 0; i < nrows; ++i) {
                for (size_t j = 0; j < ncols; ++j) {
                    if (data[i][j] == value)
                        indices.push_back(std::make_pair(i, j));
                }
            }
            auto image_with_points = ImageWithPoints(image, indices);
            return image_with_points;
        }
    };

    struct merge {
        size_t square_side_;

        merge(size_t square_side) : square_side_(square_side) {}

        shared_ptr<Image> operator()(const tbb::flow::tuple<ImageWithPoints, ImageWithPoints, ImageWithPoints>& tuple) {
            shared_ptr<Image> image = get<0>(tuple).image_;
            typedef std::vector<std::pair<size_t, size_t>> index_vector;
            typedef std::shared_ptr<index_vector> index_ptr;
            index_ptr indices_0 = std::make_shared<index_vector>(get<0>(tuple).indices_);
            index_ptr indices_1 = std::make_shared<index_vector>(get<1>(tuple).indices_);
            index_ptr indices_2 = std::make_shared<index_vector>(get<2>(tuple).indices_);
            vector<index_ptr> index_vectors ({indices_0, indices_1, indices_2});
            for (const auto& ptr: index_vectors) {
                for (size_t i = 0; i < ptr->size(); ++i) {
                    image->HighlightPixel((*ptr)[i].first, (*ptr)[i].second, square_side_);
                }
            }
            return image;
        }
    };

    struct invert {
        shared_ptr<Image> operator()(shared_ptr<Image> image) {
            Image inverted = image->GetInverted();
            shared_ptr<Image> inverted_ptr = std::make_shared<Image>(inverted);
            return inverted_ptr;
        }
    };

    struct count_average {
        float_with_id operator()(shared_ptr<Image> image) {
            int id = image->GetId();
            double average = image->GetAverage();
            auto tuple = std::tuple<double, int>(average, id);
            return tuple;
        }
    };

    struct logger {
        std::string filename_;
        logger(std::string filename) : filename_(filename) {}

        float_with_id operator()(const float_with_id& average_tuple) {
            int id = std::get<1>(average_tuple);
            double value = std::get<0>(average_tuple);
            std::ofstream fout;
            fout.open(filename_, std::ios_base::app);
            fout << "Image id: " << id << std::endl;
            fout << "Average: " << value << std::endl;
            return average_tuple;
        }
    };
}


