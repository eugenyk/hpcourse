//
// Created by maksim on 29.11.16.
//

#ifndef MULTITHREADINGLAB1_FINDVECTORPAIRINDEXES_H
#define MULTITHREADINGLAB1_FINDVECTORPAIRINDEXES_H

#include <vector>
#include "Image.h"

typedef std::pair<unsigned int ,unsigned int> FindElementIndexPair;
typedef std::pair<unsigned int,Image*> FindElementAndImagePair;

struct FindVectorPairIndexes{
    std::pair< int, std::vector<FindElementIndexPair >* >* operator()(FindElementAndImagePair* _pair){
      //  printf("FindVector_Begin:%d %p\n",_pair->first,_pair->second);
       // _pair->second->showImage();
        unsigned int findBright=_pair->first;
        Image *image=_pair->second;
        std::vector<FindElementIndexPair >*indexVector=
                new std::vector<std::pair<unsigned int ,unsigned int> >;
        unsigned int **matrix=image->matrix;
        for (int i = 0; i < image->sizeM; ++i) {
            for (int j=0;j<image->sizeN;++j){
                if(matrix[i][j]==findBright)indexVector->emplace_back(i,j);
            }
        }
        /*/
        for(auto it=indexVector->begin();it!=indexVector->end();it++){
            printf("Ind_Find:[%d][%d] ",(*it).first,(*it).second);
        }/*/
        //printf("FindVector_End:%p\n",*indexVector);
        delete _pair;
        //return indexVector;
        return new std::pair<int,std::vector<FindElementIndexPair >*>(image->id,indexVector);
    }
};


#endif //MULTITHREADINGLAB1_FINDVECTORPAIRINDEXES_H
