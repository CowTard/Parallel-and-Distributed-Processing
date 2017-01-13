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

    if (rank == 0) {

		int people_elevator[MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR];
		memset (people_elevator, -1, sizeof(people_elevator));
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

			if(number_people_elevator == 0){
				MPI_Recv(&desired_floor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
				printf("[Elevator] Received a request from %d.\n", desired_floor);
			}
			else {
				for (size_t i = 0; i < number_people_elevator; i++) {
					if (people_elevator[i] != -1){
						desired_floor = people_elevator[i];
						printf("[Elevator] People inside. Going to floor %d.\n", desired_floor);
						break;
					}
				}
			}

			int direction_change = -1;

			while(direction_change != 0){

				direction_change = retrieve_direction_needed(current_floor, desired_floor);
				int number_of_people_leaving_in_this_floor = 0;

				int new_arr[MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR];
				memset (new_arr, -1, sizeof(new_arr));
				int number_of_new_aloc = 0;

				for (size_t i = 0; i < number_people_elevator; i++) {

					if (people_elevator[i] != -1) {

						int person_direction = retrieve_direction_needed(current_floor, people_elevator[i]);

						if (person_direction == 0){
							number_of_people_leaving_in_this_floor += 1;
							people_elevator[i] = -1;
						} else {
							new_arr[number_of_new_aloc] = people_elevator[i];
							number_of_new_aloc += 1;
						}
					}
				}

				memcpy(people_elevator, new_arr, sizeof(people_elevator));
				number_people_elevator -= number_of_people_leaving_in_this_floor;

				// Open doors
				if (current_floor == desired_floor || number_of_people_leaving_in_this_floor > 0){
					printf("[Elevator] Opened doors on floor %d.\n", current_floor);

					// Get people in
					MPI_Send(&current_floor, 1, MPI_INT, current_floor + 1, desired_floor, MPI_COMM_WORLD);

					int people_to_get_in[MAX_NUMBER_OF_PEOPLE_WAITING];
					memset (people_to_get_in, -1, sizeof(people_to_get_in));
					MPI_Recv(&people_to_get_in, MAX_NUMBER_OF_PEOPLE_WAITING, MPI_INT, current_floor + 1, 1, MPI_COMM_WORLD, &status);

					// Add the people to elevator people
					for (size_t i = 0; i < MAX_NUMBER_OF_PEOPLE_WAITING; i++) {
						if (people_to_get_in[i] != -1) {
							people_elevator[number_people_elevator] = people_to_get_in[i];
							number_people_elevator += 1;
						}
					}
				}

				sleep(TIME_BETWEEN_FLOORS);
				current_floor += direction_change;
				printf("[Elevator] On floor %d with %d people.\n", current_floor, number_people_elevator);

			}
		}
	} else {
		/*
			- Create new people. [x]
			- Send a request to the elevator. [x]
			- Send people in if elevator stops in this floor. []
		*/
		MPI_Request req;
		int people_waiting[MAX_NUMBER_OF_PEOPLE_WAITING];
		int number_people_waiting = 0, floor = rank - 1;
		memset (people_waiting, -1, sizeof(people_waiting));

		while (1) {
			// Create people
			int number_people_to_create = rand() % 3;

			for (size_t i = 0; i < number_people_to_create; i++) {
				people_waiting[number_people_waiting] = generate_floor(floor, number_of_processes - 1);
				number_people_waiting += 1;
			}

			printf("[Floor %d] %d person/people created.\n", floor, number_people_to_create);

			if (number_people_waiting > 0)
				MPI_Isend(&floor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &req);

			int elevator_current_floor;
			MPI_Recv(&elevator_current_floor, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

			// Create an array of people that goes in the same direction of the elevator.
			if (status.MPI_TAG == floor){
				//printf("[Floor %d] About to enter %d person/people in elevator.\n", floor, number_people_waiting );

				MPI_Send(people_waiting, MAX_NUMBER_OF_PEOPLE_WAITING, MPI_INT, 0, 1, MPI_COMM_WORLD);
				for (size_t i = 0; i < number_people_waiting; i++) {
					people_waiting[i] = -1;
				}
				number_people_waiting = 0;

			} else {

				int direction_of_elevator = retrieve_direction_needed(floor, status.MPI_TAG);
				int people_to_send[MAX_NUMBER_OF_PEOPLE_WAITING];
				memset (people_to_send, -1, sizeof(people_to_send));
				int temp[MAX_NUMBER_OF_PEOPLE_WAITING];
				memset (temp, -1, sizeof(temp));

				int pts = 0, tmp = 0;
				for (size_t i = 0; i < number_people_waiting; i++) {

					int direction_of_person = retrieve_direction_needed(floor, people_waiting[i]);

					if (direction_of_person == direction_of_elevator) {
						people_to_send[pts] = people_waiting[i];
						people_waiting[i] = -1;
						pts += 1;
					} else {
						temp[tmp] = people_waiting[i];
						tmp += 1;
						people_waiting[tmp] = -1;
					}
				}

				printf("[Floor %d] About to enter %d person/people in elevator.\n", floor, pts );

				MPI_Send(people_to_send, MAX_NUMBER_OF_PEOPLE_WAITING, MPI_INT, 0, 1, MPI_COMM_WORLD);
				memcpy(people_waiting, temp, sizeof(people_waiting));
			}

			MPI_Wait( &req, MPI_STATUS_IGNORE );
		}

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
