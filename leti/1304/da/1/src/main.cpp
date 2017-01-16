//
//  main.cpp
//  ImageProcessing
//
//  Created by Anton Davydov on 26/11/2016.
//  Copyright © 2016 dydus. All rights reserved.
//

#include <iostream>
#include "flow.h"
using namespace std;

int main(int argc, const char * argv[]) {
    try {
        auto input = Utils::parseArgs(argc, argv);
        startFlow(input);
    } catch (string ex) {
        cerr<<ex<<endl;
    }
    return 0;
}
