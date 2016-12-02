//
// Created by maksim on 29.11.16.
//

#ifndef MULTITHREADINGLAB1_FINDMINIMUM_H
#define MULTITHREADINGLAB1_FINDMINIMUM_H


#include "Image.h"
#include "utility"
typedef std::pair<unsigned int,Image*> FindElementAndImagePair;
struct FindMinimum{

    FindElementAndImagePair* operator()(Image *image) {
        int i,j;
        unsigned int min=image->matrix[0][0];
        for ( i = 0;min!=0&& i <image->sizeM; ++i) {
            for (j = 0;min!=0&& j < image->sizeN; j++) {
                if (image->matrix[i][j]<min){
                    min=image->matrix[i][j];
                }
            }
        }
       // printf("min=%d %p\n",min,image);
        //image->showImage();
        return new FindElementAndImagePair(min,image);
    }


};

#endif //MULTITHREADINGLAB1_FINDMINIMUM_H
