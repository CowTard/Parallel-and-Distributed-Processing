/*

	INPUT : [ -nf: number of floors;
			  -sp: time in seconds the elevator takes to travel between floors;
			  -np: number of people to simulate
		 ]

	[] Random people calling at random times for elevator
	[] Elevator has 3 states [IDLE, UP, DOWN]
	[] If people is in state IDLE, any people in that floor can enter if the desire direction is the same.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define MAXNUMBER_OF_PEOPLE_IN_ELEVATOR 10
#define TIME_BETWEEN_FLOORS 1

char* convertEnum(int);
int generateFloor(int, int);

enum ElevatorState {IDLE, UP, DOWN};

typedef struct
{
	int desiredFloor;
} Person;

typedef struct
{
	int number_of_people, floor;
	Person people[MAXNUMBER_OF_PEOPLE_IN_ELEVATOR];
} Floor;

typedef struct
{
	Person people[MAXNUMBER_OF_PEOPLE_IN_ELEVATOR];
	int floor, desiredFloor;
	enum ElevatorState state;
} Elevator;


int main(int argc, char** argv)
{
	// Create seed for random
	srand(time(NULL));

	int number, rank, source, number_of_processes, dest, received;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);

	if (rank == 0)
	{
		Elevator lift;
		lift.floor = 0;
		lift.desiredFloor = 5;
		lift.state = IDLE;
		// -------- ### --------
		int elevator_change;


		printf(" + Elevator is in floor %d and its state is %s.\n", lift.floor, convertEnum(lift.state));

		while(1)
		{
			sleep(1); // Time to open doors
			MPI_Recv(&lift.desiredFloor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf(" > Request from floor %d.\n", lift.desiredFloor);

			if (lift.floor < lift.desiredFloor) { lift.state = UP; elevator_change = 1; }
			else { lift.state = DOWN; elevator_change = -1; }

			while(lift.floor != lift.desiredFloor)
			{
				sleep(TIME_BETWEEN_FLOORS);
				printf(" + Elevator is in floor %d and its state is %s.\n", lift.floor, convertEnum(lift.state));
				lift.floor += elevator_change;
			}

			lift.state = IDLE;
			printf(" + Elevator is in floor %d and it is opening doors.\n", lift.floor);
		}

	}
	else
	{

		Floor fl;
		fl.number_of_people = 0;
		fl.floor = rank;

		while(1)
		{
			if (rand() % 10000 < 1)
			{

				Person t1;
				t1.desiredFloor = generateFloor(fl.floor, number_of_processes);

				fl.number_of_people += 1;
				fl.people[fl.number_of_people - 1] = t1;
				printf("A new person was created in floor %d.\n", fl.floor);
			}

			if (fl.number_of_people > 0){
				MPI_Send(&fl.floor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			}
		}

	}

	MPI_Finalize();
	return 0;
}

// Convert number to enum relative
char* convertEnum(int number)
{
	if (number == 0) return "IDLE";
	if (number == 1) return "UP";
	return "DOWN";
}

// Generate a desired floor based on current floor
int generateFloor(int current, int number_of_processes)
{
	int desired_floor = rand() % number_of_processes;

	if (current == desired_floor)
		return generateFloor(current, number_of_processes);

	return  desired_floor;
}
