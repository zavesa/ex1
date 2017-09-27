#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>
#include <iostream>
#include <conio.h>
#include <vector>

using namespace std::chrono;
using namespace std;

void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n",
		maximum->get_reference(), maximum->get_index_reference());
}

void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n",
		minimum->get_reference(), minimum->get_index_reference());
}

void ParallelSort(int *begin, int *end)
{
	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

void CompareForAndCilk_For(size_t sz)
{
	cout << "Count of elemets = " << sz << endl;
	vector<int> vec;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (int i = 0; i < sz; i++)
	{
		vec.push_back(rand() % 20000 + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> diff1 = (t2 - t1);
	cout << "Standard filling time is: " << diff1.count() << " seconds" << endl;
	
	cilk::reducer<cilk::op_vector<int>>red_vec;
	high_resolution_clock::time_point t3 = high_resolution_clock::now();
	cilk_for(int j = 0; j < sz; j++)
	{
		red_vec->push_back(rand() % 20000 + 1);
	}
	high_resolution_clock::time_point t4 = high_resolution_clock::now();
	duration<double> diff2 = (t4 - t3);
	cout << "Cilk_for filling time is: " << diff2.count() << " seconds" << endl;
	cout << "==================================================================" << endl;
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	long i;
	const int loop_val[8] = { 1000000, 100000, 10000, 1000, 500, 100, 50, 10 }; // значения входного параметра sz для функции "CompareForAndCilk_For"
	const long mass_size[4] = { 10000, 100000, 500000, 1000000 }; // значения входного параметра mass_size для функций "ReducerMaxTest", "ReducerMinTest"
	int *mass_begin, *mass_end;

	for (int l = 0; l < 4; l++) // выполнение задания 3
	{
		int *mass = new int[mass_size[l]]; // создание 
		for (i = 0; i < mass_size[l]; i++) // и заполнения массива случайными значениями
		{
			mass[i] = (rand() % 25000) + 1;
		}
		mass_begin = mass;
		mass_end = mass_begin + mass_size[l];
		
		cout << "=================================================== \nmass_size is: " << mass_size[l] << endl;
		ReducerMaxTest(mass, mass_size[l]);
		ReducerMinTest(mass, mass_size[l]);
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		ParallelSort(mass_begin, mass_end);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		duration<double> diff = (t2 - t1);
		cout << "   =>>> Sort time is: " << diff.count() << " seconds" << endl;
		ReducerMaxTest(mass, mass_size[l]);
		ReducerMinTest(mass, mass_size[l]);
		delete[]mass;
	}

cout << "******************************************************************" << endl;

	for (int k = 0; k < 8; k++)  // выполнение задания 4
	{
		CompareForAndCilk_For(loop_val[k]); 
	}
	
	getch();
	return 0;
}