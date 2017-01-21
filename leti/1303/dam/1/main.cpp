#include <tbb/flow_graph.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <regex>

#include "heads/CImage.h"
#include "heads/CImageGen.h"
#include "heads/Maximum.h"
#include "heads/Minimum.h"
#include "heads/MinMaxIndexesVector.h"
#include "heads/SetFrame.h"
#include "heads/CInversion.h"
#include "heads/CAverage.h"
#include "heads/CSaveAverage.h"


using namespace tbb::flow;
using namespace std;


typedef pair< unsigned int, CImage *> FoundElement;
typedef pair<unsigned int, unsigned int> Coords;

static const char *optString = "b:l:f:";

int main(int argc,  char* argv[])
{
	std::string fileName="log.txt";
	unsigned int imgLimit=5;
	unsigned int size_M=8;
	unsigned int size_N=8;
	unsigned int setPixel;
	int opt = 0;
	regex regular("[0-9]*");
	bool correctCmdLine= true;

    opt = getopt( argc, argv, optString );
    while( opt != -1 ) 
	{
        switch( opt ) 
		{
            case 'b':
		
                if( regex_match(optarg , regular))
				{
					if(  (unsigned int)atoi(optarg) <0 || (unsigned int)atoi(optarg) > 255 )
					{
						
						correctCmdLine=false;
					}
					else
					{
						setPixel = (unsigned int)atoi(optarg);
					}
					
				}
				else
				{
					correctCmdLine=false;
				}
                break;
                
            case 'l':
                if(regex_match(optarg , regular))
				{
					if( atoi(optarg) <1 || atoi(optarg) > 1000 )
					{
						correctCmdLine=false;
					}
					else
					{
						imgLimit = (unsigned int) atoi(optarg);
						
					}
					
				}
				else
				{
					correctCmdLine=false;
				} 
                break;
                
            case 'f':
				fileName ="";
                fileName = optarg;
                break;
                
        }
        
        opt = getopt( argc, argv, optString );
    }
	
	
	printf("-b:%d\n",setPixel);
    printf("-l:%d\n",imgLimit);
    printf("-f:%s\n",fileName.data());
	
	if (!correctCmdLine)
	{
		printf("Uncorrect command line parameters\n");
		return 0;
	}
	
//********************************************	
	std::ofstream ofs; 
	ofs.open(fileName, std::ofstream::out);
	graph myGraph;
	
	limiter_node<CImage*> limit_Node (myGraph, imgLimit);
	broadcast_node< CImage*> br_Node( myGraph);
	function_node<CImage*, FoundElement*> maximum_Node( myGraph, unlimited, Maximum() );
	function_node<CImage*, FoundElement*> minimum_Node( myGraph, unlimited, Minimum() );
	function_node<CImage*, FoundElement*> search_Node( myGraph, unlimited, [&setPixel] ( CImage* im1) -> FoundElement* {
		
		return new FoundElement( setPixel,im1);
	} );
	
	function_node<FoundElement *, std::pair<CImage* , std::vector<Coords>  *> *> maximumVector_Node( myGraph, unlimited, MinMaxIndexesVector() );
	function_node<FoundElement *, std::pair<CImage* , std::vector<Coords>  *> *> minimumVector_Node( myGraph, unlimited, MinMaxIndexesVector() );
	function_node<FoundElement *, std::pair<CImage* , std::vector<Coords>  *> *> searchVector_Node( myGraph, unlimited,  MinMaxIndexesVector() );
	
	join_node< tuple <
		std::pair<CImage* , std::vector<Coords>  *> *,
		std::pair<CImage* , std::vector<Coords>  *> *,
		std::pair<CImage* , std::vector<Coords>  *> *
	> > joinNode (myGraph );
	
	function_node< tuple <
		std::pair<CImage* , std::vector<Coords>  *> *,
		std::pair<CImage* , std::vector<Coords>  *> *,
		std::pair<CImage* , std::vector<Coords>  *> *
	> , CImage*> setFrame_Node(myGraph, unlimited,  SetFrame() );  //? serial ???
	
	broadcast_node< CImage*> broadcastSetFrame_Node( myGraph);

	function_node<CImage*, CImage*> inversion_Node(myGraph, unlimited, CInversion() );
	
	function_node<CImage* , std::pair<double, CImage *> *> average_Node(myGraph, unlimited, CAverage() );
	
	function_node<std::pair<double ,CImage *>* > saveAverage_Node(myGraph, unlimited, CSaveAverage(ofs) );
	
	make_edge(limit_Node  , br_Node );
	make_edge( br_Node, maximum_Node );
	make_edge( br_Node, minimum_Node );
	make_edge( br_Node, search_Node );
	make_edge( maximum_Node,  maximumVector_Node);
	make_edge( minimum_Node,  minimumVector_Node);
	make_edge( search_Node,   searchVector_Node);
	make_edge( maximumVector_Node , input_port<0>(joinNode) );
	make_edge( minimumVector_Node , input_port<1>(joinNode) );
	make_edge( searchVector_Node ,  input_port<2>(joinNode) );
	make_edge(joinNode, setFrame_Node); //new
	make_edge(setFrame_Node, broadcastSetFrame_Node);
	make_edge( broadcastSetFrame_Node , inversion_Node );
	make_edge( broadcastSetFrame_Node , average_Node );
	make_edge( average_Node , saveAverage_Node );
	
		
    CImageGen  Img_Generator(size_M, size_N );
    for (unsigned int i=0;i<imgLimit; i++)
    {
        CImage *img=  Img_Generator.generate() ;
	img->show();
        limit_Node.try_put(img);
		
    }
	
	myGraph.wait_for_all();
	ofs.close();
	return 0;
}
