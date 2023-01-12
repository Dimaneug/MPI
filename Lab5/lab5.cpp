#include <iostream>
#include <vector>
#include <mpi.h>

const int A = 3;
const int N = 10;

std::string StrSum(std::string a, std::string b) {
    if (a.size() < b.size()) {
        std::swap(a, b);
    }
    int j = a.size() - 1;
    for (int i = b.size() - 1; i >= 0; i--, j--) {
        a[j] += (b[i] - '0');
    }
    for (int i = a.size() - 1; i > 0; i--) {
        if (a[i] > '9') {
            int d = a[i] - '0';
            a[i - 1] = ((a[i - 1] - '0') + d / 10) + '0';
            a[i] = (d % 10) + '0';
        }
    }
    if (a[0] > '9') {
        std::string k;
        k += a[0];
        a[0] = ((a[0] - '0') % 10) + '0';
        k[0] = ((k[0] - '0') / 10) + '0';
        a = k + a;
    }
    return a;
}

std::string Multiplicate(std::string x, std::string y) {
    std::string result;
    for (int i = 0; i < y.size(); i++) {
        int carry = 0;
        std::string inter_res;
        for (int j = x.size() - 1; j >= 0; j--) {
            // intermediate multiplication of each digit and addition of carry
            int num = (y[i] - '0') * (x[j] - '0') + carry;

            // if intermediate multiplication is of two digits and j > 0
            // then second digit is appended to intermediate result
            // and first digit is stored as carry
            if (num > 9 && j > 0) {
                inter_res = std::to_string(num % 10) + inter_res;
                carry = num / 10;
            } else {
                // else the digit is append to intermediate result
                inter_res = std::to_string(num) + inter_res;
                carry = 0;
            }
        }
        // Adding the intermediate results
        result.push_back('0');
        result = StrSum(result, inter_res);
    }
    return result;
}

std::string Divide(std::vector<std::string> numbers, int start, int end) {
    std::string temp = numbers[start];
    for (int i = start + 1; i < end; i++) {
        temp = Multiplicate(temp, numbers[i]);
    }
    return temp;
}

int main(int argc, char** argv) {
    // srand((unsigned int)time(NULL));
    std::vector<std::string> numbers(A, "");
    for (int i = 0; i < A; i++) {
        for (int j = 0; j < N; j++) {
            numbers[i].push_back(rand() % 9 + 1 + '0');
        }
    }

    int ProcNum, ProcRank;

    MPI_Group base_group;
    MPI_Group new_group;
    MPI_Comm new_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    MPI_Comm_group(MPI_COMM_WORLD, &base_group);
    int ranks[ProcNum];
    for (int i=0; i<ProcNum; i++) {
        ranks[i] = i;
    }
    MPI_Group_incl(base_group, 4, ranks, &new_group);  //создание группы
    MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);  //создание коммуникатора

    std::string result;
    int len;
    if (ProcRank == 0) {
        printf("Исходные числа:\n");
        for (size_t i = 0; i < numbers.size(); i++) {
            std::cout << numbers[i] << std::endl;
        }
        if (ProcNum == 1) {
            result = Divide(numbers, 0, A);
        } else {
            std::vector<std::string> result_numbers(ProcNum - 1, "");
            for (int i = 0; i < ProcNum-1; i++) {
                MPI_Recv(&len, 1, MPI_INT, i+1, 1, new_comm, MPI_STATUS_IGNORE);
                int* received = new int[len];
                MPI_Recv(received, len, MPI_INT, i+1, 2, new_comm, MPI_STATUS_IGNORE);
                result_numbers[i].resize(len);
                for (int j = 0; j < len; j++) {
                    result_numbers[i][j] = received[j] + '0';
                }
                delete[] received;
                //std::cout << "Received: " << result_numbers[i] << std::endl;
            }
            result = Divide(result_numbers, 0, ProcNum - 1);
        }
    } else {
        int start = (ProcRank - 1) * A / (ProcNum - 1);
        int end;
        if (ProcRank != ProcNum - 1) {
            end = ProcRank * A / (ProcNum - 1);
        } else {
            end = A;
        }
        result = Divide(numbers, start, end);
        int len = static_cast<int>(result.size());
        int* toSend = new int[len];
        for (int i = 0; i < len; i++) {
            toSend[i] = result[i] - '0';
        }
        // std::cout << std::endl;
        // std::cout << "Will send: " << result << std::endl;
        MPI_Send(&len, 1, MPI_INT, 0, 1, new_comm);
        MPI_Send(toSend, len, MPI_INT, 0, 2, new_comm);
        delete[] toSend;
    }

    if (ProcRank == 0) {
        std::cout << "Result\n" << result << std::endl;
    }

    MPI_Group_free(&new_group);
    MPI_Finalize();
    return 0;
}