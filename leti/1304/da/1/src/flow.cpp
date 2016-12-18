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
/**
 Base node to calculate brightness of image.
 Takes compare and reduce operators and find list of pixels with found value.
 */
class BrightnessNode {
protected:
    CommandLineInput& args;
public:
    PixelsType pixels;
    BrightnessType &result;
    BrightnessOperation& brightnessOperation;
    ReduceOperation& reduceOperation;
    BrightnessNode(CommandLineInput& args, BrightnessType &result,
                   BrightnessOperation &operation, ReduceOperation& reduceOperation):
                        args(args), result(result), brightnessOperation(operation), reduceOperation(reduceOperation) { }
    
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
        
        if (args.verbose)
            cout<<"Pixels with brightness " << (int)result << " for image \"" << Utils::imageName(im) << "\""<<endl;
    
        return pixels;
    }
};

typedef tuple<Image*, PixelsType,PixelsType,PixelsType> HighlightInputType;
/**
 Allows to draw rectangles on a given image.
 red rect for pixel with maximum brightness; green - with minimum brightness; blue - according to given value from command line.
 */
class HighlightNode {
protected:
    CommandLineInput& args;
public:
    
    HighlightNode(CommandLineInput& args) : args(args) {}
    
    Image* operator()(HighlightInputType input) {
        int n = task_scheduler_init::default_num_threads();
        auto image = new Image(*get<0>(input));
        PixelsType maxBrightPixels = get<1>(input);
        PixelsType minBrightPixels = get<2>(input);
        PixelsType givenBrightPixels = get<3>(input);
        
        
        image->strokeWidth(2);
        auto fillColor = Color("white");
        fillColor.alpha(1.0);
        image->fillColor(fillColor);
        
        image->strokeColor("red");
        parallel_for(blocked_range<size_t>(0, maxBrightPixels.size(), maxBrightPixels.size()/n), [&image, &maxBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, maxBrightPixels[i]);
            }
        });
        
        image->strokeColor("green");
        parallel_for(blocked_range<size_t>(0, minBrightPixels.size(), minBrightPixels.size()/n), [&image, &minBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, minBrightPixels[i]);
            }
        });
        
        image->strokeColor("blue");
        parallel_for(blocked_range<size_t>(0, givenBrightPixels.size(), givenBrightPixels.size()/n), [&image, &givenBrightPixels](const blocked_range<size_t> &r) {
            for (size_t i = r.begin(); i != r.end(); ++i) {
                Utils::highlightPixel(image, givenBrightPixels[i]);
            }
        });
        
        auto fileName = "highlight_" + Utils::imageName(image);
        image->write(args.output + fileName);
        delete image;
        
        if (args.verbose)
            cout<<"Image " << fileName << " has been written to output dir" <<endl;
        
        
        return get<0>(input);
    }
};

/**
 Inverses colors/brightness of pixels and write to output dir.
 */
class InverseBrightnessNode {
protected:
    CommandLineInput& args;
public:
    InverseBrightnessNode(CommandLineInput& args): args(args) {}
    
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
        auto fileName = "inverse_" + Utils::imageName(im);
        im->write(args.output + fileName);
        
        if (args.verbose)
            cout<<"Image " << fileName << " has been written to output dir" <<endl;
        
        return im;
    }
};

/**
 Allows to find average value of brightness and print it if verbose mode.
 */
class AverageBrightnessNode {
protected:
    CommandLineInput& args;
public:
    AverageBrightnessNode(CommandLineInput& args): args(args) {}
    
    Image* operator()(Image *im) {
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
        
        if (args.verbose)
            cout<<"Average brightness of \"" << Utils::imageName(im) << "\" = "<< (int)result <<endl;
        
        return im;
    }
};

void startFlow(CommandLineInput input) {
    
    if (input.verbose)
        cout<<"Start"<<endl;
    graph g;
    auto sourceIter = input.images.begin();
    
    BrightnessType maxBright = 0;
    BrightnessType minBright = 255;
    
    source_node<Image*> readNode( g, [&]( Image* &v ) -> bool {
        while (sourceIter != input.images.end()) {
            try {
                auto image = new Image();
                auto fileName = *sourceIter;
                image->read(fileName);
                v = image;
                sourceIter++;
                
                if (input.verbose)
                    cout<<"Image " + Utils::imageName(image) + " has been read from input dir" <<endl;
                
                return true;
            } catch (Exception &e)  {
                sourceIter++;
            }
        }
        return false;
    });
    
    limiter_node<Image*> l(g, 1);
    
    auto maxBrigNode = BrightnessNode(input, maxBright,
                                     [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                         if (l > r) { r = l; ps.clear(); ps.push_back(p); }
                                         else if (l == r) { ps.push_back(p); }
                                        },
                                     [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                         if (l > r) { r = l; r_p.clear(); r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                         else if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                     });
    auto minBrigNode = BrightnessNode(input, minBright,
                                      [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                          if (l < r) { r = l; ps.clear(); ps.push_back(p); }
                                          else if (l == r) { ps.push_back(p); }
                                      },
                                      [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                          if (l < r) { r = l; r_p.clear(); r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                          else if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                      });
    
    auto brigByGivenNode = BrightnessNode(input, input.inputBrightness,
                                          [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                              if (l == r) { ps.push_back(p); }
                                          },
                                          [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                              if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                          });
    function_node<Image*, PixelsType> maxBrightnessNode( g, 1, maxBrigNode);
    function_node<Image*, PixelsType> minBrightnessNode( g, 1, minBrigNode);
    function_node<Image*, PixelsType> givenBrightnessNode( g, 1, brigByGivenNode);
    function_node<HighlightInputType, Image*> highlightNode( g, 1, HighlightNode(input));
    function_node<Image*, Image*> inverseNode( g, 1, InverseBrightnessNode(input));
    function_node<Image*, Image*> averageNode( g, 1, AverageBrightnessNode(input));
    function_node<tuple<Image*,Image*>, continue_msg> garbageCollectorNode( g, 1, [](tuple<Image*,Image*> im){ delete get<0>(im); });
    join_node<tuple<Image*, Image*>> joinNode( g);
    
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
    make_edge(inverseNode, input_port<0>(joinNode));
    make_edge(averageNode, input_port<1>(joinNode));
    make_edge(joinNode, garbageCollectorNode);
    make_edge(garbageCollectorNode, l.decrement);
    
    readNode.activate();
    
    g.wait_for_all();
    if (input.verbose)
        cout<<"Finish"<<endl;
}
