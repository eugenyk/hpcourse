// FlowGraph.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "tbb/flow_graph.h"
#include <iostream>
#include <ctime> 
#include <windows.h>
#include <list>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdlib.h>
#include <fstream>
#include <functional>

using namespace std;
using namespace tbb::flow;


//Start matrix
struct matrixStr {
	int **matrix;
	int size;
	int min;
	int max;
	int set;
	int id;
};

//Matrix after maxM && minM. Send on node
struct outMatrixParalStr {
	matrixStr matrix;
	int rgb;
	list<list<int>> rgpPosList;
};

//max elm in matrix
int maxM(int **matrix, int size) {
	int max = 0;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			max = matrix[i][j] > max ? matrix[i][j] : max;
		}
	}
	//cout << "max: " << max << endl;
	return max;
}

//min elm in matrix
int minM(int **matrix, int size) {
	int min = 255;
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			min = matrix[i][j] < min ? matrix[i][j] : min;
		}
	}
	//cout << "min: " << min << endl;
	return min;
}

//show matrix
void sout(int **matrix, int size) {
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			matrix[i][j] = rand() % 255;
			cout << matrix[i][j] << " ";
		}
		cout << endl;
	}
}

//create start matrix
int ** createMatrix(int **matrix, int size) {
	srand(time(NULL));
	for (int i = 0; i < size; i++)
	{
		matrix[i] = new int[size];
	}
	//sout(matrix, size);
	return matrix;
}

//node max
struct listRgbMax {
	outMatrixParalStr operator()(matrixStr matrix) {
		list<list<int>> outListMax;
		for (int i = 0; i < matrix.size; i++)
		{
			for (int j = 0; j < matrix.size; j++)
			{
				if (matrix.matrix[i][j] == matrix.max) {
					list<int> tempListHW;
					tempListHW.push_back(i);
					tempListHW.push_back(j);
					outListMax.push_back(tempListHW);
				}
			}
		}
		// cout << "one" << matrix.size <<endl;
		outMatrixParalStr outSetMatrix;
		outSetMatrix.matrix = matrix;
		outSetMatrix.rgb = matrix.max;
		outSetMatrix.rgpPosList = outListMax;

		return outSetMatrix;
	}
};


//node min
struct listRgbMin {
	outMatrixParalStr operator()(matrixStr matrix) {
		list<list<int>> outListMin;
		for (int i = 0; i < matrix.size; i++)
		{
			for (int j = 0; j < matrix.size; j++)
			{
				if (matrix.matrix[i][j] == matrix.min) {
					list<int> tempListHW;
					tempListHW.push_back(i);
					tempListHW.push_back(j);
					outListMin.push_back(tempListHW);
				}
			}
		}
		// cout << "two" << matrix.size <<endl;
		outMatrixParalStr outSetMatrix;
		outSetMatrix.matrix = matrix;
		outSetMatrix.rgb = matrix.min;
		outSetMatrix.rgpPosList = outListMin;
		return outSetMatrix;
	}
};

//node set
struct listRgbSet {
	outMatrixParalStr operator()(matrixStr matrix) {
		list<list<int>> outListSet;
		for (int i = 0; i < matrix.size; i++)
		{
			for (int j = 0; j < matrix.size; j++)
			{
				if (matrix.matrix[i][j] == matrix.set) {
					list<int> tempListHW;
					tempListHW.push_back(i);
					tempListHW.push_back(j);
					outListSet.push_back(tempListHW);
				}
			}
		}
		// cout << "three" << matrix.size <<endl;
		outMatrixParalStr outSetMatrix;
		outSetMatrix.matrix = matrix;
		outSetMatrix.rgb = matrix.set;
		outSetMatrix.rgpPosList = outListSet;
		return outSetMatrix;
	}
};


//join node result
struct  merger {
	matrixStr operator()(std::tuple<outMatrixParalStr, outMatrixParalStr, outMatrixParalStr> v) {
		
		matrixStr outMatrix = std::get<0>(v).matrix;
		matrixStr outMatrix1 = std::get<1>(v).matrix;
		matrixStr outMatrix2 = std::get<2>(v).matrix;
		
		outMatrix = tracePoint(outMatrix, std::get<0>(v).rgb, std::get<0>(v).rgpPosList);
		outMatrix = tracePoint(outMatrix, std::get<1>(v).rgb, std::get<1>(v).rgpPosList);
		outMatrix = tracePoint(outMatrix, std::get<2>(v).rgb, std::get<2>(v).rgpPosList);

		return std::get<0>(v).matrix;
	}

	matrixStr tracePoint(matrixStr matrix, int rgb, list<list<int>> outList) {
		for (int i = outList.size(); i > 0; i--) {
			list<int> listTemp = outList.front();
			int ii = listTemp.front();
			int jj = listTemp.back();
			outList.remove(listTemp);

			int minii = ii > 0 ? ii - 1 : ii;
			int minjj = jj > 0 ? jj - 1 : jj;
			int maxii = ii < matrix.size - 1 ? ii + 1 : ii;
			int maxjj = jj < matrix.size - 1 ? jj + 1 : jj;

			for (int i2 = minii; i2 <= maxii; i2++) {
				for (int j2 = minjj; j2 <= maxjj; j2++) {
					matrix.matrix[i2][j2] = rgb;
				}
			}
		}
		//cout << "matrix new: " << endl;
		//sout(matrix.matrix, matrix.size);
		//cout << "+=+=+=+=+=+=+=+" << endl;
		return matrix;
	}
};

//
struct finalMatrix {
	int operator()(matrixStr matrix) {
		matrix = revers(matrix);
		return midlRgb(matrix);
	}
	matrixStr revers(matrixStr matrix) {
		for (int i = 0; i < matrix.size; i++) {
			for (int j = 0; j < matrix.size; j++) {
				matrix.matrix[i][j] = 255 - matrix.matrix[i][j];
			}
		}
		//cout << "matrix revers: " << endl;
		//sout(matrix.matrix, matrix.size);
		//cout << "+=+=+=+=+=+=+=+" << endl;
		return matrix;
	}

	int midlRgb(matrixStr matrix) {
		int summ = 0;
		for (int i = 0; i < matrix.size; i++) {
			for (int j = 0; j < matrix.size; j++) {
				summ += matrix.matrix[i][j];
			}
		}
		return (int)summ / (matrix.size*matrix.size);
	}
};


//list midl value
class listMidlRgb {
	list<int> &listRgb;
public:
	listMidlRgb(list<int> &results) : listRgb(results) {}
	list<int> operator()(int summ) {
		listRgb.push_back(summ);
		return listRgb;
	}
};

int getIdImage(outMatrixParalStr m) {
	return m.matrix.id;
}

int main(int argc, char* argv[]) {
	int unlimited = 10;
	string fout = "res.txt";
	int set = 5;

	if (argc > 1) {
		string oneArg = argv[1];
		istringstream kb(oneArg);
		vector<string> token1;
		copy(istream_iterator<string>(kb),
			istream_iterator<string>(),
			back_inserter(token1));
		if (token1[0] == "-b")
			set = atoi(token1[1].c_str());

		string twoArg = argv[2];
		istringstream ki(twoArg);
		vector<string> token2;
		copy(istream_iterator<string>(ki),
			istream_iterator<string>(),
			back_inserter(token2));
		if (token2[0] == "-i")
			unlimited = atoi(token2[1].c_str());

		string threeArg = argv[3];
		istringstream kf(threeArg);
		vector<string> token3;
		copy(istream_iterator<string>(kf),
			istream_iterator<string>(),
			back_inserter(token3));
		if (token3[0] == "-f")
			fout = token3[1];
	}

	graph graphTbb;
	list<int> results;

	function<int(outMatrixParalStr)> getIdImage =
		[](outMatrixParalStr matrix)->int { return matrix.matrix.id; };

	broadcast_node<matrixStr> input(graphTbb);
	function_node<matrixStr, outMatrixParalStr > listRgbMax(graphTbb, unlimited, listRgbMax());
	function_node<matrixStr, outMatrixParalStr> listRgbMin(graphTbb, unlimited, listRgbMin());
	function_node<matrixStr, outMatrixParalStr> listRgbSet(graphTbb, unlimited, listRgbSet());
	buffer_node<outMatrixParalStr> listRgbMax_buffer(graphTbb);
	buffer_node<outMatrixParalStr> listRgbMin_buffer(graphTbb);
	buffer_node<outMatrixParalStr> listRgbSet_buffer(graphTbb);
	join_node< tuple<
		outMatrixParalStr,
		outMatrixParalStr,
		outMatrixParalStr
	>, key_matching<int >> join(graphTbb, getIdImage, getIdImage, getIdImage);
	function_node<tuple<outMatrixParalStr, outMatrixParalStr, outMatrixParalStr>, matrixStr>
		merger(graphTbb, serial, merger());
	function_node<matrixStr, int> finalMatrix(graphTbb, unlimited, finalMatrix());
	buffer_node<matrixStr> finalMatrix_buffer(graphTbb);
	function_node<int, list<int>>listMidlRgb(graphTbb, serial, listMidlRgb(results));
	//queueing

	make_edge(input, listRgbMax);
	make_edge(input, listRgbMin);
	make_edge(input, listRgbSet);

	make_edge(listRgbMax, listRgbMax_buffer);
	make_edge(listRgbMax, input_port<0>(join));

	make_edge(listRgbMin, listRgbMin_buffer);
	make_edge(listRgbMin, input_port<1>(join));

	make_edge(listRgbSet, listRgbSet_buffer);
	make_edge(listRgbSet, input_port<2>(join));
	make_edge(join, merger);

	make_edge(merger, finalMatrix_buffer);
	make_edge(finalMatrix_buffer, finalMatrix);
	make_edge(finalMatrix, listMidlRgb);

	int size = 10;
	int **matrix;
	for (int i = 0; i < 10; i++) {
		matrix = new int*[size];
		//cout<< "number " << i << endl;
		matrix = createMatrix(matrix, size);
		matrixStr matrixS;
		matrixS.matrix = matrix;
		matrixS.size = size;
		matrixS.max = maxM(matrix, size);
		matrixS.min = minM(matrix, size);
		matrixS.set = 5;
		matrixS.id = i;
		input.try_put(matrixS);
	}
	graphTbb.wait_for_all();
	for (int i = 0; i < size; i++)
	{
		delete[]matrix[i];
	}
	delete[] matrix;

	ofstream fouttxt;
	fouttxt.open(fout);
	for (int ir = results.size(); ir > 0; ir--) {
		fouttxt << results.front() << " ";
		results.pop_front();
	}
	fouttxt.close();
	results.clear();

	system("pause");
	return 0;
}