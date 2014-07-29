#include <stdio.h>
#include <stdlib.h>
#define N 512

__global__ void add(int *a, int *b, int *c) {
    c[blockIdx.x] = a[blockIdx.x] + b[blockIdx.x];
}

void random_ints(int *a) {
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 10;
    } 
}


int main(void) {
    int *a, *b, *c;
//    int a, b, c;
    int *d_a, *d_b, *d_c;
    int size = N * sizeof(int);
    // int size = sizeof(int);
    
    cudaMalloc((void **)&d_a, size);
    cudaMalloc((void **)&d_b, size);
    cudaMalloc((void **)&d_c, size);
    
    a = (int *)malloc(size); random_ints(a);
    b = (int *)malloc(size); random_ints(b);
    c = (int *)malloc(size);
    
    //a = 2;
    //b = 7;

    cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice);

    // add<<<1, 1>>>(d_a, d_b, d_c);
    add<<<N, 1>>>(d_a, d_b, d_c);

    cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost);
    //printf("c=%d", c);
    for (int i = 0; i < N; i++) {
        printf("%d", c[i]);
    }
    
    free(a); free(b); free(c);
    cudaFree(d_a); cudaFree(d_b); cudaFree(d_c);
    
    return 0;
}
