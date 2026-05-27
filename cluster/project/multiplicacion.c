#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Definimos el tamaño de la matriz. ¡Recuerda que debe ser múltiplo de 8!
#define N 1024 

// --- Declaración de funciones (separando la lógica) ---

// Función para inicializar una matriz con valores aleatorios (1 al 10)
void inicializar_matriz(int matriz[N][N]);

// Función para imprimir una matriz (útil para depurar con N pequeños)
void imprimir_matriz(int matriz[N][N], const char* nombre);

// Función principal donde ocurre la magia de MPI
int main(int argc, char** argv) {
    int size, rank;
    double start_time, end_time;

    // 1. Inicializamos el entorno MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Verificamos que N sea múltiplo de 8 (Norma de la práctica)
    if (N % 8 != 0) {
        if (rank == 0) {
            printf("Error: El tamaño de la matriz (N=%d) debe ser múltiplo de 8.\n", N);
        }
        MPI_Finalize();
        return 1;
    }

    // Punteros para las matrices. Solo el maestro necesita las matrices completas (A, B, C).
    // Usamos punteros y asignación dinámica para evitar desbordar la pila (Stack Overflow) con matrices grandes.
    int (*A)[N] = NULL;
    int (*B)[N] = NULL;
    int (*C)[N] = NULL;
    
    // Todos los procesos necesitan conocer la matriz B completa y una porción de A y C.
    // Asignaremos memoria para B en todos los procesos más adelante.

    if (rank == 0) {
        // --- NODO MAESTRO ---
        // Asignamos memoria para las matrices completas
        A = malloc(sizeof(int[N][N]));
        B = malloc(sizeof(int[N][N]));
        C = malloc(sizeof(int[N][N]));

        // Inicializamos las matrices
        // Recuerda: el cálculo de las matrices iniciales NO se temporiza
        srand(time(NULL));
        inicializar_matriz(A);
        inicializar_matriz(B);
        
        // imprimir_matriz(A, "A"); // Descomentar solo para N pequeño
    } else {
        // --- NODOS TRABAJADORES ---
        // Los trabajadores también necesitan memoria para la matriz B (que recibirán completa)
        B = malloc(sizeof(int[N][N]));
    }

    // --- REPARTO DE DATOS (AQUÍ EMPIEZA LA COMUNICACIÓN) ---
// 1. Calcular cuántas filas le tocan a cada proceso
    // Al ser N múltiplo de 8 y usar 1, 2, 4 u 8 procesos, la división siempre será exacta.
    int filas_por_proceso = N / size;

    // 2. Cada proceso (incluido el maestro) necesita memoria para almacenar:
    // - Las filas de A que le tocan procesar (local_A)
    // - Las filas de C donde guardará sus resultados parciales (local_C)
    int (*local_A)[N] = malloc(sizeof(int[filas_por_proceso][N]));
    int (*local_C)[N] = malloc(sizeof(int[filas_por_proceso][N]));

    // 3. Enviar la matriz B completa a TODOS los procesos usando MPI_Bcast
    // Parámetros: Datos, Cantidad total ($N \times N$), Tipo de dato, Nodo origen (0), Comunicador
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    // 4. Repartir las filas de la matriz A usando MPI_Scatter
    // El maestro coge la matriz A y envía bloques de tamaño (filas_por_proceso * N) a los local_A de cada nodo
    MPI_Scatter(A, filas_por_proceso * N, MPI_INT, 
                local_A, filas_por_proceso * N, MPI_INT, 
                0, MPI_COMM_WORLD);

    // --- INICIO DE TEMPORIZACIÓN ---
    // El maestro empieza a contar el tiempo justo antes de que empiece el cálculo distribuido
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

// --- CÓMPUTO DISTRIBUIDO ---
    // Cada proceso multiplica su porción de filas de A (local_A) por la matriz B completa.
    for (int i = 0; i < filas_por_proceso; i++) {
        for (int j = 0; j < N; j++) {
            local_C[i][j] = 0; // Inicializamos la celda a 0 antes de sumar
            for (int k = 0; k < N; k++) {
                local_C[i][j] += local_A[i][k] * B[k][j];
            }
        }
    }

    // --- FIN DE TEMPORIZACIÓN Y RECOGIDA ---
    // Recoger los resultados parciales (local_C) en la matriz C del maestro usando MPI_Gather.
    // Es exactamente la operación inversa a MPI_Scatter.
    MPI_Gather(local_C, filas_por_proceso * N, MPI_INT, 
               C, filas_por_proceso * N, MPI_INT, 
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        // El maestro para el cronómetro justo después de recoger todos los resultados [cite: 40, 271, 276]
        end_time = MPI_Wtime();
        printf("Tiempo total de computación: %f segundos\n", end_time - start_time); 
    }

    // --- LIBERAR MEMORIA Y FINALIZAR ---
    if (rank == 0) {
        free(A);
        free(C);
    }
    free(B);

    MPI_Finalize();
    return 0;
}

// --- Implementación de funciones auxiliares ---

void inicializar_matriz(int matriz[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            // Valores aleatorios entre 1 y 10 [cite: 39]
            matriz[i][j] = (rand() % 10) + 1; 
        }
    }
}

void imprimir_matriz(int matriz[N][N], const char* nombre) {
    printf("Matriz %s:\n", nombre);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d\t", matriz[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}