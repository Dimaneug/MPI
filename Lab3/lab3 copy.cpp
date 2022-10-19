#include <mpi.h>
#include <iostream>
#define M 2

using namespace std;

void star(int argc, char *argv[]) {
    int rank, commSize;
    int *p = nullptr;
    double start[6], end[6];
    int msg = 228;    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    start[rank] = MPI_Wtime();
    for (int i = 0; i < M; i++) {
        MPI_Scatter(&msg, 1, MPI_INT,&p, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank != 0)
        {     
            cout << "process: " << rank << " received " << msg << endl; 
            //scanf("%d", &msg);
            //for (int to_thread = 1; to_thread < commSize; to_thread++) 
              //  MPI_Send(&msg, 1, MPI_INT, to_thread, 0, MPI_COMM_WORLD);
        }  
        msg++;   
    }
    
    MPI_Finalize();
    end[rank] = MPI_Wtime();
    printf("Итоговое время выполнения %d процессом: %lf\n", rank, end[rank] - start[rank]);
    if (rank == 0) {
        for (auto i = 0; i < 6; i++) {
            printf("start: %f, end: %f\n", start[i], end[i]);
        }
    }
}


int main(int argc, char *argv[]) {
    
    star(argc, argv);
    return 0;
}