#include <cstdlib>
#include <string>
#include "mpi.h"
#include <iostream>

using namespace std;

/* Размерность матрицы */
#define N 100

/* Печать двумерной матрицы */
void printMatrix(double* matrix[N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%f\t", matrix[i][j]);
        }
        printf("\n");
    }
}

/** Возвращает указатель на трёхмерный массив */
double ***alloc3d(int l, int m, int n) {
    double *data = new double [l*m*n];
    double ***array = new double **[l];
    for (int i=0; i<l; i++) {
        array[i] = new double *[m];
        for (int j=0; j<m; j++) {
            array[i][j] = &(data[(i*m+j)*n]);
        }
    }
    return array;
}

/** Возвращает указатель на двумерный массив */
double **alloc2d(int rows, int cols) {
    double *data = new double [rows*cols];
    double **array= new double* [rows];
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

/** Производит умножение матриц из массива
 * Мы передаём ей сам массив и промежуток, в котором расположены нужные матрицы
*/
double** matrix_multiply3d(double** b[N], int start, int end) {
    //Сохраняем первую матрицу в переменную
    double** a = alloc2d(N, N);
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            a[i][j] = b[start][i][j];
        }
    }
    
    double c[N][N];
    // Основное перемножение
    for (int iter=start+1; iter<end; iter++) {
        
        for (int i=0; i<N; i++){
            for (int j=0; j<N; j++){
                for (int k=0; k<N; k++) {
                    c[i][j] += a[i][k] * b[iter][k][j]; 
                }
            }
        }
        for (int i=0; i<N; i++) {
            for (int j=0; j<N; j++) {
                a[i][j] = c[i][j];
                c[i][j] = 0.0;
            }
        }
    }
    return a;
}

int main(int argc, char** argv) {
    
    // Количество матриц
    int A = 50;

    /** Длины строк в матрице
    * Нужны для создания своего типа 
    */
    int sizes[N];
    for (int i=0; i<N; i++) sizes[i] = N;

    srand((unsigned int)time(NULL));

    // Указатель на трёхмерный массив, содержащий все матрицы
    double ***a=alloc3d(A,N,N);
    for (int i=0; i<A; i++) {
        for (int j=0; j<N; j++) {
            for (int k=0; k<N; k++) {
                a[i][j][k]=float(rand())/float((RAND_MAX)) * 5.0;
            }
        }
    }

    // Указатель на итоговую матрицу
    double ** result_matrix = alloc2d(N, N);

    int ProcNum, ProcRank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

    // Создаём свой тип
    MPI_Aint base;
    MPI_Get_address(result_matrix[0], &base);
    MPI_Aint displacements[N];
    for (int i=0; i<N; ++i) {   
        MPI_Get_address(result_matrix[i], &displacements[i]);
        displacements[i] -= base;
    }
    MPI_Datatype matrix;
    MPI_Type_create_hindexed(N, sizes, displacements, MPI_DOUBLE, &matrix);
    MPI_Type_commit(&matrix);

    if (ProcRank == 0) {
        printf("Исходные матрицы:\n");
        for (int i = 0; i < A; i++) {
            printf("\n%d матрица\n", i);
            printMatrix(a[i]);
        }
        printf("\n\n");

        
        if (ProcNum == 1) {
            result_matrix = matrix_multiply3d(a, 0, A);
        } else if (ProcNum == 2) {
            MPI_Recv(&result_matrix[0][0], 1, matrix, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        } else {
            // Указатель на трёхмерный массив для сбора вычислений
            double ***b=alloc3d(ProcNum,N,N);
            for (int i=1; i<ProcNum; i++) {
                MPI_Recv(&b[i-1][0][0], 1, matrix, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            result_matrix = matrix_multiply3d(b, 0, ProcNum-1);
        }
    } else {
        // Распределяем вычисления
        int start = (ProcRank-1) * A / (ProcNum-1);
        int end;
        if (ProcRank != ProcNum-1) {
            end = ProcRank * A / (ProcNum-1);
        } else {
            end = A;
        }
        result_matrix = matrix_multiply3d(a, start, end);
        MPI_Send(&result_matrix[0][0], 1, matrix, 0, 1, MPI_COMM_WORLD);
        
    }

    if (ProcRank == 0) {
        {
            printf("Итоговая матрица:\n");
            printMatrix(result_matrix);
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}