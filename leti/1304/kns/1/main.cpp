#include <iostream>
#include "tbb/task_group.h"
#include "tbb/tbb.h"
#include "utils.h"
#include "flow.h"
#include <Magick++.h>

using namespace tbb;
using namespace std;
using namespace tbb::flow;


int main(int argc, const char * argv[]) {
    try {
        CommandLineInput args;
        vector<string> images;
        images.push_back("/home/kodoo/git/tbb_lab/in/YurijGagarin.jpg");
        args.images = images;
        args.output = "/home/kodoo/git/tbb_lab/out/";
        args.inputBrightness = 100;
        startFlow(args);
    } catch (string ex) {
        cerr<<ex<<endl;
    }
    return 0;
}