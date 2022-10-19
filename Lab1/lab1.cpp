#include <mpi.h>
#include <iostream>
#define M 2

using namespace std;

void star(int argc, char *argv[]) {
    int rank, commSize;
    int msg = 254, ans = 0;
    MPI_Status status;
    double start, end;
    
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    start = MPI_Wtime();
    for (int i = 0; i < M; i++) {
        if (rank == 0)
        {        
            //scanf("%d", &msg);
            for (int to_thread = 1; to_thread < commSize; to_thread++) {
                MPI_Send(&msg, 1, MPI_INT, to_thread, 0, MPI_COMM_WORLD);
                MPI_Recv(&ans, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
                if (ans == to_thread)
                    cout << "message successfully delivered to " << to_thread << endl;
            }

        }
        
        else {
            MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            //cout << "process: " << rank << " received " << msg << endl;
            ans = rank;
            MPI_Send(&ans, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
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