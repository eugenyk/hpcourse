//
//  main.cpp
//  ImageProcessing
//
//  Created by Anton Davydov on 26/11/2016.
//  Copyright © 2016 dydus. All rights reserved.
//

#include <iostream>
#include "tbb/tbb.h"
#include "ImageMagick-6/Magick++.h"

using namespace std;
using namespace tbb;
using namespace tbb::flow;
using namespace Magick;

void run_example3();


int main(int argc, const char * argv[]) {
    auto image = new Image();
    
    try {
        image->read("1.png");
        cout<<image->rows()<<endl;
        cout<<image->pixelColor(10, 10).redQuantum()<<endl;
    } catch (Exception &e)  {
        cout<<&e<<endl;
    }
    cout << "Hello, World!\n";
    
    return 0;
}

void run_example3() {
    int sum = 0;
    graph g;
    function_node< int, int > squarer( g, unlimited, [](const int &v) {
        return v*v;
    } );
    function_node< int, int > cuber( g, unlimited, [](const int &v) {
        return v*v*v;
    } );
    function_node< int, int > summer( g, 1, [&](const int &v ) -> int {
        return sum += v;
    } );
    make_edge( squarer, summer );
    make_edge( cuber, summer );
    broadcast_node<int> br(g);
    make_edge(br, squarer);
    make_edge(br, cuber);
    
    for ( int i = 1; i <= 10; ++i ) {
        br.try_put(i);
    }
    g.wait_for_all();
    
    cout << "Sum is " << sum << "\n";
}
