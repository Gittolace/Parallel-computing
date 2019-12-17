#include<iostream>
#include<cmath>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "malloc.h"
using namespace std;

struct Matrix
{
	int width;
	int height;
	float* elements;
};


__device__ float getElement(Matrix *A, int row, int col)
{
	return A->elements[row * A->width + col];
}

__device__ void setElement(Matrix *A, int row, int col, float value)
{
	A->elements[row * A->width + col] = value;
}

__global__ void matMul(Matrix *A, Matrix *B, Matrix *C)
{
	float Cvalue = 0.0;
	int row = threadIdx.y + blockIdx.y * blockDim.y;
	int col = threadIdx.x + blockIdx.x * blockDim.x;
	for (int i = 0; i < A->width; ++i)
	{
		Cvalue += getElement(A, row, i) * getElement(B, i, col);
	}
	setElement(C, row, col, Cvalue);
}

int main()
{

	int dev = 0;
	cudaDeviceProp devProp;
	cudaGetDeviceProperties(&devProp, dev);
	cout << "ʹ��GPU device " << dev << ": " << devProp.name << endl;
	cout << "SM��������" << devProp.multiProcessorCount << endl;
	cout << "ÿ���߳̿�Ĺ����ڴ��С��" << devProp.sharedMemPerBlock / 1024.0 << " KB" << endl;
	cout << "ÿ���߳̿������߳�����" << devProp.maxThreadsPerBlock << endl;
	cout << "ÿ��EM������߳�����" << devProp.maxThreadsPerMultiProcessor << endl;
	cout << "ÿ��EM������߳�������" << devProp.maxThreadsPerMultiProcessor / 32 << endl;


	int T = 10;

	while (T>1)
	{
		int width = pow(2,T);
		int height = pow(2,T);
		Matrix *A, *B, *C;
		// �����й��ڴ�
		cudaMallocManaged((void**)&A, sizeof(Matrix));
		cudaMallocManaged((void**)&B, sizeof(Matrix));
		cudaMallocManaged((void**)&C, sizeof(Matrix));
		int nBytes = width * height * sizeof(float);
		cudaMallocManaged((void**)&A->elements, nBytes);
		cudaMallocManaged((void**)&B->elements, nBytes);
		cudaMallocManaged((void**)&C->elements, nBytes);

		// ��ʼ������
		A->height = height;
		A->width = width;
		B->height = height;
		B->width = width;
		C->height = height;
		C->width = width;
		for (int i = 0; i < width * height; ++i)
		{
			A->elements[i] = 1.0;
			B->elements[i] = 2.0;
		}

		// ����kernel��ִ������
		dim3 blockSize(1, 1);
		dim3 gridSize((width + blockSize.x - 1) / blockSize.x,
			(height + blockSize.y - 1) / blockSize.y);
		// ִ��kernel
		matMul << < gridSize, blockSize >> > (A, B, C);


		// ͬ��device ��֤�������ȷ����
		cudaDeviceSynchronize();
		// ���ִ�н��
		float maxError = 0.0;
		for (int i = 0; i < width * height; ++i)
			maxError = fmax(maxError, fabs(C->elements[i] - 2 * width));
		std::cout << "������: " << maxError << std::endl;
		T--;
	}

	return 0;

}