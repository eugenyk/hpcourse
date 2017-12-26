#include <bits/stdc++.h>

#include <tbb/tbb.h>

#include "elements.h"

using namespace tbb::flow;

//struct square {
//    int operator()(int v) {
//        printf("squaring %d\n", v);
//        sleep(1);
//        return v*v;
//    }
//};
//
//struct cube {
//    int operator()(int v) {
//        printf("cubing %d\n", v);
//        sleep(1);
//        return v*v*v;
//    }
//};
//
//class sum {
//    int &my_sum;
//public:
//    sum( int &s ) : my_sum(s) {}
//    int operator()( std::tuple<int,int> v ) {
//        printf("adding %d and %d to %d\n", std::get<0>(v), std::get<1>(v), my_sum);
//        my_sum += std::get<0>(v) + std::get<1>(v);
//        return my_sum;
//    }
//};

std::string journal_filename;
int brightness_interesting = -1;
int images_limit = 0;

#define STR(x) #x
#define XSTR(x) STR(x)

void parse_args(int argc, char **argv) {
//    “-b 123”: интересующее значение яркости, для шага № 2
//    “-l 5”: предел одновременно обрабатываемых приложением изображений
//    “-f log.txt”: имя файла журнала яркостей
    
    for (int i = 0; i < argc; ++argv) {
        const std::string args = argv[i];
        if (i + 1 == argc) {
            throw std::logic_error("Need to specify value for argument: " + args);
        }
        if (args == "-b") {
            brightness_interesting = atoi(argv[i + 1]);
        } else if (args == "-l") {
            images_limit = atoi(argv[i + 1]);
        } else if (args == "-f") {
            journal_filename = argv[i + 1];
        } else {
            throw std::logic_error("Wrong argument: " + args);
        }
        ++i;
    }
}

struct ImageHighlighter {
    explicit ImageHighlighter(const Image &image)
            : image_(image)
    {}
    
    Image operator()(const std::vector<Position> &positions) {
        return image_.clone();
    }
    
private:
    const Image &image_;
};

int main(int argc, char **argv) {
//    На вход подаются генерированные случайным образом изображения

//    Различные алгоритмы ищут || на изображении:
//    Набор максимальных элементов (максимальная яркость)
//    Набор минимальных элементов (минимальная яркость)
//    Набор элементов, равных заданному в командной строке значению (0-255)

//    По результатам предыдущего шага на избражении выделяются все найденные элементы (например, квадрат с максимальной яркостью вокруг точки)
//    Результат предудущего шага передаётся на:
//    Расчёт обратного изображения (инверсия яркости)
//    Расчёт средней яркости изображения
//    Результат расчёта средней яркости выводится в файл в произвольном формате, если указан соответствующий флаг запуска приолжения
//    Результат расчёта обратного изображения никуда не идёт

// Предполагается использовать:
//    broadcast_node
//    join_node
//    function_node
//    limiter_node
//    …
//    Приложение принимает в качестве параметров:
//
//    “-b 123”: интересующее значение яркости, для шага № 2
//    “-l 5”: предел одновременно обрабатываемых приложением изображений
//    “-f log.txt”: имя файла журнала яркостей

//    broadcast_node - посылает сообщение всем приконнектившимся.
//    join_node -
//    function_node
//    limiter_node
    
    
//    int result = 0;
//
//    graph g;
//    broadcast_node<int> input (g);
//    function_node<int,int> squarer( g, unlimited, square() );
//    function_node<int,int> cuber( g, unlimited, cube() );
//    buffer_node<int> square_buffer(g);
//    buffer_node<int> cube_buffer(g);
//    join_node< std::tuple<int,int>, queueing > join(g);
//    function_node<std::tuple<int,int>,int>
//            summer( g, serial, sum(result) );
//
//    make_edge( input, squarer );
//    make_edge( input, cuber );
//    make_edge( squarer, square_buffer );
//    make_edge( squarer, input_port<0>(join) );
//    make_edge( cuber, cube_buffer );
//    make_edge( cuber, input_port<1>(join)		);
//    make_edge( join, summer );
//
//    for (int i = 1; i <= 10; ++i)
//        input.try_put(i);
//    g.wait_for_all();
//
//    printf("Final result is %d\n", result);
    return 0;
}
