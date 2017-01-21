#ifndef CIMAGE_H
#define CIMAGE_H

#include "stdio.h"
#include "stdlib.h"


struct CImage
{
	
	unsigned int M;
	unsigned int N;
	unsigned int ** matrix;
	
	CImage(unsigned int sizeM,unsigned int sizeN)
	{
        M= sizeM;
        N= sizeN;
        matrix=new unsigned int*[sizeM];
        for (unsigned int i=0;i<sizeM;i++)
            matrix[i] = new unsigned int[sizeN];
    }


	CImage(unsigned int sizeM,unsigned int sizeN, unsigned int **mtx)
	{
        M= sizeM;
        N= sizeN;
        matrix= mtx;
    }
	
	void show()
	{
		for(unsigned int i=0; i<M; i++ )
		{
			for(unsigned int j=0; j<N; j++)
			{
				printf(" [%d]",matrix[i][j]);
			}
			printf("\n");
		}
	}
	
	~CImage()
	{
        for (unsigned int i = 0; i < M; i++)
            delete matrix[i];
        delete matrix;
    }


};

#endif
