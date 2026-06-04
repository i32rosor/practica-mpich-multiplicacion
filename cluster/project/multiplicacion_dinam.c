#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024 


void inicializar_matriz(int matriz[N][N]);

void multiplicar_bloque(int filas_por_proceso, int local_A[][N], int B[][N], int local_C[][N]);

int main(int argc, char** argv) {
    int size, rank;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (N % 8 != 0) {
        if (rank == 0) {
            printf("Error: El tamaño de la matriz (N=%d) debe ser múltiplo de 8.\n", N);
        }
        MPI_Finalize();
        return 1;
    }

    int (*A)[N] = NULL;
    int (*B)[N] = NULL;
    int (*C)[N] = NULL;
    

    if (rank == 0) {
        A = malloc(sizeof(int[N][N]));
        B = malloc(sizeof(int[N][N]));
        C = malloc(sizeof(int[N][N]));

        srand(time(NULL));
        inicializar_matriz(A);
        inicializar_matriz(B);
        
    } else {

        B = malloc(sizeof(int[N][N]));
    }

    int filas_por_proceso = N / size;

    int (*local_A)[N] = malloc(sizeof(int[filas_por_proceso][N]));
    int (*local_C)[N] = malloc(sizeof(int[filas_por_proceso][N]));

    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Scatter(A, filas_por_proceso * N, MPI_INT, local_A, filas_por_proceso * N, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    multiplicar_bloque(filas_por_proceso, local_A, B, local_C);

    MPI_Gather(local_C, filas_por_proceso * N, MPI_INT, C, filas_por_proceso * N, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Tiempo total de computación: %f segundos\n", end_time - start_time); 
    }

    if (rank == 0) {
        free(A);
        free(C);
    }
    free(B);

    MPI_Finalize();
    return 0;
}


void inicializar_matriz(int matriz[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matriz[i][j] = (rand() % 10) + 1; 
        }
    }
}

void multiplicar_bloque(int filas_por_proceso, int local_A[][N], int B[][N], int local_C[][N])
{
    for (int i = 0; i < filas_por_proceso; i++) {
        for (int j = 0; j < N; j++) {
            local_C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                local_C[i][j] += local_A[i][k] * B[k][j];
            }
        }
    }
}
