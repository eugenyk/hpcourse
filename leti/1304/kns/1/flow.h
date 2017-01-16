//
//  flow.h
//  ImageProcessing
//
//  Created by Anton Davydov on 17/12/2016.
//
//

#ifndef flow_h
#define flow_h
#include "utils.h"
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

#include "inverseNode.h"
#include "lightNode.h"
#include "averageNode.h"

void startFlow(CommandLineInput input);

#endif /* flow_h */