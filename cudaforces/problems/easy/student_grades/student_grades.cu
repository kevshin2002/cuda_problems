#include <cuda_runtime.h>
#include <stdio.h>
#include <vector>

#include "../../common/problem_runner.h"

__global__ void sum_avg_kernel(const int* grades, int count, int* out_sum, int* out_avg) {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        int sum = 0;
        for (int i = 0; i < count; ++i) {
            sum += grades[i];
        }
        *out_sum = sum;
        *out_avg = sum / count;
    }
}

static bool CheckCuda(cudaError_t err, const char* what) {
    if (err != cudaSuccess) {
        printf("CUDA error at %s: %s\n", what, cudaGetErrorString(err));
        return false;
    }
    return true;
}

struct StudentGradesKernel {
    void operator()(const ProblemRunner::Line& input, ProblemRunner::Line& output) const {
        const int count = 5;
        int host_grades[count];
        for (int i = 0; i < count; ++i) {
            host_grades[i] = input[i];
        }

        int* d_grades = nullptr;
        int* d_sum = nullptr;
        int* d_avg = nullptr;

        if (!CheckCuda(cudaMalloc(&d_grades, sizeof(int) * count), "cudaMalloc d_grades") ||
            !CheckCuda(cudaMalloc(&d_sum, sizeof(int)), "cudaMalloc d_sum") ||
            !CheckCuda(cudaMalloc(&d_avg, sizeof(int)), "cudaMalloc d_avg")) {
            if (d_grades) cudaFree(d_grades);
            if (d_sum) cudaFree(d_sum);
            if (d_avg) cudaFree(d_avg);
            return;
        }

        if (!CheckCuda(cudaMemcpy(d_grades, host_grades, sizeof(int) * count, cudaMemcpyHostToDevice),
                       "cudaMemcpy to device")) {
            cudaFree(d_grades);
            cudaFree(d_sum);
            cudaFree(d_avg);
            return;
        }

        sum_avg_kernel<<<1, 1>>>(d_grades, count, d_sum, d_avg);
        if (!CheckCuda(cudaGetLastError(), "kernel launch")) {
            cudaFree(d_grades);
            cudaFree(d_sum);
            cudaFree(d_avg);
            return;
        }

        int host_sum = 0;
        int host_avg = 0;
        if (!CheckCuda(cudaMemcpy(&host_sum, d_sum, sizeof(int), cudaMemcpyDeviceToHost),
                       "cudaMemcpy sum") ||
            !CheckCuda(cudaMemcpy(&host_avg, d_avg, sizeof(int), cudaMemcpyDeviceToHost),
                       "cudaMemcpy avg")) {
            cudaFree(d_grades);
            cudaFree(d_sum);
            cudaFree(d_avg);
            return;
        }

        cudaFree(d_grades);
        cudaFree(d_sum);
        cudaFree(d_avg);

        output.push_back(host_sum);
        output.push_back(host_avg);
    }
};

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <input.txt> <solutions.txt>\n", argv[0]);
        return 1;
    }

    const char* input_path = argv[1];
    const char* solution_path = argv[2];

    return ProblemRunner::Run(input_path, solution_path, StudentGradesKernel());
}
