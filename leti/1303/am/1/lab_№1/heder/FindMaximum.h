//
// Created by maksim on 29.11.16.
//

#ifndef MULTITHREADINGLAB1_FINDMAIMUM_H
#define MULTITHREADINGLAB1_FINDMAIMUM_H

#include "Image.h"

#include <utility>

typedef std::pair<unsigned int,Image*> FindElementAndImagePair;
struct FindMaximum{
    FindElementAndImagePair* operator()(Image *image) {
        int i, j;
        unsigned int max = image->matrix[0][0];
        for (i = 0; max != 255 && i < image->sizeM; ++i) {
            for (j = 0; max != 255 && j < image->sizeN; j++) {
                if (image->matrix[i][j] > max) {
                    max = image->matrix[i][j];
                }
            }
        }
       // printf("max =%d %p\n", max,image);
        //image->showImage();
        return new FindElementAndImagePair(max, image);
    }
};

#endif //MULTITHREADINGLAB1_FINDMAIMUM_H
