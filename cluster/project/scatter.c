#include <mpi.h>
#include <stdio.h>
// #include <stdlib.h>

int main (int argc, char** argv)
{
    int root = 0;
    int size = 8;
    MPI_Init(NULL, NULL);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(size % world_size != 0)
    {
        printf("oye! que no es divisible, salimos de aquí\n");
        MPI_Finalize();
        return 1;
    }

    int numbers[size];
    int localNumber;

    if(world_rank == root)
    {
        for(int i = 0; i < size; i++)
        {
            numbers[i] = i;
        }
    }

    MPI_Scatter( &numbers, 1, MPI_INT, &localNumber, 
        1, MPI_INT, root,  MPI_COMM_WORLD);

    localNumber = localNumber * 2;

    MPI_Gather( &localNumber, 1, MPI_INT, &numbers, 
        1, MPI_INT, root,  MPI_COMM_WORLD);

    if(world_rank == root)
    {
        printf("he recibido los datos: ");
        for(int i = 0; i < size; i++)
        {
            if(i < 7)
                printf("%d, ", numbers[i]);
            else
                printf("%d)", numbers[i]);
        }
    }
    MPI_Finalize();
    return 0;
}