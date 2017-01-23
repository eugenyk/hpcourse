#ifndef CIMAGEGEN_H
#define CIMAGEGEN_H



#include "CImage.h"

struct CImageGen
{
	unsigned int M;
	unsigned int N;
	
	CImageGen( unsigned int M1 , unsigned int N1):M(M1), N(N1)
	{
		
	}
	
	
	CImage * generate()
	{
		unsigned int **mtx= new unsigned int*[M];
		
		for(unsigned int i=0; i<M; i++)
		{
			mtx[i]=new unsigned int [N]; 
			for(unsigned int j=0; j<N; j++)
			{
				
				mtx[i][j]= rand()%256;
			}
		}
		
		
		return new CImage(M, N, mtx);
	}
	
};
#endif
