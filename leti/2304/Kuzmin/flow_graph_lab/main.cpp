#include <tbb/flow_graph.h>
#include <CImg.h>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>

#define DEBUG

using namespace tbb::flow;
using namespace std;
using namespace cimg_library;

struct Dot {
    int x,y;
};

typedef CImg<unsigned char> Cimg;
typedef tuple<Cimg, string> ImgTuple; //image + name
typedef tuple<ImgTuple, vector<Dot>, vector<Dot>, vector<Dot>, int, int> ImgDotsTuple; //image + max, min, custom dots + max val, min val

int pink[] = {255, 0, 255}, green[] = {68, 255, 0}, orange[] = { 255, 170, 0 };

string intens_file_name;
int custom_intens, threads;
ifstream imgs_file;
ofstream intens_file;

string imgs_file_name = "list.txt";
string results_dir = "results";
string imgs_dir = "imgs";


//source_node
struct load_imgs {
  bool operator()(ImgTuple& output) {
    if (!imgs_file.is_open()) imgs_file.open(imgs_dir+"/"+imgs_file_name);
    string img_name;
    if (imgs_file >> img_name)
    {
        Cimg data((imgs_dir+"/"+img_name).c_str());
        output = ImgTuple(data, img_name);
#ifdef DEBUG
        cout << img_name << " - image loaded" << endl;
#endif
        return true;
    }
    imgs_file.close();
    return false;
  }
};

//func_node
struct calc_intens {
  ImgDotsTuple operator()(ImgTuple input) {

      Cimg image = get<0>(input);
      vector<Dot> maxs, mins, customs;
      int max_val=0, min_val=255, custom_val=custom_intens;

     for (int x=0; x<image.height(); x++) {
         for (int y=0; y<image.width(); y++){

             int dot_val = (image(x,y,0)+image(x,y,1)+image(x,y,2))/3;

             if (max_val == dot_val) {
                 maxs.push_back({x,y});
             }
             else if (max_val < dot_val) {
                 max_val = dot_val;
                 maxs.clear();;
                 maxs.push_back({x,y});
             }

             if (min_val == dot_val) {
                 mins.push_back({x,y});
             }
             else if (min_val > dot_val) {
                 min_val = dot_val;
                 mins.clear();;
                 mins.push_back({x,y});
             }

             if (custom_val == dot_val) {
                 customs.push_back({x,y});
             }
         }
     }

     ImgDotsTuple result(input, maxs, mins, customs, max_val, min_val);

#ifdef DEBUG
      cout << get<1>(input) << " - max, min pixels found" << endl;
#endif

    return result;
  }
};

//func_node
struct select_dots {
  ImgTuple operator()(ImgDotsTuple input) {

      Cimg image = get<0>(get<0>(input));
      string file_name = get<1>(get<0>(input));
      vector<Dot> maxs = get<1>(input), mins = get<2>(input), customs = get<3>(input);

      for(int i = 0; i < maxs.size(); i++) {
          auto dot = maxs.at(i);
          image(dot.x, dot.y, 0) = pink[0];
          image(dot.x, dot.y, 1) = pink[1];
          image(dot.x, dot.y, 2) = pink[2];
      }

      for(int i = 0; i < mins.size(); i++) {
          auto dot = mins.at(i);
          image(dot.x, dot.y, 0) = green[0];
          image(dot.x, dot.y, 1) = green[1];
          image(dot.x, dot.y, 2) = green[2];
      }

      for(int i = 0; i < customs.size(); i++) {
          auto dot = customs.at(i);
          image(dot.x, dot.y, 0) = orange[0];
          image(dot.x, dot.y, 1) = orange[1];
          image(dot.x, dot.y, 2) = orange[2];
      }

      file_name="intens_"+file_name;

#ifdef DEBUG
      cout << file_name << " - pixels selected on image" << endl;
#endif
    return ImgTuple(image, file_name);
  }
};

//func_node
struct calc_avg_intens {
  bool operator()(ImgDotsTuple input) {

      string img_name = get<1>(get<0>(input));
      vector<Dot> maxs = get<1>(input), mins = get<2>(input), customs = get<3>(input);
      int max_val=get<4>(input), min_val=get<5>(input), custom_val=custom_intens;

      int dots_count = maxs.size() + mins.size() + customs.size();

      int avg = (maxs.size() * max_val + mins.size() * min_val + customs.size() * custom_val) / dots_count;

      if (!intens_file_name.empty()) {
          if (!intens_file.is_open()) intens_file.open(results_dir+"/"+intens_file_name);
          intens_file << img_name << "=" << avg << endl;
      }

#ifdef DEBUG
        cout << img_name << " - average intensity calculated" << endl;
#endif
    return true;
  }
};

//func_node
struct invert_img {
  ImgTuple operator()(ImgTuple input) {

      Cimg image = get<0>(input);
      string file_name = get<1>(input);

      for (int x=0; x<image.height(); x++) {
          for (int y=0; y<image.width(); y++){

              image(x,y,0)=255-image(x,y,0);
              image(x,y,1)=255-image(x,y,1);
              image(x,y,2)=255-image(x,y,2);
          }
      }

      file_name="invert_"+file_name;

#ifdef DEBUG
        cout << file_name << " - image inverted" << endl;
#endif

    return ImgTuple(image, file_name);
  }
};

//func_node
struct save_img {
  bool operator()(ImgTuple input) {

      Cimg image = get<0>(input);
      string file_name = get<1>(input);

      image.save((results_dir+"/"+file_name).c_str());

#ifdef DEBUG
        cout << file_name << " - image saved" << endl;
#endif
    return true;
  }
};

int main(int argc, char *argv[]) {

    for (int i=1; i<(argc-1); i++) {
        if (strcmp(argv[i], "-b") == 0) custom_intens = strtol(argv[i+1], NULL, 10);
        else if (strcmp(argv[i], "-l") == 0) threads = strtol(argv[i+1], NULL, 10);
        else if (strcmp(argv[i], "-f") == 0) intens_file_name = string(argv[i+1]);
    }

    if (custom_intens==0 || threads == 0) return 0;

    graph g;
    source_node<ImgTuple> load_imgs_node(g, load_imgs(), false);
    broadcast_node<ImgTuple> bcast_imgs(g);
    buffer_node<ImgTuple> calc_intens_buffer(g);
    buffer_node<ImgTuple> invert_buffer(g);
    buffer_node<ImgDotsTuple> bcast_dots_buffer(g);
    broadcast_node<ImgDotsTuple> bcast_dots(g);
    buffer_node<ImgDotsTuple> select_dots_buffer(g);
    buffer_node<ImgDotsTuple> calc_avg_intens_buffer(g);
    buffer_node<ImgTuple> save_imgs_buffer(g);
    function_node<ImgTuple, ImgDotsTuple> calc_intens_node( g, threads, calc_intens() );
    function_node<ImgDotsTuple, ImgTuple> select_dots_node( g, threads, select_dots() );
    function_node<ImgDotsTuple, bool> calc_avg_intens_node( g, threads, calc_avg_intens() );
    function_node<ImgTuple, ImgTuple> invert_img_node( g, threads, invert_img() );
    function_node<ImgTuple, bool> save_img_node( g, serial, save_img() );


    make_edge(load_imgs_node, bcast_imgs);
    //to invert func
    make_edge(bcast_imgs, invert_buffer);
    make_edge(invert_buffer, invert_img_node);
    make_edge(invert_img_node, save_imgs_buffer);
    make_edge(save_imgs_buffer, save_img_node);
    //to select func
    make_edge(bcast_imgs, calc_intens_buffer);
    make_edge(calc_intens_buffer, calc_intens_node);
    make_edge(calc_intens_node, bcast_dots_buffer);
    make_edge(bcast_dots_buffer, bcast_dots);
    make_edge(bcast_dots, select_dots_buffer);
    make_edge(select_dots_buffer, select_dots_node);
    make_edge(select_dots_node, save_imgs_buffer);
    //to calc avg func
    make_edge(bcast_dots, calc_avg_intens_buffer);
    make_edge(calc_avg_intens_buffer, calc_avg_intens_node);

    load_imgs_node.activate();
    g.wait_for_all();

    if (intens_file) intens_file.close();

    return 0;
}
