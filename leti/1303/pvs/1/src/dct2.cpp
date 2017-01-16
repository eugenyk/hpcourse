#include <iostream>
#include <tbb/flow_graph.h>
#include <math.h>
#include <thread>

using namespace tbb::flow;

unsigned concurrentThreadsSupported = std::thread::hardware_concurrency();

const int rows = 10;
const int cols = 10;
double data[rows][cols];

double alpha(double v) {
    return v == 0.0 ? 1.0 / sqrt(2.0) : 1.0;
}

void dct1(double *data, double *to) {
    int length = sizeof(*data) / sizeof(double);
    double scale = sqrt(2.0 / (double) length);

    int i;
    for (i = 0; i < length; ++i) {
        double sum = 0.0;

        for (int t = 0; t < length; ++t) {
            double resCos = cos((2.0 * (double) t + 1.0) * (double) i * M_PI / (2.0 * (double) length));
            sum += data[t] * resCos * alpha((double) i);
        }

        to[i] = scale * sum;
    }
}

int main(int argc, char *argv[]) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            data[i][j] = (i + 1) * (j + 1);
        }
    }

    graph g;
    function_node<tuple<double*, int>, tuple<double*, int>> firstStep(g, concurrentThreadsSupported, [](tuple<double*, int> v) -> tuple<double*, int> {
        double result[cols];
        dct1(get<0>(v), result);
        tuple<double*, int> pair(result, get<1>(v));
        for (int i = 0; i < cols; i++) {
            data[get<1>(v)][i] = result[i];
        }
        return pair;
    });

    for (int i = 0; i < rows; i++) {
        tuple<double*,int> pair(data[i], i);
        firstStep.try_put(pair);
    }

    g.wait_for_all();

    function_node<tuple<double*, int>, tuple<double*, int>> secondStep(g, concurrentThreadsSupported, [](tuple<double*, int> v) -> tuple<double*, int> {
        double result[cols];
        dct1(get<0>(v), result);
        tuple<double*, int> pair(result, get<1>(v));
        for (int i = 0; i < rows; i++) {
            data[i][get<1>(v)] = result[i];
        }
        return pair;
    });
    for (int i = 0; i < cols; i++) {
        double col[rows];
        for (int j = 0; j < rows; j++) {
            col[j] = data[j][i];
        }
        tuple<double*,int> pair(col, i);
        secondStep.try_put(pair);
    }
    g.wait_for_all();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << data[i][j] << "\t";
        }
        std::cout << std::endl;
    }

    return 0;
}