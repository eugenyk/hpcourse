#include <tbb/flow_graph.h>
#include <fstream>
#include <iostream>

#include "heder/Image.h"
#include "heder/ImageSave.h"
#include "heder/ImageInversion.h"
#include "heder/ImageAverageBright.h"
#include "heder/FindVectorPairIndexes.h"
#include "heder/FindMaximum.h"
#include "heder/FindMinimum.h"
#include "heder/SetBorder.h"

#include <regex>
#include "heder/ImageGenerator.h"

using namespace tbb::flow;
using namespace std;
const unsigned int SIZE_M=2;
const unsigned int SIZE_N=2;

typedef pair<unsigned int ,unsigned int> ElementIndexPair;
typedef pair<unsigned int,Image*> FindElementAndImagePair;

//typedef pair< int, vector<FindElementIndexPair >* >* ImageIdAndFindElementAndImagePair

int main(int argc, char* argv[]){
    regex re("[0-9]*");
    int userBright=-1;
    int imageLimit=-1;
///Добавил параметр количества изображений
    int imageCount=-1;
    int userHeight=SIZE_M;
    int userWidth=SIZE_N;
    string pathToLogFile;
    bool isErrorInCommandLine=false;
    int rez=0;
    while (!isErrorInCommandLine&& (rez = getopt(argc,argv,"b:l:h:w:c:f:")) != -1){
        switch (rez){
            case 'b':
                if ( regex_match(optarg,re) ) {
                    printf("found option b and argument %s.\n",optarg);
                    userBright=atoi(optarg);
                }else {
                isErrorInCommandLine=true;
                    printf("option b don't have number argument - %s.\n",optarg);
                }
                break;
            case 'l':
                if ( regex_match(optarg,re) ){
                    printf("found option l and argument %s.\n",optarg);
                    imageLimit=atoi(optarg);
                    }else {
                    isErrorInCommandLine=true;
                    printf("option l don't have number argument - %s.\n",optarg);
                }
                break;
            case 'f':
                printf("found option f and argument %s.\n",optarg);
                pathToLogFile=optarg;
                break;
            case 'w':
                if ( regex_match(optarg,re) ){
                    printf("found option w and argument %s.\n",optarg);
                    userWidth=atoi(optarg);
                }else {
                    isErrorInCommandLine=true;
                    printf("option w don't have number argument - %s.\n",optarg);
                }
                break;
            case 'h':
                if ( regex_match(optarg,re) ){
                    printf("found option h and argument %s.\n",optarg);
                    userHeight=atoi(optarg);
                }else {
                    isErrorInCommandLine=true;
                    printf("option h don't have number argument - %s.\n",optarg);
                }
                break;
            case 'c':
                if ( regex_match(optarg,re) ){
                    printf("found option n and argument %s.\n",optarg);
                    imageCount=atoi(optarg);
                }else {
                    isErrorInCommandLine=true;
                    printf("option l don't have number argument - %s.\n",optarg);
                }
                break;
            case '?': isErrorInCommandLine=true;break;
        };
    }
    printf("ImLimit:%d\n",imageLimit);
    printf("UserBright:%d\n",userBright);
    printf("UserHeight:%d\n",userHeight);
    printf("UserWidth:%d\n",userWidth);
    printf("ImCount:%d\n",imageCount);
    printf("PathLogFile:%s\n",pathToLogFile.data());
    if (isErrorInCommandLine) return 0;
    ///-------------
     ofstream out;
     out.open(pathToLogFile,ios_base::out);
    /// Lambda
    function<FindElementAndImagePair*(Image*)> getUserBright=[&userBright](Image *image)
            -> FindElementAndImagePair* {
       // printf("userBright :=%d %p\n",userBright,image);
        return new FindElementAndImagePair(userBright,image);
    };
    function<int(pair< int, vector<FindElementIndexPair >* >*)> getIdImageFromPair=
    [](pair< int, vector<FindElementIndexPair >* >* _pair)->int { return _pair->first;};
    function<int(Image * )> getImageId=
    [](Image* image)->int { return image->id;};

    function<void (tuple<Image*,Image*>)> removeImage=[](tuple<Image*,Image*> _tuple)
    {
        Image* originalImage= get<0>(_tuple);
        Image* inversionImage= get<1>(_tuple);
        //printf("Del originalImage %p\n",originalImage);
        //printf("Del inversionImage %p\n",inversionImage);
        delete  originalImage;
        delete  inversionImage;
    };
    ///Nodes
    printf("Start\n");
    graph imageGraph;

    limiter_node<Image*> limiterImageNode (imageGraph,imageLimit);
    buffer_node<Image *>bufferImageNode(imageGraph);
    broadcast_node<Image*>broadcastImageNode(imageGraph);

    function_node<Image*,FindElementAndImagePair* > maxBrightNode(imageGraph,unlimited,FindMaximum());
    function_node<Image*,FindElementAndImagePair* > minBrightNode(imageGraph,unlimited,FindMinimum());
    function_node<Image*,FindElementAndImagePair* > userBrightNode(imageGraph,unlimited,getUserBright);

    queue_node<Image *>queueImageNode(imageGraph);

    function_node<FindElementAndImagePair*,pair< int, vector<FindElementIndexPair >* >* > vectorMaxBrightIndexesNode(imageGraph,unlimited,FindVectorPairIndexes());
    function_node<FindElementAndImagePair*,pair< int, vector<FindElementIndexPair >* >* > vectorMinBrightIndexesNode(imageGraph,unlimited,FindVectorPairIndexes());
    function_node<FindElementAndImagePair*,pair< int, vector<FindElementIndexPair >* >* > vectorSetBrightIndexesNode(imageGraph,unlimited,FindVectorPairIndexes());


    queue_node<
            pair< int, vector<FindElementIndexPair >* >* > queueIndexMaxBrightPairVectorNode(imageGraph);
    queue_node<
            pair< int, vector<FindElementIndexPair >* >*> queueIndexMinBrightPairVectorNode(imageGraph);
    queue_node<
            pair< int, vector<FindElementIndexPair >* >*> queueIndexUserBrightPairVectorNode(imageGraph);

    join_node< tuple<
            pair< int, vector<FindElementIndexPair >* >*,
            pair< int, vector<FindElementIndexPair >* >*,
            pair< int, vector<FindElementIndexPair >* >*,
            Image *
    > ,key_matching<int > > joinNode(imageGraph,getIdImageFromPair,getIdImageFromPair,getIdImageFromPair,getImageId);

    function_node<tuple<
            std::pair< int, std::vector<FindElementIndexPair >* >*,
            std::pair< int, std::vector<FindElementIndexPair >* >*,
            std::pair< int, std::vector<FindElementIndexPair >* >*,
            Image *
    > ,Image* > imageTransformNode( imageGraph,serial, SetBorder() );


    broadcast_node<Image* >  imageTransformBroadcastNode(imageGraph);

    function_node<Image*,Image*> imageInversionNode(imageGraph,unlimited,ImageInversion());

    function_node<Image*,pair<double ,Image*>*> imageAverageBrightNode(imageGraph,unlimited,AverageBright());

    function_node<std::pair<double ,Image*>*,Image*> imageSaverNode(imageGraph,serial,ImageSave(out));

    join_node< tuple<Image*,Image*>,key_matching<int > > joinRemoveNode(imageGraph,getImageId,getImageId);

    function_node<tuple<Image*,Image*> > imageRemoveNode(imageGraph,serial,removeImage);
    //function_node<Image* > imageRemoveNode(imageGraph,serial,removeImage);
   // make_edge(limiterImageNode,queueImageMainNode);
    //make_edge(queueImageMainNode,broadcastImageNode);

     make_edge(limiterImageNode,bufferImageNode);
     make_edge(bufferImageNode,broadcastImageNode);
///2
    make_edge( broadcastImageNode, maxBrightNode );
    make_edge( broadcastImageNode, minBrightNode );
    make_edge( broadcastImageNode, userBrightNode );
    make_edge( broadcastImageNode, queueImageNode );
///3
    make_edge( maxBrightNode, vectorMaxBrightIndexesNode );
    make_edge( minBrightNode, vectorMinBrightIndexesNode );
    make_edge( userBrightNode,vectorSetBrightIndexesNode );

    make_edge( vectorMaxBrightIndexesNode ,queueIndexMaxBrightPairVectorNode);
    make_edge( vectorMinBrightIndexesNode ,queueIndexMinBrightPairVectorNode);
    make_edge( vectorSetBrightIndexesNode ,queueIndexUserBrightPairVectorNode);
///3
    make_edge( queueIndexMaxBrightPairVectorNode, input_port<0>(joinNode) );
    make_edge( queueIndexMinBrightPairVectorNode, input_port<1>(joinNode) );
    make_edge( queueIndexUserBrightPairVectorNode, input_port<2>(joinNode) );
    make_edge( queueImageNode, input_port<3>(joinNode) );

    make_edge( joinNode, imageTransformNode );

    ///4
    make_edge( imageTransformNode,imageTransformBroadcastNode );
    make_edge( imageTransformBroadcastNode, imageInversionNode);
    make_edge( imageTransformBroadcastNode, imageAverageBrightNode);

    ///5
    make_edge( imageAverageBrightNode, imageSaverNode);
    ///6 -Remove
    make_edge( imageSaverNode, input_port<0>(joinRemoveNode) );
    make_edge( imageInversionNode, input_port<1>(joinRemoveNode) );

    make_edge( joinRemoveNode,imageRemoveNode );

    ///Добавил декремент
    make_edge( imageRemoveNode,limiterImageNode.decrement );

    ImageGenerator imageGenerator(userHeight,userWidth);

    ///Снял ограничения с входного потока
     for (int i=0;i<imageCount; i++)
     {
         Image *image= imageGenerator.getImage(i);
         image->showImage();
         printf("Im__________%p\n",image);

    ///Добавил цикл ожидания пока количество изображений в графе не станет
    ///менеше imageLimit(те не сработает декремент)
        while(!limiterImageNode.try_put(image));
    }
    imageGraph.wait_for_all();
    out.close();
    return 0;

}
