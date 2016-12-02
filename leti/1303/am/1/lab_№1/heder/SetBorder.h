//
// Created by maksim on 29.11.16.
//

#ifndef MULTITHREADINGLAB1_SETBORDER_H
#define MULTITHREADINGLAB1_SETBORDER_H

#include <tuple>
#include "Image.h"
#include "vector"
#include "utility"
#include "tuple"

typedef std::pair<unsigned int ,unsigned int> ElementIndexPair;

struct SetBorder {
    Image* operator()(const std::tuple<
            std::pair< int, std::vector<FindElementIndexPair >* >*,
            std::pair< int, std::vector<FindElementIndexPair >* >*,
            std::pair< int, std::vector<FindElementIndexPair >* >*,
            Image *
    > tuple1) {
        std::vector<ElementIndexPair >* vectorMax=(std::get<0>(tuple1))->second;
        std::vector<ElementIndexPair >* vectorMin=(std::get<1>(tuple1))->second;
        std::vector<ElementIndexPair >* vectorSet=(std::get<2>(tuple1))->second;

       // printf("***BEGIN\n");
        //printf("Key1:%d Key2:%d Key3:%d Key4:%d",std::get<0>(tuple1)->first,std::get<1>(tuple1)->first,
          //     std::get<2>(tuple1)->first,std::get<3>(tuple1)->id);

        Image * image=(std::get<3>(tuple1));
   //     printf("Transform_Image_Before%p %p\n",image,*vectorMax);
     //   image->showImage();
/*/
        for(auto it=vectorMax->begin();it!=vectorMax->end();it++){
            printf("Ind_Max:[%d][%d] ",(*it).first,(*it).second);
        }
        printf("\n");
        for(auto it=vectorMin->begin();it!=vectorMin->end();it++){
            printf("Ind_Min:[%d][%d] ",(*it).first,(*it).second);
        }
        printf("\n");
        for(auto it=vectorSet->begin();it!=vectorSet->end();it++){
            printf("Ind_Set:[%d][%d] ",(*it).first,(*it).second);
        }
        printf("\n");
/*/
        setImage(image,vectorMax,255);
        setImage(image,vectorMin,0);
        if (vectorSet->size()>0)
            setImage(image,vectorSet,128);
        //printf("Transform_Image_After\n");
        //image->showImage();
        //printf("***END\n");
        //image->showImage();

        delete vectorMax;
        delete vectorMin;
        delete vectorSet;

        return image;
    }

    void setImage(Image*image,std::vector<ElementIndexPair >*indexVector, unsigned int setNumber){
        unsigned int **matrix=image->matrix;

        int rowIndex;
        int columnIndex;

        for(ElementIndexPair _pair:*indexVector){
            rowIndex=_pair.first;
            columnIndex=_pair.second;

            if(rowIndex-1>=0)matrix[rowIndex-1][columnIndex]=setNumber;
            if(rowIndex+1<image->sizeM)matrix[rowIndex+1][columnIndex]=setNumber;

            if(columnIndex-1>=0)matrix[rowIndex][columnIndex-1]=setNumber;
            if(columnIndex+1<image->sizeN)matrix[rowIndex][columnIndex+1]=setNumber;

            if(columnIndex-1>=0&&rowIndex-1>=0)matrix[rowIndex-1][columnIndex-1]=setNumber;
            if(columnIndex-1>=0&&rowIndex+1<image->sizeM)matrix[rowIndex+1][columnIndex-1]=setNumber;

            if(columnIndex+1<image->sizeN&&rowIndex-1>=0)matrix[rowIndex-1][columnIndex+1]=setNumber;
            if(columnIndex+1<image->sizeN&&rowIndex+1<image->sizeM)matrix[rowIndex+1][columnIndex+1]=setNumber;
        }
    }

};


#endif //MULTITHREADINGLAB1_SETBORDER_H
