#ifndef CINVERSION_H
#define CINVERSION_H


#include "CImage.h"


struct CInversion
{
	
	CImage * operator()( CImage * img)
	{
		
		
		unsigned int **mtx= new unsigned int*[img->M];
		
		for(unsigned int i=0; i<img->M; i++)
		{
			mtx[i]=new unsigned int [img->N]; 
			for(unsigned int j=0; j<img->N; j++)
			{
				srand(time(NULL));
				mtx[i][j]= 255- img->matrix[i][j] ;
			}
		}
		
		return new CImage(img->M, img->N, mtx);
	}


};
#endif