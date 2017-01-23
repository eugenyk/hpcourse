#ifndef SETFRAME_H
#define SETFRAME_H


#include <tuple>
#include <vector>
#include <utility>
#include "CImage.h"

using namespace std;

typedef pair<unsigned int, unsigned int> Coords;

struct SetFrame
{
	
	CImage * operator() ( tuple< std::pair<CImage* , std::vector<Coords>  *> *, 
								 std::pair<CImage* , std::vector<Coords>  *> *, 
								 std::pair<CImage* , std::vector<Coords>  *> * >  tp   )
	{
		
		std::vector<Coords>* Max_vector=(std::get<0>(tp))->second;
        std::vector<Coords >* Min_vector=(std::get<1>(tp))->second;
		std::vector<Coords >* Search_vector=(std::get<2>(tp))->second;
        CImage * img1=(std::get<0>(tp))->first;
		
		
		make_frame(255, img1, Max_vector);
		make_frame(0, img1, Min_vector);
		if(Search_vector->size()>0) make_frame(128, img1, Search_vector);
		
		
		return img1;
	}
	
	
	void make_frame(unsigned int value, CImage * img, vector<Coords>  *vC )
	{
		unsigned int r;
		unsigned int c;
		
		for(Coords coord: *vC  )
		{
			r= coord.first;
			c= coord.second;
			
			if(r-1>=0)img->matrix[r-1][c]=value;
            if(r+1<img->M)img->matrix[r+1][c]=value;
			
			if(c-1>=0)img->matrix[r][c-1]=value;
            if(c+1<img->N)img->matrix[r][c+1]=value;
			
			if(c-1>=0&&r-1>=0)img->matrix[r-1][c-1]=value;
            if(c-1>=0&&r+1<img->M)img->matrix[r+1][c-1]=value;

            if(c+1<img->N && r-1>=0)img->matrix[r-1][c+1]=value;
            if(c+1<img->N && r+1<img->M)img->matrix[r+1][c +1]=value;
			
			
		}
		
		
	}
	
};
#endif
