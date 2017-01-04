#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define MAXNUMBER_OF_PEOPLE_IN_ELEVATOR 10
#define MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR 5
#define TIME_BETWEEN_FLOORS 1

enum ElevatorState {IDLE, UP, DOWN};

char* convertEnum(int);
char* convert_to_direction(int, int);
int generateFloor(int, int);

typedef struct
{
	int desired_floor;
} Person;

typedef struct
{
	int number_of_people, floor;
	Person* people[MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR];
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

		// wait for the elevator to be ready
		int elevator_current_floor;
		MPI_Recv( &elevator_current_floor, 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		while (1) {

			// Waiting for a request of a floor
			int desired_floor;
			MPI_Recv(&desired_floor, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,
			MPI_COMM_WORLD, &status);

			printf("[ Scheduler ] -> Got a request from %d.\n", desired_floor);

			// Send to elevator a request to move
			MPI_Send(&desired_floor, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

			printf("[ Scheduler ] -> Just send the elevator to floor %d.\n", desired_floor);

			// Check for people to get in or get out in each floor which the elevator passes.
			while(1)
			{
				int current_floor;
				MPI_Recv( &current_floor, 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

				printf("[ Scheduler ] -> Elevator is in floor %d\n", current_floor);
				// We ask for every floor to give information
				for (size_t i = 2; i < number_of_processes; i++) {
					MPI_Send(&current_floor, 1, MPI_INT, i, desired_floor, MPI_COMM_WORLD);
				}

				// Get number of people worth to come in
				int i;
				MPI_Recv(&i, 1, MPI_INT, current_floor + 2, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

				// Send info to the elevator
				MPI_Send(&i, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

				if (current_floor == desired_floor)
				{
					break;
				}
			}

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

		// IF IDLE, elevator send a message to scheduler.
		MPI_Send(&lift.floor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

		while (1)
		{
			// Receive from Scheduler the floor to go
			MPI_Recv(&lift.desired_floor, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			// Calculate direction of change
			if (lift.desired_floor > lift.floor)
			{
				lift.state = UP;
				elevator_change = 1;
			} else if (lift.desired_floor == lift.floor)
			{
				lift.state = IDLE;
				elevator_change = 0;
			}
			else
			{
				lift.state = DOWN;
				elevator_change = -1;
			}

			while(1)
			{
				sleep(TIME_BETWEEN_FLOORS);
				lift.floor += elevator_change;

				MPI_Send(&lift.floor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

				// Receive from Scheduler if there is some people who wants to go in the same direction
				int number_of_people_to_take;
				MPI_Recv(&number_of_people_to_take, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

				if(lift.floor == lift.desired_floor)
				{
					lift.state = IDLE;
					break;
					printf("[ Elevator ] -> Arrived at its destiny.\n");

				}
				//printf("Elevator will take %d people.\n", number_of_people_to_take);
			}

		}
	}
	else
	{

		// Variable to control necessity of sending information
		int already_sent = 0;

		Floor fl;
		fl.number_of_people = 0;
		fl.floor = rank - 2;

		// Every floor starts with one person each except for floor = 0;
		if (fl.floor != 0){
			Person p1;
			p1.desired_floor = generateFloor(fl.floor, number_of_processes);
			fl.people[0] = &p1;
			fl.number_of_people++;
		}

		//printf(" ** Initializing floor %d\n", fl.floor);

		MPI_Request req;

		while(1)
		{
			//Generate random people with random floors.
			if ((rand() % 1000) < 1 && fl.number_of_people < MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR)
			{
				Person p1;
				p1.desired_floor = generateFloor(fl.floor, number_of_processes);
				fl.people[fl.number_of_people] = &p1;
				fl.number_of_people++;
				already_sent = 1;
			}

			// Make sure this floors only the elevator if there is some person or people.
			if (fl.number_of_people > 0 && already_sent == 0)
			{
				printf("[ Floor %d ] Have %d people waiting.\n", fl.floor, fl.number_of_people);
				// Send the request to Scheduler
				MPI_Send(&fl.floor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
				printf("[ Floor %d ] -> Requesting to Scheduler the elevator.\n", fl.floor);
				already_sent = 1;
			}

			int elevator_current_floor;
			MPI_Recv(&elevator_current_floor, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			if (elevator_current_floor == fl.floor)
			{

				Person people[MAXNUMBER_OF_PEOPLE_WAITING_FOR_ELEVATOR];
				int i = 0;
				for (size_t i = 0; i < fl.number_of_people; i++)
				{
					int elevator_desired_floor = status.MPI_TAG;

					char* elevator_direction = convert_to_direction(elevator_current_floor, elevator_desired_floor);

					char* person_direction = convert_to_direction(elevator_current_floor, people[i].desired_floor);

					if (person_direction == elevator_direction || elevator_current_floor == elevator_desired_floor)
					{
						printf("[ Floor %d ] -> Eliminating people.\n", fl.floor);
						people[i] = *fl.people[i];
						i++;
						fl.number_of_people--;
						fl.people[i] = NULL;
					}
				}

				// For now let's just send number of people.
				MPI_Send(&i, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			}
		}
	}

	MPI_Finalize();
	return 0;
}

// Convert desired floor and current floor to a direction
char* convert_to_direction(int current_floor, int desired_floor)
{
	if (current_floor < desired_floor) return "UP";
	if (current_floor == desired_floor) return "IDLE";
	return "DOWN";
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
