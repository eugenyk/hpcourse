#include "tbb/flow_graph.h"
#include "BodyObjects.h"

using namespace std;
using namespace tbb::flow;

typedef function_node<tuple<vector<Pixel>, vector<Pixel>, vector<Pixel>, ImagePtr>, ImagePtr> InvConvType;
typedef function_node<tuple<vector<Pixel>, vector<Pixel>, vector<Pixel>, ImagePtr>, tuple<int, int>> AvgConvType;

graph g;
Params params;

struct ProcessingBranch
{
	broadcast_node<ImagePtr> * input;
	function_node<ImagePtr, vector<Pixel>> * min;
	function_node<ImagePtr, vector<Pixel>> * max;
	function_node<ImagePtr, vector<Pixel>> * equ;
	function_node<ImagePtr, ImagePtr> * inv;
	function_node<tuple<int, int>, int> * avg;
	join_node<tuple<vector<Pixel>, vector<Pixel>, vector<Pixel>, ImagePtr>, queueing> * join;
	InvConvType * invConverter;
	AvgConvType * avgConverter;

	ProcessingBranch()
	{
		input = new broadcast_node<ImagePtr>(g);
		min = new function_node<ImagePtr, vector<Pixel>>(g, unlimited, MinBrightness());
		max = new function_node<ImagePtr, vector<Pixel>>(g, unlimited, MaxBrightness());
		equ = new function_node<ImagePtr, vector<Pixel>>(g, unlimited, EquBrightness(params.b));

		inv = new function_node<ImagePtr, ImagePtr>(g, unlimited, Inverser());
		avg = new function_node<tuple<int, int>, int>(g, unlimited, Average());

		join = new join_node<tuple<vector<Pixel>, vector<Pixel>, vector<Pixel>, ImagePtr>, queueing>(g);

		invConverter = new InvConvType(g, unlimited, InvConverter());
		avgConverter = new AvgConvType(g, unlimited, AvgConverter());

		make_edge(*input, *min);
		make_edge(*input, *max);
		make_edge(*input, *equ);
		make_edge(*min, get<0>(join->input_ports()));
		make_edge(*max, get<1>(join->input_ports()));
		make_edge(*equ, get<2>(join->input_ports()));
		make_edge(*input, get<3>(join->input_ports()));

		make_edge(*join, *invConverter);
		make_edge(*join, *avgConverter);

		make_edge(*invConverter, *inv);
		make_edge(*avgConverter, *avg);
	}

	~ProcessingBranch()
	{
		delete input;
		delete min;
		delete max;
		delete equ;
		delete inv;
		delete avg;
		delete join;
		delete invConverter;
		delete avgConverter;
	}
};

struct Executor
{
	vector<ProcessingBranch*> branches;
	function_node<int, bool> * logWriter;

	Executor(int threadCount)
	{
		if (params.filename != "")
			logWriter = new function_node<int, bool>(g, unlimited, LogWriter(params.filename));
			
		for (int i = 0; i < threadCount; i++)
		{
			ProcessingBranch * b = new ProcessingBranch();
			branches.push_back(b);

			if (params.filename != "")
				make_edge(*(b->avg), *logWriter);
		}
	}

	~Executor()
	{
		for (int i = 0; i < branches.size(); i++)
			delete branches[i];
	}

	void execute()
	{
		for (int k = 0; k < params.k - params.k % branches.size(); k += branches.size())
		{
			for (int i = 0; i < branches.size(); i++)
				branches[i]->input->try_put(generateImage(params.m, params.n));
		}
		
		for (int k = 0; k < params.k % branches.size(); k++)
			branches[k]->input->try_put(generateImage(params.m, params.n));

		g.wait_for_all();
	}
};

int main(int argc, char ** argv)
{
	srand(time(0));

	/*
	testAll();
	system("pause > null");
	return 0;
	*/

	params = parseParams(argc, argv);
	if (!params.ok)
		return 1;

	Executor executor(params.l);
	executor.execute();

	return 0;
}