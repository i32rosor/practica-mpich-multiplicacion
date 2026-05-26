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
    // 1. Enviar la matriz B completa a TODOS los procesos usando MPI_Bcast
    // ... (Lo implementaremos en el siguiente paso)

    // 2. Repartir las filas de la matriz A usando MPI_Scatter
    // ... (Lo implementaremos en el siguiente paso)

    // --- INICIO DE TEMPORIZACIÓN ---
    // El maestro empieza a contar el tiempo justo antes de que empiece el cálculo distribuido
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    // --- CÓMPUTO DISTRIBUIDO ---
    // Cada proceso multiplica su porción de filas de A por la matriz B completa.
    // ... (Lo implementaremos en el siguiente paso)

    // --- FIN DE TEMPORIZACIÓN Y RECOGIDA ---
    // Recoger los resultados parciales (filas de C calculadas) en la matriz C del maestro usando MPI_Gather