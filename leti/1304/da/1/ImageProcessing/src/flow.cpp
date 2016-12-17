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

Output startFlow(ConsoleInput input) {
    
    graph g;
    auto sourceIter = input.images.begin();
    
    BrightnessType maxBright = -1;
    
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
    
    function_node<Image*, continue_msg> maxBrightnessNode( g, 1, [&maxBright](Image* im) -> continue_msg {
        
        const auto& nodeImage = im;
        spin_mutex mutex;
        int n = task_scheduler_init::default_num_threads();
        int r = nodeImage->rows();
        int c = nodeImage->columns();
        
        parallel_for(blocked_range2d<size_t>(0, r, r/n, 0, c, c/n), [&nodeImage, &maxBright, &mutex](const blocked_range2d<size_t> &r) {
        
            const auto& image = nodeImage;
            BrightnessType localMax = 0;
            cout<<"started "<<endl;
            for (size_t i = r.rows().begin(); i != r.rows().end(); ++i) {
                for (size_t j = r.cols().begin(); j != r.cols().end(); ++j) {
                    auto pixelColor = image->pixelColor(i, j);
                    auto currentValue = Utils::brightnessOfPixelColor(pixelColor);
                    if (currentValue > localMax) {
                        localMax = currentValue;
                    }
                }
            }
            {
                spin_mutex::scoped_lock lock(mutex);
                if (localMax>maxBright) {
                    maxBright = localMax;
                }
            }
        });
        
        cout<<"max result: " << maxBright<<endl;
        return continue_msg();
    });
    
    make_edge(readNode, l);
    make_edge(l, maxBrightnessNode);
    make_edge(maxBrightnessNode, l.decrement);
    
    readNode.activate();
    g.wait_for_all();
    cout<<"wait stopped"<<endl;
    
    return Output();
}
