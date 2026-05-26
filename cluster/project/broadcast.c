#include <mpi.h>
#include <stdio.h>

#define PING_PONG_LIMIT 10

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);
    
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int number;
    if(world_rank == 0)
    {
        //estpoy pensando en el numero 7
        number = 7;
        
    }
    
    MPI_Bcast(&number, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(world_rank != 0)
    {
        printf("Soy el nodo %d, el nodo cero penso en el numero %d\n", world_rank, number);
    } 

    MPI_Finalize();
}
