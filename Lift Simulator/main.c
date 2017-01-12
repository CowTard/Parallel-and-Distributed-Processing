#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define MAXNUMBER_OF_PEOPLE_IN_ELEVATOR 10
#define MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR 5
#define TIME_BETWEEN_FLOORS 1


int main(int argc, char** argv)
{
	// Create seed for random
	srand(time(NULL));

	int number, rank, source, number_of_processes, dest, received;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);



	MPI_Finalize();
	return 0;
}
