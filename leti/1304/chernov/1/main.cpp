#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <memory>
#include <algorithm>
#include "tbb/task_group.h"
#include "tbb/flow_graph.h"

using CommandLineArguments = std::tuple<uint8_t, size_t, size_t, std::string>;

void printUsage(std::ostream& out) {
    out << " Usage: \n";
    out << " -b searched value \n";
    out << " -l parallelism \n";
    out << " -f log path \n";
    out << " -n image count \n";
}

CommandLineArguments parseCommandLineArguments(int argc, const char** argv) {
    uint8_t searchedValue = 0;
    size_t parallelism = 5;
    size_t matrixCount = 100;
    std::string logFilePath = "./result.log";
    bool errorEncountered = false;

    for (size_t i = 1; i < argc; i += 2) {
        std::cout << std::string(argv[i]) << " " << std::string(argv[i + 1]) << std::endl;
        if (std::strcmp(argv[i], "-b") == 0) {
            searchedValue = (uint8_t) std::stoi(argv[i + 1]);
        } else if (std::strcmp(argv[i], "-n") == 0) {
            matrixCount = (size_t) std::stoi(argv[i + 1]);
        } else if (std::strcmp(argv[i], "-l") == 0) {
            parallelism = (size_t) std::stoi(argv[i + 1]);
        } else if (std::strcmp(argv[i], "-f") == 0) {
            logFilePath = argv[i + 1];
        } else {
            errorEncountered = true;
        }
    }

    if (errorEncountered) {
        printUsage(std::cout);
        std::terminate();
    }

    return CommandLineArguments{searchedValue, parallelism, matrixCount, logFilePath};
}

class Matrix {
public:
    using Shape = std::pair<size_t, size_t>;

    Matrix(const Shape& shape) :
            m_shape(shape),
            m_matrix(shape.first * shape.second, 0) {
    }

    const std::pair<size_t, size_t>& getShape() const {
        return m_shape;
    }

    const size_t getLength() const {
        return m_shape.first * m_shape.second;
    }

    const std::vector<uint8_t>& getMatrix() const {
        return m_matrix;
    }

    std::vector<uint8_t>& getMatrix() {
        return m_matrix;
    }

    void operator>>(std::ostream& out) const {
        for (size_t i = 0; i < m_shape.first; ++i) {
            for (size_t j = 0; j < m_shape.second; ++j) {
                out << std::setw(3) << (int) m_matrix[i * m_shape.second + j] << " ";
            }
            out << "\n";
        }
    }

    template<class Generator>
    Generator& operator<<(Generator& g) {
        std::transform(m_matrix.begin(),
                       m_matrix.end(),
                       m_matrix.begin(),
                       [&g](std::uint8_t val) { return g(); });
        return g;
    }

private:
    Shape m_shape;
    std::vector<std::uint8_t> m_matrix;
};

template<class Number>
class RandomNumberGenerator {
public:
    Number operator()() {
        return rd(re);
    }

private:
    std::default_random_engine re = std::default_random_engine(
            (unsigned int) std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<Number> rd;
};

int main(int argc, const char** argv) {
    uint8_t searchedValue;
    size_t parallelism;
    size_t matrixCount;
    std::string logFilePath;

    std::tie(searchedValue, parallelism, matrixCount, logFilePath) = parseCommandLineArguments(argc, argv);

    auto outputStream = std::fstream(logFilePath, std::fstream::out | std::fstream::trunc);

    using namespace tbb;
    flow::graph graph;

    size_t currentTaskId = 0;
    flow::source_node<Matrix::Shape> taskSourceNode(graph,
                                                    [&currentTaskId, matrixCount](Matrix::Shape& shape) mutable {
                                                        shape.first = 100;
                                                        shape.second = 100;
                                                        return currentTaskId++ < matrixCount;
                                                    });
    flow::limiter_node<Matrix::Shape> limiterNode(graph, parallelism);

    flow::function_node<
            Matrix::Shape,
            std::shared_ptr<Matrix>> generateMatrixNode(graph,
                                                        flow::unlimited,
                                                        [](const Matrix::Shape& shape) {
                                                            RandomNumberGenerator<std::uint8_t> generator;
                                                            auto matrix = std::shared_ptr<Matrix>(new Matrix(shape));
                                                            *matrix << generator;

                                                            return matrix;
                                                        });

    flow::broadcast_node<std::shared_ptr<Matrix>> broadcastNode(graph);

    flow::function_node<
            std::shared_ptr<Matrix>,
            std::vector<size_t>> minValue(graph, flow::unlimited,
                                          [](std::shared_ptr<Matrix> inputMatrix) {
                                              const auto& data = inputMatrix->getMatrix();
                                              uint8_t min = data[0];

                                              std::vector<size_t> indexes;
                                              for (size_t i = 1; i < inputMatrix->getLength(); i++) {
                                                  auto currentValue = data[i];
                                                  if (currentValue < min) {
                                                      min = currentValue;
                                                      indexes.clear();
                                                      indexes.emplace_back(i);
                                                      continue;
                                                  } else if (currentValue == min) {
                                                      indexes.emplace_back(i);
                                                  }
                                              }

                                              return indexes;
                                          });

    flow::function_node<
            std::shared_ptr<Matrix>,
            std::vector<size_t>> maxValue(graph, flow::unlimited,
                                          [](std::shared_ptr<Matrix> inputMatrix) {
                                              const auto& data = inputMatrix->getMatrix();
                                              uint8_t max = data[0];

                                              std::vector<size_t> indexes;
                                              for (size_t i = 1; i < inputMatrix->getLength(); i++) {
                                                  auto currentValue = data[i];
                                                  if (currentValue > max) {
                                                      max = currentValue;
                                                      indexes.clear();
                                                      indexes.emplace_back(i);
                                                      continue;
                                                  } else if (currentValue == max) {
                                                      indexes.emplace_back(i);
                                                  }
                                              }

                                              return indexes;
                                          });

    flow::function_node<
            std::shared_ptr<Matrix>,
            std::vector<size_t>> equalToValue(graph, flow::unlimited,
                                              [searchedValue](std::shared_ptr<Matrix> inputMatrix) {
                                                  const auto& data = inputMatrix->getMatrix();
                                                  uint8_t max = data[0];

                                                  std::vector<size_t> indexes;
                                                  for (size_t i = 1; i < inputMatrix->getLength(); i++) {
                                                      auto currentValue = data[i];
                                                      if (currentValue == searchedValue) {
                                                          indexes.emplace_back(i);
                                                      }
                                                  }

                                                  return indexes;
                                              });


    using HighlightInput =flow::tuple<
            std::shared_ptr<Matrix>,
            std::vector<size_t>,
            std::vector<size_t>,
            std::vector<size_t>>;

    flow::join_node<HighlightInput> joinNode(graph);

    flow::function_node<
            HighlightInput,
            std::shared_ptr<Matrix>> highlight_node(graph, flow::unlimited, [](HighlightInput highlightInput) {

        auto highlighter = [](std::shared_ptr<Matrix> matrix, const std::vector<size_t>& indexes) {
            auto& data = matrix->getMatrix();
            auto shape = matrix->getShape();
            auto length = matrix->getLength();

            size_t rows = shape.first;
            size_t columns = shape.second;

            for (auto index : indexes) {
                auto row = index / columns;
                auto column = index % columns;

                if (row + 1 != rows) {
                    if (column != 0) {
                        data[(row + 1) * columns + column - 1] = 0;
                    }
                    data[(row + 1) * columns + column] = 0;
                    if (column + 1 != columns) {
                        data[(row + 1) * columns + column + 1] = 0;
                    }
                }

                if (column != 0)data[row * columns + column - 1] = 0;
                data[row * columns + column] = 0;
                if (column + 1 != columns)data[row * columns + column + 1] = 0;

                if (row != 0) {
                    if (column != 0)data[(row - 1) * rows + column - 1] = 0;
                    data[(row - 1) * columns + column] = 0;
                    if (column + 1 != columns)data[(row - 1) * columns + column + 1] = 0;
                }
            }
        };

        auto matrix = flow::get<0>(highlightInput);
        highlighter(matrix, flow::get<1>(highlightInput));
        highlighter(matrix, flow::get<2>(highlightInput));
        highlighter(matrix, flow::get<3>(highlightInput));

        return matrix;
    });

    flow::broadcast_node<std::shared_ptr<Matrix>> broadcastNode1(graph);

    flow::function_node<
            std::shared_ptr<Matrix>,
            std::shared_ptr<Matrix>> inverseNode(graph,
                                                 flow::unlimited,
                                                 [](std::shared_ptr<Matrix> matrix) {
                                                     std::shared_ptr<Matrix> inverseMatrix = std::make_shared<Matrix>(
                                                             matrix->getShape());

                                                     auto& input = matrix->getMatrix();
                                                     auto& output = inverseMatrix->getMatrix();

                                                     for (size_t i = 0; i < matrix->getLength(); ++i) {
                                                         output[i] = std::numeric_limits<uint8_t>::max() - input[i];
                                                     }

                                                     return inverseMatrix;
                                                 });

    flow::function_node<
            std::shared_ptr<Matrix>,
            double> meanNode(graph,
                             flow::unlimited,
                             [](std::shared_ptr<Matrix> matrix) {
                                 const auto& data = matrix->getMatrix();
                                 auto length = matrix->getLength();

                                 double mean = 0.0;
                                 for (size_t i = 0; i < length; ++i) {
                                     mean += data[i];
                                 }
                                 return mean / length;
                             });

    flow::function_node<std::shared_ptr<Matrix>> matrixDebugOutput(graph,
                                                                   flow::serial,
                                                                   [](std::shared_ptr<Matrix> matrix) {
                                                                       *matrix >> std::cout;
                                                                       std::cout << "\n";
                                                                   });

    flow::function_node<double> doubleOutputNode(graph,
                                                 flow::serial,
                                                 [&outputStream](double mean) {
                                                     outputStream << std::setprecision(10) << mean << "\n";
                                                 });

    flow::function_node<std::shared_ptr<Matrix>> decrementer(graph, flow::serial, [](std::shared_ptr<Matrix> val) {});

    flow::make_edge(taskSourceNode, limiterNode);
    flow::make_edge(limiterNode, generateMatrixNode);
    flow::make_edge(generateMatrixNode, broadcastNode);
    flow::make_edge(broadcastNode, minValue);
    flow::make_edge(broadcastNode, maxValue);
    flow::make_edge(broadcastNode, equalToValue);

    flow::make_edge(broadcastNode, flow::input_port<0>(joinNode));
    flow::make_edge(minValue, flow::input_port<1>(joinNode));
    flow::make_edge(maxValue, flow::input_port<2>(joinNode));
    flow::make_edge(equalToValue, flow::input_port<3>(joinNode));

    flow::make_edge(joinNode, highlight_node);
    flow::make_edge(highlight_node, broadcastNode1);

    flow::make_edge(broadcastNode1, decrementer);
    flow::make_edge(decrementer, limiterNode.decrement);

    flow::make_edge(broadcastNode1, meanNode);
    flow::make_edge(broadcastNode1, inverseNode);

    flow::make_edge(inverseNode, matrixDebugOutput);
    flow::make_edge(meanNode, doubleOutputNode);

    graph.wait_for_all();

    outputStream.close();

    return 0;
}
