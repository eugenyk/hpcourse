//
// Created by maksim on 29.11.16.
//

#ifndef MULTITHREADINGLAB1_IMAGEAVERAGEBRIGHT_H
#define MULTITHREADINGLAB1_IMAGEAVERAGEBRIGHT_H

#include "Image.h"
#include "stdio.h"
struct AverageBright{

    std::pair<double ,Image*>* operator()(Image* image){
        int countEl=image->sizeN*image->sizeN;
        double sum=0;
        for (int i=0;i<image->sizeM;i++)
            for(int j=0;j<image->sizeN;j++)
                sum+=image->matrix[i][j];
/*/
        printf("Average_Before:\n");
        image->showImage();/*/
        //printf("Average:%0.3f\n",sum/countEl);

        //printf("Average_After:\n");
        //image->showImage();
        return new std::pair<double ,Image*>(sum/countEl,image);
    }

};
#endif //MULTITHREADINGLAB1_IMAGEAVERAGEBRIGHT_H
