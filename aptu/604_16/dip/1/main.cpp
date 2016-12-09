#include <tbb/flow_graph.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <map>

using namespace tbb::flow;

struct square {
    int operator()(int v) {
        printf("squaring %d\n", v);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return v * v;
    }
};

struct cube {
    int operator()(int v) {
        printf("cubing %d\n", v);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return v * v * v;
    }
};

class sum {
    int &my_sum;
public:
    sum(int &s) : my_sum(s) {}

    int operator()(std::tuple<int, int> v) {
        printf("adding %d and %d to %d\n", std::get<0>(v), std::get<1>(v), my_sum);
        my_sum += std::get<0>(v) + std::get<1>(v);
        return my_sum;
    }
};

int main(int argc, char *argv[]) {
    int result = 0;
    graph g;
    broadcast_node<int> input(g);
    function_node<int, int> squarer(g, unlimited, square());
    function_node<int, int> cuber(g, unlimited, cube());
    buffer_node<int> square_buffer(g);
    buffer_node<int> cube_buffer(g);
    join_node<std::tuple<int, int>, queueing> join(g);
    function_node<std::tuple<int, int>, int> summer(g, serial, sum(result));

    make_edge(input, squarer);
    make_edge(input, cuber);
    make_edge(squarer, square_buffer);
    make_edge(squarer, input_port<0>(join));
    make_edge(cuber, cube_buffer);
    make_edge(cuber, input_port<1>(join));
    make_edge(join, summer);

    for (int i = 1; i <= 10; ++i) {
        input.try_put(i);
    }
    g.wait_for_all();

    printf("Final result is %d\n", result);
    return 0;
}
