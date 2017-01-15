#ifndef MAXIMUM_H
#define MAXIMUM_H

#include  <utility>
#include "CImage.h"

using namespace std;

typedef pair< unsigned int, CImage *> FoundElement;

struct Maximum {
	
	
	 FoundElement* operator()(CImage * img)
	 {
		unsigned int m= img->matrix[0][0];
		//int p, q;

		for(unsigned int i=0; i<img->M; i++)
		{
			for(unsigned int j=0; j<img->N; j++)
			{
				if(img->matrix[i][j]> m) 
				{
					m=img->matrix[i][j];
					//p= i;
					//q= j;
				}
			}
		}
		 
		 
		 return  new FoundElement(m, img);
		 
	 }
	
};
#endif
