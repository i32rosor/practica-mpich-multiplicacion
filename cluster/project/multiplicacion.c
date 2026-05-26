#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int* allocate_matrix(int rows, int cols) {
    return (int*)malloc(rows * cols * sizeof(int));
}

void init_matrix_random(int* matrix, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (rand() % 10) + 1;
    }
}

void multiply_matrices(int* local_A, int* B, int* local_C, int local_rows, int N) {
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < N; j++) {
            local_C[i * N + j] = 0;
            for (int k = 0; k < N; k++) {
                local_C[i * N + j] += local_A[i * N + k] * B[k * N + j];
            }
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            printf("Uso: mpirun -np <procesos> ./multiplicacion <Tamanio_Matriz_N>\n");
        }
        MPI_Finalize();
        return 1;
    }

    int N = atoi(argv[1]);

    if (N % 8 != 0 || N % size != 0) {
        if (rank == 0) {
            printf("Error: N debe ser multiplo de 8 y divisible por el numero de procesos.\n");
        }
        MPI_Finalize();
        return 1;
    }

    int local_rows = N / size;

    int *A = NULL;
    int *B = allocate_matrix(N, N);
    int *C = NULL;
    
    int *local_A = allocate_matrix(local_rows, N);
    int *local_C = allocate_matrix(local_rows, N);

    if (rank == 0) {
        A = allocate_matrix(N, N);
        C = allocate_matrix(N, N);
        srand(time(NULL));
        init_matrix_random(A, N, N);
        init_matrix_random(B, N, N);
    }

    double start_time = 0.0;
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(A, local_rows * N, MPI_INT, local_A, local_rows * N, MPI_INT, 0, MPI_COMM_WORLD);

    multiply_matrices(local_A, B, local_C, local_rows, N);

    MPI_Gather(local_C, local_rows * N, MPI_INT, C, local_rows * N, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double end_time = MPI_Wtime();
        printf("%d,%d,%f\n", N, size, end_time - start_time);
        free(A);
        free(C);
    }

    free(B);
    free(local_A);
    free(local_C);

    MPI_Finalize();
    return 0;
}