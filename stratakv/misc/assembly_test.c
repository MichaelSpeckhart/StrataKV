#include <stdint.h>
#include <stdlib.h>

#define N 64

void loop_function(int* arr)
{
    for (int i = 0; i < N; ++i)
    {
        arr[i] = i * N;
    }
}


int main()
{
    int* arr = (int*)malloc(sizeof(int) * 64);
}
