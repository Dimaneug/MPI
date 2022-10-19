#include <mpi.h>
#include <iostream>
#define M 2

using namespace std;

void star(int argc, char *argv[]) {
    int rank, commSize;
    int *p, *ans;
    double start, end;
    int msg = 228;  
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    p = new int[commSize];
    ans = new int[commSize];
    start = MPI_Wtime();
    for (int i = 0; i < M; i++) {
        if (rank == 0)
            MPI_Scatter(&msg, 1, MPI_INT, &p, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(&rank, 1, MPI_INT, ans, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            for (int j = 1; j < commSize; j++)
                if (ans[j] == j)
                    cout << "Messages successfully delivered to " << j << "!\n";
        }
        msg++;   
    }
    
    MPI_Finalize();
    end = MPI_Wtime();
    printf("Итоговое время выполнения %d процессом: %lf\n", rank, end - start);
}


int main(int argc, char *argv[]) {
    
    star(argc, argv);
    return 0;
}