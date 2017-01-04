#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define TIME_BETWEEN_FLOORS 2
#define MAXNUMBER_OF_PEOPLE_IN_ELEVATOR 10

int convert_to_direction_number(int, int);

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

        int current_floor = rand() % number_of_processes;
        int number_of_people_in_elevator = 0;
        Person* people[MAXNUMBER_OF_PEOPLE_IN_ELEVATOR];

        while(1)
        {
            // Wait for a request
            int floor_requested;
            MPI_Recv(&floor_requested, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            // As soon as it got a request, gets on its way
            int direction_of_request = convert_to_direction_number(current_floor, floor_requested);
            while(current_floor != floor_requested)
            {
                sleep(TIME_BETWEEN_FLOORS);
                current_floor += direction_of_request;

                // Check if there is people on elevator who wants to get out on current floor.
                int number_of_modifications = 0;
                for (size_t i = 0; i < number_of_people_in_elevator; i++) {

                    if (people[i]->desired_floor == current_floor){
                        people[i] = NULL;
                        number_of_modifications += 1;
                    }
                }

                if (number_of_modifications > 0)
                {
                    printf("[ Elevator ] -* Just deliverd %d person/people.\n", number_of_modifications);
                    number_of_people_in_elevator -= number_of_modifications;

                    // Request if there is people in this floor who wants to come in.

                    MPI_Send(&current_floor, 1, MPI_INT, current_floor + 1, 0, MPI_COMM_WORLD);

                    // For now just receiving ints
                    int no_people;
                    MPI_Recv(&no_people, 1, MPI_INT, current_floor + 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                    number_of_people_in_elevator += no_people;
                }


            }
        }
    }
    else
    {

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
