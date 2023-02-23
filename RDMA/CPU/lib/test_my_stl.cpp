#include <iostream>
#include "fixed_size_container.h"

using namespace std;

int main()
{
    fixed_size_array<int, 20> a, b;
    for (auto i = 0; i < 10; ++i)
        a.push_back(i);
    
    for (auto i = 0; i < 10; ++i)
        b.push_back(i);

    cout << (a == b) << endl;

    return 0;
}