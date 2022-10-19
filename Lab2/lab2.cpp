#include <mpi.h>
#include <iostream>
#define M 2

using namespace std;

void star(int argc, char *argv[]) {
    int rank, commSize;
    int msg = 228, ans;
    double start, end;
    
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    start = MPI_Wtime();
    for (int i = 0; i < M; i++) {
        if (rank == 0)
            MPI_Bcast(&msg, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Reduce(&rank, &ans, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            int temp = 0;
            for (int j = 1; j < commSize; j++)
                temp += j;
            if (ans == temp)
                cout << "Messages successfully delivered!\n";
        }
            
        msg++;   
    }
    
    MPI_Finalize();
    end = MPI_Wtime();
    printf("Итоговое время выполнения %d процессом: %f\n", rank, end - start);
    
}

int main(int argc, char *argv[]) {
    star(argc, argv);
    return 0;
}