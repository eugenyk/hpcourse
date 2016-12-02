//
// Created by maksim on 29.11.16.
//

#ifndef MULTITHREADINGLAB1_IMAGEINVERSION_H
#define MULTITHREADINGLAB1_IMAGEINVERSION_H


#include "Image.h"

struct ImageInversion{
    Image* operator()(Image* image){
        Image *imageInverse=new Image(image->id,image->sizeM,image->sizeN);
       // printf("Inversion_Before %p %p:\n",image,imageInverse);
        for (int i=0;i<imageInverse->sizeM;i++)
            for(int j=0;j<imageInverse->sizeN;j++)
                imageInverse->matrix[i][j]=255-image->matrix[i][j];
        printf("Inversion:\n");
        imageInverse->showImage();
        return imageInverse;
    }
};



#endif //MULTITHREADINGLAB1_IMAGEINVERSION_H
