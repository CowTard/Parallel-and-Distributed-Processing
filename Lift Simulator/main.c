#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define MAXNUMBER_OF_PEOPLE_IN_ELEVATOR 10
#define MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR 2
#define TIME_BETWEEN_FLOORS 1

char* convertEnum(int);
int generateFloor(int, int);

enum ElevatorState {IDLE, UP, DOWN};

typedef struct
{
	int desired_floor;
} Person;

typedef struct
{
	int number_of_people, floor;
	Person* people[MAXNUMBER_OF_PEOPLE_IN_ELEVATOR];
} Floor;

typedef struct
{
	Person* people[MAXNUMBER_OF_PEOPLE_IN_ELEVATOR];
	int floor, desired_floor;
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

	/*
		RANK 0 : Scheduler;
		RANK 1: Elevator;
		Rank [2:n]: Floors.
	*/
	if (rank == 0)
	{
		// Array of people in each floor waiting
		Person* people_waiting[MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR];

		printf(" ** Initializing Scheduler!\n");

		while (1) {
			// wait for the elevator to reach a floor
			int current_floor;
			MPI_Recv( &current_floor, 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			printf("Elevator is in floor %d\n", current_floor);

			// Destination of this message is current_floor + 2 because the first ranks are the Scheduler and Elevator
			MPI_Send(&current_floor, 1, MPI_INT, current_floor + 2, 0, MPI_COMM_WORLD);

			int i;
			MPI_Recv(&i, 1, MPI_INT, current_floor + 2, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			MPI_Send(&i, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

		}
	}
	else if (rank == 1)
	{
		Elevator lift;
		lift.floor = 0;
		lift.state = IDLE;
		// -------- ### --------
		int elevator_change;
		printf(" ** Initializing Elevator!\n");

		while (1) {

			MPI_Send(&lift.floor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			// Receive from Scheduler if there is some people who wants to go in the same direction
			int number_of_people_to_take;
			MPI_Recv(&number_of_people_to_take, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			printf("Elevator will take %d people.\n", number_of_people_to_take);
			break;

		}
	}
	else
	{

		Floor fl;
		fl.number_of_people = 0;
		fl.floor = rank - 2;

		printf(" ** Initializing floor %d\n", fl.floor);

		// Create people
		while(1)
		{
			if ((rand() % 100000) < 1 && fl.number_of_people < MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR)
			{
				Person p1;
				p1.desired_floor = generateFloor(fl.floor, number_of_processes);
				fl.people[fl.number_of_people] = &p1;
				fl.number_of_people++;
			}

			int elevator_current_floor;
			MPI_Recv(&elevator_current_floor, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			Person people[MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR];
			int i = 0;
			for (size_t i = 0; i < fl.number_of_people; i++) {
				printf("%d\n", fl.people[i]->desired_floor);
				if (fl.people[i]->desired_floor > elevator_current_floor)
				{
					people[i] = *fl.people[i];
					i++;
					fl.people[i] = NULL;
				}
			}

			// For now let's just send number of people.
			MPI_Send(&i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
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
	int desired_floor = rand() % (number_of_processes - 2);

	if (current == desired_floor)
		return generateFloor(current, number_of_processes);

	return  desired_floor;
}
