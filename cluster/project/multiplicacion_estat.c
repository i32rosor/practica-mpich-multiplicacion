#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024 

// --- MEMORIA ESTÁTICA GLOBAL ---
// Al declararlas fuera del main, evitamos el Stack Overflow en la pila.
int A[N][N];
int B[N][N];
int C[N][N];

// Como el número de procesos (size) no se sabe hasta que ejecutamos mpirun,
// en memoria estática tenemos que reservar el tamaño MÁXIMO posible (N filas)
// por si ejecutamos el programa con 1 solo nodo.
int local_A[N][N];
int local_C[N][N];

// --- Declaración de funciones ---
void inicializar_matriz_estatica(int matriz[N][N]);

void multiplicar_bloque(int filas_por_proceso, int local_A[][N], int B[][N], int local_C[][N]);

int main(int argc, char** argv) {
    int size, rank;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (N % 8 != 0) {
        if (rank == 0) printf("Error: N=%d debe ser múltiplo de 8.\n", N);
        MPI_Finalize(); return 1;
    }

    if (rank == 0) {
        // Inicializamos las matrices globales A y B
        srand(time(NULL));
        inicializar_matriz_estatica(A);
        inicializar_matriz_estatica(B);
    }

    // Calculamos cuántas filas le tocan a cada proceso
    int filas_por_proceso = N / size;

    // Reparto de datos
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Aunque local_A tiene capacidad para N filas, solo le enviamos 'filas_por_proceso'
    MPI_Scatter(A, filas_por_proceso * N, MPI_INT, local_A, filas_por_proceso * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Temporización
    if (rank == 0) start_time = MPI_Wtime();

    // Cómputo distribuido
    multiplicar_bloque(filas_por_proceso, local_A, B, local_C);

    // Recogida de datos
    MPI_Gather(local_C, filas_por_proceso * N, MPI_INT, C, filas_por_proceso * N, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("V1 (Memoria Estática) - Tiempo total: %f segundos\n", end_time - start_time);
    }

    // NO hay que hacer free() porque la memoria es estática.
    MPI_Finalize();
    return 0;
}

void inicializar_matriz_estatica(int matriz[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matriz[i][j] = (rand() % 10) + 1; 
        }
    }
}


void multiplicar_bloque(int filas_por_proceso, int local_A[][N], int B[][N], int local_C[][N]) {
    for (int i = 0; i < filas_por_proceso; i++) {
        for (int j = 0; j < N; j++) {
            local_C[i][j] = 0; 
            for (int k = 0; k < N; k++) {
                local_C[i][j] += local_A[i][k] * B[k][j];
            }
        }
    }
}