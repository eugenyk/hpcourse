#include "stdafx.h"
#include <process.h>
#include "tbb/flow_graph.h"
#include <windows.h>
using namespace tbb::flow;

struct square {
	int operator()(int v) {
		printf("squaring %d\n", v);
		//Sleep(1000);
		return v*v;
	}
};

struct cube {
	int operator()(int v) {
		printf("cubing %d\n", v);
		//Sleep(1000);
		return v*v*v;
	}
};

class sum {
	int &s;

public:
	sum(int &s) : s(s) {}
	int operator()(tuple<int, int> v) {
		printf("summing %d and %d\n", get<0>(v), get<1>(v));
		//Sleep(1000);
		s += get<0>(v) + get<1>(v);
		return s;
	}
};

int main() {
	graph g;
	int result = 0;

	split_node<tuple<int, int> > s(g);
	function_node<int, int> foo1(g, unlimited, square());
	function_node<int, int> foo2(g, unlimited, cube());
	join_node<tuple<int, int>, queueing> join(g);
	function_node<tuple<int, int>, int> summer(g, serial, sum(result));

	make_edge(output_port<0>(s), foo1);
	make_edge(output_port<1>(s), foo2);
	make_edge(foo1, input_port<0>(join));
	make_edge(foo2, input_port<1>(join));
	make_edge(join, summer);

	s.try_put(std::make_tuple(2, 3));
	g.wait_for_all();

	printf("Final result is %d\n", result);
	system("pause");
	return 0;
}