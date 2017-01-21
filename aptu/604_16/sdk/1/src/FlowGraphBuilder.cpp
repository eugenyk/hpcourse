//
// Created by dsavvinov on 09.12.16.
//

#include <fstream>
#include <iostream>
#include "FlowGraphBuilder.h"

template <class T>
struct ImageResult {
    Image img;
    T result;

    ImageResult() : img(Image()) { }
    ImageResult(Image const & img) : img(img) { }

    ImageResult & operator=(ImageResult const & other) {
        img = other.img;
        result = other.result;
        return *this;
    }
};

typedef std::vector < std::pair<size_t, size_t> > Positions;
typedef tbb::flow::tuple<ImageResult<Positions>, ImageResult<Positions>, ImageResult<Positions>> FlowTriple;

void FlowGraphBuilder::highlight(Image & image, std::vector<std::pair<size_t, size_t>> positions) {
    int dx[] = {-1, -1, -1,  0, 0,  1, 1, 1};
    int dy[] = {-1,  0,  1, -1, 1, -1, 0, 1};

    for (auto pos: positions) {
        for (int i = 0; i < 8; ++i) {
            int64_t x = pos.first + dx[i];
            int64_t y = pos.second + dy[i];

            if (x < 0 || x >= int64_t(image.size()) || y < 0 || y >= int64_t(image[0].size()) )
                continue;

            image[x][y] = 255;
        }
    }
}

void FlowGraphBuilder::buildFlowGraph(
        std::vector<Image> images,
        size_t limit,
        uint8_t brightness,
        std::string logfile
) {
    using namespace tbb::flow;

    graph g;
    size_t cur_ind = 0;
    source_node<Image> source(g,
          [&cur_ind, &images] (Image & img) {
              if (cur_ind >= images.size()) {
                  return false;
              }
              img = images[cur_ind++];
              return true;
          }
    , false);

    limiter_node<Image> limiterNode(g, limit);

    function_node<Image, ImageResult<Positions>> maxBrightnessEvaluator(g, tbb::flow::unlimited,
            [] (Image const & img) {
                ImageResult<Positions> result (img);

                int mx = -1;
                for (size_t i = 0; i < img.size(); ++i) {
                    for (size_t j = 0; j < img.size(); ++j) {
                        if (mx < img[i][j]) {
                            mx = img[i][j];
                        }
                    }
                }

                for (size_t i = 0; i < img.size(); ++i) {
                    for (size_t j = 0; j < img.size(); ++j) {
                        if (img[i][j] == mx) {
                            result.result.push_back(std::make_pair(i, j));
                        }
                    }
                }

                return result;
            }
    );

    function_node<Image, ImageResult<Positions>> eqBrightnessEvaluator(g, tbb::flow::unlimited,
        [brightness] (Image const & img) {
            ImageResult<Positions> result(img);
            for (size_t i = 0; i < img.size(); ++i) {
                for (size_t j = 0; j < img.size(); ++j) {
                    if (img[i][j] == brightness) {
                        result.result.push_back(std::make_pair(i, j));
                    }
                }
            }

            return result;
        }
    );

    function_node<Image, ImageResult<Positions>> minBrightnessEvaluator(g, tbb::flow::unlimited,
           [] (Image const & img) {
               ImageResult<Positions> result (img);
               int mn = (1 << 30);
               for (size_t i = 0; i < img.size(); ++i) {
                   for (size_t j = 0; j < img.size(); ++j) {
                       if (mn > img[i][j]) {
                           mn = img[i][j];
                       }
                   }
               }

               for (size_t i = 0; i < img.size(); ++i) {
                   for (size_t j = 0; j < img.size(); ++j) {
                       if (img[i][j] == mn) {
                           result.result.push_back(std::make_pair(i, j));
                       }
                   }
               }

               return result;
           }
    );

    join_node<FlowTriple> combiner(g);

    function_node<FlowTriple, Image> highlighterNode(g, tbb::flow::unlimited,
            [] (FlowTriple const & triple) {
                ImageResult<Positions> imgRes = get<0>(triple);

                Image result (imgRes.img);

                highlight(result, imgRes.result);
                highlight(result, get<1>(triple).result);
                highlight(result, get<2>(triple).result);

                return result;
            }
    );

    function_node<Image, Image> inverter(g, tbb::flow::unlimited,
            [] (Image const & img) {
                Image result (img);

                for (size_t i = 0; i < img.size(); ++i) {
                    for (size_t j = 0; j < img.size(); ++j) {
                        result[i][j] = uint8_t(255) - img[i][j];
                    }
                }

                return result;
            }
    );

    function_node<Image, ImageResult<double>> avarageBrightnessEvaluator(g, tbb::flow::unlimited,
            [] (Image const & img) {
                ImageResult<double> res (img);
                res.result = 0.0;

                for (size_t i = 0; i < img.size(); ++i) {
                    for (size_t j = 0; j < img.size(); ++j) {
                        res.result += img[i][j];
                    }
                }

                res.result /= (img.size() * img[0].size());
                return res;
            }
    );

    function_node<ImageResult<double>, int> printer(g, 1,
            [logfile] (ImageResult<double> imgRes) {
                if (logfile.size() == 0) {
                    return 0;
                }

                std::ofstream out;
                out.open(logfile, std::fstream::out | std::fstream::app);

                out << "Avg Brightness = " << imgRes.result << std::endl;

                out << std::endl << "==================================" << std::endl << std::endl;

                return 0;
            }
    );


    make_edge(source, limiterNode);

    make_edge(limiterNode, maxBrightnessEvaluator);
    make_edge(limiterNode, eqBrightnessEvaluator);
    make_edge(limiterNode, minBrightnessEvaluator);

    make_edge(maxBrightnessEvaluator, input_port<0>(combiner));
    make_edge(eqBrightnessEvaluator, input_port<1>(combiner));
    make_edge(minBrightnessEvaluator, input_port<2>(combiner));

    make_edge(combiner, highlighterNode);

    make_edge(highlighterNode, inverter);
    make_edge(highlighterNode, avarageBrightnessEvaluator);

    make_edge(avarageBrightnessEvaluator, printer);

    source.activate();
    g.wait_for_all();

    return;
}
