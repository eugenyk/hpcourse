//
// Created by kodoo on 27.12.16.
//

#ifndef TBB_LAB_BRIGHTNODE_H
#define TBB_LAB_BRIGHTNODE_H

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
            cout << "Pixels with brightness " << (int)result << " for image \"" << Utils::imageName(im) << "\"" << endl;

        return pixels;
    }
};

#endif //TBB_LAB_BRIGHTNODE_H
