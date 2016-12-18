//
//  flow.cpp
//  ImageProcessing
//
//  Created by Anton Davydov on 17/12/2016.
//
//

#include "flow.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "ImageMagick-6/Magick++.h"
#include "utils.h"
#include <vector>
#include <iostream>
using namespace std;
using namespace tbb;
using namespace tbb::flow;
using namespace Magick;

typedef const function <void (BrightnessType&, BrightnessType&, PixelType pixel, PixelsType&)> BrightnessOperation;
typedef const function <void (BrightnessType&, BrightnessType&, PixelsType&, PixelsType&)> ReduceOperation;
class BrightnessNode {
public:
    PixelsType pixels;
    BrightnessType &result;
    BrightnessOperation& brightnessOperation;
    ReduceOperation& reduceOperation;
    BrightnessNode(BrightnessType &result, BrightnessOperation &operation, ReduceOperation& reduceOperation):
                                    result(result), brightnessOperation(operation), reduceOperation(reduceOperation) { }
    
    PixelsType operator()(Image* im) {
        spin_mutex mutex;
        int n = task_scheduler_init::default_num_threads();
        int r = im->columns();
        int c = im->rows();
        BrightnessType initRes = result;
        
        parallel_for(blocked_range2d<size_t>(0, r, r/n, 0, c, c/n), [&im, &mutex, &initRes, this](const blocked_range2d<size_t> &r) {
            
            const auto& image = im;
            BrightnessType localResult = initRes;
            PixelsType localPixels;
        
            for (size_t i = r.rows().begin(); i != r.rows().end(); ++i) {
                for (size_t j = r.cols().begin(); j != r.cols().end(); ++j) {
                    auto pixelColor = image->pixelColor(i, j);
                    auto currentValue = Utils::brightnessOfPixelColor(pixelColor);
                    this->brightnessOperation(currentValue, localResult, make_tuple(i, j), localPixels);
                }
            }
            {
                spin_mutex::scoped_lock lock(mutex);
                this->reduceOperation(localResult, this->result, localPixels, pixels);
            }
        });
    
        return pixels;
    }
};

typedef tuple<Image*, PixelsType,PixelsType,PixelsType> HighlightInputType;
class HighlightNode {
public:
    Image* operator()(HighlightInputType input) {
        int n = task_scheduler_init::default_num_threads();
        auto image = new Image(*get<0>(input));
        PixelsType maxBrightPixels = get<1>(input);
        PixelsType minBrightPixels = get<2>(input);
        PixelsType givenBrightPixels = get<3>(input);
        
        parallel_for(blocked_range<size_t>(0, maxBrightPixels.size(), maxBrightPixels.size()/n), [&image, &maxBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, maxBrightPixels[i], "red");
            }
        });
        
        parallel_for(blocked_range<size_t>(0, minBrightPixels.size(), minBrightPixels.size()/n), [&image, &minBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, minBrightPixels[i], "green");
            }
        });
        
        parallel_for(blocked_range<size_t>(0, givenBrightPixels.size(), givenBrightPixels.size()/n), [&image, &givenBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, givenBrightPixels[i], "blue");
            }
        });
        
        image->write("/Volumes/FlashDrive/Projects/hpcourse/leti/1304/da/1/ImageProcessing/src/5.jpg");
        delete image;
        
        return get<0>(input);
    }
};

class InverseBrightnessNode {
public:
    Image* operator()(Image *im) {
        
        // TODO: Default imp "im->negate();"
        int n = task_scheduler_init::default_num_threads();
        int rows = im->rows();
        int columns = im->columns();
        
        Pixels view(*im);
        auto pixels = view.get(0, 0, columns, rows);
        
        parallel_for(blocked_range2d<size_t>(0, rows, rows/n, 0, columns, columns/n), [&pixels, &columns, this](const blocked_range2d<size_t> &r) {
            for (size_t i = r.rows().begin(); i != r.rows().end(); ++i) {
                for (size_t j = r.cols().begin(); j != r.cols().end(); ++j) {
                    Utils::inverseBrightnessOfPixel(pixels, i*columns + j);
                }
            }
        });
        
        view.sync();
        im->write("/Volumes/FlashDrive/Projects/hpcourse/leti/1304/da/1/ImageProcessing/src/6.jpg");
        
        return im;
    }
};

class AverageBrightnessNode {
public:
    BrightnessType operator()(Image *im) {
        int n = task_scheduler_init::default_num_threads();
        int rows = im->rows();
        int columns = im->columns();
        BrightnessType result;
        spin_mutex mutex;
        
        parallel_for(blocked_range2d<size_t>(0, rows, rows/n, 0, columns, columns/n), [&result, &mutex, &im, this](const blocked_range2d<size_t> &r) {
            BrightnessType localResult = 0;
            for (size_t i = r.rows().begin(); i != r.rows().end(); ++i) {
                for (size_t j = r.cols().begin(); j != r.cols().end(); ++j) {
                    auto color = im->pixelColor(i, j);
                    localResult += Utils::brightnessOfPixelColor(color);
                }
            }
            
            {
                spin_mutex::scoped_lock lock(mutex);
                result += localResult;
            }
        });
        
        return result;
    }
};

Output startFlow(ConsoleInput input) {
    
    graph g;
    auto sourceIter = input.images.begin();
    
    BrightnessType maxBright = 0;
    BrightnessType minBright = 255;
    
    source_node<Image*> readNode( g, [&]( Image* &v ) -> bool {
        if (sourceIter != input.images.end()) {
            try {
                auto image = new Image();
                image->read(*sourceIter);
                v = image;
                cout<<image->rows()<<endl;
            } catch (Exception &e)  {
                cout<<&e<<endl;
            }
            sourceIter++;
            
            return true;
        } else {
            return false;
        }
    });
    
    limiter_node<Image*> l(g, 1);
    
    auto maxBrigNode = BrightnessNode(maxBright,
                                     [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                         if (l > r) { r = l; ps.clear(); ps.push_back(p); }
                                         else if (l == r) { ps.push_back(p); }
                                        },
                                     [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                         if (l > r) { r = l; r_p.clear(); r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                         else if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                     });
    auto minBrigNode = BrightnessNode(minBright,
                                      [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                          if (l < r) { r = l; ps.clear(); ps.push_back(p); }
                                          else if (l == r) { ps.push_back(p); }
                                      },
                                      [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                          if (l < r) { r = l; r_p.clear(); r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                          else if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                      });
    
    auto brigByGivenNode = BrightnessNode(input.inputBrightness,
                                          [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                              if (l == r) { ps.push_back(p); }
                                          },
                                          [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                              if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                          });
    function_node<Image*, PixelsType> maxBrightnessNode( g, 1, maxBrigNode);
    function_node<Image*, PixelsType> minBrightnessNode( g, 1, minBrigNode);
    function_node<Image*, PixelsType> givenBrightnessNode( g, 1, brigByGivenNode);
    function_node<HighlightInputType, Image*> highlightNode( g, 1, HighlightNode());
    function_node<Image*, Image*> inverseNode( g, 1, InverseBrightnessNode());
    function_node<Image*, BrightnessType> averageNode( g, 1, AverageBrightnessNode());
    broadcast_node<Image*> broabcastImage( g );
    join_node<HighlightInputType> j(g);
    
    make_edge(readNode, l);
    make_edge(l, maxBrightnessNode);
    make_edge(l, minBrightnessNode);
    make_edge(l, givenBrightnessNode);
    make_edge(l, input_port<0>(j));
    make_edge(maxBrightnessNode, input_port<1>(j));
    make_edge(minBrightnessNode, input_port<2>(j));
    make_edge(givenBrightnessNode, input_port<3>(j));
    make_edge(j, highlightNode);
    make_edge(highlightNode, broabcastImage);
    make_edge(broabcastImage, inverseNode);
    make_edge(broabcastImage, averageNode);
//    make_edge(highlightNode, l.decrement);
    
    readNode.activate();
    g.wait_for_all();
    cout<<"wait stopped"<<endl;
    
    cout << maxBrigNode.result << " " << minBrigNode.result << " " << brigByGivenNode.result <<endl;
    
    return Output();
}
