#include <tbb/flow_graph.h>

#include <iostream>
#include <vector>
#include <fstream>

using namespace std;
using namespace tbb;
using namespace tbb::flow;

constexpr size_t N = 255;
constexpr size_t M = 255;

constexpr size_t MAX_MATRIX_COUNT = 10;

constexpr uint8_t MAX_BRIGHTNESS = 255;
constexpr uint8_t MIN_BRIGHTNESS = 0;

using Matrix = vector<uint8_t>;
using PosOfMatrix = vector<size_t>;
using PosTriple = flow::tuple<PosOfMatrix, PosOfMatrix, PosOfMatrix>;
using MatrixWithAverage = flow::tuple<Matrix, uint8_t>;

size_t getPointByCoords(size_t row, size_t col) {
    return row * N + col;
}

void markSquare(Matrix &matrix, size_t row, size_t col) {
    if (col > 0) {
        matrix[getPointByCoords(row, col - 1)] = MAX_BRIGHTNESS;
    }
    if (col < M - 1) {
        matrix[getPointByCoords(row, col + 1)] = MAX_BRIGHTNESS;
    }
    if (row > 0) {
        matrix[getPointByCoords(row - 1, col)] = MAX_BRIGHTNESS;
    }
    if (row < N - 1) {
        matrix[getPointByCoords(row + 1, col)] = MAX_BRIGHTNESS;
    }
    if (row > 0 && col > 0) {
        matrix[getPointByCoords(row - 1, col - 1)] = MAX_BRIGHTNESS;
    }
    if (row > 0 && col < M - 1) {
        matrix[getPointByCoords(row - 1, col + 1)] = MAX_BRIGHTNESS;
    }
    if (row < N - 1 && col > 0) {
        matrix[getPointByCoords(row + 1, col - 1)] = MAX_BRIGHTNESS;
    }
    if (row < N - 1 && col < M - 1) {
        matrix[getPointByCoords(row + 1, col + 1)] = MAX_BRIGHTNESS;
    }
}

class BrightFinder {
public:
    BrightFinder(const uint8_t targetValue) : targetValue(targetValue) {}

    PosOfMatrix operator()(Matrix const &matrix) {
        vector<size_t> pos{};
        for (size_t i = 0; i < matrix.size(); ++i) {
            if (matrix[i] == targetValue) {
                pos.push_back(i);
            }
        }
        cout << "Found " << pos.size() << " elements with brightness " << (int) targetValue << endl;
        return pos;
    }

private:
    const uint8_t targetValue;

};


int main(int argc, char **argv) {
    graph g{};

    size_t matrixCount = 0;

    uint8_t targetBrightness = 128;
    size_t threshold = 4;
    string logFileName = "log.log";


    char opt;
    while (-1 != (opt = (char) getopt(argc, argv, "b:l:f:"))) {
        switch (opt) {
            case 'b':
                targetBrightness = (uint8_t) atoi(optarg);
                break;
            case 'l':
                threshold = (size_t) atoi(optarg);
                break;
            case 'f':
                logFileName = optarg;
                break;
            default:
                std::cerr << "unknown opt: " << opt << std::endl;
                return -1;
        }
    }

    ofstream logFile{logFileName};

    source_node<Matrix> source(g, [&logFile, &matrixCount](Matrix &matrix) -> bool {
                                   matrix.resize(M * N);
                                   for (size_t i = 0; i < N * M; ++i) {
                                       matrix[i] = (uint8_t) rand();
                                   }
                                   logFile << "Matrix #" << matrixCount << " generated!" << endl;
                                   return ++matrixCount < MAX_MATRIX_COUNT;
                               },
                               false);
    limiter_node<Matrix> limiter{g, threshold};
    make_edge(source, limiter);

    broadcast_node<Matrix> broadcastNode{g};
    make_edge(limiter, broadcastNode);

    function_node<Matrix, PosOfMatrix> maxBrightFinder{g,
                                                       serial,
                                                       BrightFinder{MAX_BRIGHTNESS}
    };

    function_node<Matrix, PosOfMatrix> minBrightFinder{g,
                                                       serial,
                                                       BrightFinder{MIN_BRIGHTNESS}
    };

    function_node<Matrix, PosOfMatrix> targetBrightFinder{g,
                                                          serial,
                                                          BrightFinder{targetBrightness}
    };

    make_edge(broadcastNode, maxBrightFinder);
    make_edge(broadcastNode, minBrightFinder);
    make_edge(broadcastNode, targetBrightFinder);

    join_node<PosTriple> joinNode{g};

    make_edge(maxBrightFinder, input_port<0>(joinNode));
    make_edge(minBrightFinder, input_port<1>(joinNode));
    make_edge(targetBrightFinder, input_port<2>(joinNode));

    function_node<PosTriple, PosOfMatrix> joinPositions{g,
                                                        unlimited,
                                                        [&logFile](PosTriple const &posTriple) -> PosOfMatrix {
                                                            vector<size_t> pos{};
                                                            pos.insert(pos.end(), get<0>(posTriple).begin(),
                                                                       get<0>(posTriple).end());
                                                            pos.insert(pos.end(), get<1>(posTriple).begin(),
                                                                       get<1>(posTriple).end());
                                                            pos.insert(pos.end(), get<2>(posTriple).begin(),
                                                                       get<2>(posTriple).end());
                                                            logFile << "Sum pos count " << pos.size() << endl;
                                                            return pos;
                                                        }
    };
    make_edge(joinNode, joinPositions);

    function_node<PosOfMatrix, Matrix> markPositions{g,
                                                     unlimited,
                                                     [&logFile](PosOfMatrix const &posOfMatrix) -> Matrix {
                                                         Matrix matrix;
                                                         matrix.resize(N * M);
                                                         for (size_t pos = 0; pos < posOfMatrix.size(); ++pos) {
                                                             size_t row = pos / N;
                                                             size_t col = pos - row * N;
                                                             markSquare(matrix, row, col);
                                                         }
                                                         logFile << "Marks all positions" << endl;
                                                         return matrix;
                                                     }
    };
    make_edge(joinPositions, markPositions);

    broadcast_node<Matrix> broadcastNode2{g};
    make_edge(markPositions, broadcastNode2);

    function_node<Matrix, Matrix> inversionBrightness{g,
                                                      serial,
                                                      [&logFile](Matrix const &matrix) -> Matrix {
                                                          logFile << "Start invert" << endl;
                                                          Matrix result;
                                                          result.resize(N * M);
                                                          for (size_t j = 0; j < matrix.size(); ++j) {
                                                              result[j] = (uint8_t) (matrix[j] + 255);
                                                          }
                                                          return result;
                                                      }
    };

    function_node<Matrix, uint8_t> averageBrightness{g,
                                                     serial,
                                                     [&logFile](Matrix const &matrix) -> uint8_t {
                                                         logFile << "Start calc average" << endl;
                                                         size_t sum = 0;
                                                         for (size_t j = 0; j < matrix.size(); ++j) {
                                                             sum += matrix[j];
                                                         }
                                                         return (uint8_t) (sum / matrix.size());
                                                     }
    };

    make_edge(broadcastNode2, inversionBrightness);
    make_edge(broadcastNode2, averageBrightness);

    join_node<MatrixWithAverage> joinNode2{g};
    make_edge(inversionBrightness, input_port<0>(joinNode2));
    make_edge(averageBrightness, input_port<1>(joinNode2));


    function_node<MatrixWithAverage> finalNode{g,
                                               unlimited,
                                               [&logFile](MatrixWithAverage const &matrixWithAverage) {
                                                   logFile << "Average brightness is "
                                                           << (int) get<1>(matrixWithAverage)
                                                           << endl;
                                               }
    };
    make_edge(joinNode2, finalNode);

    make_edge(finalNode, limiter.decrement);

    source.activate();
    g.wait_for_all();
    return 0;
}