#include <iostream>
#include <random>
#include <vector>
#include <ctime>
#include <string>

#include "tbb/flow_graph.h"
#include "tbb/tbb.h"
#include "matrix.h"
#include "matrix_algorithm.h"
#include "test.h"

using namespace std;
using namespace tbb::flow;

int main(int argc, char *argv[])
{
    bool testMode = false;
    int sizeMatrix = 20000;
    int limitMatrix = 8;
    int searchBrightness = 4;
    int countMatrix = 15;
    std::string filename;
    FindElements::TypeSearch typeSearch = FindElements::TypeSearch::NOTHING;

    for (int i(1); i < argc; ++i) {
        string prm = string(argv[i]);
        if (i + 1 != argc) {
            string value = string(argv[i + 1]);
            if (prm == "-b") {
                searchBrightness = std::atoi(value.c_str());
                typeSearch = FindElements::TypeSearch::CUSTOM;
            } else if (prm == "-l") {
                limitMatrix = std::atoi(value.c_str());
            } else if (prm == "-f") {
                filename = string(value.c_str());
            } else if (prm == "-c") {
                countMatrix = std::atoi(value.c_str());
            } else if (prm == "-s") {
                sizeMatrix = std::atoi(value.c_str());
            }
        } else {
            if (prm == "-test") {
                testMode = true;
            }
        }
    }

    cout << "-brightness:" << searchBrightness << endl;
    cout << "-file:" << filename << endl;
    cout << "-countMatrix" << countMatrix << endl;
    cout << "-sizeMatrix:" << sizeMatrix << endl;
    cout << "-limitMatrix:" << limitMatrix << endl;
    cout << "-testMode:" << testMode << endl;

    if (sizeMatrix < 2 || countMatrix < 1 || limitMatrix < 1) {
        cout << "Bad value for paramaters." << endl;
        return 0;
    }

    std::mt19937 generate(time(0));
    std::uniform_int_distribution<> distribution_koef1(0, 127);
    std::uniform_int_distribution<> distribution_koef2(128, 255);

    graph g;
    queue_node< tagged_msg<size_t, Matrix*> > queueMatrixNode(g);

    limiter_node< tagged_msg<size_t, Matrix*> > limitMatrixNode(g, limitMatrix);

    function_node< tagged_msg<size_t, Matrix*>, tagged_msg<size_t, vector<Index>>
            > findMaxNode(g, unlimited, FindElements(FindElements::TypeSearch::MAX));

    function_node< tagged_msg<size_t, Matrix*>, tagged_msg<size_t, vector<Index>>
            > findMinNode(g, unlimited, FindElements(FindElements::TypeSearch::MIN));

    function_node< tagged_msg<size_t, Matrix*>, tagged_msg<size_t, vector<Index>>
            > findSelectNode(g, unlimited, FindElements(typeSearch, searchBrightness));

    join_node< tuple< tagged_msg<size_t, Matrix*>, tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, vector<Index>>, tagged_msg<size_t, vector<Index>>>
            ,
            tag_matching> joinMatrixAndFindElemNode(g,
                                                    [](tagged_msg<size_t, Matrix*> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, vector<Index>> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, vector<Index>> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, vector<Index>> data) -> size_t { return data.tag(); });


    function_node< tuple< tagged_msg<size_t, Matrix*>, tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, vector<Index>>, tagged_msg<size_t, vector<Index>>>
            ,
            tagged_msg<size_t, Matrix*>
            > highlightFoundElementNode(g, unlimited, HighLightFoundElementMatrix());

    function_node< tagged_msg<size_t, Matrix*>, tagged_msg<size_t, double> > calcBrightnessNode(g, unlimited, CalcBrightnessNode());
    function_node< tagged_msg<size_t, Matrix*>, tagged_msg<size_t, Matrix*> > calcInvMatrixNode(g, unlimited, CalcInvMatrixNode());
    function_node< tagged_msg<size_t, double> > outBrightnessNode(g, serial, OutBrightnessNode(filename));

    function_node < tuple < tagged_msg<size_t, Matrix*>, tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>, tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, vector<Index>>, tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, double> >
            ,
            tuple < tagged_msg<size_t, Matrix*>, tagged_msg<size_t, Matrix*>, tagged_msg<size_t, Matrix*>>
            > formMessageForDeleteMatrix(g, unlimited, [] (tuple <
                                         tagged_msg<size_t, Matrix*>,
                                         tagged_msg<size_t, Matrix*>,
                                         tagged_msg<size_t, Matrix*>,
                                         tagged_msg<size_t, vector<Index>>,
                                         tagged_msg<size_t, vector<Index>>,
                                         tagged_msg<size_t, vector<Index>>,
                                         tagged_msg<size_t, double>
                                         > data) ->
            tuple<tagged_msg<size_t, Matrix*>, tagged_msg<size_t, Matrix*>, tagged_msg<size_t, Matrix*>>
    {
                                                                                                         return make_tuple(std::get<0>(data), std::get<1>(data), std::get<2>(data));
});

    function_node<
            tuple<
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>
            >,
            continue_msg > deleteMatrix(g, unlimited, DeleteMatrix());

    function_node<
            tuple
            <
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, double>
            >,
            tuple
            <
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>
            >
            > outputTestInfo(g, serial, TestOutput());

    join_node<
            tuple
            <
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, Matrix*>,
            tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, vector<Index>>,
            tagged_msg<size_t, double>
            >,
            tag_matching> joinAllInfoNode(g,
                                          [](tagged_msg<size_t, Matrix*> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, Matrix*> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, Matrix*> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, vector<Index>> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, vector<Index>> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, vector<Index>> data) -> size_t { return data.tag(); },
    [](tagged_msg<size_t, double> data) -> size_t { return data.tag(); });

    make_edge(limitMatrixNode, input_port<0>(joinAllInfoNode));
    make_edge(calcInvMatrixNode, input_port<1>(joinAllInfoNode));
    make_edge(highlightFoundElementNode, input_port<2>(joinAllInfoNode));
    make_edge(findMaxNode, input_port<3>(joinAllInfoNode));
    make_edge(findMinNode, input_port<4>(joinAllInfoNode));
    make_edge(findSelectNode, input_port<5>(joinAllInfoNode));
    make_edge(calcBrightnessNode, input_port<6>(joinAllInfoNode));

    if (testMode) {
        make_edge(joinAllInfoNode, outputTestInfo);
        make_edge(outputTestInfo, deleteMatrix);
    } else {
        make_edge(joinAllInfoNode, formMessageForDeleteMatrix);
        make_edge(formMessageForDeleteMatrix, deleteMatrix);
    }
    make_edge(deleteMatrix, limitMatrixNode.decrement);

    make_edge(queueMatrixNode, limitMatrixNode);
    make_edge(limitMatrixNode, findMaxNode);
    make_edge(limitMatrixNode, findMinNode);
    make_edge(limitMatrixNode, findSelectNode);
    make_edge(limitMatrixNode, input_port<0>(joinMatrixAndFindElemNode));
    make_edge(findMaxNode, input_port<1>(joinMatrixAndFindElemNode));
    make_edge(findMinNode, input_port<2>(joinMatrixAndFindElemNode));
    make_edge(findSelectNode, input_port<3>(joinMatrixAndFindElemNode));
    make_edge(joinMatrixAndFindElemNode, highlightFoundElementNode);
    make_edge(highlightFoundElementNode, calcBrightnessNode);
    make_edge(highlightFoundElementNode, calcInvMatrixNode);
    if (!filename.empty())make_edge(calcBrightnessNode, outBrightnessNode);

    const clock_t begin_time = clock();
    for(int i(0); i < countMatrix; ++i) {
        Matrix *m = new Matrix(sizeMatrix, sizeMatrix);

        int koef1 = distribution_koef1(generate);
        int koef2 = distribution_koef2(generate);
        tbb::parallel_for(0, (int)m->getSize(), 1, [m, koef1, koef2](int i) {
            m->setValue(i, ((i * koef1) + koef2)%255);
        });

        queueMatrixNode.try_put(tagged_msg<size_t, Matrix*>(i, m));
    }
    g.wait_for_all();
    cout << "time = " << (double(clock() - begin_time) / CLOCKS_PER_SEC) << endl;
    return 0;
}
