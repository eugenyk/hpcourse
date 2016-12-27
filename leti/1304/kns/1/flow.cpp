//
//  flow.cpp
//  ImageProcessing
//
//  Created by Anton Davydov on 17/12/2016.
//
//

#include "flow.h"
#include "lightNode.h"
#include "brightNode.h"


typedef tuple<Image*, PixelsType,PixelsType,PixelsType> HighlightInputType;

void startFlow(CommandLineInput input) {

    if (input.verbose)
        cout<<"Start"<<endl;
    graph g;
    auto sourceIter = input.images.begin();

    BrightnessType maxBright = 0;
    BrightnessType minBright = 255;

    source_node<Image*> readNode( g, [&]( Image* &v ) -> bool {
        while (sourceIter != input.images.end()) {
            try {
                auto image = new Image();
                auto fileName = *sourceIter;
                image->read(fileName);
                v = image;
                sourceIter++;

                if (input.verbose)
                    cout<<"Image " + Utils::imageName(image) + " has been read from input dir" <<endl;

                return true;
            } catch (Exception &e)  {
                sourceIter++;
            }
        }
        return false;
    });

    limiter_node<Image*> l(g, input.limit);

    auto maxBrigNode = BrightnessNode(input, maxBright,
                                      [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                          if (l > r) { r = l; ps.clear(); ps.push_back(p); }
                                          else if (l == r) { ps.push_back(p); }
                                      },
                                      [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                          if (l > r) { r = l; r_p.clear(); r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                          else if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                      });
    auto minBrigNode = BrightnessNode(input, minBright,
                                      [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                          if (l < r) { r = l; ps.clear(); ps.push_back(p); }
                                          else if (l == r) { ps.push_back(p); }
                                      },
                                      [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                          if (l < r) { r = l; r_p.clear(); r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                          else if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                      });

    auto brigByGivenNode = BrightnessNode(input, input.inputBrightness,
                                          [](BrightnessType& l, BrightnessType& r, PixelType p, PixelsType& ps) {
                                              if (l == r) { ps.push_back(p); }
                                          },
                                          [](BrightnessType& l, BrightnessType& r, PixelsType& l_p, PixelsType& r_p) {
                                              if (l == r) { r_p.insert(end(r_p), begin(l_p), end(l_p)); }
                                          });
    function_node<Image*, PixelsType> maxBrightnessNode( g, 1, maxBrigNode);
    function_node<Image*, PixelsType> minBrightnessNode( g, 1, minBrigNode);
    function_node<Image*, PixelsType> givenBrightnessNode( g, 1, brigByGivenNode);
    function_node<HighlightInputType, Image*> highlightNode( g, 1, HighlightNode(input));
    function_node<Image*, Image*> inverseNode( g, 1, InverseBrightnessNode(input));
    function_node<Image*, Image*> averageNode( g, 1, AverageBrightnessNode(input));
    function_node<tuple<Image*,Image*>, continue_msg> garbageCollectorNode( g, 1, [](tuple<Image*,Image*> im){ delete get<0>(im); });
    join_node<tuple<Image*, Image*>> joinNode( g);

    broadcast_node<Image*> broabcastImage( g );
    join_node<HighlightInputType> j(g);

    make_edge(readNode, l);
    make_edge(l, maxBrightnessNode);
    make_edge(l, minBrightnessNode);
    make_edge(l, givenBrightnessNode);
    make_edge(l, input_port<0>(j));
    make_edge(maxBrightnessNode, input_port<1>(j));
    make_edge(minBrightnessNode, input_port<2>(j));
    make_edge(givenBrightnessNode, input_port<3>(j));
    make_edge(j, highlightNode);
    make_edge(highlightNode, broabcastImage);
    make_edge(broabcastImage, inverseNode);
    make_edge(broabcastImage, averageNode);
    make_edge(inverseNode, input_port<0>(joinNode));
    make_edge(averageNode, input_port<1>(joinNode));
    make_edge(joinNode, garbageCollectorNode);
    make_edge(garbageCollectorNode, l.decrement);

    readNode.activate();

    g.wait_for_all();
    if (input.verbose)
        cout<<"Finish"<<endl;
}