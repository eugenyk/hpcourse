//
// Created by kodoo on 27.12.16.
//
#include "flow.h"
#ifndef TBB_LAB_INVERSENODE_H
#define TBB_LAB_INVERSENODE_H
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
                    Utils::inverseBrightnessOfPixel(pixels, i * columns + j);
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
#endif //TBB_LAB_INVERSENODE_H
