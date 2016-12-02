//
// Created by maksim on 29.11.16.
//

#ifndef MULTITHREADINGLAB1_IMAGESAVE_H
#define MULTITHREADINGLAB1_IMAGESAVE_H

#include <iomanip>
#include "Image.h"
#include "fstream"
struct ImageSave{
    std::ofstream &out;
    ImageSave (std::ofstream &_out):out(_out){}
    Image* operator()(std::pair<double ,Image*>* _pair){
        double average=_pair->first;
        Image *image =_pair->second;
     
        for (int i=0;i<image->sizeM;i++) {
            for (int j = 0; j < image->sizeN; j++)
                out <<"["<<image->matrix[i][j]<<"]\t";
            out<<std::endl;
        }
        out<<"__________Average:"<<average<<std::endl;
        
       // printf("Save\n");
        //image->showImage();
        return image;
    }
};
#endif //MULTITHREADINGLAB1_IMAGESAVE_H
