#include <math.h>
#include "mpi.h"
#include <thread>
#include <mutex>

double ans = 0;
const int N = 10;
std::mutex my_lock;
std::string function = "";

double f(double x)
{
	function = "x";
	return x;
}

// функция интегрирования методом прямоугольника
void my_integrate(double a, double b, double h, int power) 
{
	double sum = 0;
	for (double x = a; x < b; x += h) 
	{
		sum += f(x)*pow(sin(x), power)*h;  
	}
	my_lock.lock();
	ans += sum;
	my_lock.unlock();
}

typedef struct {
	int number;
	double a;
	double b;
} info;

int main(int argc, char *argv[])
{
	int ProcNum, ProcRank;
	MPI_Status Status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	
	MPI_Datatype myStructType;
	int len[4] = { 1, 1, 1, 1 };
	MPI_Aint pos[4] = { offsetof(info, number), offsetof(info, a), offsetof(info, b), sizeof(info) };
	MPI_Datatype typ[4] = { MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_UB };
	MPI_Type_create_struct(4, len, pos, typ, &myStructType);
	MPI_Type_commit(&myStructType);
	
	if (ProcRank == 0)
	{
		// пределы интегрирования!
		double global_a = 5;
		double global_b = 50;

		// отправка информации остальным пр
		for (int i = 1; i < ProcNum; i++)
		{
			info to_send = {i, global_a, global_b};
			MPI_Send(&to_send, 1, myStructType, i, 0, MPI_COMM_WORLD);
		}

		// считаем свою часть
		double h = (global_b - global_a) / N;
		std::thread* my_thread_pool[N];
		for (int i = 0; i < N; i++)
		{
			my_thread_pool[i] = new std::thread(my_integrate, global_a+h*i, global_a+h*(i+1), 0.001, 0);
		}

		for (int i = 0; i < N; i++)
		{
			my_thread_pool[i]->join();
		}

		printf("\n proc #%d", 0);
		printf(": integrate ");
		printf("%s", function.c_str());
		printf(" from %lf", global_a);
		printf(" to %lf", global_b);
		printf(" = %lf", ans);

		// получаем результаты от остальных пр
		double recv;
		for (int i = 1; i<ProcNum; i++)
		{
			MPI_Recv(&recv, 1, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
			//printf("\n info from process %3d ", i);
			//printf("  %lf ", recv);
			ans += recv;
		}
		
		printf("\n FINAL SUM = %lf ", ans);
	}
	else // работа с итыми процессами
	{
		int j = ProcRank;
		info to_recv;
		MPI_Recv(&to_recv, 1, myStructType, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);

		double h = (to_recv.b - to_recv.a) / N;

		std::thread* my_thread_pool[N];
		for (int i = 0; i < N; i++)
		{
			my_thread_pool[i] = new std::thread(my_integrate, to_recv.a + h*i, to_recv.a + h*(i + 1), 0.001, j);
		}

		for (int i = 0; i < N; i++)
		{
			my_thread_pool[i]->join();
		}

		printf("\n proc #%d", j);
		printf(": integrate ");
		printf("%s", function.c_str());
		printf("*(sin(x))^%d", j);
		printf(" from %lf", to_recv.a);
		printf(" to %lf", to_recv.b);
		printf(" = %lf", ans);

		MPI_Send(&ans, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();

	return 0;
}

