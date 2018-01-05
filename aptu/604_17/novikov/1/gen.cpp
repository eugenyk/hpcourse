#include <iostream>

using namespace std;

int const N = 10000000;

int main()
{
    long long sum = 0;
    for (size_t i = 0; i < N; i++) {
        int val = rand() % 10;
        sum += val;
        cout << val << endl; 
    }

    cerr << "sum: " << sum << endl;

    return 0;
}
