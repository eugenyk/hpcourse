/*
Group 1303
Budaeva Liubov
lab #1 
*/

#include "tbb/flow_graph.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <set>
#include <string>
#include <algorithm>

using namespace std;
using namespace tbb::flow;

typedef unsigned int uint;

struct Coords{
    uint y; //row index
    uint x; //column index
    Coords(){
        y = 0;
        x = 0;
    }
    Coords(uint y, uint x){
        this->y = y;
        this->x = x;
    }
    static int comparatorByXY(const void * val1, const void * val2){
        Coords* p1 = (Coords*)val1;
        Coords* p2 = (Coords*)val2;

        if(p1->x == p2->x)
            return p1->y - p2->y;
        else
            return p1->x - p2->x;
    }
};


bool operator<(const Coords& val1, const Coords& val2){
    return Coords::comparatorByXY(&val1, &val2) < 0;
}

typedef vector<vector<uint>> Matrix;
typedef vector<Coords> Elements;
typedef tagged_msg<size_t, Matrix*> taggedMatrix;
typedef tagged_msg<size_t, Elements*> taggedElements;
typedef tuple<taggedMatrix, taggedElements, taggedElements, taggedElements> FindsResult;
typedef tuple<Matrix*,Elements*> MatrixAndRes;
typedef tagged_msg<size_t, tuple<Matrix*,Elements*>> taggedMatrixAndRes;
typedef tuple<Matrix*, Matrix*> EndMatrixes;
typedef tagged_msg<size_t, tuple<Matrix*, Matrix*>> taggedEndMatrixes;
typedef tagged_msg<size_t, double> taggedAverage;
typedef tuple<taggedEndMatrixes, taggedAverage> EndWorkThings;

string log_filename = "";
int brightness = 4;
int limit = 1;
int M = 3;
int N = 3;
int COUNT = 5;
bool db = false;

Elements* findMax(Matrix* const mtx){
    Elements* output = new Elements;

    uint n = mtx->size();
    uint m = (*mtx)[0].size();
    uint max = 0;

    for(uint i = 0; i < n; i++){
        for(uint j = 0; j < m; j++){
            if(max == (*mtx)[i][j]){
                output->push_back(Coords(i, j));
            }
            if(max < (*mtx)[i][j]){
                max = (*mtx)[i][j];
                output->clear();
                output->push_back(Coords(i, j));
            }
        }
    }
    return output;
}

Elements* findMin(Matrix* const mtx){
    Elements* output = new Elements;

    uint n = mtx->size();
    uint m = mtx[0].size();
    uint min = 256;

    for(uint i = 0; i < n; i++){
        for(uint j = 0; j < m; j++){
            if(min == (*mtx)[i][j]){
                output->push_back(Coords(i, j));
            }
            if(min > (*mtx)[i][j]){
                min = (*mtx)[i][j];
                output->clear();
                output->push_back(Coords(i, j));
            }
        }
    }
    return output;
}

Elements* findValue(Matrix* const mtx){
    Elements* output = new Elements;

    uint n = mtx->size();
    uint m = (*mtx)[0].size();

    for(uint i = 0; i < n; i++){
        for(uint j = 0; j < m; j++){
            if(brightness == (int)(*mtx)[i][j]){
                output->push_back(Coords(i, j));
            }
        }
    }
    return output;
}

Matrix* generateMatrix(uint n, uint m){
    Matrix* output = new Matrix;

    mt19937 mt(time(0));
    uniform_int_distribution<> dis(0, 255);

    for(uint i = 0; i < n; i++){
        vector<uint> row(m);
        generate(row.begin(), row.end(), bind(dis, ref(mt)));
        output->push_back(row);
    }
    return output;
}

Matrix* inverseMatrix(Matrix* mtx){
    Matrix* inv = new Matrix(*mtx);
    uint n = mtx->size();
    uint m = mtx[0].size();
    for(uint i = 0; i < n; i++){
        for (uint j = 0; j < m; j++){
            (*inv)[i][j] = 255 - (*mtx)[i][j];
        }
    }
    return inv;
}

double getAverageValue(Matrix* mtx){
    uint average = 0;
    uint n = mtx->size();
    uint m = (*mtx)[0].size();

    for(uint i = 0; i < n; i++){
        average = accumulate((*mtx)[i].begin(), (*mtx)[i].end(), average);
    }
    average = average/(n*m);
    return average;
}

Elements* selectElements(Elements* elems){
    Elements* output = new Elements;
    uint size = elems->size();
    for(uint i = 0; i < size; i++){
        output->push_back((*elems)[i]);
        int y_up = (*elems)[i].y - 1;
        int y_below = (*elems)[i].y + 1;
        int x_left = (*elems)[i].x - 1;
        int x_right = (*elems)[i].x + 1;
        int x = (*elems)[i].x;
        int y = (*elems)[i].y;


        output->push_back((*elems)[i]);

        if(y_up >= 0 && x_left >= 0)
            output->push_back(Coords(y_up, x_left));

        if(y_up >= 0 && x_right < M)
            output->push_back(Coords(y_up, x_right));

        if(y_up >= 0)
            output->push_back(Coords(y_up, x));

        if(x_left >= 0 && y_below < N)
            output->push_back(Coords(y_below, x_left));

        if(y_below < N && x_right < M)
            output->push_back(Coords(y_below, x_right));

        if(y_below < N)
            output->push_back(Coords(y_below, x));

        if(x_left >= 0)
            output->push_back(Coords(y, x_left));

        if(x_right < M)
            output->push_back(Coords(y, x_right));
    }
    return output;
}

Elements* getUnion(vector<Elements*> src){
    set<Coords> all;

    for(uint i = 0; i < src.size(); i++){
        all.insert(src[i]->begin(), src[i]->end());
    }

    Elements* output = new Elements(all.begin(), all.end());
    return output;
}

void outputNumberToFile(double numb){
    ofstream outfile;
    outfile.open(log_filename, ios_base::app | ios_base::out);
    outfile << numb << endl;
    outfile.close();
}

void outputMatrix(Matrix* mtx){
    uint n = mtx->size();
    uint m = mtx[0].size();
    for(uint i = 0; i < n; i++){
        for (uint j = 0; j < m; j++){
            cout << setw(4) << (*mtx)[i][j];
        }
        cout << endl;
    }
}

void outputElements(Elements* elems){
    cout << endl;
    uint n = elems->size();
    for(uint i = 0; i < n; i++){
        cout << setw(5) << "(" << (*elems)[i].x << ", " << (*elems)[i].y << ")";
    }
    cout << endl;
}

bool processArgs(int argc, char *argv[]){
    string args_format_error = "Error: arguments error.\n";
    string args_skip_warning = "Warning: some arguments were skipped.\n";

    if(argc == 2){
        string h_flag = argv[1];
        if(h_flag.compare("-h") == 0 || h_flag.compare("-help")){
            cout << "Syntax: main flags[...] [options ...]" <<endl;
            cout << "flags:" <<endl;
            cout << setw(5) << "-n" << setw(4) << " " << "rows number" << endl;
            cout << setw(5) << "-m" << setw(4) << " " << "colums number" << endl;
            cout << setw(5) << "-c" << setw(4) << " " << "count of matrixes" << endl;
            cout << setw(5) << "-b" << setw(4) << " " << "brightness for search" << endl;
            cout << setw(5) << "-l" << setw(4) << " " << "limit for parallel matrixes processing" << endl;
            cout << "options:" <<endl;
            cout << setw(5) << "-f" << setw(4) << " " << "filename to output brightness" << endl;
            cout << setw(5) << "-db" << setw(4) << " " << "to see debug output. This parameter should be last." << endl;
        }
        return 0;
    }

    if(argc < 11){
        cout << args_format_error;
        return 0;
    }

    for(int i = 1; i < argc; i += 2){
        string flag = argv[i];
        if(flag == "-n") N = atoi(argv[i + 1]);
        else if(flag == "-m") M = atoi(argv[i + 1]);
        else if(flag == "-c") COUNT = atoi(argv[i + 1]);
        else if(flag == "-b") brightness = atoi(argv[i + 1]);
        else if(flag == "-l") limit = atoi(argv[i + 1]);
        else if(flag == "-f") log_filename = argv[i + 1];
        else if(flag == "-db") db = true;
        else cout << args_skip_warning;
    }

    if(brightness == -1 || limit == -1 || M == -1 || N == -1 || COUNT == -1){
        cout << args_format_error;
        return 0;
    }

    return 1;
}

void outputArgs(){
    cout << "-n " << N << " -m " << M << " -c " << COUNT << " -b " << brightness
         << " -l " << limit << " -f " << log_filename << " -db " << db << endl;
}

int main(int argc, char *argv[]) {
    if(processArgs(argc, argv)){
        if(db){
            cout << "Input argenumts:\n";
            outputArgs();
        }

        // creating of the graph
        graph g;

        queue_node<taggedMatrix> queueMatrixNode(g);
        limiter_node<taggedMatrix> limitMatrixNode(g, limit);

        function_node<taggedMatrix, taggedMatrix> stubNode(g, unlimited, [](taggedMatrix mtx){return mtx;});
        function_node<taggedMatrix, taggedElements> findMaxNode(g, unlimited, [](taggedMatrix mtx){
                                                                                    Matrix* matrix = cast_to<Matrix*>(mtx);
                                                                                    Elements* maxs = findMax(matrix);
                                                                                    return taggedElements(mtx.tag(), maxs);
                                                                                });
        function_node<taggedMatrix, taggedElements> findMinNode(g, unlimited, [](taggedMatrix mtx){
                                                                                    Matrix* matrix = cast_to<Matrix*>(mtx);
                                                                                    Elements* mins = findMin(matrix);
                                                                                    return taggedElements(mtx.tag(), mins);
                                                                                });
        function_node<taggedMatrix, taggedElements> findValueNode(g, unlimited, [](taggedMatrix mtx){
                                                                                    Matrix* matrix = cast_to<Matrix*>(mtx);
                                                                                    Elements* vals = findValue(matrix);
                                                                                    return taggedElements(mtx.tag(), vals);
                                                                                });

        join_node<FindsResult, tag_matching> joinAllFindsNode(g,
                                                              [](taggedMatrix mtx)-> size_t{return mtx.tag();},
                                                              [](taggedElements elems)-> size_t{return elems.tag();},
                                                              [](taggedElements elems)-> size_t{return elems.tag();},
                                                              [](taggedElements elems)-> size_t{return elems.tag();}
                                                             );

        function_node<FindsResult, taggedMatrixAndRes> getUnionNode(g, unlimited,
                                                           [](FindsResult t){
                                                              Matrix* mtx = cast_to<Matrix*>(get<0>(t));
                                                              Elements* maxs = cast_to<Elements*>(get<1>(t));
                                                              Elements* mins = cast_to<Elements*>(get<2>(t));
                                                              Elements* vals = cast_to<Elements*>(get<3>(t));

                                                              vector<Elements*> src;
                                                              src.push_back(mins);
                                                              src.push_back(maxs);
                                                              src.push_back(vals);

                                                              MatrixAndRes res;
                                                              get<0>(res) = mtx;
                                                              get<1>(res) = getUnion(src);

                                                              if(db){
                                                                  cout << "\nInput matrix:\n";
                                                                  outputMatrix(mtx);

                                                                  cout << "\nMax elements:";
                                                                  outputElements(maxs);

                                                                  cout << "\nMin elements:";
                                                                  outputElements(mins);

                                                                  cout << "\nSet value elements:";
                                                                  outputElements(vals);

                                                                  cout << "\nUnion of all above:";
                                                                  outputElements(get<1>(res));
                                                              }

                                                              if (mins)
                                                                  delete mins;
                                                              if (maxs)
                                                                  delete maxs;
                                                              if (vals)
                                                                  delete vals;

                                                              return taggedMatrixAndRes(get<0>(t).tag(), res);
                                                           });

        function_node<taggedMatrixAndRes, taggedMatrixAndRes> selectElementsNode(g, unlimited,
                                                                     [](taggedMatrixAndRes t){
                                                                        MatrixAndRes mtxres = cast_to<MatrixAndRes>(t);

                                                                        MatrixAndRes res;
                                                                        get<0>(res) = get<0>(mtxres);
                                                                        get<1>(res) = selectElements(get<1>(mtxres));

                                                                        if (get<1>(mtxres)) delete get<1>(mtxres);

                                                                        if(db){
                                                                            cout << "\nSelected elements:";
                                                                            outputElements(get<1>(res));
                                                                        }
                                                                        return taggedMatrixAndRes(t.tag(), res);
                                                                     });

        function_node<taggedMatrixAndRes, taggedEndMatrixes> inverseMatrixNode(g, unlimited,
                                                      [](taggedMatrixAndRes t){
                                                         MatrixAndRes mtxres = cast_to<MatrixAndRes>(t);

                                                         EndMatrixes res;
                                                         get<0>(res) = get<0>(mtxres);
                                                         get<1>(res) = inverseMatrix(get<0>(mtxres));

                                                         if (get<1>(mtxres))
                                                             delete get<1>(mtxres);

                                                         if(db){
                                                             cout << "\nInversed matrix:";
                                                             outputMatrix(get<1>(res));
                                                         }
                                                         return taggedEndMatrixes(t.tag(), res);
                                                      });

        function_node<taggedMatrixAndRes, taggedAverage> getAverageValueNode(g, unlimited,
                                                      [](taggedMatrixAndRes t){
                                                         MatrixAndRes mtxres = cast_to<MatrixAndRes>(t);

                                                         double avg = getAverageValue(get<0>(mtxres));
                                                         if(db){
                                                             cout << "\nAverage value:" << avg << endl;
                                                         }
                                                         return taggedAverage(t.tag(), avg);
                                                      });

        join_node<EndWorkThings, tag_matching> joinEndWorkNode(g,
                                                               [](taggedEndMatrixes mtxs)-> size_t{return mtxs.tag();},
                                                               [](taggedAverage avg)-> size_t{return avg.tag();}
                                                               );

        function_node<EndWorkThings, continue_msg> outputAndDeleteNode(g, unlimited,
                                                      [](EndWorkThings t){
                                                         EndMatrixes for_clear = cast_to<EndMatrixes>(get<0>(t));
                                                         if (get<0>(for_clear)) delete get<0>(for_clear);
                                                         if (get<1>(for_clear)) delete get<1>(for_clear);

                                                         double numb = cast_to<double>(get<1>(t));

                                                         if(log_filename != ""){
                                                             outputNumberToFile(numb);

                                                             if(db){
                                                                 cout << "\nSee average value " << numb << " in " << log_filename << "." << endl;
                                                             }
                                                         }

                                                         return continue_msg();
                                                      });

        make_edge(queueMatrixNode, limitMatrixNode);

        make_edge(limitMatrixNode, stubNode);
        make_edge(limitMatrixNode, findMaxNode);
        make_edge(limitMatrixNode, findMinNode);
        make_edge(limitMatrixNode, findValueNode);

        make_edge(stubNode, input_port<0>(joinAllFindsNode));
        make_edge(findMaxNode, input_port<1>(joinAllFindsNode));
        make_edge(findMinNode, input_port<2>(joinAllFindsNode));
        make_edge(findValueNode, input_port<3>(joinAllFindsNode));

        make_edge(joinAllFindsNode, getUnionNode);

        make_edge(getUnionNode, selectElementsNode);

        make_edge(selectElementsNode, inverseMatrixNode);
        make_edge(selectElementsNode, getAverageValueNode);

        make_edge(inverseMatrixNode, input_port<0>(joinEndWorkNode));
        make_edge(getAverageValueNode, input_port<1>(joinEndWorkNode));

        make_edge(joinEndWorkNode, outputAndDeleteNode);
        make_edge(outputAndDeleteNode, limitMatrixNode.decrement);

        // run on graph
        for(int i = 0; i < COUNT; i++){
            Matrix* mtx = generateMatrix(N, M);
            queueMatrixNode.try_put(taggedMatrix(i, mtx));
        }
        g.wait_for_all();
    }
    cout << "end" << endl;
    return 0;
}