#include <iostream>
#include "mpi.h"
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <unistd.h>
using namespace std;

int main(int argc, char **argv)
{
    int rank, root = 0, nprocs, namelen;
    char processorName[10];

    // getting information of process and machine
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Get_processor_name(processorName, &namelen);

    if (rank == root) {     // master process
        MPI_Request request;
        int value_to_send = 9; // value to send
        MPI_Isend(&value_to_send, 1, MPI_INT, 1, 1234, MPI_COMM_WORLD, &request);   // sending value from process 0 to process 1

        MPI_Status status;
        char data[9];
        int flag = 0;
        MPI_Irecv(data, 9, MPI_CHAR, 1, 1235, MPI_COMM_WORLD, &request);   // receiving acknowledgement message from process 1
        while(!flag) {
            MPI_Test(&request, &flag, &status);
        }
        cout << data << endl;
        
    } else {    // slave processes
        MPI_Request request, request1;
        MPI_Status status, status1;
        int value_to_send, flag = 0, flag1 = 0, check = 0, check1 = 0;
        char data[9] = "Received";

        MPI_Irecv(&value_to_send, 1, MPI_INT, MPI_ANY_SOURCE, 1234, MPI_COMM_WORLD, &request);  // receiving value sent from any process
        while(!flag) {
            if (rank != nprocs - 1 && check == 0) { // if its not the last process then send and receive message
                MPI_Isend(&value_to_send, 1, MPI_INT, rank + 1, 1234, MPI_COMM_WORLD, &request1);   // sending message to next process
                MPI_Irecv(data, 9, MPI_CHAR, MPI_ANY_SOURCE, 1235, MPI_COMM_WORLD, &request1);  // receiving acknowlegement message from that process
                while(!flag1) {
                    MPI_Test(&request1, &flag1, &status1);
                    MPI_Test(&request, &flag, &status);
                    if (flag) { // if value is receieved then send acknowlegement message
                        MPI_Isend(data, 9, MPI_CHAR, status.MPI_SOURCE, 1235, MPI_COMM_WORLD, &request);    // sending acknowledgement message
                        check1 = 1;
                    }
                }
                cout << data << endl;
                check = 1;
            }
            MPI_Test(&request, &flag, &status);
        }
        
        if (flag && check1 == 0) {  // if value is receieved then send acknowlegement message
            MPI_Isend(data, 9, MPI_CHAR, status.MPI_SOURCE, 1235, MPI_COMM_WORLD, &request);    // sending acknowledgement message
        }
    }
    MPI_Finalize();
}