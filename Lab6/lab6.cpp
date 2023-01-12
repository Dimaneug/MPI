#include <stdio.h>
#include <mpi.h>
#include <iostream>

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);

    int ProcNum, ProcRank;
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Status status;

    MPI_Group base_group;

    MPI_Group GridGroup;
    MPI_Comm GridBaseComm;
    MPI_Comm GridComm;

    MPI_Group GraphGroup;
    MPI_Comm GraphBaseComm;
    MPI_Comm GraphComm;

    MPI_Comm_group(MPI_COMM_WORLD, &base_group); // заполняем базовую группу процессорами
    int GridCount = ProcNum / 2; // количество процессов для решетки
    int GraphCount = GridCount;  // количество процессов для графа
    if (ProcNum % 2 == 1) GridCount++;
    int* GridRanks = new int[GridCount];   // создаем для них массивы
    int* GraphRanks = new int[GraphCount];

    int i;
    for (i = 0; i < GridCount; i++)
    {
        GridRanks[i] = i;
    }
    for (i = GridCount; i < ProcNum; i++)
    {
        GraphRanks[i - GridCount] = i;
    }

    MPI_Group_incl(base_group, GridCount, GridRanks, &GridGroup);  // группы для решетки
    MPI_Group_incl(base_group, GraphCount, GraphRanks, &GraphGroup);  // группа для графа
    MPI_Comm_create(MPI_COMM_WORLD, GridGroup, &GridBaseComm); // создаем коммуникаторы 
    MPI_Comm_create(MPI_COMM_WORLD, GraphGroup, &GraphBaseComm);

    if (ProcRank < GridCount) // если решетка, то создаем для нее кокммуникатор 
    {
        // создание одномерной решетки
        int dims[1], periods[1], reoreder = 1, ndims = 1;
        dims[0] = GridCount;
        periods[0] = 1;           
        MPI_Cart_create(GridBaseComm, ndims, dims, periods, reoreder, &GridComm); // если решетка, то создаем для нее кокммуникатор

        int nextRank, previousRank, message;
        if (ProcRank == 0) 
        {
            message = 1;
            MPI_Cart_shift(GridComm, 0, 1, &previousRank, &nextRank);  // 0 - одномерная, 1 - период, возращает предыдущий и следующий ранги 
            printf("rank = %d, previous = %d, next = %d\n", ProcRank, previousRank, nextRank);
            MPI_Send(&message, 1, MPI_INT, nextRank, 0, GridComm); // отпр сообщение следующему рангу
            printf("sent %d to %d\n", message, nextRank);
            MPI_Recv(&message, 1, MPI_INT, previousRank, 0, GridComm, &status); // принимает от предыдущего 
            printf("received %d from %d\n", message, previousRank);
        }
        else
        {
            MPI_Cart_shift(GridComm, 0, 1, &previousRank, &nextRank);
            printf("rank = %d, previous = %d, next = %d\n", ProcRank, previousRank, nextRank);  // принимает от пред
            MPI_Recv(&message, 1, MPI_INT, previousRank, 0, GridComm, &status);
            printf("received %d from %d\n", message, previousRank); 
            message++;  
            MPI_Send(&message, 1, MPI_INT, nextRank, 0, GridComm); //  отправляет
            printf("sent %d to %d\n", message, nextRank);

            // prev next - решетка 
        }
    }
    else
    {
        // создание графа ( 1 в центре, остальные вокруг)
        int* index = new int[GraphCount];   
        int* edges = new int[GraphCount];
        for (i = 0; i < GraphCount; i++)
        {
            index[i] = GraphCount + i - 1;
        }
        for (i = 0; i < GraphCount - 1; i++)
        {
            edges[i] = i + 1;
            edges[GraphCount + i - 1] = 0;
        }

        MPI_Graph_create(GraphBaseComm, GraphCount, index, edges, 1, &GraphComm);
        // Cоздаем для нее коммуникатор
        int count;
        MPI_Graph_neighbors_count(GraphComm, ProcRank - GridCount, &count); // возвр количество соседей ( 0 - все, кроме 1 , у ост - по одному)  вычитаем  ProcRank - GridCount, тк нумерация снова пойдет с 0
        int* ranks = new int[count];
        MPI_Graph_neighbors(GraphComm, ProcRank - GridCount, count, ranks); // получили ранги соседей
        printf("Process - %d, count - %d\n", ProcRank, count);
        int messageSend;
        int messageRecv;
        //printf("\nRank - %d", ProcRank);
        for (i = 0; i < count; i++)
        {
            //printf("\n\t\t%d", ranks[i]);
            messageSend = ProcRank;
            MPI_Sendrecv(&messageSend, 1, MPI_INT, ranks[i], 0, &messageRecv, 1, MPI_INT, ranks[i], 0, GraphComm, &status); // отправка каждому и получаем сразу 
            printf("Sent %d to %d\n", messageSend, ranks[i]);
            printf("Received %d from %d\n", messageRecv, ranks[i]);

            // процесс, количество соседей 
        }
    }
    MPI_Finalize();
}
