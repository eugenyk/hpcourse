//
// Created by maksim on 30.11.16.
//

#ifndef MULTITHREADINGLAB1_IMAGEGENERATOR_H
#define MULTITHREADINGLAB1_IMAGEGENERATOR_H

#include <cstdlib>
#include "Image.h"

struct ImageGenerator{
    int sizeM;
    int sizeN;
    ImageGenerator(int _sizeM,int _sizeN){
        sizeM=_sizeM;
        sizeN=_sizeN;
    }
    Image *getImage(int id){
        unsigned int **matrix=new unsigned int*[sizeM];
        for (int i=0;i<sizeM;i++) {
            matrix[i] = new unsigned int[sizeN];
            for (int j=0;j<sizeN;j++)
                matrix[i][j]=rand()%256;
        }
        return new Image(id,matrix,sizeM,sizeN);
    }
};
#endif //MULTITHREADINGLAB1_IMAGEGENERATOR_H
