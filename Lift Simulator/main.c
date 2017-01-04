#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define TIME_BETWEEN_FLOORS 1
#define MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR 10
#define MAX_NUMBER_OF_PEOPLE_WAITING 5

int convert_to_direction_number(int, int);
int generateFloor(int, int);

typedef struct
{
    int desired_floor;
} Person;

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

        int current_floor = 0;//rand() % number_of_processes;
        int number_of_people_in_elevator = 0;
        Person* people[MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR];

        while(1)
        {
            // Wait for a request
            printf("[ Elevator ] ~  Waiting for a request.\n");
            int floor_requested;
            MPI_Recv(&floor_requested, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            printf("[ Elevator ] ~  Received a request from %d.\n", floor_requested);

            // As soon as it got a request, gets on its way
            int direction_of_request = convert_to_direction_number(current_floor, floor_requested);
            while(current_floor != floor_requested)
            {
                printf("[ Elevator ] ~ %d people on elevator.\n", number_of_people_in_elevator);
                sleep(TIME_BETWEEN_FLOORS);
                current_floor += direction_of_request;

                printf("[ Elevator ] ~  Now on floor %d.\n", current_floor);

                // Check if there is people on elevator who wants to get out on current floor.
                int number_of_modifications = 0;
                for (size_t i = 0; i < number_of_people_in_elevator; i++) {

                    if (people[i]->desired_floor == current_floor){
                        people[i] = NULL;
                        number_of_modifications += 1;
                    }
                }

                number_of_people_in_elevator -= number_of_modifications;

            }
        }
    }
    else
    {
        int number_of_people_waiting = 0, floor = rank - 1;
        Person* people_waiting[MAX_NUMBER_OF_PEOPLE_IN_ELEVATOR];

        MPI_Request req;

        while(1)
        {
            // Generate people
            if (number_of_people_waiting < MAX_NUMBER_OF_PEOPLE_WAITING) {
                if (rand() % 100 < 50)
                {
                    Person p1;
                    p1.desired_floor = generateFloor(floor, number_of_processes);
                    people_waiting[number_of_people_waiting] = &p1;
                    number_of_people_waiting += 1;
                }
            }

            // For now just sending number of people
            if (number_of_people_waiting > 0)
            {
                MPI_Send(&floor, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }

            // Request to know how much people there are with the same direction of elevator
            int direction_of_request;
            MPI_Recv(&direction_of_request, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            int toSend = 0;
            if (status.MPI_TAG == floor) toSend = number_of_people_waiting;
            else {
                for (size_t i = 0; i < number_of_people_waiting; i++) {
                    if (convert_to_direction_number(floor, people_waiting[i]->desired_floor) == direction_of_request){
                        people_waiting[i] = NULL;
                        toSend++;
                    }
                }
            }
            number_of_people_waiting -= toSend;

            MPI_Send(&toSend, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        }

    }

    MPI_Finalize();
    return 0;
}

int convert_to_direction_number(int current_floor, int desired_floor)
{
	if (current_floor < desired_floor) return 1;
	if (current_floor == desired_floor) return 0;
	return -1;
}

// Generate a desired floor based on current floor
int generateFloor(int current, int number_of_processes)
{
	int desired_floor = rand() % (number_of_processes - 2);

	if (current == desired_floor)
		return generateFloor(current, number_of_processes);

	return  desired_floor;
}
