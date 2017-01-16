//
// Created by kodoo on 27.12.16.
//

#include "lightNode.h"
#include "flow.h"

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
            cout << "Average brightness of \"" << Utils::imageName(im) << "\" = " << (int)result << endl;

        return im;
    }
};