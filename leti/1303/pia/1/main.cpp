
#include <fstream>
#include <iostream>
#include <vector>
#include <tbb/flow_graph.h>

using namespace std;
using namespace tbb;
using namespace tbb::flow;

constexpr size_t  COUNT             = 10;
constexpr uint8_t BRIGHTNESS_MAX    = 255;
constexpr uint8_t BRIGHTNESS_MIN    = 0;
constexpr uint8_t BRIGHTNESS_BORDER = 0;
constexpr size_t  MATRIX_WIDTH      = 255;
constexpr size_t  MATRIX_HEIGHT     = 255;


using Matrix = vector<uint8_t>;
using PositionList = vector<size_t>;
using PositionTriple = flow::tuple<PositionList, PositionList, PositionList>;
using MatrixWithAverage = flow::tuple<Matrix, uint8_t>;


void printUsage() {
    cout << "Usage: [-t target_brightness] [-f log_file] [-l threshold]";
}


size_t calculatePosition(size_t row, size_t col) {
    return row * MATRIX_HEIGHT + col;
}


int main(int argc, char **argv) {
    srand(time(NULL));

    graph   g{};
    size_t  count            = 0;
    uint8_t targetBrightness = 128;
    size_t  threshold        = 4;
    string  fileName         = "output.log";
    char    opt;

    while (-1 != (opt = (char) getopt(argc, argv, "t:l:f:"))) {
        switch (opt) {
            case 't':
                targetBrightness = (uint8_t) atoi(optarg);
                break;
            case 'l':
                threshold = (size_t) atoi(optarg);
                break;
            case 'f':
                fileName = optarg;
                break;
            default:
                printUsage();
                return -1;
        }
    }

    ofstream logFile{fileName};

    source_node<Matrix> source(g, [&logFile, &count](Matrix &matrix) -> bool {
        matrix.resize(MATRIX_WIDTH * MATRIX_HEIGHT);
        
        for (size_t i = 0; i < MATRIX_HEIGHT * MATRIX_WIDTH; ++i) {
            matrix[i] = (uint8_t) rand() % 256;
        }

        logFile << "generated matrix " << count << endl;

        return ++count < COUNT;
    }, false);

    limiter_node<Matrix> limiter{g, threshold};

    broadcast_node<Matrix> firstBroadcast{g};

    function_node<Matrix, PositionList> maxBrightFinder{
        g,
        serial,
        [&logFile](Matrix const &matrix) -> vector<size_t> {
            vector<size_t> pos{};

            for (size_t i = 0; i < matrix.size(); ++i) {
                if (matrix[i] == BRIGHTNESS_MAX) {
                    pos.push_back(i);
                }
            }

            logFile << "found " << pos.size() << " points with brightness " << (int) BRIGHTNESS_MAX << endl;

            return pos;
        }
    };

    function_node<Matrix, PositionList> minBrightFinder{
        g,
        serial,
        [&logFile](Matrix const &matrix) -> vector<size_t> {
            vector<size_t> pos{};

            for (size_t i = 0; i < matrix.size(); ++i) {
                if (matrix[i] == BRIGHTNESS_MIN) {
                    pos.push_back(i);
                }
            }

            logFile << "found " << pos.size() << " points with brightness " << (int) BRIGHTNESS_MIN << endl;

            return pos;
        }
    };

    function_node<Matrix, PositionList> targetBrightFinder{
        g,
        serial,
        [&logFile, targetBrightness](Matrix const &matrix) -> vector<size_t> {
            vector<size_t> pos{};

            for (size_t i = 0; i < matrix.size(); ++i) {
                if (matrix[i] == targetBrightness) {
                    pos.push_back(i);
                }
            }

            logFile << "found " << pos.size() << " points with brightness " << (int) targetBrightness << endl;

            return pos;
        }
    };

    join_node<PositionTriple> firstJoinNode{g};

    function_node<PositionTriple, PositionList> joinPositions{
        g,
        unlimited,
        [&logFile](PositionTriple const &positionTriple) -> PositionList {
            vector<size_t> pos{};

            pos.insert(
                pos.end(),
                get<0>(positionTriple).begin(),
                get<0>(positionTriple).end()
            );

            pos.insert(
                pos.end(),
                get<1>(positionTriple).begin(),
                get<1>(positionTriple).end()
            );

            pos.insert(
                pos.end(),
                get<2>(positionTriple).begin(),
                get<2>(positionTriple).end()
            );

            logFile << "sum pos count " << pos.size() << endl;

            return pos;
        }
    };

    function_node<PositionList, Matrix> markPositions{
        g,
        unlimited,
        [&logFile](PositionList const &PositionList) -> Matrix {
            Matrix matrix;
            matrix.resize(MATRIX_HEIGHT * MATRIX_WIDTH);
            
            for (size_t pos = 0; pos < PositionList.size(); ++pos) {
                size_t row = pos / MATRIX_HEIGHT;
                size_t col = pos - row * MATRIX_HEIGHT;

                if (col > 0) {
                    matrix[calculatePosition(row, col - 1)] = BRIGHTNESS_BORDER;
                }

                if (col < MATRIX_WIDTH - 1) {
                    matrix[calculatePosition(row, col + 1)] = BRIGHTNESS_BORDER;
                }

                if (row > 0) {
                    matrix[calculatePosition(row - 1, col)] = BRIGHTNESS_BORDER;
                }

                if (row < MATRIX_HEIGHT - 1) {
                    matrix[calculatePosition(row + 1, col)] = BRIGHTNESS_BORDER;
                }

                if (row > 0 && col > 0) {
                    matrix[calculatePosition(row - 1, col - 1)] = BRIGHTNESS_BORDER;
                }

                if (row > 0 && col < MATRIX_WIDTH - 1) {
                    matrix[calculatePosition(row - 1, col + 1)] = BRIGHTNESS_BORDER;
                }

                if (row < MATRIX_HEIGHT - 1 && col > 0) {
                    matrix[calculatePosition(row + 1, col - 1)] = BRIGHTNESS_BORDER;
                }

                if (row < MATRIX_HEIGHT - 1 && col < MATRIX_WIDTH - 1) {
                    matrix[calculatePosition(row + 1, col + 1)] = BRIGHTNESS_BORDER;
                }
            }

            return matrix;
        }
    };

    broadcast_node<Matrix> secondBroadcast{g};

    function_node<Matrix, Matrix> inversionBrightness{
        g,
        serial,
        [&logFile](Matrix const &matrix) -> Matrix {
            Matrix result;

            result.resize(MATRIX_HEIGHT * MATRIX_WIDTH);

            for (size_t j = 0; j < matrix.size(); ++j) {
                result[j] = (uint8_t) (matrix[j] + 255);
            }

            return result;
        }
    };

    function_node<Matrix, uint8_t> averageBrightness{
        g,
        serial,
        [&logFile](Matrix const &matrix) -> uint8_t {
            size_t sum = 0;

            for (size_t j = 0; j < matrix.size(); ++j) {
                sum += matrix[j];
            }

            return (uint8_t) (sum / matrix.size());
        }
    };

    join_node<MatrixWithAverage> secondJoinNode{g};

    function_node<MatrixWithAverage> finalNode{
        g,
        unlimited,
        [&logFile](MatrixWithAverage const &matrixWithAverage) {
            logFile << "average = "<< (int) get<1>(matrixWithAverage) << endl;
        }
    };

    make_edge(source, limiter);
    make_edge(limiter, firstBroadcast);
    make_edge(firstBroadcast, maxBrightFinder);
    make_edge(firstBroadcast, minBrightFinder);
    make_edge(firstBroadcast, targetBrightFinder);
    make_edge(maxBrightFinder, input_port<0>(firstJoinNode));
    make_edge(minBrightFinder, input_port<1>(firstJoinNode));
    make_edge(targetBrightFinder, input_port<2>(firstJoinNode));
    make_edge(firstJoinNode, joinPositions);
    make_edge(joinPositions, markPositions);
    make_edge(markPositions, secondBroadcast);
    make_edge(secondBroadcast, inversionBrightness);
    make_edge(secondBroadcast, averageBrightness);
    make_edge(inversionBrightness, input_port<0>(secondJoinNode));
    make_edge(averageBrightness, input_port<1>(secondJoinNode));
    make_edge(secondJoinNode, finalNode);
    make_edge(finalNode, limiter.decrement);

    source.activate();
    g.wait_for_all();

    return 0;
}
