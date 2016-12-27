/*
	
	INPUT : [ -nf: number of floors;
			  -sp: time in seconds the elevator takes to travel between floors;
			  -np: number of people to simulate
		 ]

	[] Random people calling at random times for elevator
	[] Elevator has 3 states [IDLE, UP, DOWN]
	[] If people is in state IDLE, any people in that floor can enter if the desire direction is the same. 

	Explanation about tags sent.

	[1] -> Normal floor request.
	[2] -> Confirmation that the elevator received indeed the requested floor.
	[3] -> Used to say the floor the elevator is.
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mpi.h" 

#define MAXNUMBER_OF_PEOPLE_IN_ELEVATOR 10

char* convertEnum(int);


enum ElevatorState {IDLE, UP, DOWN};

typedef struct
{
	int intention; // Intention: 0 for DOWN, 1 for UP
	int desireFloor;
} Person;

typedef struct
{
	int number_of_people, floor;
	Person people[MAXNUMBER_OF_PEOPLE_IN_ELEVATOR];
} Floor;

typedef struct 
{
	Person people[MAXNUMBER_OF_PEOPLE_IN_ELEVATOR];
	int floor;
	enum ElevatorState state;
} Elevator;


int main(int argc, char** argv){

	// Create seed for random
	srand(time(NULL));

	int number, rank, source, number_of_processes, dest, received;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);

	if (rank == 0){

		Elevator lift;
		lift.floor = 0;
		lift.state = IDLE;

		printf("Elevator is in floor %d and its state %s.\n", lift.floor, convertEnum(lift.state));
		
	} else {

	}


	MPI_Finalize();
	return 0;
}


// Convert number to enum relative
char* convertEnum(int number){
	if (number == 0) return "IDLE";
	if (number == 1) return "UP";
	return "DOWN";
}
