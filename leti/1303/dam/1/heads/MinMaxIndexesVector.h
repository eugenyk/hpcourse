#ifndef MINMAXINDEXESVECTOR_H
#define MINMAXINDEXESVECTOR_H

#include "CImage.h"



using namespace std;

typedef pair< unsigned int, CImage *> FoundElement;
typedef pair<unsigned int, unsigned int> Coords;

struct MinMaxIndexesVector
{
	
	
	std::pair<CImage* , std::vector<Coords>  *> * operator()(FoundElement *fE)
	{
		
		unsigned int searchE= fE->first; 
		CImage * img1 = fE->second;
		vector<Coords>  *v= new vector< std::pair<unsigned int, unsigned int> >();
		
		for(unsigned int i; i< img1->M; i++ )
		{
			for(unsigned int j; j< img1->N; j++ )
			{
				if(searchE==img1->matrix[i][j])
				{
					v->emplace_back(i,j);
				}
				
			}
		}
		
		
		return  new std::pair<CImage* , std::vector<Coords>  *>(img1, v);
	}
	
};

#endif
