#ifndef CSAVEAVERAGE_H
#define CSAVEAVERAGE_H

#include "CImage.h"
#include <iostream>
#include <fstream>
using namespace std;

struct CSaveAverage
{
	ofstream &myfile;
	CSaveAverage (ofstream & of):myfile(of)
	{
		
	}

	

	CImage * operator() (std::pair<double ,CImage *>* p)
	{
		CImage * img= p->second;
		
		
		myfile<< "Average: "<<p->first<<"\n";
		
		for(unsigned int i=0; i<p->second->M; i++)
		{
			for(unsigned int j=0; j<p->second->N; j++)
			{
				myfile<< p->second->matrix[i][j]<<"  \t";
			}
			
			myfile<<"\n";
		}
		
		
		return img;
	}

		
};

#endif
