//
// Created by maksim on 29.11.16.
//

#ifndef MULTITHREADINGLAB1_IMAGE_H
#define MULTITHREADINGLAB1_IMAGE_H

#include "stdio.h"
struct Image{
    int id;
    unsigned int ** matrix;
    int sizeM;
    int sizeN;
    Image(int _id,unsigned int ** _matrix,int _sizeM,int _sizeN){
        id=_id;
        matrix=_matrix;
        sizeM=_sizeM;
        sizeN=_sizeN;
    }

    Image(int _id,int _sizeM,int _sizeN){
        id=_id;
        sizeM=_sizeM;
        sizeN=_sizeN;
        matrix=new unsigned int*[sizeM];
        for (int i=0;i<sizeM;i++)
            matrix[i] = new unsigned int[sizeN];
    }

    void showImage(){
        for (int i=0;i<sizeM;i++) {
            for (int j=0;j<sizeN;j++)
                printf("Im :[%d][%d]=%-5d ",i,j,matrix[i][j]);
            printf("\n");
        }
    }

    ~Image(){
        for (int i = 0; i < sizeM; i++)
            delete []matrix[i];
        delete []matrix;
    }
};

#endif //MULTITHREADINGLAB1_IMAGE_H
