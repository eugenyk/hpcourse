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

struct CommandLineArguments
{
    uint8_t searchedValue = 0;
    size_t parallelism = 5;
    size_t matrixCount = 100;
    size_t matrixWidth = 100;
    size_t matrixHeight = 100;
    bool shouldPrint = false;
    std::string logFilePath = "./result.log";
};

void printUsage(std::ostream& out)
{
    out << " Usage: \n";
    out << " -b searched value \n";
    out << " -l parallelism \n";
    out << " -f log path \n";
    out << " -n image count \n";
}

CommandLineArguments parseCommandLineArguments(int argc, const char** argv)
{
    CommandLineArguments arguments;
    bool errorEncountered = false;

    for (size_t i = 1; i < argc; i += 2)
    {
        if (std::strcmp(argv[i], "-b") == 0)
        {
            arguments.searchedValue = (uint8_t) std::stoi(argv[i + 1]);
        }
        else if (std::strcmp(argv[i], "-n") == 0)
        {
            arguments.matrixCount = (size_t) std::stoi(argv[i + 1]);
        }
        else if (std::strcmp(argv[i], "-l") == 0)
        {
            arguments.parallelism = (size_t) std::stoi(argv[i + 1]);
        }
        else if (std::strcmp(argv[i], "-f") == 0)
        {
            arguments.logFilePath = argv[i + 1];
        }
        else if (std::strcmp(argv[i], "-w") == 0)
        {
            arguments.matrixWidth = (size_t) std::stoi(argv[i + 1]);
        }
        else if (std::strcmp(argv[i], "-h") == 0)
        {
            arguments.matrixHeight = (size_t) std::stoi(argv[i + 1]);
        }
        else if (std::strcmp(argv[i], "-v") == 0)
        {
            arguments.shouldPrint = true;
            --i;
        }
        else
        {
            errorEncountered = true;
        }
    }

    if (errorEncountered)
    {
        printUsage(std::cout);
        std::terminate();
    }

    return arguments;
}

class Matrix
{
public:
    using Value = uint8_t;
    using Shape = std::pair<size_t, size_t>;

    Matrix(const Shape& shape) :
            m_shape(shape),
            m_matrix(shape.first * shape.second, 0)
    {
    }

    const std::pair<size_t, size_t>& getShape() const
    {
        return m_shape;
    }

    const size_t getLength() const
    {
        return m_shape.first * m_shape.second;
    }

    const std::vector<Value>& getMatrix() const
    {
        return m_matrix;
    }

    std::vector<Value>& getMatrix()
    {
        return m_matrix;
    }

    void operator>>(std::ostream& out) const
    {
        for (size_t i = 0; i < m_shape.first; ++i)
        {
            for (size_t j = 0; j < m_shape.second; ++j)
            {
                out << std::setw(3) << (int) m_matrix[i * m_shape.second + j] << " ";
            }
            out << "\n";
        }
    }

    template <class Generator>
    Generator& operator<<(Generator& g)
    {
        std::transform(m_matrix.begin(),
                       m_matrix.end(),
                       m_matrix.begin(),
                       [&g](Value val)
                       { return g(); });
        return g;
    }

private:
    Shape m_shape;
    std::vector<Value> m_matrix;
};

class TaskDescription
{
public:
    size_t getTaskId() const
    {
        return m_taskId;
    }

    void setTaskId(size_t taskId)
    {
        m_taskId = taskId;
    }

    const Matrix::Shape& getShape() const
    {
        return m_shape;
    }

    void setShape(const Matrix::Shape& shape)
    {
        m_shape = shape;
    }

private:
    size_t m_taskId;
    Matrix::Shape m_shape;
};

template <class IdType>
class IdGenerator
{

public:
    IdGenerator(IdType id) : id(id)
    {
    }

    IdType operator()()
    {
        return id++;
    }

private:
    IdType id;
};

template <class Number>
class RandomNumberGenerator
{
public:
    Number operator()()
    {
        return rd(re);
    }

private:
    std::default_random_engine re = std::default_random_engine(
            (unsigned int) std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<Number> rd;
};

using IdType = size_t;
using MatrixPtr = std::shared_ptr<Matrix>;
using MatrixWithId = std::pair<IdType, MatrixPtr>;
using ParticialResults = std::pair<IdType, std::vector<size_t>>;

int main(int argc, const char** argv)
{
    CommandLineArguments arguments = parseCommandLineArguments(argc, argv);

    auto outputStream = std::fstream(arguments.logFilePath, std::fstream::out | std::fstream::trunc);

    using namespace tbb;
    flow::graph graph;

    IdGenerator<size_t> idGenerator(0);
    flow::source_node<TaskDescription> taskSourceNode(graph,
                                                      [&arguments, &idGenerator](TaskDescription& task)
                                                      {
                                                          size_t currentTaskId = idGenerator();
                                                          Matrix::Shape matrixShape(arguments.matrixWidth,
                                                                                    arguments.matrixHeight);

                                                          task.setTaskId(currentTaskId);
                                                          task.setShape(matrixShape);

                                                          return currentTaskId++ < arguments.matrixCount;
                                                      });
    flow::limiter_node<TaskDescription> limiterNode(graph, arguments.parallelism);

    flow::function_node<
            TaskDescription,
            MatrixWithId> generateMatrixNode(graph,
                                             flow::unlimited,
                                             [](const TaskDescription& task)
                                             {
                                                 RandomNumberGenerator<Matrix::Value> generator;
                                                 auto matrix = std::make_shared<Matrix>(task.getShape());
                                                 *matrix << generator;

                                                 return std::make_pair(task.getTaskId(), matrix);
                                             });

    flow::broadcast_node<MatrixWithId> broadcastNode(graph);

    flow::function_node<
            MatrixWithId,
            ParticialResults> minValue(graph, flow::unlimited,
                                       [](const MatrixWithId& task)
                                       {
                                           const auto& inputMatrix = task.second;
                                           const auto& data = inputMatrix->getMatrix();

                                           auto min = data[0];
                                           std::vector<size_t> indexes{min};

                                           for (size_t i = 1; i < inputMatrix->getLength(); i++)
                                           {
                                               auto currentValue = data[i];
                                               if (currentValue < min)
                                               {
                                                   min = currentValue;
                                                   indexes.clear();
                                                   indexes.emplace_back(i);
                                                   continue;
                                               }
                                               else if (currentValue == min)
                                               {
                                                   indexes.emplace_back(i);
                                               }
                                           }

                                           return std::make_pair(task.first, std::move(indexes));
                                       });

    flow::function_node<
            MatrixWithId,
            ParticialResults> maxValue(graph, flow::unlimited,
                                       [](const MatrixWithId& task)
                                       {
                                           const auto& inputMatrix = task.second;
                                           const auto& data = inputMatrix->getMatrix();

                                           auto max = data[0];
                                           std::vector<size_t> indexes{max};

                                           for (size_t i = 1; i < inputMatrix->getLength(); i++)
                                           {
                                               auto currentValue = data[i];
                                               if (currentValue > max)
                                               {
                                                   max = currentValue;
                                                   indexes.clear();
                                                   indexes.emplace_back(i);
                                                   continue;
                                               }
                                               else if (currentValue == max)
                                               {
                                                   indexes.emplace_back(i);
                                               }
                                           }

                                           return std::make_pair(task.first, std::move(indexes));
                                       });

    flow::function_node<
            MatrixWithId,
            ParticialResults> equalToValue(graph, flow::unlimited,
                                           [&arguments](const MatrixWithId& task)
                                           {
                                               const auto& inputMatrix = task.second;
                                               const auto& data = inputMatrix->getMatrix();

                                               std::vector<size_t> indexes;

                                               for (size_t i = 0; i < inputMatrix->getLength(); i++)
                                               {
                                                   auto currentValue = data[i];
                                                   if (currentValue == arguments.searchedValue)
                                                   {
                                                       indexes.emplace_back(i);
                                                   }
                                               }

                                               return std::make_pair(task.first, std::move(indexes));
                                           });


    using HighlightInput =flow::tuple<
            MatrixWithId,
            ParticialResults,
            ParticialResults,
            ParticialResults>;

    flow::join_node<HighlightInput, flow::tag_matching> joinNode(
            graph,
            [](const MatrixWithId& matrixWithId) { return matrixWithId.first; },
            [](const ParticialResults& results) { return results.first; },
            [](const ParticialResults& results) { return results.first; },
            [](const ParticialResults& results) { return results.first; });

    flow::function_node<HighlightInput, std::shared_ptr<Matrix>> highlight_node(
            graph,
            flow::unlimited,
            [](HighlightInput highlightInput)
            {
                auto highlighter = [](MatrixPtr matrix, const std::vector<size_t>& indexes)
                {
                    auto& data = matrix->getMatrix();
                    auto shape = matrix->getShape();
                    auto length = matrix->getLength();

                    size_t rows = shape.first;
                    size_t columns = shape.second;

                    for (auto index : indexes)
                    {
                        auto row = index / columns;
                        auto column = index % columns;

                        if (row + 1 != rows)
                        {
                            if (column != 0)
                            {
                                data[(row + 1) * columns + column - 1] = 0;
                            }
                            data[(row + 1) * columns + column] = 0;
                            if (column + 1 != columns)
                            {
                                data[(row + 1) * columns + column + 1] = 0;
                            }
                        }

                        if (column != 0)
                        {
                            data[row * columns + column - 1] = 0;
                        }
                        data[row * columns + column] = 0;
                        if (column + 1 != columns)
                        {
                            data[row * columns + column + 1] = 0;
                        }

                        if (row != 0)
                        {
                            if (column != 0)
                            {
                                data[(row - 1) * rows + column - 1] = 0;
                            }
                            data[(row - 1) * columns + column] = 0;
                            if (column + 1 != columns)
                            {
                                data[(row - 1) * columns + column + 1] = 0;
                            }
                        }
                    }
                };

                auto matrix = flow::get<0>(highlightInput).second;
                highlighter(matrix, flow::get<1>(highlightInput).second);
                highlighter(matrix, flow::get<2>(highlightInput).second);
                highlighter(matrix, flow::get<3>(highlightInput).second);

                return matrix;
            });

    flow::broadcast_node<std::shared_ptr<Matrix>> broadcastNode1(graph);

    flow::function_node<
            std::shared_ptr<Matrix>,
            std::shared_ptr<Matrix>> inverseNode(graph,
                                                 flow::unlimited,
                                                 [](std::shared_ptr<Matrix> matrix)
                                                 {
                                                     std::shared_ptr<Matrix> inverseMatrix = std::make_shared<Matrix>(
                                                             matrix->getShape());

                                                     auto& input = matrix->getMatrix();
                                                     auto& output = inverseMatrix->getMatrix();

                                                     for (size_t i = 0; i < matrix->getLength(); ++i)
                                                     {
                                                         output[i] = std::numeric_limits<Matrix::Value>::max() - input[i];
                                                     }

                                                     return inverseMatrix;
                                                 });

    flow::function_node<
            std::shared_ptr<Matrix>,
            double> meanNode(graph,
                             flow::unlimited,
                             [](std::shared_ptr<Matrix> matrix)
                             {
                                 const auto& data = matrix->getMatrix();
                                 auto length = matrix->getLength();

                                 double mean = 0.0;
                                 for (size_t i = 0; i < length; ++i)
                                 {
                                     mean += data[i];
                                 }
                                 return mean / length;
                             });

    flow::function_node<std::shared_ptr<Matrix>> matrixDebugOutput(graph,
                                                                   flow::serial,
                                                                   [&arguments](MatrixPtr matrix)
                                                                   {
                                                                       if (arguments.shouldPrint)
                                                                       {
                                                                           *matrix >> std::cout;
                                                                           std::cout << "\n";
                                                                       }
                                                                   });

    flow::function_node<double> doubleOutputNode(graph,
                                                 flow::serial,
                                                 [&outputStream](double mean)
                                                 {
                                                     outputStream << std::setprecision(10) << mean << "\n";
                                                 });

    flow::function_node<std::shared_ptr<Matrix>> decrementer(graph, flow::serial, [](std::shared_ptr<Matrix> val)
    {});

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