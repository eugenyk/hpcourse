#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <time.h>

#include <tbb/tbb.h>
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include <cmath>
#include <numeric>
#include <cassert>
#include <set>
#include <iomanip>

using namespace tbb;


struct{
	int exactlyBrightness;
	int maxParallelImages;
	char* log = nullptr;
} settings;

typedef std::pair<int, int> pii;

inline bool parse_args(int argc, char * argv[]) {
	if ((argc != 7) && (argc != 5)){
		std::cerr << "We expected exactly 5 or 7 arguments, but only " <<  argc-1 << "was passed\n";
		return false;
	}
	int current = 1;
	while (current < argc) {
		if (!strcmp(argv[current], "-b")) {
			settings.exactlyBrightness = atoi(argv[++current]);
			++current;
		}
		else if (!strcmp(argv[current], "-l")) {
			settings.maxParallelImages = atoi(argv[++current]);
			++current;
		}
		else if (!strcmp(argv[current], "-f")) {
			if (argc != 7) {
				std::cerr << "We expected exactly 7 arguments if you pass -f, but only " << argc - 1 << "was passed\n";
				return false;
			}
			settings.log = argv[++current];
			++current;
		}
		else {
			std::cerr << "Unknow argument [" <<argv[current] << "] was passed\n";
			return false;
		}
	}
	return true;
}


bool readtask(std::vector <int> &a, size_t &N, size_t &M) {
	while (std::cin >> N) {
		std::cin >> M;
		a.assign(N* M, 0);
		for (size_t i = 0; i < N; ++i) {
			for (size_t j = 0; j < M; ++j) {
				std::cin >> a[i* M + j];
			}
		}
		return true;
	}
	return false;
}


template <class T> struct max : std::binary_function <T, T, T> {
	T operator() (const T& x, const T& y) const { return x < y ? y : x; }
};

template <class T> struct min : std::binary_function <T, T, T> {
	T operator() (const T& x, const T& y) const { return x < y ? x : y; }
};


struct calculationResult {
	double medium;
};
std::vector<int> empty_vector;

struct rowMessage {
	std::vector<int> a; /* input vector */
	calculationResult *result; /* result vector */
	int id;
	int M;
	int N;
	rowMessage(int id, std::vector<int> a, calculationResult* result, int N, int M) : a(a), result(result), id(id), M(M), N(N) { }
	rowMessage() : a(empty_vector), result(nullptr), id(-1), M(0), N(0) {}
};

template <class T>
struct maxOrMin {
	int id; /* id of image */
	std::vector<int> a; /* input vector */
	calculationResult *result; /* result target pointer */
	size_t M; /* count columns */
	T value; /* max or min */
	maxOrMin(int id, const std::vector<int> &a, calculationResult* result, size_t M, T value) : id(id), a(a), result(result), M(M), value(value) { };
	maxOrMin() : id(-1), a(empty_vector), result(nullptr), M(-1), value(-1) {};
	maxOrMin& operator=(const maxOrMin& right) {
		//check on self assignment
		if (this == &right) {
			return *this;
		}
		id = right.id;
		result=right.result;
		M = right.M;
		a = right.a;
		value = right.value;
		return *this;
	}
};

template <class T>
struct maxAndMin {
	int id; /* id of image */
	std::vector<int> a; /* input vector */
	calculationResult* result; /* result target pointer */
	size_t M;  /* count columns */
	T _max;
	T _min;
	maxAndMin(int id, const std::vector<int> &a, calculationResult* result, size_t M, T _max, T _min) : id(id), a(a), result(result), M(M), _max(_max), _min(_min) {};
	maxAndMin() : id(-1), a(empty_vector), result(nullptr), M(-1), _max(INT_MIN), _min(INT_MAX) {};
};

template <class T>
struct arrayQuery {
	int id; /* id of image */
	std::vector<int> a; /* input vector */
	calculationResult* result; /* result target pointer */
	size_t M;  /* count columns */
	arrayQuery(int id, const std::vector<int> &a, calculationResult* result, size_t M) : id(id), a(a),  result(result), M(M) {};
	arrayQuery() : id(-1), a(empty_vector), result(nullptr), M(-1) {};
};

template <class T>
struct rowMaxer {
	maxOrMin<T> operator()(const rowMessage &mes) {
		int res =  std::accumulate(mes.a.begin(), 
										mes.a.end(), 
										*mes.a.begin(), 
										max<T>());
		return maxOrMin<T>(mes.id, mes.a, mes.result, mes.M, res);
	}
};

template <class T>
struct rowMiner {
	maxOrMin<T> operator()(const rowMessage &mes) {
		T res =  std::accumulate(mes.a.begin(), 
									mes.a.end(),
									*mes.a.begin(),
									min<T>());
		return maxOrMin<T>(mes.id, mes.a, mes.result, mes.M,  res);
	}
};


struct rowEquals {
public:
	rowEquals() {}
	arrayQuery<int> operator()(const maxAndMin<int> &mes) {
		std::set<int> important = {mes._max, mes._min, settings.exactlyBrightness};
		std::vector<int> result = mes.a;
		const int maxIndex = mes.a.size();
		int index = 0;
		std::for_each(	mes.a.begin(), 
							mes.a.end(), 
			[this, &important, &index, &mes, &result, &maxIndex](int x)
		{
			if (important.find(x) != important.end()) {
				visitValue(result, important, index, 0, -1, mes.M, maxIndex);
				visitValue(result, important, index, 0, +1, mes.M, maxIndex);
				visitValue(result, important, index, -1, -1, mes.M, maxIndex);
				visitValue(result, important, index, -1, -0, mes.M, maxIndex);
				visitValue(result, important, index, -1, 1, mes.M, maxIndex);
				visitValue(result, important, index, 1, -1, mes.M, maxIndex);
				visitValue(result, important, index, 1, -0, mes.M, maxIndex);
				visitValue(result, important, index, 1, 1, mes.M, maxIndex);
			}
			++index;
		});
		return arrayQuery<int>(mes.id, result, mes.result, mes.M);
	}

	static inline void visitValue(std::vector<int> &a, const std::set<int> &important, const int index, const int x, const int y, const int M,const int maxIndex) {
		const int currentX = index % M;
		const int targetIndex = index + y * M + x;
		if ((targetIndex >=0) && (currentX + x >= 0) && (currentX + x < M) && (targetIndex < maxIndex)) {
			if (important.find(a[targetIndex]) == important.end()) {
				a[targetIndex] = 255;
			}
		}
	}
};


struct rowReverse {
public:
	rowReverse() {}
	int operator()(const arrayQuery<int> &mes) const {
		std::vector<int> result(mes.a.size());
		std::transform(mes.a.begin(),
			mes.a.end(),
			result.begin(),
			[](int x) {return 255 - x; });
		// arrayQuery<int>(mes.id, result, mes.result, mes.M); /* answer not going anywhere */
		return mes.id; /* id for join tag */
	}
};

struct mediumAnswer {
	int id;
	double res;
	mediumAnswer(int id, double res) : id(id), res(res) {};
	mediumAnswer(): id(-1), res(0) {};
};

template <class T>
struct rowMedium {
	int operator()(const arrayQuery<T> &mes) const {
		T sum = std::accumulate(mes.a.begin(),
			mes.a.end(),
			0,
			std::plus<T>());
		double res = ((1.0 * sum) / mes.a.size());
		mes.result->medium = res;
		// mediumAnswer(mes.id, res); /* answer already writed to pointer */
		return mes.id; /* id for join tag */
	}
};

template<class T>
struct totalMaxMin {
	totalMaxMin() {}
	maxAndMin<T> operator()(std::tuple</*  max  */ maxOrMin<T>, /*   min  */maxOrMin<T>> v) {
		T my_max = std::get<0>(v).value;
		T my_min = std::get<1>(v).value;
		return maxAndMin<T>(std::get<0>(v).id, std::get<0>(v).a, std::get<0>(v).result, std::get<0>(v).M, my_max, my_min);
	}
};



inline void printMatrix(std::vector<int> &a, int &M) {
	for (size_t i = 0; i < a.size(); ++i) {
		std::cout << std::setfill('0') << std::setw(3) << a[i] << " ";
		if ((i+ 1) % M == 0) {
			std::cout << std::endl;
		}
	}
	std::cout << std::endl  << "===" << std::endl;
}

int main(int argc, char * argv[]) {
	assert(freopen("input.txt", "r+", stdin));

	std::ios::sync_with_stdio(false);
	std::cin.tie(0);

	//Initialise task scheduler as automatic (default).
	task_scheduler_init init;

	parse_args(argc, argv);

	if (settings.log != nullptr) {
		assert(freopen("output.txt", "w+", stdout));
	}


	flow::graph g;
	flow::broadcast_node<rowMessage> input(g);
	flow::buffer_node<rowMessage> inputBuffer(g);
	flow::function_node<rowMessage, maxOrMin<int>> maxer(g, flow::unlimited, rowMaxer<int>());
	flow::function_node<rowMessage, maxOrMin<int>> miner(g, flow::unlimited, rowMiner<int>());

	flow::buffer_node<maxOrMin<int>> maxerBuffer(g);
	flow::buffer_node<maxOrMin<int>> minerBuffer(g);


	flow::join_node< std::tuple<maxOrMin<int>, maxOrMin<int>>, flow::tag_matching > join_maxMin(g,
		[](const maxOrMin<int> &p) -> size_t { return (size_t)p.id; },
		[](const maxOrMin<int> &p) -> size_t { return (size_t)p.id; });
	flow::function_node<std::tuple<maxOrMin<int>, maxOrMin<int>>, maxAndMin<int>>
		maxMinimizer(g, flow::serial, totalMaxMin<int>());


	flow::limiter_node< rowMessage > limiter(g, settings.maxParallelImages);

	flow::make_edge(input, inputBuffer);
	flow::make_edge(inputBuffer, limiter);
	
	flow::make_edge(limiter, maxer);
	flow::make_edge(limiter, miner);

	flow::make_edge(maxer, maxerBuffer);
	flow::make_edge(miner, minerBuffer);


	flow::make_edge(maxerBuffer, flow::input_port<0>(join_maxMin));
	flow::make_edge(minerBuffer, flow::input_port<1>(join_maxMin));

	flow::make_edge(join_maxMin, maxMinimizer);

	flow::function_node<maxAndMin<int>, arrayQuery<int>> equals1(g, flow::unlimited, rowEquals());

	flow::function_node<arrayQuery<int>, int> mediumer(g, flow::unlimited, rowMedium<int>());
	flow::function_node<arrayQuery<int>, int> reverser(g, flow::unlimited, rowReverse());


	flow::make_edge(maxMinimizer, equals1);
	flow::make_edge(equals1, mediumer);
	flow::make_edge(equals1, reverser);


	flow::join_node< std::tuple<int, int>, flow::tag_matching > join_final(g,
		[](const int &p) -> size_t { return (size_t)p; },
		[](const int &p) -> size_t { return (size_t)p; });

	flow::make_edge(mediumer, flow::input_port<0>(join_final));
	flow::make_edge(reverser, flow::input_port<1>(join_final));

	flow::function_node<std::tuple<int, int>, flow::continue_msg > decrementManager(g, flow::unlimited,
			[](std::tuple<int, int> p) -> flow::continue_msg {
		return flow::continue_msg();
	});

	flow::make_edge(join_final, decrementManager);

	flow::make_edge(decrementManager, limiter.decrement);
	

	std::vector < calculationResult > result;
	size_t N = 0, M = 0;
	size_t cntTasks;
	std::cin >> cntTasks;
	result.resize(cntTasks);

	for (size_t i = 0; i < cntTasks; ++i) {
		std::vector < int > a;
		readtask(a, N, M);

		auto message = rowMessage(i, a, &result[i], N, M);
		while(!input.try_put(message)) {
			Sleep(100);
		}
	}

	g.wait_for_all();
	for (size_t i = 0; i < cntTasks; ++i) {
		std::cout << "id = " << i << "; medium =" << result[i].medium << std::endl;
	}
	

	return 0;
}