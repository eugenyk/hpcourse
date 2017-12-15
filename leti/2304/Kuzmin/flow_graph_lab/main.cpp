#include <tbb/flow_graph.h>
#include <CImg.h>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdio>

#define DEBUG

using namespace tbb::flow;
using namespace std;
using namespace cimg_library;
using namespace tbb;

struct Dot {
    int x,y;
};

typedef CImg<unsigned char> Cimg;
typedef tuple<Cimg, string> ImgTuple; //image + name
typedef tuple<ImgTuple, vector<Dot>, vector<Dot>, vector<Dot>, int, int> ImgDotsTuple; //image + max, min, custom dots + max val, min val

int pink[] = {255, 0, 255}; //max intens
int green[] = {68, 255, 0}; //min intens
int orange[] = { 255, 120, 0 }; //custom intens

int custom_intens;
string imgs_file_name = "list.txt";
string results_dir = "results";
string imgs_dir = "imgs";


//source_node
class load_imgs {
private:
    vector<string>* imgs_list;
    int pos;
public:

  bool operator()(ImgTuple& output) {
    //read files list
    if (imgs_list == NULL) {
        imgs_list = new vector<string>();
        pos = 0;
        ifstream list(imgs_dir+"/"+imgs_file_name);
        string line;
        while (list >> line)
            imgs_list->push_back(line);
        list.close();
    }

    //read one image
    if (pos < imgs_list->size())
    {
        string img_name = imgs_list->at(pos);
        Cimg data((imgs_dir+"/"+img_name).c_str());
        output = ImgTuple(data, img_name);
        pos++;
#ifdef DEBUG
        cout << img_name << " - image loaded" << endl;
#endif
        return true;
    }

    //free memory
    delete imgs_list;
    return false;
  }
};

//func_node
struct calc_intens {
  ImgDotsTuple operator()(ImgTuple input) {
      Cimg image = get<0>(input);
      vector<Dot> maxs, mins, customs;
      int max_val=0, min_val=255, custom_val=custom_intens;
int dot_val;

     for (int x=0; x<image.width(); x++) {
         for (int y=0; y<image.height(); y++){

              dot_val = (image(x,y,0)+image(x,y,1)+image(x,y,2))/3;

             if (max_val == dot_val) {
                 maxs.push_back({x,y});
             }
             else if (max_val < dot_val) {
                 max_val = dot_val;
                 maxs.clear();
                 maxs.push_back({x,y});
             }

             if (min_val == dot_val) {
                 mins.push_back({x,y});
             }
             else if (min_val > dot_val) {
                 min_val = dot_val;
                 mins.clear();
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

      file_name="intensity_"+file_name;

#ifdef DEBUG
      cout << file_name << " - pixels selected on image" << endl;
#endif
    return ImgTuple(image, file_name);
  }
};

//func_node
class calc_avg_intens {
private:
  string file_name;
  bool file_exists;
public:
  calc_avg_intens(string file_name) {
      this->file_name = file_name;
      file_exists = true;
  }

  bool operator()(ImgDotsTuple input) {

      string img_name = get<1>(get<0>(input));
      vector<Dot> maxs = get<1>(input), mins = get<2>(input), customs = get<3>(input);
      int max_val=get<4>(input), min_val=get<5>(input), custom_val=custom_intens;

      int dots_count = maxs.size() + mins.size() + customs.size();

      int avg = (maxs.size() * max_val + mins.size() * min_val + customs.size() * custom_val) / dots_count;

      if (!file_name.empty()) {
          if (file_exists) {
              file_exists = false;
              remove(("./"+results_dir+"/"+file_name).c_str());
          }

          ofstream intens_file;
          intens_file.open("./"+results_dir+"/"+file_name, ios_base::app);
          intens_file << img_name << "=" << avg << endl;
          intens_file.close();
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

      for (int x=0; x<image.width(); x++) {
          for (int y=0; y<image.height(); y++){

              image(x,y,0)=255-image(x,y,0);
              image(x,y,1)=255-image(x,y,1);
              image(x,y,2)=255-image(x,y,2);
          }
      }

      file_name="inverted_"+file_name;

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

      image.save(("./"+results_dir+"/"+file_name).c_str());

#ifdef DEBUG
        cout << file_name << " - image saved" << endl;
#endif
    return true;
  }
};



int main(int argc, char *argv[]) {

    string intens_file_name;
    int threads;

    for (int i=1; i<(argc-1); i++) {
        if (strcmp(argv[i], "-b") == 0) custom_intens = strtol(argv[i+1], NULL, 10);
        else if (strcmp(argv[i], "-l") == 0) threads = strtol(argv[i+1], NULL, 10);
        else if (strcmp(argv[i], "-f") == 0) intens_file_name = string(argv[i+1]);
    }

    if (custom_intens==0 || threads == 0) return 0;

    mkdir(results_dir.c_str(), ACCESSPERMS);

    graph g;
    source_node<ImgTuple> load_imgs_node(g, load_imgs(), false);
    broadcast_node<ImgTuple> bcast_imgs(g);
    buffer_node<ImgTuple> calc_intens_buffer(g);
    buffer_node<ImgTuple> invert_buffer(g);
    broadcast_node<ImgDotsTuple> bcast_dots(g);
    buffer_node<ImgDotsTuple> select_dots_buffer(g);
    buffer_node<ImgDotsTuple> calc_avg_intens_buffer(g);
    buffer_node<ImgTuple> save_imgs_buffer(g);
    function_node<ImgTuple, ImgDotsTuple> calc_intens_node( g, threads, calc_intens() );
    function_node<ImgDotsTuple, ImgTuple> select_dots_node( g, threads, select_dots() );
    function_node<ImgDotsTuple, bool> calc_avg_intens_node( g, serial, calc_avg_intens(intens_file_name) );
    function_node<ImgTuple, ImgTuple> invert_img_node( g, threads, invert_img() );
    function_node<ImgTuple, bool> save_img_node( g, threads, save_img() );


    make_edge(load_imgs_node, bcast_imgs);
    //to invert func
    make_edge(bcast_imgs, invert_buffer);
    make_edge(invert_buffer, invert_img_node);
    make_edge(invert_img_node, save_imgs_buffer);
    make_edge(save_imgs_buffer, save_img_node);
    //to select func
    make_edge(bcast_imgs, calc_intens_buffer);
    make_edge(calc_intens_buffer, calc_intens_node);
    make_edge(calc_intens_node, bcast_dots);
    make_edge(bcast_dots, select_dots_buffer);
    make_edge(select_dots_buffer, select_dots_node);
    make_edge(select_dots_node, save_imgs_buffer);
    //to calc avg func
    make_edge(bcast_dots, calc_avg_intens_buffer);
    make_edge(calc_avg_intens_buffer, calc_avg_intens_node);

    load_imgs_node.activate();
    g.wait_for_all();

    return 0;
}
