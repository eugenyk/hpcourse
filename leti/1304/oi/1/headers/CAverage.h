#ifndef CAVERAGE_H
#define CAVERAGE_H


#include "CImage.h"

struct CAverage
{

  std::pair<double, CImage *> * operator() (CImage *img)
  {
	  double sum=0;
	  double size= img->M * img->N;
	  
	  for(unsigned int i=0; i< img->M; i++)
	  {
		  for(unsigned int j=0; j< img->M; j++)
		  {
			  sum+= img->matrix[i][j];
			  
		  }
		  
	  }
	  
	  return new  std::pair<double, CImage *> (sum/size, img);
  }

};
#endif