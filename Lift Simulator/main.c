#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "mpi.h"

#define TIME_BETWEEN_FLOORS 1
#define MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR 10
#define MAX_NUMBER_OF_PEOPLE_WAITING 5

int retrieve_direction_needed(int, int);
int generate_floor(int, int);

typedef struct {
	int desired_floor;
} Person;

/*
	Tag 0: Used for requests
	Tag 1: Used for conversation in floors
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

	// Creation of datatype
    int blocklengths[1] = {1};
    MPI_Datatype types[1] = {MPI_INT};
    MPI_Datatype mpi_person_type;
    MPI_Aint offsets[1];
    offsets[0] = offsetof(Person, desired_floor);

    MPI_Type_create_struct(1, blocklengths, offsets, types, &mpi_person_type);
    MPI_Type_commit(&mpi_person_type);

    if (rank == 0) {

		Person* people_elevator[MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR] = {NULL};
		int number_people_elevator = 0;
		int current_floor = 0, desired_floor = 0;

		/*
			- Wait for a request from a floor. [x]
			- Change floors after a requets. [x]
			- Open doors in each floor a person gets out. [x]
			- Open door when reach the desired floor. [x]
			- Send a message to know how much people will get it in a floor. [x]
			- Receive a message with how many people will get in. [x]
			- Add people who got in. [x]
		*/

		while(1) {

			if(number_people_elevator == 0)
				MPI_Recv(&desired_floor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
			else
				desired_floor = people_elevator[0]->desired_floor;

			do {

				int direction_change = retrieve_direction_needed(current_floor, desired_floor);
				int number_of_people_leaving_in_this_floor = 0;

				Person* new_arr[MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR] = {NULL};
				int number_of_new_aloc = 0;

				for (size_t i = 0; i < number_people_elevator; i++) {
					int person_direction = retrieve_direction_needed(current_floor, people_elevator[i]->desired_floor);
					if (person_direction == direction_change){
						number_of_people_leaving_in_this_floor += 1;
						people_elevator[i] = NULL;
					} else {
						new_arr[number_of_new_aloc] = people_elevator[i];
						number_of_new_aloc += 1;
					}
				}

				memcpy(people_elevator, new_arr, sizeof(people_elevator));
				number_people_elevator -= number_of_new_aloc;

				// Open doors
				if (current_floor == desired_floor || number_of_people_leaving_in_this_floor > 0){
					printf("[Elevator] Just left %d on this floor.\n", number_people_elevator);

					// Get people in
					MPI_Send(&current_floor, 1, MPI_INT, current_floor + 1, 1,MPI_COMM_WORLD);

					Person* people_to_get_in[MAX_NUMBER_OF_PEOPLE_WAITING];
					MPI_Recv(&people_to_get_in, MAX_NUMBER_OF_PEOPLE_WAITING, mpi_person_type, current_floor + 1, 1, MPI_COMM_WORLD, &status);

					// Add the people to elevator people
					for (size_t i = 0; i < MAX_NUMBER_OF_PEOPLE_WAITING; i++) {
						if (people_to_get_in[i] != NULL) {
							people_elevator[number_people_elevator] = people_to_get_in[i];
							number_people_elevator += 1;
						} else {
							break;
						}
					}
				}

				sleep(TIME_BETWEEN_FLOORS);
				current_floor += direction_change;

			} while(current_floor != desired_floor);
		}
	} else {
		/*
			- Create new people. [x]
			- Send a request to the elevator. []
			- Send people in if elevator stops in this floor. []
		*/

		// Create people
		Person* people_waiting[MAX_NUMBER_OF_PEOPLE_WAITING] = {NULL};
		int number_people_to_create = rand() % 2, number_people_waiting = 0;
		int floor = rank - 1;

		for (size_t i = 0; i < number_people_to_create; i++) {
			Person p1;
			p1.desired_floor = generate_floor(floor, number_of_processes);
			number_people_waiting += 1;
			people_waiting[number_people_waiting] = &p1;
		}

		printf("[Floor %d] %d person/people created.\n", floor, number_people_to_create);
	}

    MPI_Finalize();
    return 0;
}


int retrieve_direction_needed(int current_floor, int desired_floor){
	if (current_floor == desired_floor) return 0;
	if (current_floor > desired_floor) return -1;
	return 1;
}

int generate_floor(int current_floor, int number_of_floors) {

	int generated_floor = rand() % number_of_floors;

	if (generated_floor == current_floor)
		return generate_floor(current_floor, number_of_floors);

	return generated_floor;
}
