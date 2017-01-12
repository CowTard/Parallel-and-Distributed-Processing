#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define TIME_BETWEEN_FLOORS 1
#define MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR 10
#define MAX_NUMBER_OF_PEOPLE_WAITING 5

int retrieve_direction_needed(int, int);

typedef struct {
	int desired_floor;
} Person;

/*
	Tag 0: used for requests
*/
int main(int argc, char** argv)
{
    // Create seed for random
    srand(time(NULL));

    int number, rank, source, number_of_processes, dest, received;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);

    if (rank == 0) {

		Person* people_elevator[MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR];
		int number_people_elevator = 0;
		int current_floor = 0, desired_floor = 0;

		/*
			- Wait for a request from a floor. [x]
			- Change floors after a requets. [x]
			- Open doors in each floor a person gets out. []
			- Open door when reach the desired floor. []
		*/

		MPI_Recv(&desired_floor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

		do {

			int direction_change = retrieve_direction_needed(current_floor, desired_floor);

			sleep(TIME_BETWEEN_FLOORS);
			current_floor += direction_change;

		} while(current_floor != desired_floor);

	} else {
		/*
			- Create new people. []
			- Send a request to the elevator. []
			- Send people in if elevator stops in this floor. []
		*/
	}

    MPI_Finalize();
    return 0;
}

int retrieve_direction_needed(int current_floor, int desired_floor){
	if (current_floor == desired_floor) return 0;
	if (current_floor > desired_floor) return -1;
	return 1;
}
