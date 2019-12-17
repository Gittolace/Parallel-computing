#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;
const int N = 317; // 随便写的，随机数位于[0, N]之间

void matGene(int *A, int row, int column) {
	srand(time(NULL));
	for (int i = 0; i < row; i++)
		for (int j = 0; j < column; j++)
			A[i * row + j] = rand() % N; //A[i][j]
}

void matMulti(int *A, int *B, int*C, int row, int n) {
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < n; j++) {
			C[i*n + j] = 0;
			for (int k = 0; k < n; k++)
				C[i*n + j] = A[i*n + k] * B[k*n + j];
		}
	}
}

int main(int argc, char *argv[]) {
	// Sorry but Only Deal With Square Matrixs

	// To Run: 
	// mpicxx matrix_multi.cpp 
	// mpiexec -n comm_sz ./a.out mat_dim

	// Calculate Parameters Definition
	int n; // matrix dimension
	cin >> n;
	int beginRow, endRow; // the range of rows calculating in certain process
	double beginTime, endTime; // time record

	// MPI Common Head
	int my_rank = 0, comm_sz = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Status status;

	if (comm_sz == 1) { // no parallel
		// Prepare data
		int* A = new int[n * n];
		int* B = new int[n * n];
		int* C = new int[n * n];
		matGene(A, n, n);
		matGene(B, n, n);

		// Calculate C[i][j] & Time
		beginTime = MPI_Wtime();
		matMulti(A, B, C, n, n);
		endTime = MPI_Wtime();
		cout << "Time: " << endTime - beginTime << endl;

		// End
		delete[] A;
		delete[] B;
		delete[] C;
	}

	else { // parallel: main process collect the result, others calculate for result of "each_row" rows

		int each_row = n / (comm_sz - 1);

		if (my_rank == 0) { // process 0: main process, data distribution & collect calculate results
			// Prepare data
			int* A = new int[n * n];
			int* B = new int[n * n];
			int* C = new int[n * n];
			matGene(A, n, n);
			matGene(B, n, n);

			beginTime = MPI_Wtime();

			// Send: A[beginRow:endRow, :], whole B
			// beginRow = each_row * (my_rank-1), endRow = each_row * my_rank;
			for (int i = 0; i < comm_sz - 1; i++) {
				beginRow = each_row * i, endRow = each_row * (i + 1);
				MPI_Send(&A[beginRow * n + 0], each_row * n, MPI_INT, i + 1, 1, MPI_COMM_WORLD);
				MPI_Send(&B[0 * n + 0], n * n, MPI_INT, i + 1, 2, MPI_COMM_WORLD);
				// cout << "I am alive in sending data to process " << i << endl;
			}

			// Recv: C[beginRow:endRow, :]
			for (int i = 0; i < comm_sz - 1; i++) {
				MPI_Recv(&C[beginRow * n + 0], each_row*n, MPI_INT, i + 1, 3, MPI_COMM_WORLD, &status);
				// cout << "I am alive in receiving data from process " << i << endl;
			}

			endTime = MPI_Wtime();
			cout << "Time: " << endTime - beginTime << endl;

			delete[] A;
			delete[] B;
			delete[] C;
		}

		if (my_rank != 0) { // other processes: calculate A * B

			// beginRow = each_row * (my_rank-1), endRow = each_row * my_rank;

			int* A = new int[each_row * n]; // A[beginRow:endRow, :]
			int* B = new int[n * n];
			int* C = new int[each_row * n]; // C[beginRow:endRow, :]

			MPI_Recv(&A[0 * n + 0], each_row * n, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);//从A[0][0]和B[0][0]开始接受
			MPI_Recv(&B[0 * n + 0], n * n, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

			matMulti(A, B, C, each_row, n);


			// cout << "Hello and I am process " << my_rank << endl;

			MPI_Send(&C[0 * n + 0], each_row*n, MPI_INT, 0, 3, MPI_COMM_WORLD);//起始地址是C[my_rank-1][0],大小是each_row*n

			delete[] A;
			delete[] B;
			delete[] C;
		}
	}

	MPI_Finalize();
	while (1)
	{
	}
	return 0;
}
